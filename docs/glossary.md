# Glossary

**Badge.** The small label topdo shows in the top bar, for example
`● Write report · 3`. Clicking it opens the popup.

**CMake preset.** A named set of build settings in `CMakePresets.json`, used
as `cmake --preset dev`.

**Golden file.** A test input stored together with its expected output. The
test passes when the code produces exactly that output.

**KDE Plasma.** The desktop environment topdo runs in: the top bar, the
dock, the wallpaper and the system settings.

**MCP (Model Context Protocol).** An open protocol through which AI
assistants such as Claude call tools that a program offers. topdo's MCP
server offers tools to list, add and tick off todos.

**Oracle.** An independent implementation that a test compares our code
against. If both give the same answer on many inputs, ours is likely right.

**Plasmoid.** A Plasma widget: a small program that lives in a panel or on
the desktop. topdo's badge and popup are one plasmoid.

**Popup.** The window that opens when you click the badge, with the todo
list.

**Property test.** A test that states a rule ("writing back an unchanged
file gives the same bytes") and checks it on many random inputs.

**QML.** Qt's language for user interfaces. Plasma widgets are written in it.

**Qt.** The C++ toolkit that KDE software is built on.

**Task list item.** A Markdown list item that starts with a checkbox:
`- [ ] open` or `- [x] done`. GitHub introduced this syntax; topdo calls
each one a todo.

**Todo file.** The Markdown file that holds the todos, by default
`~/Documents/todo.md`. Its format is defined in
[design/format.md](design/format.md).
