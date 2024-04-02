cmake_minimum_required(VERSION 3.11)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
## use the hint from above to find where 'zmq.hpp' is located
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from above to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

protobuf_generate_cpp(TRANSPORT_CATALOGUE_PROTO_SRCS TRANSPORT_CATALOGUE_PROTO_HDRS
        proto/graph.proto
        proto/map_renderer.proto
        proto/svg.proto
        proto/transport_router.proto
        proto/transport_catalogue.proto)

set(JSON_FILES json.cpp
        json.h
        json_reader.cpp
        json_reader.h
        json_builder.cpp
        json_builder.h)

set(ROUTER_FILES graph.h
        ranges.h
        transport_router.cpp
        transport_router.h
        router.h
        proto/graph.proto
        proto/transport_router.proto)

set(RENDER_FILES svg.cpp
        svg.h
        proto/svg.proto
        map_renderer.cpp
        map_renderer.h
        proto/map_renderer.proto)

set(TRANSPORT_CATALOGUE_FILES geo.h
        main.cpp
        transport_catalogue.cpp
        transport_catalogue.h
        domain.h
        request_handler.cpp
        request_handler.h
        proto/transport_catalogue.proto)

set(SERIALIZATION_FILES serialization.cpp
        serialization.h)

add_executable(transport_catalogue
        ${TRANSPORT_CATALOGUE_PROTO_SRCS} ${TRANSPORT_CATALOGUE_PROTO_HDRS}
        ${TRANSPORT_CATALOGUE_FILES}
        ${JSON_FILES}
        ${ROUTER_FILES}
        ${RENDER_FILES}
        ${SERIALIZATION_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(transport_catalogue PUBLIC ${ZeroMQ_INCLUDE_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue ${ZeroMQ_LIBRARY} "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
