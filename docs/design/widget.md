# The Plasma widget

Status: Draft, 2026-11-19.

The widget is a badge in the middle of the top bar and a popup that opens
under it. The popup works like a launcher such as rofi: one input line, a
list that follows what you type, and the keyboard for everything. It looks
like a tiling-desktop window (Hyprland): dark translucent glass, round
corners, a gradient border, a monospace font.

## Badge

```
            ┌──────────────────────────────┐
  Konsole   │ ● Write report · 5           │   wifi  Thu 19 Nov 14:32
            └──────────────────────────────┘
```

- A pill with a gradient border: a dot, the first open top-level todo, and
  the number of open top-level todos. With nothing open it reads
  "all done".
- It sits at the exact centre of the screen, whatever the width of the app
  menu on the left and the tray on the right.
- A click, or a global shortcut (Meta+T), opens and closes the popup.

## Popup

```
╭──────────────────────────────────────────────────────╮
│ todo  type to filter, or a new todo           5 open │
├──────────────────────────────────────────────────────┤
│  1 ▢ Buy milk                                        │
│    WORK                                              │
│  2 ▢ Write report                               Sat  │
│  3 ▢ Plan trip                                       │
│  5 ▢ └ Book hotel                          yesterday │
│    HOME                                              │
│  6 ▢ Call the bank                             today │
│    ▸ done · 2                                        │
├──────────────────────────────────────────────────────┤
│ ↵ tick / add  ⌃↵ add  ⌃E edit  ⇧⌦ remove  esc close  │
╰──────────────────────────────────────────────────────╯
```

- The **input line** has the focus as soon as the popup opens. Its label
  shows the mode: `todo`, or `edit 3` while editing todo 3.
- The **list** shows open todos grouped by section, subtasks indented, with
  the same numbers as `topdo list`. Due dates show as a small chip: a
  weekday within a week, "today" and "tomorrow" in yellow, late in red.
- **Done todos** collapse into "done · 2" at the bottom.
- **Typing filters** the list (case-insensitive, by substring) and adds a
  last row, `add "what you typed"`. The first match is highlighted; with
  no match, the add row is.
- The **hint bar** at the bottom lists the keys, as rofi's message bar
  does.

## Keys

| Key | Does |
|---|---|
| typing | filters the list |
| ↑ ↓, Ctrl+J/K | moves the highlight |
| Enter | ticks or unticks the highlighted todo; on the add row, adds |
| Ctrl+Enter | adds what you typed, whatever is highlighted |
| Ctrl+E | edits the highlighted todo in the input line; Enter saves, Esc cancels |
| Shift+Delete | removes the highlighted todo with its subtasks |
| Ctrl+H | shows or hides done todos |
| Esc | clears the input; on an empty input, closes the popup |

The mouse works too: hovering highlights, a click ticks.

## Look

- Glass: a dark, translucent background with the desktop blurred behind
  it (KWin's blur).
- A 2 px border with a diagonal gradient, round corners (about 14 px).
- JetBrains Mono throughout; section names in small spaced capitals.
- The highlighted row has a faint tint and a thin outline.

## How it is built

- The widget is a plasmoid: `compactRepresentation` is the badge,
  `fullRepresentation` the popup.
- A C++ QML plugin (`src/qml/`) offers the todos as a list model over the
  store, and watches the todo file, so edits from the CLI or a text editor
  show at once.
- Centring: the widget fills the space between the app menu and the tray
  and places the badge at the screen's centre within it.

## Open questions

- **Colours:** the agreement says to take colours from the Plasma theme.
  The gradient border could run from the theme's highlight colour to a
  second theme colour, or use fixed Hyprland colours (cyan to green), set
  in the widget's settings.
- **Glass and border on a Plasma popup:** a plasmoid's popup gets the
  Plasma theme's own frame. Drawing our own frame needs either a popup
  without the theme background, or a separate window placed through
  layer-shell. This needs a short experiment before anything else.
- **Numbers in the popup:** shown dim, to match `topdo list`, or hidden?

## Rejected options

- **A form with buttons** (add field, checkboxes, edit and delete icons):
  needs the mouse for everything, and doesn't feel like a launcher.
