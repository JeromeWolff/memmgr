cmake_minimum_required(VERSION 3.30)
project(memmgr C)

set(CMAKE_C_STANDARD 23)

set(MEMMGR_VERSION_MAJOR 1)
set(MEMMGR_VERSION_MINOR 0)

set(SOURCE_DIRECTORY src)
set(INCLUDE_DIRECTORY include)
set(TEST_DIRECTORY test)

# Collect source files
file(GLOB_RECURSE SOURCES "${SOURCE_DIRECTORY}/*.c")

# Create library
add_library(memmgr_static STATIC ${SOURCES})
add_library(memmgr_shared include ${SOURCES})

# Set include directories
target_include_directories(memmgr_static PUBLIC ${INCLUDE_DIRECTORY})
target_include_directories(memmgr_shared PUBLIC ${INCLUDE_DIRECTORY})

# Set output names
set_target_properties(memmgr_static PROPERTIES OUTPUT_NAME "memmgr")
set_target_properties(memmgr_shared PROPERTIES OUTPUT_NAME "memmgr")

# Add versioning for the include library
set_target_properties(memmgr_shared PROPERTIES
        VERSION "${MEMMGR_VERSION_MAJOR}.${MEMMGR_VERSION_MINOR}"
        SOVERSION "${MEMMGR_VERSION_MAJOR}"
)

# Enable testing
enable_testing()

# Add tests
add_executable(memmgr_test ${TEST_DIRECTORY}/test_memmgr.c)
target_link_libraries(memmgr_test PRIVATE memmgr_static)

add_test(NAME MemMGRTest COMMAND memmgr_test)

# Installation
install(TARGETS memmgr_static memmgr_shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
