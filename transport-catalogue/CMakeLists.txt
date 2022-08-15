cmake_minimum_required(VERSION 3.11)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(TRANSPORT_CATALOGUE_PROTO_SRCS TRANSPORT_CATALOGUE_PROTO_HDRS transport_catalogue.proto)

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
        router.h)

set(RENDER_FILES svg.cpp
        svg.h
        map_renderer.cpp
        map_renderer.h)

set(TRANSPORT_CATALOGUE_FILES geo.h
        main.cpp
        transport_catalogue.cpp
        transport_catalogue.h
        domain.h
        request_handler.cpp
        request_handler.h
        transport_catalogue.proto)

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

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
