include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_executable (autocorr-example main.c)
target_link_libraries (autocorr-example autocorr)
