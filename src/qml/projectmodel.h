#pragma once

#include "todomodel.h"

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>

namespace topdo {

// One project of the todo file for the panel's project column: its milestones (the project's
// top-level todos) and their steps (their subtasks). See docs/design/project-today.md.
class ProjectModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(TodoModel *todos READ todos WRITE setTodos NOTIFY todosChanged)
    // The project asked for, without its "+". If the file has no such project, the model shows
    // the first project.
    Q_PROPERTY(QString project READ project WRITE setProject NOTIFY projectChanged)
    // Every project of the file, in the order they first appear.
    Q_PROPERTY(QStringList projects READ projects NOTIFY changed)
    // The project shown; empty if the file has no projects.
    Q_PROPERTY(QString name READ name NOTIFY changed)
    // One map per milestone: number, title, done, and steps (a list of maps: number, title, done).
    Q_PROPERTY(QVariantList milestones READ milestones NOTIFY changed)
    // The index in milestones of the first open milestone, or -1.
    Q_PROPERTY(int current READ current NOTIFY changed)

  public:
    explicit ProjectModel(QObject *parent = nullptr);

    [[nodiscard]] TodoModel *todos() const { return m_todos; }
    void setTodos(TodoModel *todos);
    [[nodiscard]] QString project() const { return m_project; }
    void setProject(const QString &project);
    [[nodiscard]] QStringList projects() const { return m_projects; }
    [[nodiscard]] QString name() const { return m_name; }
    [[nodiscard]] QVariantList milestones() const { return m_milestones; }
    [[nodiscard]] int current() const { return m_current; }

  Q_SIGNALS:
    void todosChanged();
    void projectChanged();
    void changed();

  private:
    void refresh();

    QPointer<TodoModel> m_todos;
    QString m_project;
    QStringList m_projects;
    QString m_name;
    QVariantList m_milestones;
    int m_current = -1;
};

} // namespace topdo
