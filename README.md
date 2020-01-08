# Dashcam GPS Viewer

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

The `examples\html+svg` is an example taking a library (provided by the `mgps-70mai`) and printing out a HTML sheet of all the trips. The "+svg" part of the name comes from embedded `<svg>` images built from GPS data.

#### dashboard-gps-player (Qt5)

The `src\qt5-player` aims at presenting the GPS data over a map and animating a marker in sync with the video.

#### dashboard-gps-player (UWP)

The `src\uwp-player` aims to be more integrated into Windows 10, than any Qt app could.

_This app is only planned and scheduled **after** Qt5 application is ready-ish._

#### dashboard-gps-player (Android)

The `src\android-player` aims at presenting the clips using a DCIM directory directly, without downloading the clips to the PC first.

_This app is only planned and scheduled **after** Qt5 application is ready-ish._

## Dependencies

The repository depends on Qt5, which needs to be pre-installed. There are also two submodules, [compile time regular expressions](https://github.com/hanickadot/compile-time-regular-expressions) by Hana Dusíková and [date](https://github.com/HowardHinnant/date) by Howard Hinnant. To populate the submodules, they need to be initialized after checkout:

```sh
git submodule update --init
```
