# traffic-blaster



## Description

Traffic-Blaster is an efficient C++ application for stress testing a REST server. It opens N connections to a host:port and uses a threadpool to push a high volume of requests on the opened connections. This simulates a very high volume of traffic.

## Example

```bash

# Example 1: dry run
$ ./traffic_blaster --host server001.company.com --port 80 --endpoint /endpoint?qs=10 --request request.json --connections 2 --threads 2 --volume 5 --delay 100
Settings:
Host: server001.company.com
Port: 80
Endpoint: /endpoint?qs=10
Request file: request.json
Volume: 5 requests sent on each connection
Response Wait Timeout: 5 seconds
Connections: 2 total connections to be opened
Threads: 2 total threads pushing to all available connections
Delay between sending requests: 100(ms) 
Logging enabled: 1. If true, prints additional information to console
Start sending ...
[thread:140334287365888][conn:0] message sent ok
[thread:140334295758592][conn:0] Connected to the server at a010.casalemedia.com:80, 
[thread:140334295758592][conn:1] Connected to the server at a010.casalemedia.com:80, 
[thread:140334287365888][conn:0] Response received: HTTP/1.1 204 No Content
[thread:140334287365888][conn:0] message sent ok
[thread:140334295758592][conn:0] Response received: HTTP/1.1 204 No Content
[thread:140334287365888][conn:0] message sent ok
[thread:140334295758592][conn:0] Response received: HTTP/1.1 204 No Content
[thread:140334287365888][conn:0] message sent ok
[thread:140334295758592][conn:0] Response received: HTTP/1.1 204 No Content
[thread:140334295758592][conn:0] message sent ok
[thread:140334287365888][conn:0] Response received: HTTP/1.1 204 No Content
[thread:140334287365888][conn:1] message sent ok
[thread:140334295758592][conn:1] Response received: HTTP/1.1 204 No Content
[thread:140334295758592][conn:1] message sent ok
[thread:140334287365888][conn:1] Response received: HTTP/1.1 204 No Content
[thread:140334295758592][conn:1] message sent ok
[thread:140334287365888][conn:1] Response received: HTTP/1.1 204 No Content
[thread:140334287365888][conn:1] message sent ok
[thread:140334295758592][conn:1] Response received: HTTP/1.1 204 No Content
[thread:140334295758592][conn:1] message sent ok
[thread:140334287365888][conn:1] Response received: HTTP/1.1 204 No Content
[thread:140334310639424][conn:0] closing the socket
[thread:140334310639424][conn:1] closing the socket
Summary:
Total Requests Sent: 10
Total Responses Received: 10

# Example 2: stress test
./traffic_blaster --host 127.0.0.1 --port 80 --endpoint /endpoint?s=1 --request request.json --connections 20 --threads 3 --volume 5000 --delay 1 --logging 0
Settings:
Host: 127.0.0.1
Port: 4063
Endpoint: /endpoint?s=1
Request file: request.json
Volume: 5000 requests sent on each connection
Response Wait Timeout: 5 seconds
Connections: 20 total connections to be opened
Threads: 3 total threads pushing to all available connections
Delay between sending requests: 1(ms) 
Logging enabled: 0. If true, prints additional information to console
Start sending ...
Summary:
Total Requests Sent: 100000
Total Responses Received: 99885
```

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


