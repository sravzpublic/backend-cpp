add_library(controllers)


file(GLOB CONTROLLER_SOURCES ./*.cpp)

target_sources(controllers
  PRIVATE
  ${CONTROLLER_SOURCES}
  )

target_include_directories(controllers
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ../../include/    
    ../../include/controllers
    ../../include/models
    ../../include/services
    /usr/src/duckdb/src/include
    /usr/src/cpp-jwt/include/
    /usr/local/include/mongocxx/v_noabi
	  /usr/local/include/bsoncxx/v_noabi 
    /usr/local/lib    
  )

# Link libraries
# find_library(DUCKDB_LIB libduckdb HINTS /usr/local/lib)  
# target_link_libraries(controllers
#       ${DUCKDB_LIB} 
# )


# we use this to get code coverage
# flags are only valid with the GNU compiler and on Linux
if(CMAKE_CXX_COMPILER_ID MATCHES GNU AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(controllers
    PUBLIC
      "--coverage"
    )
  target_link_options(controllers
    INTERFACE
      "--coverage"
    )
endif()