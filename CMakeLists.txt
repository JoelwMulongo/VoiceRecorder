cmake_minimum_required(VERSION 3.10)
project(VoiceRecorder)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_executable(VoiceRecorder src/main.cpp src/VoiceRecorder.cpp)
target_link_libraries(VoiceRecorder winmm)
