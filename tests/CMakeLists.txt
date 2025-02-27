find_package (CUnit REQUIRED)

include_directories (${CUNIT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_executable (autocorr-tests tests.c)
target_link_libraries (autocorr-tests autocorr ${CUNIT_LIBRARY})
