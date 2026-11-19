# User manual

topdo has no features yet; this manual covers building and testing. It
grows with each feature.

## Install the build dependencies

On openSUSE Tumbleweed:

```sh
sudo zypper install cmake ninja gcc-c++ clang \
    kf6-extra-cmake-modules qt6-base-devel qt6-test-devel \
    qt6-declarative-devel qt6-declarative-tools \
    libplasma6-devel kf6-kirigami-devel plasma6-sdk
```

| Package | Gives |
|---|---|
| `kf6-extra-cmake-modules` | KDE's CMake helpers |
| `qt6-base-devel`, `qt6-test-devel` | Qt Core and QTest |
| `qt6-declarative-devel`, `qt6-declarative-tools` | QML, plus `qmlformat` and `qmllint` |
| `libplasma6-devel`, `kf6-kirigami-devel` | The Plasma widget API and Kirigami |
| `plasma6-sdk` | `plasmoidviewer`, to run the widget in a window |
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
not on `PATH`; call them with that prefix or add it to `PATH`.
