cmake_minimum_required(VERSION 3.5)

project (Lachter)
# add_subdirectory("rapidcheck")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-O3")

add_library(thud1 thud1.cc)
add_library(thudbit thudbit.cc)
add_library(mcts mcts.cc)
add_library(minimax minimax.cc)
add_library(rollout2 rollout2.cc)

add_executable(test_thud1 test_thud1.cc)
target_link_libraries(test_thud1 thud1)

add_executable(test_thudbit test_thudbit.cc)
target_link_libraries(test_thudbit thudbit)

add_executable(test_randomgames test_randomgames.cc)
target_link_libraries(test_randomgames thud1)

add_executable(test_mcts test_mcts.cc)
target_link_libraries(test_mcts thud1 mcts)

add_executable(test_minimax test_minimax.cc)
target_link_libraries(test_minimax thud1 minimax)

add_executable(test_rollout2 test_rollout2.cc)
target_link_libraries(test_rollout2 thud1 rollout2)

add_executable(play_dwarf_mcts play_dwarf_mcts.cc)
target_link_libraries(play_dwarf_mcts thud1 mcts)

add_executable(selfplay_mcts selfplay_mcts.cc)
target_link_libraries(selfplay_mcts thud1 mcts)

enable_testing()
add_test(NAME test_thud1 COMMAND test_thud1)
add_test(NAME test_thudbit COMMAND test_thudbit)
add_test(NAME test_mcts COMMAND test_mcts)
add_test(NAME test_rollout2 COMMAND test_rollout2)
add_test(NAME test_minimax COMMAND test_minimax)
