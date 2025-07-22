set(TARGET_NAME backend-cpp)

set(INCLUDE_DIR ../include)
set(WEB_INCLUDES ../web/include)
set(INCLUDE_SERVICES_DIR ../include/services)
set(INCLUDE_MODELS_DIR ../include/models)
set(INCLUDE_CONTROLLERS_DIR ../include/controllers)
set(INCLUDE_ROUTER_DIR ../include/router)

include_directories(
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

file(GLOB SRC_FILES ./*.cpp) 
# list(FILTER SRC_FILES EXCLUDE REGEX "util.cpp$") #TODO: Remove util.cpp
file(GLOB WEB_SOURCES ./web/*.cpp)
file(GLOB SERVICES_SOURCES ./services/*.cpp)
file(GLOB ROUTER_SOURCES ./router/*.cpp)
file(GLOB CONTROLLER_SOURCES ./controllers/*.cpp)

set(SRC_FILES 
    ${SRC_FILES}
    ${WEB_SOURCES}
    ${SERVICES_SOURCES}
    ${ROUTER_SOURCES}
    ${CONTROLLER_SOURCES}
)

add_executable(${TARGET_NAME} ${SRC_FILES} /usr/local/lib/libduckdb.so)

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
    ${DUCKDB_LIB}  
)

# Add library directories
link_directories(
    /usr/local/lib/
)

# List of config files
set(CONFIG_FILES
    app.vagrant.cfg
    app.staging.cfg
    app.production.cfg
)

# file(GLOB CONFIG_FILES app.*.cfg)

# Copy each config file after build
foreach(CONFIG_FILE ${CONFIG_FILES})
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/${CONFIG_FILE}
            $<TARGET_FILE_DIR:${TARGET_NAME}>/${CONFIG_FILE}
    )
endforeach()