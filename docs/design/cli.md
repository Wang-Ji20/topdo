# The `topdo` command

Status: Draft, 2026-11-19.

`topdo` reads and edits the todo file from a terminal or a script. It
follows the Unix conventions: plain text output, one todo per line, nothing
printed on success, errors on standard error with a non-zero exit status.

## Example

```
$ topdo list
1  - [ ] Buy milk
## Work
2  - [ ] Write report due:2026-11-21
3  - [ ] Plan trip
5      - [ ] Book hotel
## Home
6  * [ ] Call the bank
$ topdo done 2
$ topdo add Pay rent due:2026-12-01 --section Home
$ topdo list | grep rent
8  - [ ] Pay rent due:2026-12-01
$ $EDITOR "$(topdo path)"
```

## Numbers

- Every todo has a **number**: its position among all todos in the file,
  counting from 1, open and done alike.
- The number of a todo changes only when todos above it are added or
  removed. Hiding done todos in `list` leaves gaps (4 above is the done
  "Book flight") instead of renumbering.

## Commands

| Command | Does |
|---|---|
| `topdo list` | prints the open todos and the headings |
| `topdo list --all` | also prints the done todos |
| `topdo add <text>...` | adds a todo; the words are joined with spaces |
| `topdo add <text>... --section <name>` | adds it to the section with that heading |
| `topdo done <n>...` | ticks todos |
| `topdo undone <n>...` | unticks todos |
| `topdo edit <n> <text>...` | replaces a todo's text |
| `topdo rm <n>...` | removes todos with their subtasks and notes |
| `topdo path` | prints the path of the todo file |

- `--dir <folder>` before or after any command uses that todo folder
  instead of the configured one.
- Commands that take several numbers apply them from the highest to the
  lowest, so the numbers given all refer to the list as it was.

## Output of `list`

```
<number>  <indentation><the todo line as written, without its indentation>
```

- Each todo line keeps its indentation, so subtasks show as indented.
- Headings are printed as written, without a number.
- The number column is right-aligned to the widest number.

## Errors

| Situation | Message on standard error | Exit status |
|---|---|---|
| A number that isn't a todo | `topdo: no todo 9` | 1 |
| A section that doesn't exist | `topdo: no section "Garden"` | 1 |
| The file can't be read or written | the store's message, e.g. `topdo: cannot write /path/todo.md: ...` | 1 |
| Unknown command or option | the usage text | 2 |

Nothing is changed when any argument is wrong: `topdo done 2 9` with no
todo 9 ticks nothing.

## Open questions

- Should `list` show done todos by default, with `--open` to hide them?
- `rm` removes without asking. Should it print what it removed?

## Rejected options

- **Renumbering after hiding done todos:** the number of a todo would then
  depend on the options of the last `list`.
- **Ids stored in the file** (`- [ ] Buy milk id:3`): stable numbers, but
  they clutter a file meant to be read and written by hand.
