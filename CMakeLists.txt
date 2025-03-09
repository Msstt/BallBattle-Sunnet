cmake_minimum_required (VERSION 3.10)

project (sunnet)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

aux_source_directory(./src DIR_SRCS)
add_executable(sunnet ${DIR_SRCS})
target_link_libraries(sunnet epoll-shim)