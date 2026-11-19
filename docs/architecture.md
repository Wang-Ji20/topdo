# Architecture

topdo is a set of layers around one todo file. Stable layers sit at the top;
each layer may use only the layers above it.

```
docs/design/format.md   the todo file format: the contract
        |
src/core/               parse, edit and render a todo file; pure functions
        |
src/store/              read and write the file on disk, watch it for changes
        |
        +-- src/qml/    exposes the store to QML as a list model
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
file:   "- [ ] a\r\n- [x] b"
lines:  "- [ ] a", "- [x] b"
file:   "- [ ] a\n- [x] b\n"     (written back)
```

`findTodos` reads a `Document` line by line and returns a `TodoList`: the
sections and the todos, each todo pointing at its line, its parent todo and
its section. It applies the line rules of
[design/format.md](design/format.md) and nothing more.

Edits are plain functions from a `Document` to the edited copy:

| Function | Changes |
|---|---|
| `setDone(document, todo, done)` | the character between the brackets |
| `setText(document, todo, text)` | the text after the checkbox |
| `removeTodo(document, todo)` | deletes the todo's line and the indented lines below it |
| `addTodo(document, text, section)` | inserts `- [ ] text` after the first run of todos |

`todo` and `section` are indexes into what `findTodos` returns for the same
document. The rules for where each edit acts are in
[design/format.md](design/format.md), rules 6 to 8. The line rules that
both `findTodos` and the edits use live in one place, `todoline.h`.

**store** owns the file on disk. It reads the file, applies an edit through
core, and writes the result atomically, so a crash never leaves a
half-written file. When the file changes on disk (an edit in a text editor,
a sync tool), the store reloads it before its next write.

**Frontends** translate between their world and the store, and hold no
logic of their own:

- `qml` offers the todos as a list model that the plasmoid displays.
- `cli` maps command-line arguments to store calls.
- `mcp` maps MCP tool calls (JSON-RPC over standard input and output) to
  store calls.

The frontends never use each other, so each one can change without
touching the rest.

## How an edit flows

Ticking off a todo in the popup:

```
plasmoid  --click-->  qml model  --toggle(item)-->  store
store: read file -> core.parse -> core.setDone -> core.render -> atomic write
store  --changed-->  qml model  --> plasmoid redraws
```

The CLI and the MCP server follow the same path from `store` down.

So far `core` exists: reading and writing the file, finding the todos, and
editing them. See [../PLAN.md](../PLAN.md).
