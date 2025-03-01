find_package (CUnit)

if (CUnit_FOUND)
  include_directories (${CUNIT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../src)
  add_executable (autocorr-tests tests.c)
  target_link_libraries (autocorr-tests autocorr ${CUNIT_LIBRARY})
endif (CUnit_FOUND)
