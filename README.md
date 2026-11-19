# topdo

A todo list for the KDE Plasma desktop. A badge in the middle of the top bar
shows how many todos are open; clicking it opens a popup to add, tick off and
edit them.

The list is a plain Markdown file that you can also edit in any text editor.
A command-line tool, `topdo`, and an MCP server for AI assistants read and
edit the same file.

```
~/Documents/todo/todo.md     top bar              popup
- [ ] Write report      ->   ● Write report · 3   [ add a todo...      ]
- [ ] Call the bank                               [ ] Write report
- [x] Renew passport                              [ ] Call the bank
```

topdo is at an early stage: see [PLAN.md](PLAN.md) for current work and
[ROADMAP.md](ROADMAP.md) for where it is going. Documentation starts at
[docs/README.md](docs/README.md); rules for contributors and coding agents are
in [AGENTS.md](AGENTS.md).

License: MIT.
