include("${CMAKE_CURRENT_LIST_DIR}/common.cmake")

set(CTEST_CONFIGURATION_TYPE "Debug")

find_program(CTEST_MEMORYCHECK_COMMAND valgrind)
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--error-exitcode=1 -q --track-origins=yes --leak-check=yes --show-reachable=yes --num-callers=50")
set(CTEST_MEMORYCHECK_TYPE "Valgrind")

ctest_start(Experimental)
if(CTEST_MEMORYCHECK_COMMAND)
    set(options -DCMAKE_CXX_COMPILER=clang++)
    ctest_configure(OPTIONS "${options}")
    ctest_build()
    ctest_memcheck()
else()
    message(WARNING "valgrind command not found, skipping check")
endif()
