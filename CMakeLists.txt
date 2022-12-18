cmake_minimum_required(VERSION 3.23)
project(LogStorage)

set(CMAKE_CXX_STANDARD 20)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)
add_executable(LogStorage main.cpp DurableLog.h VectorBasedDurableLog.cc VectorBasedDurableLog.h StateMachine.h Counter.cc Counter.h CounterApplication.cc CounterApplication.h CounterEntry.pb.h CounterEntry.pb.cc Sequencer.cc Sequencer.h)
target_link_libraries(LogStorage protobuf)