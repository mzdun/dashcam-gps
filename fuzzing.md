# Notes on fuzzing

Needs [AFL fuzzer](https://github.com/google/AFL) built and installed.
A new build directory created for fuzzable binaries
Create the config with:

```sh
conan install .. --build missing -s build_type=Debug
CC=afl-gcc CXX=afl-g++ cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_FUZZ=ON
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

Fuzzer over the box parser. Initial data is the `small_movie.mp4` from [testcases/multimedia/h264](https://github.com/google/AFL/tree/master/testcases/multimedia/h264).

```sh
afl-fuzz -i ../dashcam-gps/extras/mgps-70mai/fuzzer/boxes-in -o ./boxes-out -- ./bin/fuzzer-70mai-boxes @@
```
