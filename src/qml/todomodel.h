#pragma once

#include "document.h"
#include "todos.h"

#include <QAbstractListModel>
#include <QDate>
#include <QFileSystemWatcher>
#include <QQmlEngine>

#include <functional>

namespace topdo {

// The todos of the todo file, as the panel lists them, plus what the badge shows. Edits go
// through the store; the model watches the file, so edits from elsewhere (the CLI, a text editor)
// show at once. Todos are addressed by number, counting from 1 as `topdo list` does.
class TodoModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    // The todo file; by default the one in the configured todo folder.
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    // Text to filter the list by; see filterTodos().
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool showDone READ showDone WRITE setShowDone NOTIFY showDoneChanged)
    Q_PROPERTY(int openCount READ openCount NOTIFY todosChanged)   // open top-level todos
    Q_PROPERTY(int doneCount READ doneCount NOTIFY todosChanged)   // done todos, at any depth
    Q_PROPERTY(int totalCount READ totalCount NOTIFY todosChanged) // all todos, at any depth
    Q_PROPERTY(int lateCount READ lateCount NOTIFY todosChanged)   // open todos due before today
    Q_PROPERTY(
        QString nextTitle READ nextTitle NOTIFY todosChanged) // the badge's todo; see nextTodo()
    Q_PROPERTY(QDate nextDue READ nextDue NOTIFY todosChanged)
    // The last failure to read or write the file, or empty.
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

  public:
    enum Role {
        NumberRole = Qt::UserRole + 1,
        TitleRole,
        TextRole,
        DoneRole,
        DepthRole,
        SectionRole,
        DueRole,
    };
    Q_ENUM(Role)

    explicit TodoModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString path() const { return m_path; }
    void setPath(const QString &path);
    [[nodiscard]] QString filter() const { return m_filter; }
    void setFilter(const QString &filter);
    [[nodiscard]] bool showDone() const { return m_showDone; }
    void setShowDone(bool showDone);
    [[nodiscard]] int openCount() const;
    [[nodiscard]] int doneCount() const;
    [[nodiscard]] int totalCount() const { return static_cast<int>(m_list.todos.size()); }
    [[nodiscard]] int lateCount() const;
    [[nodiscard]] QString nextTitle() const;
    [[nodiscard]] QDate nextDue() const;
    [[nodiscard]] QString error() const { return m_error; }

    Q_INVOKABLE void add(const QString &text);
    Q_INVOKABLE void addSubtask(int number, const QString &text);
    Q_INVOKABLE void setDone(int number, bool done);
    Q_INVOKABLE void setText(int number, const QString &text);
    Q_INVOKABLE void remove(int number);

  Q_SIGNALS:
    void pathChanged();
    void filterChanged();
    void showDoneChanged();
    void todosChanged();
    void errorChanged();

  private:
    void reload();
    void edit(const std::function<Document(const Document &)> &change);
    void editTodo(int number, const std::function<Document(const Document &, qsizetype)> &change);
    void show(const Document &document);
    void refilter();
    void watch();
    void setError(const QString &error);

    QString m_path;
    QString m_filter;
    bool m_showDone = false;
    QString m_error;
    std::optional<Document> m_document; // empty until the first load
    TodoList m_list;
    QList<qsizetype> m_rows; // indexes into m_list.todos, one per row
    QFileSystemWatcher m_watcher;
};

} // namespace topdo
