FROM ubuntu:22.04


RUN apt update && \
    apt install -y \
    libzmq3-dev \
    protobuf-compiler
WORKDIR /app
COPY transport-catalogue/cmake-build-debug/transport_catalogue .
COPY transport_catalogue.db .

ENTRYPOINT ["./transport_catalogue", "zmq"]