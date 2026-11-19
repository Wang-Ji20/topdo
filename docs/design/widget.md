# The Plasma widget

Status: Draft, 2026-11-19.

The widget is a badge in the middle of the top bar and a panel that opens
under it. The panel holds three cards side by side: **todo**, **project**
and **today**. It opens as a calm overview; a card you select expands into
its working view, and only then does editing appear. The look is plain: a
dark, translucent panel, neutral greys and one muted accent.

## Badge

```
            ┌───────────────────────────────────────┐
  Konsole   │ ● Write report · 5  |  15:00 Review   │   wifi  Thu 19 Nov 14:32
            └───────────────────────────────────────┘
```

- A pill with a thin plain border: a dot, the first open top-level todo,
  the number of open top-level todos, and the next event of the day.
- It sits at the exact centre of the screen, whatever the width of the app
  menu on the left and the tray on the right.
- A click, or a global shortcut (Meta+T), opens and closes the panel.

## Panel: overview

```
╭────────────────────────────────────────────────────────────────────────────╮
│ ┌─ TODO ─────── 1 ─┐ ┌─ PROJECT ──── 2 ─┐ ┌─ TODAY ────────── 3 ─┐ │
│ │ 5  open · 2 done │ │ topdo  2 of 5    │ │ in 28 min · 15:00    │ │
│ │ ███████░░░░░░░░  │ │ ██ ██ ▓▓ ░░ ░░   │ │ Review               │ │
│ │ ▢ Buy milk       │ │ now: Plasma      │ │ 14:00 Deep work      │ │
│ │ ▢ Write report Sat│ │ · Decide design  │ │ ── 14:32 ─────────   │ │
│ │ ▢ Plan trip      │ │ · Experiment     │ │ 15:00 Review         │ │
│ │ ▢ └ Book hotel  …│ │ last commit …    │ │ 17:00 Gym            │ │
│ └──────────────────┘ └──────────────────┘ └──────────────────────┘ │
│ ← → pick   1 2 3 jump   ↵ expand   esc close                       │
╰────────────────────────────────────────────────────────────────────────────╯
```

- Each card shows a summary first: a big number, a thin progress bar, and
  the few items that matter now. Nothing is editable here.
- **← →** or **1 2 3** pick a card; **Enter** or a click expands it.

## Panel: an expanded card

The expanded card takes about half the width; the other two stay visible,
narrower. **Esc** steps back: it clears the input, then returns to the
overview, then closes the panel.

### Todo

```
┌─ TODO ──────────────────────────────────────── 5 open ─┐
│ ┌───────────────────────────────────────────────────┐ │
│ │ todo  type to filter, or a new todo               │ │
│ └───────────────────────────────────────────────────┘ │
│  1 ▢ Buy milk                                         │
│    WORK                                               │
│  2 ▢ Write report      ↵ tick  ⌃E edit  ⇧⌦ remove Sat│  <- highlighted row
│  3 ▢ Plan trip                                        │
│  5 ▢ └ Book hotel                           yesterday │
│    ▸ done · 2                                         │
└───────────────────────────────────────────────────────┘
```

- The input line appears and has the focus. Typing filters the list
  (case-insensitive, by substring) and adds a last row,
  `add "what you typed"`.
- Numbers, the same as `topdo list`, appear only here.
- The actions appear only on the highlighted row.

| Key | Does |
|---|---|
| typing | filters the list |
| ↑ ↓, Ctrl+J/K | moves the highlight |
| Enter | ticks or unticks the highlighted todo; on the add row, adds |
| Ctrl+Enter | adds what you typed, whatever is highlighted |
| Ctrl+E | edits the highlighted todo in the input line; Enter saves, Esc cancels |
| Shift+Delete | removes the highlighted todo with its subtasks |
| Ctrl+H | shows or hides done todos |

### Project (stub)

The current project: its milestones as segments, the current milestone,
the next steps and the last commit. Expanded, it lists every milestone.
Where this comes from is an open question.

### Today (stub)

The day's schedule: the next event with a countdown, and a timeline with a
line at the current time. Expanded, it shows the whole day. Where this
comes from is an open question.

## Look

- A dark, translucent panel with the desktop blurred behind it (KWin's
  blur), a thin plain border, round corners (about 14 px).
- Neutral greys and one muted accent, used for selection, progress and the
  current time. Red and amber appear only for late and soon due dates.
- JetBrains Mono throughout; card names in small spaced capitals.

## How it is built

- The widget is a plasmoid: `compactRepresentation` is the badge,
  `fullRepresentation` the panel.
- A C++ QML plugin (`src/qml/`) offers the todos as a list model over the
  store, and watches the todo file, so edits from the CLI or a text editor
  show at once.
- Centring: the widget fills the space between the app menu and the tray
  and places the badge at the screen's centre within it.

## Open questions

- **Accent colour:** the theme's highlight colour (the agreement says to
  take colours from the theme), or a fixed muted blue-grey?
- **Our own frame on a Plasma popup:** a plasmoid's popup gets the Plasma
  theme's frame. Drawing our own needs either a popup without the theme
  background or a separate window placed through layer-shell. This needs a
  short experiment before anything else.
- **Where project and today come from:** for example a git repository and
  its `ROADMAP.md` for the project, and a calendar (an `.ics` file or
  KDE's calendar) for today. Until decided, both cards are stubs.

## Rejected options

- **A form with buttons** (add field, checkboxes, edit and delete icons):
  needs the mouse for everything.
- **A colourful gradient border** (Hyprland's cyan to green): too loud
  next to the rest of the desktop.
- **Editing controls always visible:** the overview should read at a
  glance; editing appears only in the selected card and row.
