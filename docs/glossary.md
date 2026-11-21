# Glossary

**Badge.** The small label topdo shows in the top bar, for example
`● Write report · 3`. Clicking it opens the popup.

**CMake preset.** A named set of build settings in `CMakePresets.json`, used
as `cmake --preset dev`.

**Context.** A word that starts with `@` in a todo, such as `@phone`:
where or how you do it.

**Due date.** A `due:2026-11-21` word in a todo's text. The popup shows it
as a date.

**Golden file.** A test input stored together with its expected output. The
test passes when the code produces exactly that output.

**KDE Plasma.** The desktop environment topdo runs in: the top bar, the
dock, the wallpaper and the system settings.

**Line ending.** The characters that end a line: `\n` on Linux and macOS,
`\r\n` on Windows. topdo reads both and writes `\n`.

**Manual.** `README.md` in the todo folder, which topdo writes with a new
todo file: the format rules in short, by example.

**MCP (Model Context Protocol).** An open protocol through which AI
assistants such as Claude call tools that a program offers. topdo's MCP
server offers tools to list, add and tick off todos.

**Plasmoid.** A Plasma widget: a small program that lives in a panel or on
the desktop. topdo's badge and popup are one plasmoid.

**Popup.** The window that opens when you click the badge, with the todo
list.

**Priority.** `(A) ` to `(Z) ` at the start of an open todo; A is the
highest. A done todo keeps it as a `pri:A` tag.

**Project.** A word that starts with `+` in a todo, such as `+Work`. The
panel groups todos by their first project.

**Property test.** A test that states a rule ("writing back an unchanged
file gives the same bytes") and checks it on many random inputs.

**QML.** Qt's language for user interfaces. Plasma widgets are written in it.

**Qt.** The C++ toolkit that KDE software is built on.

**Section.** A group of the panel: the todos whose first project is the
same. Todos without a project come first.

**Subtask.** A todo with a `p:1` tag, under the todo with `id:1`.

**Tag.** A `key:value` word in a todo, such as `due:2026-11-21`.

**Todo file.** `todo.txt` in the todo folder: the todo.txt file that holds
the todos. Its format is defined in [design/format.md](design/format.md).

**Todo folder.** The folder that holds the todo file, by default
`~/Documents/todo/`. It is set in `~/.config/topdo/topdo.conf`.

**todo.txt.** A plain-text todo format, one todo per line, read by many
apps: <https://github.com/todotxt/todo.txt>.
