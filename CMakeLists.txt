cmake_minimum_required(VERSION 3.11)
project(reactor)
set(CXX_STANDARD_REQUIRED 17)
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(server server.cpp ${SRC_LIST})
target_link_libraries(server PUBLIC Threads::Threads)

add_executable(client client.cpp ${SRC_LIST})
target_link_libraries(client PUBLIC Threads::Threads)
