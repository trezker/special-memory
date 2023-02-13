cmake_minimum_required(VERSION 3.16)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(database)
file(GLOB database_SRC "database/*.h" "database/*.c")
add_library(database STATIC ${database_SRC})

add_executable(test "test/main.c")
target_link_libraries(test database)
