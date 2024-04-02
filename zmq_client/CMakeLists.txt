cmake_minimum_required(VERSION 3.11)
project(zmq_client)

set(CMAKE_CXX_STANDARD 17)

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

add_executable(zmq_client main.cpp)
target_include_directories(zmq_client PUBLIC ${ZeroMQ_INCLUDE_DIR})

target_link_libraries(zmq_client ${ZeroMQ_LIBRARY})