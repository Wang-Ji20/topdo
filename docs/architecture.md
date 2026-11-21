# Architecture

topdo is a set of layers around one todo file. Stable layers sit at the top;
each layer may use only the layers above it.

```
docs/design/format.md   the todo file format: the contract
        |
src/core/               parse, edit and render a todo file; pure functions
        |
src/store/              read and write the file on disk, read the config
        |
        |   src/calendar/  today's events: pure day rules, read from Akonadi
        |       |
        +-- src/qml/    exposes the store and the calendar to QML
        +-- src/cli/    the `topdo` command
        +-- src/mcp/    `topdo mcp`: tools for AI assistants
plasmoid/               the badge and popup in the top bar (QML)
```

## Layers

**core** turns text into a `Document` and back, and applies edits to a
`Document`. It does no I/O and holds no state, so tests can call it
directly. Its central rule: rendering an unchanged document gives back
every line as it was written.

A `Document` is the file's text as a list of lines, without line endings.
Rendering joins them with `\n` and adds a final newline:

```
file:   "(A) a\r\nx 2026-11-20 b"
lines:  "(A) a", "x 2026-11-20 b"
file:   "(A) a\nx 2026-11-20 b\n"     (written back)
```

`findTodos` reads a `Document` line by line and returns a `TodoList`: the
sections (one per `+project`) and the todos, each todo pointing at its
line, its parent todo (by `id:`/`p:` tags) and its section. It applies the line rules of
[design/format.md](design/format.md) and nothing more.

Edits are plain functions from a `Document` to the edited copy:

| Function | Changes |
|---|---|
| `setDone(document, todo, done, today)` | writes or removes `x <today> `; moves a priority to `pri:` and back |
| `setText(document, todo, text)` | the text after the priority and the dates |
| `removeTodo(document, todo)` | deletes the todo's line and its subtasks' lines |
| `restoreBlock(document, block)` | puts back lines that `todoBlock(document, todo)` named before a removal |
| `addTodo(document, text, project)` | appends the text, with ` +project` if given |
| `addSubtask(document, todo, text)` | inserts `text p:ID` after the todo's last subtask, giving it an `id:` |

`todo` is an index into what `findTodos` returns for the same
document.

Small functions in `query.h` decide what the widget shows:
`filterTodos` (the todos the panel lists for a filter text), `nextTodo` (the
todo on the badge: late, then due today, then the first open one),
`depthOf` (how deep a subtask is nested), and `milestonesOf` and
`subtasksOf` (a project's milestones and their steps). The rules for where each edit acts are in
[design/format.md](design/format.md), rules 11 to 14. The line rules that
both `findTodos` and the edits use live in one place, `todoline.h`.

**store** owns the file on disk and the config file. It holds no state:

| Function | Does |
|---|---|
| `load(path)` | reads the todo file; a missing file reads as an empty document |
| `save(path, document)` | writes it atomically (`QSaveFile`), creating the folder; a new file also gets the `README.md` manual beside it |
| `update(path, change)` | `load`, apply an edit from core, `save` if anything changed |
| `configuredDir(configFile)` | the todo folder from `~/.config/topdo/topdo.conf` |

Because `update` reads the file again for every edit, an edit made
elsewhere in the meantime (a text editor, a sync tool) is kept, not
overwritten. Operations that can fail return `std::expected` with a message
that names the file.

**calendar** reads the day's events. `day.h` is pure, like core: an
`Event` value, `happensOn`, `sortedDay`, `nextEvent` and `countdown`.
`AkonadiCalendar` reads every calendar in Akonadi, the KDE service that
also syncs Google calendars, read-only, and signals when they change. Only
the QML plugin uses this layer; core, store, the CLI and the MCP server
don't depend on it.

**Frontends** translate between their world and the store, and hold no
logic of their own:

- `qml` is the QML module `io.github.wangji20.topdo` that the plasmoid
  imports. `TodoModel` lists the todos the panel shows (after
  `filterTodos`), with each todo's number, title, text, done state,
  depth, section and due date as roles, and what the badge needs as
  properties. Its `add`, `setDone`, `setText` and `remove` go through the
  store's `update`, taking todo numbers as `topdo list` shows them; a
  number that no longer names a todo changes nothing. `ProjectModel`
  shows one project of the same `TodoModel` for the project column: its
  milestones with their steps, and the list of projects to choose from.
  `TodayModel` offers today's events from `AkonadiCalendar`, the next
  one and its countdown, with a clock that moves on every 30 seconds.
  `WindowEffects` asks KWin to blur behind the panel.
- `cli` maps command-line arguments to store calls.
- `mcp` maps MCP tool calls (JSON-RPC over standard input and output) to
  store calls.

The frontends never use each other, so each one can change without
touching the rest.

## How an edit flows

Ticking off a todo in the popup:

```
plasmoid  --click-->  qml model  --update(path, setDone)-->  store
store: read file -> core.parse -> core.setDone -> core.render -> atomic write
file changed on disk  -->  qml model reloads  -->  plasmoid redraws
```

The qml model watches the file, so edits from anywhere (the popup, the CLI,
a text editor) reach the popup the same way. It watches the folder as well
as the file: an atomic save, by topdo or by an editor, replaces the file,
which ends a watch on the old one.

The CLI and the MCP server follow the same path from `store` down.

So far `core`, `store`, `cli` and `qml` exist; the plasmoid is a
placeholder, and `mcp` doesn't exist yet.
See [../PLAN.md](../PLAN.md).
