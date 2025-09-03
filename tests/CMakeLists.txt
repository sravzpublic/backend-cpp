# CMakeLists.txt in the tests/ directory
include(FetchContent)
FetchContent_Declare(gtest
  QUIET
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# configure build of googletest and googlemock
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

# Include subdirectories containing tests
add_subdirectory(unit_tests)
add_subdirectory(integration_tests)