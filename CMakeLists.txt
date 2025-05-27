cmake_minimum_required(VERSION 3.11)
project(reactor)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
FILE(GLOB_RECURSE SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
# FILE(GLOB_RECURSE DEF_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.def)
# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# add_executable(server server.cpp ${SRC_LIST})
# target_link_libraries(server PUBLIC Threads::Threads)

add_executable(test_mapping test/test_mapping.cpp ${SRC_LIST})
target_link_libraries(test_mapping PUBLIC Threads::Threads)

add_executable(client client.cpp ${SRC_LIST})
target_link_libraries(client PUBLIC Threads::Threads)
