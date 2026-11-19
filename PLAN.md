# Plan

Current work. Tick items as they land; see [ROADMAP.md](ROADMAP.md) for the
long term.

## Milestone 1: core and store

- [x] Repository skeleton: AGENTS.md, build presets, lint configs, docs.
- [x] Decide the todo file format:
      [docs/design/format.md](docs/design/format.md).
- [x] Install the build dependencies (see
      [docs/user-manual.md](docs/user-manual.md)).
- [x] `core`: split a file into lines and join it back, with the round-trip
      property test and the random-file generator.
- [x] `core`: find the todos, subtasks, sections and due dates in the lines.
- [x] `core`: edits (add, toggle, edit text, remove).
- [x] `store`: load, atomic save, update that starts from the file on disk,
      and the config file.

## Milestone 2: command line

- [x] Decide the command design: [docs/design/cli.md](docs/design/cli.md).
- [x] `topdo list` and `topdo path`.
- [x] `topdo add`, `done`, `undone`, `edit`, `rm`.

## Milestone 3: Plasma widget

- [x] Decide the widget design: [docs/design/widget.md](docs/design/widget.md).
- [x] Experiment: our own glass frame on a Plasma popup (works in
      plasmoidviewer; see docs/design/widget.md).
- [x] Check in the real top bar: the panel opens under the badge.
- [x] `core`: which todos the panel lists, and the badge's next todo.
- [x] `qml`: a list model over the store that watches the todo file.
- [x] The badge: next todo, open and late counts, at the screen's centre.
- [x] The overview: three columns, project and today as stubs.
- [x] The open todo column: input line, filtering, keys.
- [x] Install and placement through the dotfiles (clone of `main` once it exists).
