cmake_minimum_required(VERSION 3.5)

project (Lachter)
# add_subdirectory("rapidcheck")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-O3")

add_library(lachter lachter.cc)
add_library(mcts mcts.cc)
add_library(minimax minimax.cc)

add_executable(test_lachter test_lachter.cc)
target_link_libraries(test_lachter lachter)
#target_compile_options(test_lachter PRIVATE "-Dlest_FEATURE_AUTO_REGISTER=1")

add_executable(test_randomgames test_randomgames.cc)
target_link_libraries(test_randomgames lachter)

add_executable(test_mcts test_mcts.cc)
target_link_libraries(test_mcts lachter mcts)

add_executable(test_minimax test_minimax.cc)
target_link_libraries(test_minimax lachter minimax)

add_executable(play_dwarf_mcts play_dwarf_mcts.cc)
target_link_libraries(play_dwarf_mcts lachter mcts)

add_executable(selfplay_mcts selfplay_mcts.cc)
target_link_libraries(selfplay_mcts lachter mcts)

enable_testing()
add_test(NAME test_lachter COMMAND test_lachter)
add_test(NAME test_mcts COMMAND test_mcts)
add_test(NAME test_minimax COMMAND test_minimax)
