cmake_minimum_required(VERSION 3.5)

project (Lachter)
# add_subdirectory("rapidcheck")

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_BUILD_TYPE relwithdebinfo)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O3 -funroll-loops")
set(INTERPROCEDURAL_OPTIMIZATION TRUE)

add_subdirectory("extern/libwebsockets")

add_library(thud1 thud1.cc)
add_library(thudbit thudbit.cc)
add_library(mcts mcts.cc)
add_library(minimax minimax.cc)
add_library(alphabeta alphabeta.cc)
add_library(rollout2 rollout2.cc)

find_package(Threads)
add_executable(thurisaz_bot thurisaz_bot.cc)
target_link_libraries(thurisaz_bot thudbit alphabeta websockets
                      ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(thurisaz_bot PRIVATE
                           ${PROJECT_BINARY_DIR}/libwebsockets/
                           ${PROJECT_SOURCE_DIR}/extern/libwebsockets/include/)

add_executable(test_thud1 test_thud1.cc)
target_link_libraries(test_thud1 thud1)

add_executable(test_thudbit test_thudbit.cc)
target_link_libraries(test_thudbit thudbit)

add_executable(test_randomgames test_randomgames.cc)
target_link_libraries(test_randomgames thud1)

add_executable(test_mcts test_mcts.cc)
target_link_libraries(test_mcts thudbit mcts)

add_executable(test_minimax test_minimax.cc)
target_link_libraries(test_minimax thudbit minimax)

add_executable(test_alphabeta test_alphabeta.cc)
target_link_libraries(test_alphabeta thudbit alphabeta)

add_executable(test_rollout2 test_rollout2.cc)
target_link_libraries(test_rollout2 thudbit rollout2)

add_executable(play_dwarf_mcts play_dwarf_mcts.cc)
target_link_libraries(play_dwarf_mcts thudbit mcts)

add_executable(selfplay_mcts selfplay_mcts.cc)
target_link_libraries(selfplay_mcts thudbit mcts)

add_executable(selfplay_rollout2 selfplay_rollout2.cc)
target_link_libraries(selfplay_rollout2 thudbit rollout2)

add_executable(selfplay_alphabeta selfplay_alphabeta.cc)
target_link_libraries(selfplay_alphabeta thudbit alphabeta)

enable_testing()
add_test(NAME test_thud1 COMMAND test_thud1)
add_test(NAME test_thudbit COMMAND test_thudbit)
add_test(NAME test_mcts COMMAND test_mcts)
add_test(NAME test_rollout2 COMMAND test_rollout2)
add_test(NAME test_minimax COMMAND test_minimax)
add_test(NAME test_alphabeta COMMAND test_alphabeta)
