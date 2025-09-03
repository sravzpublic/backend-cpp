set(TARGET_NAME unit_tests)

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)
set(INCLUDE_DIR ../../include)
set(WEB_INCLUDES ../../web/include)
set(INCLUDE_MODELS_DIR ../../include/models)
set(INCLUDE_SERVICES_DIR ../../include/services)
set(INCLUDE_CONTROLLERS_DIR ../../include/controllers)
set(INCLUDE_ROUTER_DIR ../../include/router)

file(GLOB SRC_FILES ../../src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main.cpp$")
file(GLOB WEB_SOURCES ../../src/web/*.cpp)
file(GLOB SERVICES_SOURCES ../../src/services/*.cpp)
file(GLOB ROUTER_SOURCES ../../src/router/*.cpp)
file(GLOB CONTROLLER_SOURCES ../../src/controllers/*.cpp)

add_executable(
  ${TARGET_NAME}
  ${TEST_SOURCES}
  ${SRC_FILES}
  ${WEB_SOURCES}
  ${SERVICES_SOURCES}
  ${ROUTER_SOURCES}
  ${CONTROLLER_SOURCES}
)

find_package(ZLIB REQUIRED)

# Sets up Boost_INCLUDE_DIRS and Boost_LIBRARIES variables
# Boost header files are at /usr/local/include/boost/
find_package(Boost REQUIRED COMPONENTS
  thread
  program_options
  json
  url
  log
  filesystem
  system
  date_time
  regex
  log_setup
  context
  coroutine
)

target_link_library_redis_plus_plus_hiredis(${TARGET_NAME})
find_library(DUCKDB_LIB duckdb HINTS /usr/local/lib)
# Link libraries
target_link_libraries(${TARGET_NAME}
  PRIVATE
  pthread
  crypto
  ssl
  mongocxx
  bsoncxx
  aws-cpp-sdk-core
  aws-cpp-sdk-s3
  ${Boost_LIBRARIES}
  ZLIB::ZLIB
  gtest_main
  gmock_main
  ${DUCKDB_LIB} 
)

target_include_directories(${TARGET_NAME}
  PRIVATE
  ${INCLUDE_DIR}
  ${WEB_INCLUDES}
  ${INCLUDE_SERVICES_DIR}
  ${INCLUDE_CONTROLLERS_DIR}
  ${INCLUDE_ROUTER_DIR}
  ${INCLUDE_MODELS_DIR}
  /usr/local/include/mongocxx/v_noabi
  /usr/local/include/bsoncxx/v_noabi
  /usr/local/include/aws/include
  /usr/src/duckdb/src/include
  /usr/src/cpp-jwt/include/
)

# automatic discovery of unit tests
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME}
  PROPERTIES
  LABELS "unit"
  DISCOVERY_TIMEOUT # how long to wait (in seconds) before crashing
  240
)
