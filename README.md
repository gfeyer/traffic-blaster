# traffic-blaster



## Description

Traffic-Blaster is an efficient C++ application for stress testing a REST server. It opens N connections to a host:port and uses a threadpool to push a high volume of requests on the opened connections. This simulates a very high volume of traffic.

## Build from source


#### Install Dependencies


```bash
# get the package manager
cd /opt
git clone git@github.com:microsoft/vcpkg.git
cd vcpkg
./bootstrap.sh # linux, macos

# Json libs
vcpkg install boost-asio
vcpkg install boost-program-options

```

#### Compile

```bash
mkdir build
cd build
cmake ../ "-DCMAKE_BUILD_TYPE=Release" "-DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --parallel 2
```


