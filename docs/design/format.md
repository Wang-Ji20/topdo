# The todo file format

Status: Decided, 2026-11-21. Replaces the Markdown format of 2026-11-19.

The todo file is a [todo.txt](https://github.com/todotxt/todo.txt) file:
one line is one todo. topdo follows the todo.txt rules, so any todo.txt
app can read and edit the same file. A few `key:value` tags, which the
spec leaves to apps, add due dates and subtasks.

## Example

```
(A) Call the bank @phone due:2026-11-21
Buy milk
Plan trip +Holiday id:1
x 2026-11-20 Book flight +Holiday p:1
Book hotel +Holiday p:1
2026-11-18 Write report +Work
x 2026-11-19 Renew passport pri:B
```

topdo sees this as:

```
(no project)
  [ ] Call the bank   @phone      due 2026-11-21
  [ ] Buy milk
  [x] Renew passport
Holiday
  [ ] Plan trip
      [x] Book flight
      [ ] Book hotel
Work
  [ ] Write report
```

## Rules

### What a todo is

1. **Every line that isn't blank is a todo.** A blank line (empty, or only
   spaces and tabs) is kept as written but is not a todo.
2. **A done todo starts with `x` and a space**, a lowercase `x`. Then come
   an optional completion date and an optional creation date, each a
   `YYYY-MM-DD` word followed by a space, then the text.
   - `xylophone lesson`, `X 2026-01-01 Plan` and `(A) x Find tickets` are
     open todos.
3. **An open todo** starts with an optional priority, an uppercase letter
   in brackets and a space (`(A) `), then an optional creation date and a
   space, then the text.
   - `(b) Call`, `Call (A)` and `(B)->Submit` have no priority.
4. **The text** is the rest of the line. In it:
   - a word that starts with `+` is a project, and one that starts with
     `@` is a context (a word is a run of characters without spaces);
   - a word of the form `key:value` is a tag: the key has no colon, the
     value is at least one character, and neither has spaces.

### Tags topdo reads

5. **`due:YYYY-MM-DD` is the due date.** A value that isn't a valid date
   is plain text. If a todo has several, the first one counts.
6. **`id:` and `p:` make subtasks.** A todo with `p:1` is a subtask of the
   todo with `id:1`. Several todos can be subtasks of one parent, and
   subtasks can have subtasks.
   - If no todo has that id, or following the `p:` tags leads back in a
     circle, the todo is top-level. If several todos have the same id, the
     first one is the parent. Only a todo's first `p:` counts.
   - Ticking a todo doesn't tick its subtasks, and ticking all subtasks
     doesn't tick their parent.
   - The badge counts only open top-level todos.
7. **`pri:A` is the priority of a done todo**, as the spec suggests: it
   keeps the priority while the todo is done.
8. **The title is the text without the `due:`, `id:`, `p:` and `pri:`
   tags, the projects and the contexts.** Other words, `key:value` ones
   such as a URL included, stay. The badge shows the title alone; the panel shows the
   title and, beside it as small tags, the contexts and any project other
   than the todo's group.

### Projects group the list

9. **A top-level todo belongs to its first project; a subtask belongs to
   the group of its top-level todo**, whatever projects it names. The
   panel lists the todos without a project first, then one group per
   project, in the order each project first appears on a top-level todo.
   Inside a group, each todo comes right before its subtasks, and todos
   at the same level keep their order in the file.

### Numbers

10. **Todos are numbered from 1 in file order**, open and done alike. The
    number of a todo changes only when todos above it are added or
    removed.

### Edits

11. **An edit touches only its own lines.** Every other line keeps its
    content and its place.
    - Ticking a todo writes `x`, a space, today's date and a space in front
      of it. A priority `(A) ` moves to the end of the text as `pri:A`; the
      creation date stays.
    - Unticking removes the `x` and the completion date. A `pri:A` tag goes
      back to the front as `(A) `.
    - Editing the text replaces only the text after the priority and the
      dates. Tags in the new text are written as given.
    - Removing a todo deletes its line and the lines of its subtasks, their
      subtasks, and so on.
12. **A new todo is written as its text**, after the last line that isn't
    blank (at the top of an empty file). Adding to a project appends
    ` +Project` to the text unless the text already has that project.
13. **A new subtask goes after its parent's last subtask** (after the
    parent itself if it has none) and gets ` p:ID` with the parent's id. A
    parent without an `id:` tag gets ` id:N` at the end of its line, with N
    the smallest whole number from 1 that no todo uses as an id.
14. **Done todos stay where they are.** The panel collapses them into a
    "Done" group; the file doesn't change. topdo has no `done.txt` archive.
15. **The file is UTF-8 text with `\n` line endings.** topdo also reads
    `\r\n` endings and a byte order mark, but always writes `\n`, no byte
    order mark, and a final newline. The content of each line stays as
    written.

## Where the file lives

- topdo uses `todo.txt` inside a todo folder, by default
  `~/Documents/todo/`.
- The folder is set once in `~/.config/topdo/topdo.conf`, a plain
  `key=value` (INI) file that the widget, the CLI and the MCP server all
  read. A leading `~/` means your home folder:

  ```ini
  dir=~/Documents/todo
  ```

- `topdo --dir <folder>` overrides it for one command.
- If the folder or the file doesn't exist, topdo creates it when the first
  todo is added. todo.txt has no comments, so the file starts empty; topdo
  also writes `README.md` into the folder, a short manual of these rules,
  unless a file of that name is already there. Its text lives in
  `manualText()` in `src/core/document.cpp`.

## Decisions and the main rejected option

| Decision | Rejected |
|---|---|
| todo.txt | Markdown task lists (the format before 2026-11-21): no other todo apps read them, and the rules had to invent nesting, sections and comments |
| Subtasks with `id:` and `p:` tags | Indented lines: not todo.txt, other apps misread them |
| A subtask stays under its parent, in its parent's group | Group subtasks by their own project: a subtask without the project would land far from its parent |
| Group by the first project | By heading: todo.txt has none; by context: projects say what a todo is for |
| A manual file in the folder | A comment preamble in the file: todo.txt has no comments, so other apps would show it as todos |
| Tick writes a completion date | No date: a done todo with a creation date would then be ambiguous, which the spec warns about |
| New todos without a creation date | Always add one: clutter for a list typed by hand |
| New todos at the end | Sorted by priority: an add would move other lines |
| Done todos stay in place | Move them to `done.txt`: an edit then changes more than its line |
| One folder in a shared config file | Per-frontend settings: the widget and CLI can drift to different files |
| An INI config file, read with Qt's `QSettings` | TOML: Qt has no parser for it, and one key doesn't need one |
| Write `\n` endings and a final newline | Keep the file's own endings and byte order mark: complexity a hand-written todo file doesn't need |
