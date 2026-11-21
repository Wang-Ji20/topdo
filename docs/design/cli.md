# The `topdo` command

Status: Decided, 2026-11-19; todo.txt since 2026-11-21.

`topdo` reads and edits the todo file from a terminal or a script. It
follows the Unix conventions: plain text output, one todo per line, nothing
printed on success, errors on standard error with a non-zero exit status.

## Example

```
$ topdo list
1  (A) Call the bank @phone due:2026-11-21
2  Buy milk
3  Plan trip +Holiday id:1
5  Book hotel +Holiday p:1
6  2026-11-18 Write report +Work
$ topdo done 2
$ topdo add Pay rent due:2026-12-01 --project Home
$ topdo list | grep rent
8  Pay rent due:2026-12-01 +Home
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
| `topdo list` | prints the open todos, numbered, each line as written |
| `topdo list --all` | also prints the done todos |
| `topdo add <text>...` | adds a todo; the words are joined with spaces |
| `topdo add <text>... --project <name>` | adds it with ` +name`, unless the text has it |
| `topdo add <text>... --under <n>` | adds it as the last subtask of todo n |
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
<number>  <the todo line as written>
```

- Lines are printed in file order, not grouped by project, so `grep` and
  `sort` see the file as it is.
- The number column is right-aligned to the widest number.

## Errors

| Situation | Message on standard error | Exit status |
|---|---|---|
| A number that isn't a todo | `topdo: no todo 9` | 1 |
| `--project` and `--under` together | the usage text | 2 |
| The file can't be read or written | the store's message, e.g. `topdo: cannot write /path/todo.txt: ...` | 1 |
| Unknown command or option | the usage text | 2 |

Nothing is changed when any argument is wrong: `topdo done 2 9` with no
todo 9 ticks nothing.

## Rejected options

- **`list` shows done todos by default:** they are what you usually don't
  want to see; `--all` shows them.
- **`rm` prints what it removed:** the other commands are silent too, and
  `list` shows the result.
- **Renumbering after hiding done todos:** the number of a todo would then
  depend on the options of the last `list`.
- **Numbers from `id:` tags:** stable, but only subtask parents have one,
  and giving every todo an id clutters a file meant to be read and written
  by hand.
