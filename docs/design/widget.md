# The Plasma widget

Status: Decided, 2026-11-19.

The widget is a badge in the middle of the top bar and a panel that opens
under it. The panel is one slab of glass divided into three columns:
**todo**, **project** and **today**. It opens as a calm overview; a column
you open widens into its working view, and only then does editing appear.
The look is glass and metal with a Bauhaus geometry: few elements, set
large, built from circle, square and line.

## Badge

```
            ┌───────────────────────────────────────┐
  Konsole   │ ● Write report · 5  |  15:00 Review   │   wifi  Thu 19 Nov 14:32
            └───────────────────────────────────────┘
```

- A pill with a metal edge: a ring in the highlight colour, the next todo
  (see below), the number of open top-level todos, and, if any, the
  number of late todos in the theme's warning colour. The next event of
  the day joins it once the today column has real data; a stub event
  would mislead in the real bar.
- The next todo is the most urgent one: the earliest late or due-today
  todo, otherwise the first open todo (`nextTodo()` in core).
- It sits at the exact centre of the screen, whatever the width of the app
  menu on the left and the tray on the right.
- A click, or a global shortcut (Meta+Space; Meta+T is KWin's tiles editor), opens and closes the panel.

## Panel: overview

```
╭─────────────────────────────────────────────────────────────────────╮
│ ■ 01 TODO          │ □ 02 PROJECT        │ □ 03 TODAY              │
│                    │                     │                         │
│  ◔  5              │  topdo              │  15:00                  │
│ open · 2 done      │  ■ ■ ▣ □ □ □        │  Review                 │
│ ────────────────── │ ─────────────────── │  in 28 min · with Mia   │
│ NEXT               │ MILESTONE 3 OF 6    │                         │
│ Book hotel         │ Plasma widget       │                         │
│ yesterday          │ Experiment: frame   │                         │
│                    │                     │                         │
│ ○ Buy milk         │                     │  ━━ ━ ━━━  ●━  ━  ━━    │
│ ○ Write report     │                     │  08     14      20      │
├─────────────────────────────────────────────────────────────────────┤
│ ← → pick   1 2 3 open   ↵ open   esc close                         │
╰─────────────────────────────────────────────────────────────────────╯
```

- **One focal element per column**, set large in a geometric face: the
  number of open todos (with a ring for the share done), the project's
  name, the time of the next event. The eye lands there first.
- **Then one "next" item** under a hairline: the most urgent todo (late,
  then due today, then the first), the current milestone, the next
  event's name and countdown.
- **Then quiet details**, small and dim: a few more todos, milestone
  squares, the day as a strip from 08:00 to 20:00 with a circle at now.
- Nothing is editable here. **← →**, **Tab** or **1 2 3** pick a column;
  **Enter** or a click opens it. In an open column, **Tab** always moves to
  the next one, as in rofi; **← →** do too, except while the todo input
  holds text and needs them for its cursor.

## Panel: an expanded card

The open column takes about half the width, and the panel grows to the
full height of the screen; the other two columns stay visible, narrower
and dimmed. **Esc** steps back: it clears the input, then returns to the
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
| Tab, Shift+Tab | opens the next or previous column |
| ← → | with an empty input, opens the previous or next column; otherwise moves the cursor |

### Project (stub)

The current project: its milestones as squares, the current milestone,
the next step and the last commit. Open, it lists every milestone with
"done" or "now", then the next step and the last commit.
Where this comes from is an open question.

### Today (stub)

The day's schedule: the next event with a countdown, and the day as a
strip with a ring at the current time. Open, it shows the whole day as a
timeline: one row per event, past ones faded, the current one in the
accent colour, and a line at the current time. Where this
comes from is an open question.

## Look

- **Glass:** one translucent slab with the desktop blurred behind it
  (KWin's blur) and a faint diagonal sheen.
- **Metal:** the slab's edge is a thin border that is light at the top and
  darker below, like brushed metal catching light. Columns are divided by
  hairlines, not boxed as cards.
- **Bauhaus:** circle, square and line are the only shapes: circles for
  todos and the current time, squares for milestones and the selected
  column, lines for dividers and the day strip.
- **Type:** Poppins, a geometric face in the Futura tradition, for focal
  numbers, names and todo text; JetBrains Mono, small, for labels and data.
  (The mockup uses Jost, which openSUSE doesn't package; Poppins is in
  `google-poppins-fonts`.)
- **Colour:** greys, plus the Plasma theme's highlight colour as the only
  accent (selection, progress, now). Red and amber only for late and soon
  due dates.
- **Height:** about 430 px for the overview. Opening a column grows the
  panel to the full height of the screen below the top bar, so a long
  todo list or a whole day fits.
- **Contrast:** the glass is a lighter graphite and mostly opaque (about
  88 %), so text stays readable over any wallpaper; secondary text is a light grey, not a dim
  one, and the narrow columns stay at about 70 % while another is open.

## How it is built

- The widget is a plasmoid. The badge is shown inline in the bar; the
  panel is our own dialog, not Plasma's popup (see below).
- A C++ QML plugin (`src/qml/`) offers the todos as a list model over the
  store, and watches the todo file, so edits from the CLI or a text editor
  show at once.
- **Centring:** the widget stands where the bar's spacer was, between the
  app menu and the tray, and takes the free space. It places the pill at
  the screen's centre, from its own position on screen
  (`mapToGlobal`) and the screen's geometry, and recomputes whenever its
  width changes. The app menu stretches too (its layout ends with a
  filler item), and the bar splits free space between stretching
  widgets; the badge asks for the whole screen width as its preferred
  width, so the app menu keeps only its buttons. Checked in the real top
  bar: the pill's centre is the screen's centre.
- **Our own frame** (tried 2026-11-19): the badge opens a
  `PlasmaCore.Dialog` with `backgroundHints: NoBackground`, and the panel
  is drawn in QML. Without the theme's background KWin no longer blurs
  behind the dialog, so the QML plugin's `WindowEffects.blurBehind()` asks
  for it (`KWindowEffects`, KDE Frameworks), with round corners. Checked
  in the real top bar: the panel opens right under the badge, kept on
  screen near an edge, as a transparent, blurred slab with our border.
- **The panel's QML:** `Panel.qml` draws the slab, the columns and the
  hint line and handles the keys; each column is its own file
  (`TodoOverview.qml` and, when open, `TodoOpen.qml`; `ProjectStub.qml`,
  `TodayStub.qml`), and
  `Style.qml` holds the colours (from the Plasma theme), typefaces, sizes
  and the due-date label. The open todo column binds the model's filter
  to what is typed (not while editing) and clears it when it closes, so
  the overview always sees the whole list. Keys it doesn't use go on to
  the panel: Tab always, Esc and ← → only with an empty input.
- **No geometry animations.** The panel is a window, so animating its
  height resizes a Wayland window and re-requests the blur on every
  frame, and animating column widths re-lays out every column; both made
  opening a column lag. Sizes change at once; only the opened column's
  content fades in (`OpacityAnimator`, on the render thread), and blur is
  requested once per resize. `plasmoid/preview.qml` shows the panel in a
  plain window for development, without Plasma or blur.
- **The badge is the widget's `fullRepresentation`**, shown inline in the
  bar (`preferredRepresentation: fullRepresentation`). With only a
  `compactRepresentation`, Plasma never creates it, and the panel has
  nothing to open under.

## Decisions on the earlier open questions

- **Accent:** the Plasma theme's highlight colour.
- **Our own frame:** a Plasma dialog without the theme's background,
  plus KWin's blur requested by the QML plugin; no layer-shell window is
  needed. See "How it is built".
- **Project and today data:** stubs for now; their sources (for example
  a git repository and a calendar) are milestone 5 of the roadmap.

## Rejected options

- **A form with buttons** (add field, checkboxes, edit and delete icons):
  needs the mouse for everything.
- **A colourful gradient border** (Hyprland's cyan to green): too loud
  next to the rest of the desktop.
- **Boxed cards with many small facts:** read like a web dashboard, with
  nowhere for the eye to land.
- **Editing controls always visible:** the overview should read at a
  glance; editing appears only in the selected card and row.
