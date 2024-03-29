cmake_minimum_required(VERSION 3.16)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(database)
file(GLOB database_SRC "database/*.h" "database/*.c")
add_library(database STATIC ${database_SRC})

file(GLOB test_SRC "test/*.h" "test/*.c")
add_executable(test ${test_SRC})
target_compile_options(database PRIVATE
  -Winline -Wunused -Wall -Wextra -Wshadow -Wcast-align -Wpedantic -Werror
)
target_link_libraries(test dl uuid database)

add_custom_target(run_test ALL DEPENDS test COMMAND test)