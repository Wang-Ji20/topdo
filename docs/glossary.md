# Glossary

**Badge.** The small label topdo shows in the top bar, for example
`● Write report · 3`. Clicking it opens the popup.

**CMake preset.** A named set of build settings in `CMakePresets.json`, used
as `cmake --preset dev`.

**Due date.** A `due:2026-11-21` word in a todo's text. The popup shows it
as a date.

**Golden file.** A test input stored together with its expected output. The
test passes when the code produces exactly that output.

**KDE Plasma.** The desktop environment topdo runs in: the top bar, the
dock, the wallpaper and the system settings.

**Line ending.** The characters that end a line: `\n` on Linux and macOS,
`\r\n` on Windows. topdo reads both and writes `\n`.

**MCP (Model Context Protocol).** An open protocol through which AI
assistants such as Claude call tools that a program offers. topdo's MCP
server offers tools to list, add and tick off todos.

**Oracle.** An independent implementation that a test compares our code
against. If both give the same answer on many inputs, ours is likely right.

**Other line.** Any line of the todo file that isn't a todo: a heading, a
note, a blank line. topdo keeps other lines exactly as written.

**Plasmoid.** A Plasma widget: a small program that lives in a panel or on
the desktop. topdo's badge and popup are one plasmoid.

**Popup.** The window that opens when you click the badge, with the todo
list.

**Property test.** A test that states a rule ("writing back an unchanged
file gives the same bytes") and checks it on many random inputs.

**QML.** Qt's language for user interfaces. Plasma widgets are written in it.

**Qt.** The C++ toolkit that KDE software is built on.

**Section.** The part of the todo file under one heading. The popup groups
todos by section.

**Subtask.** A todo indented under another todo.

**Task list item.** A Markdown list item that starts with a checkbox:
`- [ ] open` or `- [x] done`. GitHub introduced this syntax; topdo calls
each one a todo.

**Todo file.** `todo.md` in the todo folder: the Markdown file that holds
the todos. Its format is defined in [design/format.md](design/format.md).

**Todo folder.** The folder that holds the todo file, by default
`~/Documents/todo/`. It is set in `~/.config/topdo/config.toml`.
