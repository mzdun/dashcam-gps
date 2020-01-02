# Dashcam GPS Viewer

## Goals

Three goals for the project:

1. Extract GPS data from camera footage. Since the camera I got is the 70mai pro, this is where I will start from.
2. Using a list of clips and GPS points from first point, build a trip library. This is both trivial and not-so-trivial task:
	1. From one hand, a list of adjoining movie clips constitute one piece of a trip. Any GPS info attached to those clips means they describe this particular piece of trip.
	2. From another, disjointed pieces may be parts of the same trip, or belong to different trips. Currently, the gap I'm allowing for is 10 minutes, but I'll leave this as an argument to trip builder.
3. Having build a library of trips, create an app allowing to present it on the screen.

## Dependencies

The repository depends on Qt5, which needs to be pre-installed. There are also two submodules, [compile time regular expressions](https://github.com/hanickadot/compile-time-regular-expressions) by Hana Dusíková and [date](https://github.com/HowardHinnant/date) by Howard Hinnant. To populate the submodules, they need to be initialized after checkout:

```sh
git submodule update --init
```
