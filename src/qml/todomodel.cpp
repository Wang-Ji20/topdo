#include "todomodel.h"
#include "edits.h"
#include "query.h"
#include "store.h"

#include <QFileInfo>

#include <algorithm>

namespace topdo {

namespace {

// One todo is one line: line breaks in text from the panel become spaces, as in the CLI.
QString oneLine(const QString &text) { return text.simplified(); }

} // namespace

TodoModel::TodoModel(QObject *parent)
    : QAbstractListModel(parent), m_path(todoFilePath(configuredDir(defaultConfigFile()))) {
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &TodoModel::reload);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &TodoModel::reload);
    reload();
}

int TodoModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

QVariant TodoModel::data(const QModelIndex &index, int role) const {
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }
    const qsizetype todo = m_rows.at(index.row());
    const Todo &item = m_list.todos.at(todo);
    switch (static_cast<Role>(role)) {
    case NumberRole:
        return static_cast<int>(todo + 1);
    case TitleRole:
        return item.title;
    case TextRole:
        return item.text;
    case DoneRole:
        return item.done;
    case DepthRole:
        return depthOf(m_list, todo);
    case SectionRole:
        return m_list.sections.at(item.section).title;
    case DueRole:
        return item.due ? QVariant(*item.due) : QVariant();
    }
    return {};
}

QHash<int, QByteArray> TodoModel::roleNames() const {
    return {
        {NumberRole, "number"},
        {TitleRole, "title"},
        {TextRole, "text"},
        {DoneRole, "done"},
        {DepthRole, "depth"},
        {SectionRole, "section"},
        {DueRole, "due"},
    };
}

void TodoModel::setPath(const QString &path) {
    if (path == m_path) {
        return;
    }
    m_path = path;
    Q_EMIT pathChanged();
    reload();
}

void TodoModel::setFilter(const QString &filter) {
    if (filter == m_filter) {
        return;
    }
    m_filter = filter;
    Q_EMIT filterChanged();
    refilter();
}

void TodoModel::setShowDone(bool showDone) {
    if (showDone == m_showDone) {
        return;
    }
    m_showDone = showDone;
    Q_EMIT showDoneChanged();
    refilter();
}

int TodoModel::openCount() const {
    const auto isOpenTopLevel = [](const Todo &todo) { return !todo.done && todo.parent < 0; };
    return static_cast<int>(std::ranges::count_if(m_list.todos, isOpenTopLevel));
}

int TodoModel::doneCount() const {
    return static_cast<int>(std::ranges::count_if(m_list.todos, &Todo::done));
}

int TodoModel::lateCount() const {
    const QDate today = QDate::currentDate();
    const auto isLate = [today](const Todo &todo) {
        return !todo.done && todo.due && *todo.due < today;
    };
    return static_cast<int>(std::ranges::count_if(m_list.todos, isLate));
}

QString TodoModel::nextTitle() const {
    const std::optional<qsizetype> next = nextTodo(m_list, QDate::currentDate());
    return next ? m_list.todos.at(*next).title : QString();
}

QDate TodoModel::nextDue() const {
    const std::optional<qsizetype> next = nextTodo(m_list, QDate::currentDate());
    return next ? m_list.todos.at(*next).due.value_or(QDate()) : QDate();
}

void TodoModel::add(const QString &text) {
    const QString line = oneLine(text);
    if (line.isEmpty()) {
        return;
    }
    edit([line](const Document &document) { return addTodo(document, line, std::nullopt); });
}

void TodoModel::addSubtask(int number, const QString &text) {
    const QString line = oneLine(text);
    if (line.isEmpty()) {
        return;
    }
    editTodo(number, [line](const Document &document, qsizetype todo) {
        return topdo::addSubtask(document, todo, line);
    });
}

void TodoModel::setDone(int number, bool done) {
    editTodo(number, [done](const Document &document, qsizetype todo) {
        return topdo::setDone(document, todo, done, QDate::currentDate());
    });
}

void TodoModel::setText(int number, const QString &text) {
    const QString line = oneLine(text);
    if (line.isEmpty()) {
        return;
    }
    editTodo(number, [line](const Document &document, qsizetype todo) {
        return topdo::setText(document, todo, line);
    });
}

void TodoModel::remove(int number) {
    std::optional<Block> block;
    const bool saved = editTodo(number, [&block](const Document &document, qsizetype todo) {
        block = todoBlock(document, todo);
        return removeTodo(document, todo);
    });
    if (!saved || !block) {
        return;
    }
    m_removed.append(*block);
    Q_EMIT canUndoRemoveChanged();
}

// The block goes back at its old line. If the file changed above it since, that line may now be
// elsewhere in the list; no line is lost either way.
void TodoModel::undoRemove() {
    if (m_removed.isEmpty()) {
        return;
    }
    const Block block = m_removed.constLast();
    const bool saved =
        edit([&block](const Document &document) { return restoreBlock(document, block); });
    if (!saved) {
        return; // keep it, to try again
    }
    m_removed.removeLast();
    Q_EMIT canUndoRemoveChanged();
}

void TodoModel::reload() {
    watch(); // an atomic save replaces the file, which ends the watch on the old one
    const std::expected<Document, QString> loaded = load(m_path);
    if (!loaded) {
        setError(loaded.error());
        return;
    }
    setError({});
    show(*loaded);
}

// Applies an edit through the store, which starts from the file on disk.
// Returns whether the edit reached the file.
bool TodoModel::edit(const std::function<Document(const Document &)> &change) {
    const std::expected<Document, QString> edited = update(m_path, change);
    if (!edited) {
        setError(edited.error());
        return false;
    }
    setError({});
    show(*edited);
    return true;
}

// Edits one todo. A number that no longer names a todo, because the file changed since the panel
// last showed it, changes nothing.
bool TodoModel::editTodo(int number,
                         const std::function<Document(const Document &, qsizetype)> &change) {
    return edit([number, &change](const Document &document) {
        const TodoList list = findTodos(document);
        const qsizetype todo = number - 1;
        if (todo < 0 || todo >= list.todos.size()) {
            return document;
        }
        return change(document, todo);
    });
}

void TodoModel::show(const Document &document) {
    if (m_document == document) {
        return; // nothing changed, for example the watcher seeing our own save
    }
    m_document = document;
    m_list = findTodos(document);
    refilter();
    Q_EMIT todosChanged();
}

void TodoModel::refilter() {
    beginResetModel();
    m_rows = filterTodos(m_list, m_filter, m_showDone);
    endResetModel();
}

void TodoModel::watch() {
    const QString dir = QFileInfo(m_path).absolutePath();
    if (!m_watcher.directories().contains(dir) && QFileInfo::exists(dir)) {
        m_watcher.addPath(dir);
    }
    if (!m_watcher.files().contains(m_path) && QFileInfo::exists(m_path)) {
        m_watcher.addPath(m_path);
    }
}

void TodoModel::setError(const QString &error) {
    if (error == m_error) {
        return;
    }
    m_error = error;
    Q_EMIT errorChanged();
}

} // namespace topdo
