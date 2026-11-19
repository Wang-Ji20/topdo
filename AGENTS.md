# Working agreement for agents

Rules for any coding agent (Claude Code and others) working on topdo. They
record what the maintainer and the agent have agreed on.

topdo is a todo list for the KDE Plasma desktop: a badge in the middle of the
top bar opens a popup with the list. The list lives in a plain Markdown file,
and a command-line tool and an MCP server read and edit the same file.

## Git workflow

- **Agents commit only to `dev`.** Never commit to `main`, merge into it, or
  rewrite its history. The maintainer reviews `dev` and merges it.
- **Agent commits are unsigned and made under the agent's own name:**

  ```
  git -c user.name="Claude" -c user.email="noreply@anthropic.com" commit --no-gpg-sign
  ```

  The maintainer's GPG key signs only the maintainer's own commits and
  merges. An agent never uses it or triggers its passphrase prompt.
- **Every commit is atomic.** It holds one logical change, builds, and passes
  `ctest --preset dev` on its own.
- **Keep commits small.** Size is insertions plus deletions in
  `git diff --cached --shortstat`:
  - Aim for **at most 500 lines**; split larger work into a sequence of
    atomic commits.
  - Up to **1000 lines** is fine when the change isn't complicated.
  - Larger is allowed only when the change truly can't be split. Say why in
    the commit message.
  - **No limit** for formatter runs or rule-based refactors (renames applied
    uniformly). Keep them in commits of their own, apart from behaviour
    changes.
- Commit messages: a type prefix (`feat:`, `fix:`, `refactor:`, `test:`,
  `docs:`, `build:`), an imperative summary line, then a body that says why.
  End with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>` when
  Claude wrote the change.

## Project philosophy

Keep the code as simple as the current task allows. Add an abstraction when a
concrete task needs it, not to anticipate every possible case. We start small
and let the design grow with the requirements.

- **Never lose the user's data.** The Markdown file belongs to the user. An
  edit changes only the lines it touches; every other line keeps its content.
  Writes are atomic (`QSaveFile`). If the file changed on disk since it was
  read, reload and reapply the edit; never overwrite.
- **The format is the contract.** `docs/design/format.md` defines what a
  todo file is. A format change starts there, with tests, before any code.
- **Keep it simple, stupid (KISS), the Unix way.** Each part does one
  thing well. Plain text is the interface: the todo file is readable and
  editable by hand, and the `topdo` command composes with `grep`, pipes and
  scripts. Prefer simple rules a user can predict from the file over
  faithful emulation of a standard. Prefer the conventional behaviour
  (UTF-8 text, `\n` line endings, a final newline) over machinery for rare
  edge cases. If an edge case could really lose data, name it in one line
  and let the maintainer decide; don't build for it unasked.
- **Explicit over implicit.** Plain function calls and visible data flow over
  signal chains, global state and hidden conventions.
- **Less state.** The core is pure functions on value types. Qt objects that
  must hold state are a thin layer on top of it.
- **No pink elephants.** Don't add defensive checks for cases that can't
  happen. Validate at the edges (file input, CLI arguments, MCP requests),
  then trust the data inside.
- **Slow-changing parts never depend on fast-changing ones.** See the layers
  below.

## Layers

From stable to fast-changing. A layer may include only the layers above it.

```
docs/design/format.md   the todo file format: the contract
        |
src/core/               pure: parse(text) -> Document,
        |               findTodos(Document), edits: Document -> Document,
        |               render(Document) -> text
        |               value types only: no QObject, no I/O
src/store/              file I/O: load, atomic save, update from the file on disk
        |
        +-- src/qml/    QML plugin: a QAbstractListModel over the store
        +-- src/cli/    the `topdo` command
        +-- src/mcp/    `topdo mcp`: MCP server on stdio
plasmoid/               the Plasma widget, QML only: badge and popup
```

- `core` never includes `store`; frontends (`qml`, `cli`, `mcp`) never
  include each other.
- Logic lives in `core` and `store`. Frontends translate between their
  world (QML, command-line arguments, JSON-RPC) and the store, and nothing
  more.
- QML is the view: bindings and small event handlers only. No parsing, file
  access or business rules in QML or JavaScript.

## Code

- Write modern, readable, modular code: small focused functions, clear
  names, no god-functions. Breaking a rule is fine when there's a reason;
  say why in a comment or the commit message.
- **Nesting:** at most one level of nested calls, and only when the inner
  call is a plain conversion or getter, as in
  `QString::fromUtf8(file.readAll())`. Otherwise assign the inner result to
  a named variable first.
- **Order:** define a function before the functions that call it, so a file
  reads bottom-up from its building blocks to its entry points. In `.cpp`
  files, helpers go in an anonymous namespace above their callers.
- Don't nest control flow deeply: return early, and split out a function
  when a block needs a comment to explain what it does.
- Comments say why, not what. English throughout.
- After getting something to work, do a cleanup pass before calling it done.

### C++

- C++23, Qt 6, KDE Frameworks 6, CMake with extra-cmake-modules, Ninja.
- Formatting: `.clang-format` (LLVM-based, 4-space indent, 100 columns).
  Run `clang-format -i` on changed files before committing.
- Naming follows Qt and KDE, since every API we call does: classes
  `PascalCase`, functions and variables `camelCase`, members `m_name`,
  constants `PascalCase`. File names are lowercase: `todomodel.cpp`.
- Warnings are errors (`-Wall -Wextra -Werror` in the `dev` preset).
  `.clang-tidy` must report nothing on changed files.
- Prefer value types, `std::optional`, and `std::expected` for operations
  that can fail, over sentinel values and out-parameters. Use `enum class`
  and exhaustive
  `switch` statements without a `default:`, so the compiler flags a missing
  case.
- Keep `Q_OBJECT` classes small; anything that can be a free function in
  `core` should be one.
- No `using namespace` in headers.

### QML

- Format with `qmlformat`; `qmllint` must report nothing.
- Order inside a component: `id`, properties, signals, functions, then child
  objects.
- Use Kirigami and PlasmaComponents 3. No hard-coded colours or pixel sizes:
  use `Kirigami.Theme` and `Kirigami.Units`, so the widget follows the theme
  and the display scale.

## Self-contained repository

- The repo builds and tests with the libraries packaged by the distribution
  (listed in `docs/user-manual.md`). Nothing third-party is vendored.
- Downloading, building or running outside code needs the maintainer's
  explicit permission.

## Testing

- `ctest --preset dev` must pass before every commit.
- Unit and golden-file tests use QTest. Golden files live in
  `tests/data/`: an input, the expected result, and nothing else.
- The plasmoid's QML is tested with Qt Quick Test (`tests/qml/tst_*.qml`,
  run offscreen by `tst_quick`), with real key presses. Its `Fixture`
  writes todo files to a temporary folder and reads them back.
- **Property tests** for new logic wherever a property can be stated. We
  have no property-testing library, so `tests/` holds a small generator of
  random todo files. It takes a seed and prints it when a test fails, so
  every failure can be replayed. Core properties:
  - render(parse(text)) == text for a file with `\n` line endings and a
    final newline.
  - An edit changes only the lines of the items it touches.
- Every bug fixed gets a regression test first.

## Documentation

- **Every commit keeps the docs current.** A commit that changes behaviour,
  a command, an interface, the format or the design updates the affected
  docs in the same commit. A commit that moves files updates every path
  that names them.
- Documentation lives in `docs/`:

  | Doc | Owns | Update when |
  |---|---|---|
  | `README.md` | Index of the docs and reading order | A doc is added, moved or removed |
  | `glossary.md` | Every term and abbreviation | A new term appears anywhere |
  | `user-manual.md` | How to install, build, test and use topdo | A command, option, dependency or path changes |
  | `architecture.md` | Layers, modules and how data flows | A module or interface changes |
  | `design/<topic>.md` | One design: status, decision, rules, rejected options, open questions | Before building a component, and when its design changes |

- **Style:** clear, concise English. Short sentences, one idea per
  paragraph, a list or table over a long paragraph. Lead with the point,
  then the detail. Prefer a small example over an abstract explanation. Use
  ASCII diagrams inside Markdown.
- **Write with clarity and grace** (after Joseph Williams, *Style: Lessons
  in Clarity and Grace*):
  - Make the main characters the subjects and their actions the verbs: "the
    store reloads the file", not "a reload of the file is performed".
  - Keep subjects short and get to the verb quickly.
  - Put familiar information first and new information last.
  - Cut words that add nothing: "in order to", "it should be noted that".
  - Prefer the active voice.
- **Self-contained:** a reader with general computer knowledge (what a file,
  a process and a compiler are) can follow every doc without knowing Qt,
  Plasma or MCP. Define a term on first use or link to the glossary. Never
  write "as discussed".
- **One home per fact.** Write a fact once, in the doc that owns it, and
  link to it from elsewhere. Code comments own the exact rules of a
  function; docs explain why and how the pieces fit.
- **No update notes.** Docs carry no "last updated" lines; git holds the
  history. A doc says what is true now. Write dates as absolute dates
  (2026-11-19).
- **Design docs** start with a status line: `Draft`, `Decided` or
  `Superseded`, with the date. A decided design states the decision and the
  main rejected option in one line each. Write the design before the code.

## Planning

- `PLAN.md` is the live checklist for current work. Update it as tasks land.
- `ROADMAP.md` holds the long-term plan. Mark items there as they finish.
  Bigger scope changes go into the roadmap first.
