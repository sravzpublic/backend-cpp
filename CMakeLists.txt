cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0070 NEW)

project(sravz LANGUAGES CXX)

# set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# place binaries and libraries according to GNU standards
include(GNUInstallDirs)
include(cmake/functions.cmake)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

# Define Macro DBOOST_LOG_DYN_LINK which links Boost.Log library dynamically
ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

execute_process(
    # Update ldconfig cache to include the shared libraries found in the /usr/local/lib directory.
    COMMAND ldconfig /usr/local/lib
    RESULT_VARIABLE LD_CONFIG_RESULT
)

if (NOT LD_CONFIG_RESULT EQUAL "0")
    message(FATAL_ERROR "ldconfig failed with error code ${LD_CONFIG_RESULT}")
endif()

# we cannot analyse results without gcov
find_program(GCOV_PATH gcov)
if(NOT GCOV_PATH)
  message(FATAL_ERROR "Code coverage analysis requires gcov!")
endif()

# Enable CTest, the testing tool provided with CMake
enable_testing()

add_subdirectory(src)
add_subdirectory(src/controllers)
add_subdirectory(src/services)
add_subdirectory(tests)
