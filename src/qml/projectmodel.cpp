#include "projectmodel.h"
#include "query.h"

namespace topdo {

namespace {

QVariantMap todoMap(const TodoList &list, qsizetype todo) {
    const Todo &item = list.todos.at(todo);
    return {
        {QStringLiteral("number"), static_cast<int>(todo + 1)},
        {QStringLiteral("title"), item.title},
        {QStringLiteral("done"), item.done},
    };
}

QVariantMap milestoneMap(const TodoList &list, qsizetype todo) {
    QVariantList steps;
    for (const qsizetype step : subtasksOf(list, todo)) {
        steps.append(todoMap(list, step));
    }
    QVariantMap milestone = todoMap(list, todo);
    milestone.insert(QStringLiteral("steps"), steps);
    return milestone;
}

// The section to show: the asked-for project, else the first project, else none (-1).
qsizetype shownSection(const TodoList &list, const QString &project) {
    for (qsizetype i = 1; i < list.sections.size(); ++i) {
        if (list.sections.at(i).title == project) {
            return i;
        }
    }
    return list.sections.size() > 1 ? 1 : -1;
}

} // namespace

ProjectModel::ProjectModel(QObject *parent) : QObject(parent) {}

void ProjectModel::setTodos(TodoModel *todos) {
    if (todos == m_todos) {
        return;
    }
    if (m_todos) {
        disconnect(m_todos, nullptr, this, nullptr);
    }
    m_todos = todos;
    if (m_todos) {
        connect(m_todos, &TodoModel::todosChanged, this, &ProjectModel::refresh);
    }
    Q_EMIT todosChanged();
    refresh();
}

void ProjectModel::setProject(const QString &project) {
    if (project == m_project) {
        return;
    }
    m_project = project;
    Q_EMIT projectChanged();
    refresh();
}

void ProjectModel::refresh() {
    const TodoList list = m_todos ? m_todos->list() : TodoList{};
    m_projects.clear();
    for (qsizetype i = 1; i < list.sections.size(); ++i) {
        m_projects.append(list.sections.at(i).title);
    }
    const qsizetype section = shownSection(list, m_project);
    m_name = section < 0 ? QString() : list.sections.at(section).title;
    m_milestones.clear();
    m_current = -1;
    if (section >= 0) {
        for (const qsizetype todo : milestonesOf(list, section)) {
            if (m_current < 0 && !list.todos.at(todo).done) {
                m_current = static_cast<int>(m_milestones.size());
            }
            m_milestones.append(milestoneMap(list, todo));
        }
    }
    Q_EMIT changed();
}

} // namespace topdo
