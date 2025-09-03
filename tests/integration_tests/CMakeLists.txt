file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_executable(
    integration_tests
    ${TEST_SOURCES}
    ../../src/utils.cpp
    )

find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED COMPONENTS log filesystem context json)

target_link_library_redis_plus_plus_hiredis(integration_tests)
find_library(DUCKDB_LIB duckdb HINTS /usr/local/lib)
target_link_libraries(integration_tests
  PRIVATE
    services
    controllers
    gtest_main
    gmock_main
    aws-cpp-sdk-core 
    aws-cpp-sdk-s3 
    pthread
    crypto 
    ssl    
    duckdb
    ${Boost_LIBRARIES}
    ZLIB::ZLIB
    ${DUCKDB_LIB}  
  )


target_include_directories(integration_tests 
  PRIVATE 
  ../../include    
  ../../include/services
  ../../include/controllers  
  ../../include/models  
  /usr/local/include/aws/include
  /usr/src/duckdb/src/include
  /usr/src/cpp-jwt/include/
  /usr/local/lib/
  ${Boost_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
)

# automatic discovery of unit tests
include(GoogleTest)
gtest_discover_tests(integration_tests
  PROPERTIES
    LABELS "integration_tests"
  DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
    240
  )

