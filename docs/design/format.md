# The todo file format

Status: Decided, 2026-11-19.

The todo file is a Markdown file. topdo reads the todos out of it and changes
only those lines; everything else in the file is the user's and stays
untouched.

## Example

```markdown
- [ ] Buy milk

## Work

- [ ] Write report due:2026-11-21
- [ ] Plan trip
  - [x] Book flight
  - [ ] Book hotel
  Ask about late check-in.

## Home

* [ ] Call the bank
- [x] Renew passport

Notes that are not todos stay exactly as written.
```

topdo sees this as:

```
(unnamed section)
  [ ] Buy milk
Work
  [ ] Write report            due 2026-11-21
  [ ] Plan trip
      [x] Book flight
      [ ] Book hotel
Home
  [ ] Call the bank
  [x] Renew passport
```

The note under "Plan trip" and the last paragraph are other lines: topdo
keeps them but never shows or changes them.

## Rules

### What a todo is

topdo reads the file line by line with the simple rules below. They follow
how Markdown task lists look, but topdo doesn't implement Markdown: you can
tell what topdo sees by looking at each line.

1. **A todo is a line of the form `- [ ] text`:** optional indentation, a
   bullet (`-`, `*` or `+`), spaces, a checkbox, spaces, and the text.
   - `[ ]` means open; `[x]` and `[X]` mean done.
   - Numbered items (`1. [ ] text`) are not todos.
2. **Everything else is an other line**, kept as written: headings,
   paragraphs, blank lines and notes. Lines between two code fences (lines
   starting with ```` ``` ```` or `~~~`) are always other lines, even when
   they look like todos.

### Structure

3. **A subtask is a todo indented more than the todo above it.** Its
   parent is the nearest todo above it with less indentation, in the same
   section. Subtasks can nest further. A tab counts as four spaces.
   - Ticking a todo doesn't tick its subtasks, and ticking all subtasks
     doesn't tick their parent.
   - The badge counts only open top-level todos.
4. **A heading starts a section** named by the heading's text. Only ATX
   headings count: `#` to `######` and a space at the start of a line,
   outside code fences. Todos before
   the first heading belong to an unnamed section.
5. **A due date is a `due:YYYY-MM-DD` word** in the todo's text, separated
   from other words by spaces. The popup shows it as a date and leaves it
   out of the title. A word that isn't a valid date is plain text. If a
   todo has several, the first one counts.

### Edits

6. **An edit touches only its own lines.**
   - Ticking or unticking changes only the character between the brackets:
     a space becomes `x`; `x` or `X` becomes a space. The list marker stays.
   - Editing the text replaces only the text after the checkbox.
   - Removing deletes the todo's line and the lines of its list item (its
     subtasks and notes).
7. **New todos are written as `- [ ] text`** and go:
   - into a given section: after the last line of that section's first run
     of todos, or right after the heading if the section has none;
   - with no section given: after the last line of the first run of todos
     in the file;
   - into a file with no todos: at the end of the file.
8. **Done todos stay where they are.** The popup collapses them into a
   "Done" group; the file doesn't change.
9. **The file is UTF-8 text with `\n` line endings.** topdo also reads
   `\r\n` endings and a byte order mark, but always writes `\n`, no byte
   order mark, and a final newline. The content of each line stays as
   written.

## Where the file lives

- topdo uses `todo.md` inside a todo folder, by default `~/Documents/todo/`.
  The folder leaves room for more files later (for example `done.md`).
- The folder is set once in `~/.config/topdo/config.toml`, which the
  widget, the CLI and the MCP server all read:

  ```toml
  dir = "~/Documents/todo"
  ```

- `topdo --dir <folder>` overrides it for one command.
- If the folder or the file doesn't exist, topdo creates it when the first
  todo is added.

## Decisions and the main rejected option

| Decision | Rejected |
|---|---|
| Read all bullet markers and `[X]`; write `- [ ]` and `[x]` | Only `- [ ]`: todos written with `*` would silently disappear |
| New todos at the end of the first run of todos | At the end of the file: they can land below unrelated notes |
| Nested todos are subtasks | A flat list: loses the structure the user wrote |
| Simple line rules | Follow CommonMark exactly (lazy lines, list item content columns): hundreds of lines of rules nobody can predict by looking at the file |
| Headings are sections | One flat list: no place to keep work and home todos apart |
| `due:YYYY-MM-DD` | Obsidian Tasks' `📅 2026-11-21`: awkward to type in a terminal |
| One folder in a shared config file | Per-frontend settings: the widget and CLI can drift to different files |
| Done todos stay in place | Move them to a `## Done` section: an edit then changes more than its line |
| Markdown | todo.txt: no place for headings and notes; JSON: unpleasant to edit by hand |
| Write `\n` endings and a final newline | Keep the file's own endings and byte order mark: complexity a hand-written todo file doesn't need |
