# cmake_clean_cache.cmake
set(BUILD_DIR "${CMAKE_BINARY_DIR}")
file(REMOVE_RECURSE "${BUILD_DIR}/CMakeCache.txt" "${BUILD_DIR}/CMakeFiles")