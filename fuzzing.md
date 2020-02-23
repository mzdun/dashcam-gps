# Notes on fuzzing

Needs [AFL fuzzer](https://github.com/google/AFL) built and installed.
The fuzzing is performed on binaries compiled with AFL wrappers:

```sh
mkdir build/fuzz && cd build/fuzz
conan install ../.. --build missing -s build_type=Release
CC=afl-gcc CXX=afl-g++ cmake ../.. -G Ninja -DCMAKE_BUILD_TYPE=Release -DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_FUZZ=ON
ninja
```

## Universal preparation

```sh
sudo su
echo core >/proc/sys/kernel/core_pattern
cd /sys/devices/system/cpu
echo performance | tee cpu*/cpufreq/scaling_governor
exit
```


## fuzzer-70mai-boxes

Fuzzer over the box parser. Initial data is the `small_movie.mp4` from [testcases/multimedia/h264](https://github.com/google/AFL/tree/master/testcases/multimedia/h264) with GPS box added to it.

```sh
afl-fuzz -i ../../extras/mgps-70mai/fuzzer/boxes-in -o ./boxes-out -- ./bin/fuzzer-70mai-boxes @@
```

- Infinite loop at the end of broken MPEG. Now discovered inside `boxes::has_box`.
- If `moov` or `mvhd` boxes are missing, the clip is interpreted as if it has 0ms, but otherwise it's valid. Now no-`mvhd` falls back to `UNKNOWN_DURATION_32`/`UNKNOWN_DURATION_64` behavior, in addition to detecting non-positive and "unknown duration" clips and treating them as invalid.
- The `mgps::track::NESW` has only four enums, but the code reading the data synthesizes a fifth one. Now, the GPS points as read from the video clip, are validated based on:
	- having only directions from the set of four;
	- as previously, vertical directions are limited to 90&deg; and horizontal &mdash; to 180&deg;;
	- the "serial number" or "the second, at which the measurements were taken" is now compared against an expected value
