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

1. **A todo is a task list item:** a bullet list marker (`-`, `*` or `+`),
   a space, a checkbox, a space, and the text.
   - `[ ]` means open; `[x]` and `[X]` mean done.
   - Ordered list items (`1. [ ] text`) are not todos; they are other lines.
2. **Everything else is an other line**, kept byte for byte: headings,
   paragraphs, blank lines, notes under a todo, and lines that only look
   like todos, such as those inside a code block.

The parser must agree with a real Markdown parser on rule 1; the tests
compare it against cmark-gfm on bullet lists.

### Structure

3. **A subtask is a todo nested in another todo's list item**, by
   indentation, as Markdown defines it. Subtasks can nest further.
   - Ticking a todo doesn't tick its subtasks, and ticking all subtasks
     doesn't tick their parent.
   - The badge counts only open top-level todos.
4. **A heading starts a section** named by the heading's text. Only ATX
   headings count (`#` to `######` at the start of a line). Todos before
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
9. **The file keeps its encoding and line endings.** topdo reads UTF-8 and
   writes back the same line endings (LF or CRLF), byte order mark and
   final newline that it read.

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
| Headings are sections | One flat list: no place to keep work and home todos apart |
| `due:YYYY-MM-DD` | Obsidian Tasks' `📅 2026-11-21`: awkward to type in a terminal |
| One folder in a shared config file | Per-frontend settings: the widget and CLI can drift to different files |
| Done todos stay in place | Move them to a `## Done` section: an edit then changes more than its line |
| Markdown | todo.txt: no place for headings and notes; JSON: unpleasant to edit by hand |
