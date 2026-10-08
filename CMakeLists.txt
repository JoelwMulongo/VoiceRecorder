cmake_minimum_required(VERSION 3.10)
project(VoiceRecorder CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "VoiceRecorder uses the Windows WinMM API and only builds on Windows.")
endif()

find_package(Threads REQUIRED)

# Windows app with Record / Stop buttons (the one to use day to day)
add_executable(VoiceRecorderApp WIN32
    src/gui_main.cpp
    src/VoiceRecorder.cpp
)
target_link_libraries(VoiceRecorderApp PRIVATE winmm shell32 Threads::Threads)

# Simple 5-second command-line test
add_executable(VoiceRecorder
    src/main.cpp
    src/VoiceRecorder.cpp
)
target_link_libraries(VoiceRecorder PRIVATE winmm Threads::Threads)

# With MSVC, link the runtime statically (no Visual C++ Redistributable needed)
if(MSVC)
    target_compile_options(VoiceRecorderApp PRIVATE $<$<CONFIG:Release>:/MT>)
    target_compile_options(VoiceRecorder PRIVATE $<$<CONFIG:Release>:/MT>)
endif()

# With MinGW, link the runtime statically so the .exe runs on any PC with no extra DLLs
if(MINGW)
    target_link_options(VoiceRecorderApp PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_options(VoiceRecorder PRIVATE -static -static-libgcc -static-libstdc++)
endif()