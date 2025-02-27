include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_executable (autocorr-benchmark benchmark.c)
target_link_libraries (autocorr-benchmark autocorr)
