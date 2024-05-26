# cpp-transport-catalogue
Second large project in Yandex Practicum

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
zmq server
```bash
./transport_catalogue zmq
```
Client part in zmq_client
