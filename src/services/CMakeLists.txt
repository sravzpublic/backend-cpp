set(TARGET_NAME services)
add_library(${TARGET_NAME})


file(GLOB SERVICES_SOURCES ./*.cpp)
file(GLOB UTILS_SOURCE ../utils.cpp)

find_package(Boost REQUIRED COMPONENTS 
  coroutine
)

find_package(ZLIB REQUIRED)
# find_package(DuckDB CONFIG REQUIRED)

# Link libraries
# find_library(DUCKDB_LIB libduckdb HINTS /usr/local/lib)  
# target_link_libraries(${TARGET_NAME} 
#     PRIVATE
#     pthread 
#     crypto 
#     ssl 
#     mongocxx 
#     bsoncxx 
#     aws-cpp-sdk-core 
#     aws-cpp-sdk-s3
#     ${Boost_LIBRARIES}
#     ZLIB::ZLIB
#     ${DUCKDB_LIB} 
# )

target_sources(${TARGET_NAME} 
  PRIVATE
  ${SERVICES_SOURCES}
  ${UTILS_SOURCE}
  )

target_include_directories(${TARGET_NAME} 
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ../../include/
    ../../include/services
    /usr/local/include/mongocxx/v_noabi
	  /usr/local/include/bsoncxx/v_noabi 
    /usr/local/lib
    /usr/local/include/aws/include  
    /usr/src/duckdb/src/include
    /usr/src/cpp-jwt/include/
  )

# we use this to get code coverage
# flags are only valid with the GNU compiler and on Linux
if(CMAKE_CXX_COMPILER_ID MATCHES GNU AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(services
    PUBLIC
      "--coverage"
    )
  target_link_options(services
    INTERFACE
      "--coverage"
    )
endif()