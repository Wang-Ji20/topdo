# User manual

topdo is at an early stage: so far there is the `topdo` command. This
manual grows with each feature.

## Install the build dependencies

On openSUSE Tumbleweed:

```sh
sudo zypper install cmake ninja gcc-c++ clang \
    kf6-extra-cmake-modules qt6-base-devel qt6-test-devel \
    qt6-declarative-devel qt6-declarative-tools \
    libplasma6-devel kf6-kirigami-devel plasma6-sdk \
    google-poppins-fonts jetbrains-mono-fonts
```

| Package | Gives |
|---|---|
| `kf6-extra-cmake-modules` | KDE's CMake helpers |
| `qt6-base-devel`, `qt6-test-devel` | Qt Core and QTest |
| `qt6-declarative-devel`, `qt6-declarative-tools` | QML, plus `qmlformat` and `qmllint` |
| `libplasma6-devel`, `kf6-kirigami-devel` | The Plasma widget API and Kirigami |
| `plasma6-sdk` | `plasmoidviewer`, to run the widget in a window |
| `google-poppins-fonts`, `jetbrains-mono-fonts` | The widget's typefaces (needed to run it, not to build) |
| `clang` | `clang-format` and `clang-tidy` |

## Build and test

```sh
cmake --preset dev          # configure into build/dev
cmake --build --preset dev  # build
ctest --preset dev          # run the tests
```

The `dev` preset is a debug build that treats compiler warnings as errors.
The `release` preset builds an optimised version into `build/release`.

The property tests check rules on random todo files. Each run picks a new
random seed and prints it:

```
QINFO  : TestDocument::roundTripsRandomFiles() seed 313152176 (replay with TOPDO_SEED=313152176)
```

To replay a failure exactly, run the test again with that seed:

```sh
TOPDO_SEED=313152176 ctest --preset dev
```

## Use the `topdo` command

The build puts the command at `build/dev/src/cli/topdo`;
`cmake --install build/release` installs it into `/usr/local/bin`.

```sh
topdo list                         # open todos, numbered, with the headings
topdo list --all                   # done todos too
topdo add Pay rent due:2026-12-01  # add a todo after the first run of todos
topdo add Water plants --section Home
topdo done 2 5                     # tick todos 2 and 5
topdo undone 2                     # untick todo 2
topdo edit 3 Call the bank today   # replace the text of todo 3
topdo rm 4                         # remove todo 4 with its subtasks and notes
topdo path                         # where the file is: $EDITOR "$(topdo path)"
topdo --dir ~/x list               # use another todo folder for one command
```

A todo's number is its position among all todos in the file, so it stays
the same until todos above it are added or removed. Commands print nothing
when they succeed. When an argument is wrong, for example a number that
isn't a todo, they print an error and change nothing.

The todo folder is `~/Documents/todo` unless `~/.config/topdo/topdo.conf`
says otherwise (`dir=~/somewhere`). The full design of the command is in
[design/cli.md](design/cli.md).

## Editor support

clangd (the C++ language server behind most editors' completion and
diagnostics) reads the compile commands of the `dev` preset, as set in
`.clangd`. Run `cmake --preset dev` once after cloning so that
`build/dev/compile_commands.json` exists; until then clangd can't find the Qt
headers.

## Format and lint

```sh
clang-format -i <changed .cpp/.h files>
clang-tidy -p build/dev <changed .cpp files>
qmlformat -i <changed .qml files>
qmllint <changed .qml files>
```

`qmlformat` and `qmllint` are installed in `/usr/lib64/qt6/bin/`, which is
not on `PATH`; call them with that prefix or add it to `PATH`. Give
`qmllint` the built QML module with `-I build/dev/qml`.

## Try the widget

```sh
QML_IMPORT_PATH=$PWD/build/dev/qml plasmoidviewer -c org.kde.panel -a $PWD/plasmoid/package
```

`plasmoidviewer` shows the panel but not the badge in the top bar, so it
can't show where the panel opens.
