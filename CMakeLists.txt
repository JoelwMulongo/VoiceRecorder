cmake_minimum_required(VERSION 3.10)
project(VoiceRecorder CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "VoiceRecorder uses the Windows WinMM API and only builds on Windows.")
endif()

find_package(Threads REQUIRED)

add_executable(VoiceRecorder
    src/main.cpp
    src/VoiceRecorder.cpp
)

target_link_libraries(VoiceRecorder PRIVATE winmm Threads::Threads)