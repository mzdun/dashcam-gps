# Dashcam GPS Viewer

[![Coverage Status](https://coveralls.io/repos/github/mzdun/dashcam-gps/badge.svg?branch=master)](https://coveralls.io/github/mzdun/dashcam-gps?branch=master)

## Goals

Three goals for the project:

1. Break a list of camera clips and GPS data into trips library. A trip consists of a lists of clips and a series of GPS paths. The camera footage clips either touch each other back-to-back, or there is some small, configurable gap between a series of clips inside a single trip.
2. Extract GPS data from camera footage. Since the camera I got is the 70mai pro, this is where I will start from.
3. Provide a GUI application able to present the footage and the GPS data.

## Structure

### Libraries

#### mGPS

The `src\mgps` (movies+GPS) library defines all the types an extractor might use to build a library and hand it over to any application.

#### mGPS (70mai)

The `extras\mgps-70mai` reads the MP4 files produced by 70mai dash cam and produces a library out of a directory containing said MP4 files.

### Applications

#### HTML+SVG

The `apps\html+svg` is an example taking a library (provided by the `mgps-70mai`) and printing out a HTML sheet of all the trips. The "+svg" part of the name comes from embedded `<svg>` images built from GPS data.

#### dashboard-gps-player (Qt5)

The `apps\qt5-player` aims at presenting the GPS data over a map and animating a marker in sync with the video.

#### dashboard-gps-player (UWP)

The `apps\uwp-player` aims to be more integrated into Windows 10, than any Qt app could.

_This app is only planned and scheduled **after** Qt5 application is ready-ish._

#### dashboard-gps-player (Android)

The `apps\android-player` aims at presenting the clips using a DCIM directory directly, without downloading the clips to the PC first.

_This app is only planned and scheduled **after** Qt5 application is ready-ish._

## Dependencies

#### Qt5
If you want to compile qt5-player

#### Conan Package Manager
For external libraries. Currently, the `extras-70mai` depends on [compile time regular expressions](https://github.com/hanickadot/compile-time-regular-expressions) by Hana Dusíková and the `libmgps` (and by extensions &mdash; everything else) depends on [date](https://github.com/HowardHinnant/date) by Howard Hinnant.

## Building

The CMake config uses few options to let you decide, what do you want to build or not. All of them are `OFF` by default and you need to set them to `ON` as needed.
- `MGPS_BUILD_TOOLS` &mdash; controls, whether or not `apps\html+svg` is built.
- `MGPS_BUILD_QT5` &mdash; turns on the `apps\qt5-player`. This one also needs `Qt5_DIR` environment variable pointing to the Qt5 cmake dir (`$QT_HOME/5.$VER/$COMPILER/lib/cmake`)
- `MGPS_BUILD_70MAI` &mdash; turns on the `extras\mgps-70mai`.

With Conan, the build script would be

```sh
mkdir build && cd build
conan install .. --build missing
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DMGPS_BUILD_70MAI=ON \
    -DMGPS_BUILD_QT5=ON \
    -DMGPS_BUILD_TOOLS=ON \
    ..
```

## Future work

### Testing 

With Conan, setting up debug builds and run tests/collect coverage from them should be easy. Priority is on the libraries (`src\mgps`, `extras\mgps-70mai`).

### SemVer

Before 1.0, the version might be `0.$MINOR.$PATCH-$STABILITY+dev.$DATE$TIME` for normal builds and `0.$MINOR.$PATCH-$STABILITY+build.$NIGHTLY` if built from tags `v0.$MINOR.$PATCH/$NIGHTLY`. 

- _STABILITY_ will be present only, if the CMake variable is not empty.
- _PATCH_ will idealy be updated before each push, _MINOR_ after a new functionality is finished, _NIGHTLY_ through a cron job (checking if anything was pushed between consecutive runs).
- _MINOR_ would only reset the _PATCH_, but not _NIGHTLY_. For file names, the meta separator will need to be `-`.

Starting with 1.0, the _MAJOR_ will also be present. This means, that before each push an analysis of the change must be done to try and check, if there were any API-breaking updates.

Releases should follow the `X.Y.Z` short form &mdash; there are no stability tag past the `rc.#`, there should be no metadata needed.

The build meta would require a single point of truth, a Jenkins stage checking the time and/or setting the build number.

### Fully-shared libraries

Produce a DLL/SO library from the `libmgps`, with some sensible fallback on JNI/Android.

### Different packages built from single platform build

- Base package with `libmgps` runtime
- `dev`/`devel` package with headers, lib files on Windows, `pkgconfig`/archive files on *nixen
- `70mai` package with plugin runtime (for products of `MGPS_BUILD_70MAI`)
- `tools` package for products of `MGPS_BUILD_TOOLS`
- `qt5` package for the `apps\qt5-player`, possibly with linux/mac/windeployqt libraries already in
- `uwp` package for the `apps\uwp-player`
- Installers for Qt5/UWP payloads

### Signing the artifacts

The APK will have to be signed for sure. There need to be (Jenkins-fueled?) way to sign all the shared libraries, programs, packages, installers?
