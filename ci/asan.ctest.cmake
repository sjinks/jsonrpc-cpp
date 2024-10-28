include("${CMAKE_CURRENT_LIST_DIR}/common.cmake")

set(CTEST_CONFIGURATION_TYPE "ASAN")
set(CTEST_MEMORYCHECK_TYPE "AddressSanitizer")

ctest_start(Experimental)
set(options -DCMAKE_CXX_COMPILER=clang++)
ctest_configure(OPTIONS "${options}")
ctest_build()
ctest_memcheck()
