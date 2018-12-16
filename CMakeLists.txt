cmake_minimum_required(VERSION 3.5)

project (Lachter)
# add_subdirectory("rapidcheck")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-O3")

#add_executable(lachter lachter.cc)
add_library(lachter lachter.cc)

add_executable(test_lachter test_lachter.cc)
#target_compile_options(test_lachter PRIVATE "-Dlest_FEATURE_AUTO_REGISTER=1")

enable_testing()
add_test(NAME test_lachter COMMAND test_lachter)

target_link_libraries(test_lachter lachter)
