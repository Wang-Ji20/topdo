# The todo file format

Status: Draft, 2026-11-19.

The todo file is a Markdown file. topdo reads the todos out of it and
changes only those lines; everything else in the file is the user's and stays
untouched.

## Example

```markdown
# Todo

- [ ] Write report
- [ ] Call the bank
  Ask about the transfer fee.
- [x] Renew passport

Notes that are not todos stay exactly as written.
```

topdo sees three todos here: two open and one done. The heading, the
indented note and the last paragraph are other lines: topdo keeps them but
never shows or changes them.

## Proposed rules

1. **A todo is a task list item:** a line `- [ ] text` (open) or
   `- [x] text` (done), the syntax GitHub introduced for Markdown.
2. **Everything else is an other line**, kept byte for byte: headings,
   paragraphs, blank lines, code blocks, and lines that only look like
   todos (for example inside a code block).
3. **Edits touch only their own line.** Ticking off a todo changes `[ ]`
   to `[x]` and nothing else. Adding a todo inserts one new line.
4. **The file keeps its encoding and line endings.** topdo reads UTF-8,
   remembers whether lines end in LF or CRLF, and writes them back the same
   way.

Rule 2 is why the parser must agree with a real Markdown parser on what a
task list item is; the tests compare it against cmark-gfm.

## Open questions

- **Which list markers count?** Markdown also allows `*` and `+`, and `[X]`
  for done. Accept them all when reading, and write `-` and `[x]`?
- **Where do new todos go?** At the end of the first task list, at the top
  of the file, or under a chosen heading?
- **Nested todos** (indented under another todo): subtasks, or treated as
  independent todos?
- **Headings as sections:** should the popup group todos by the heading
  above them?
- **Due dates:** a `due:2026-11-21` tag (as in the todo.txt format), or
  Obsidian Tasks' `📅 2026-11-21`? Either way the text stays readable as
  plain Markdown.
- **Location of the file:** `~/Documents/todo.md` by default, changeable in
  the widget's settings and by `topdo --file`?
- **Done todos:** keep them in place, or let the user move them to a
  `## Done` section?

## Rejected options

- **todo.txt** (one todo per line, no Markdown): simpler to parse, but not
  Markdown, so headings and notes have no place.
- **JSON or a database:** easy for code, but not pleasant to read or edit by
  hand, which is the point of a plain file.
