# The project and today columns

Status: Draft, 2026-11-21. Roadmap milestone 5. The open questions below
need the maintainer's decision before any code.

The panel's second and third columns are stubs with sample data (see
[widget.md](widget.md)). This design decides where their data comes from
and what they show. It keeps the widget's shape: one focal element, one
"next" item, quiet details; open, a working view.

## Project

### Proposal: the project comes from the todo file

A project is a `+project` in `todo.txt` (see [format.md](format.md)). No
new file, no new source: the column is another view of the same list.

```
Paper +AidWalk id:1                      <- a milestone: a top-level todo
x 2026-11-20 Run baselines +AidWalk p:1     with subtasks
Write discussion +AidWalk p:1
x 2026-11-15 Data pipeline +AidWalk      <- a milestone without steps
Supplementary material +AidWalk id:2
```

| Element | Shows |
|---|---|
| focal | the project's name, `AidWalk` |
| next | the current milestone (the first open top-level todo of the project) and its first open subtask as the next step |
| details | one square per top-level todo of the project: filled when done, half for the current one |
| open | every milestone with "done" or "now", the current one's steps, and keys to tick them as in the todo column |

- Milestones are the project's top-level todos in file order; steps are
  their subtasks. Nothing new in the format.
- **Which project is current** is open question 2.

### Rejected for now

- **A git repository** (name, last commit, a `PLAN.md` checklist): fits
  only projects kept in a repository with that layout. It can be added
  later as a detail line ("last commit 2 h ago") once a project names its
  repository.

## Today

### Proposal: the day's events from the KDE calendar

KDE keeps calendars in Akonadi, the service behind KOrganizer and the
clock's agenda; it already syncs Google calendars. topdo reads today's
events from it, read-only, through the `akonadi-calendar` library (KDE
PIM). Events from every calendar the user has enabled appear.

| Element | Shows |
|---|---|
| focal | the time of the next event, `15:00` |
| next | its title and a countdown, `in 28 min` |
| details | the day as a strip from 08:00 to 20:00, events as bars, a circle at now |
| open | one row per event, past ones faded, the current one in the accent colour, a line at now; todos due today listed under the events |

- The badge gains the next event once this works (see widget.md).
- topdo never writes to the calendar.
- The calendar code lives in a new layer beside `store`: `src/calendar/`,
  used only by the QML plugin. `core` stays free of it; the CLI and MCP
  server don't link it.

### Options

| Source | For | Against |
|---|---|---|
| Akonadi (proposed) | every calendar KOrganizer shows, Google included, kept in sync by KDE | needs the `akonadi-calendar` and `kcalendarcore` development packages to build; Akonadi must run |
| `.ics` files on disk | simple; `kcalendarcore` alone | Google needs a separate sync tool to write the files |
| todo.txt only (todos due today) | no new dependency | not a schedule: no times, no meetings |

## Open questions

1. **Today's source:** Akonadi (proposed), `.ics` files, or todo.txt only?
   Akonadi needs two development packages installed
   (`akonadi-calendar-devel`, `kf6-kcalendarcore-devel` or the
   distribution's names for them).
2. **The current project:** set in `topdo.conf` (`project=AidWalk`), the
   project of the badge's next todo, or chosen in the panel with keys
   (the last choice remembered)?
3. **Milestone squares:** all top-level todos of the project, or only
   those with subtasks (a todo without steps is then just a todo)?
