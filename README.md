# cpp-transport-catalogue
## Second large project in Yandex Practicum

This project allowed us to work out the basic class implementations of the c++ language. 

Example
create transport_catalogue.db
```bash
./transport_catalogue make_base < make_base.json 
```
use example
```bash
./transport_catalogue process_requests < process_request.json 
```
generate for python
```bash
protoc -I=cpp-transport-catalogue/transport-catalogue/proto  --python_out=py-proto-tc/ cpp-transport-catalogue/transport-catalogue/proto/*.proto

```
## zmq server
install 
```bash
sudo apt install libzmq3-dev
```
and fix Cmake
```Cmake
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from above to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )
```
RUN
```bash
./transport_catalogue zmq
```
Client part in zmq_client folder
