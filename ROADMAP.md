# Roadmap

Each milestone ends in something usable. Bigger scope changes go here first.

1. ~~**Core and store.**~~ Done. Parse, edit and save a todo file safely. Details in
   [PLAN.md](PLAN.md).
2. ~~**Command line.**~~ Done. `topdo list`, `topdo add`, `topdo done`: the first way
   to use topdo for real, and a test bed for the store.
3. **Plasma widget.** The badge in the middle of the top bar and a panel
   with three cards: todo (overview, then quick-add, tick off and edit),
   project and today, the last two as stubs at first. Installed and placed
   by the dotfiles.
3a. **todo.txt** (2026-11-21). The todo file becomes a todo.txt file, so
   other todo.txt apps share it: subtasks by `id:`/`p:` tags, groups by
   `+project`, a manual in the todo folder. See
   [docs/design/format.md](docs/design/format.md).
4. **MCP server.** `topdo mcp` offers list, add and tick-off tools to AI
   assistants.
5. ~~**Project and today cards with real data.**~~ Done: the project card
   shows a +project of todo.txt, the today card the KDE calendar. See
   [docs/design/project-today.md](docs/design/project-today.md).
6. **Grow with use.** Candidates: priorities in the panel, a global
   shortcut, more widgets in the popup.
