cmake_minimum_required (VERSION 3.30)

project (autocorr)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory (src)
add_subdirectory (tests)
add_subdirectory (benchmark)
