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
sudo zypper install cmark-gfm-devel   # optional: the oracle for parser tests
```

| Package | Gives |
|---|---|
| `kf6-extra-cmake-modules` | KDE's CMake helpers |
| `qt6-base-devel`, `qt6-test-devel` | Qt Core and QTest |
| `qt6-declarative-devel`, `qt6-declarative-tools` | QML, plus `qmlformat` and `qmllint` |
| `libplasma6-devel`, `kf6-kirigami-devel` | The Plasma widget API and Kirigami |
| `plasma6-sdk` | `plasmoidviewer`, to run the widget in a window |
| `clang` | `clang-format` and `clang-tidy` |
| `cmark-gfm-devel` | The reference Markdown parser for differential tests |

## Build and test

```sh
cmake --preset dev          # configure into build/dev
cmake --build --preset dev  # build
ctest --preset dev          # run the tests
```

The `dev` preset is a debug build that treats compiler warnings as errors.
The `release` preset builds an optimised version into `build/release`.

## Format and lint

```sh
clang-format -i <changed .cpp/.h files>
clang-tidy -p build/dev <changed .cpp files>
qmlformat -i <changed .qml files>
qmllint <changed .qml files>
```
