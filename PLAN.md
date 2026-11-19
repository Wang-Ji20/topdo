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
- [ ] `core`: find the todos, subtasks, sections and due dates in the lines.
- [ ] `core`: edits (add, toggle, edit text, remove).
- [ ] `store`: atomic save, file watcher, reload before write.
