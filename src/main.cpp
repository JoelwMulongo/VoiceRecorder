#include "VoiceRecorder.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    VoiceRecorder recorder;

    if (!recorder.initialize()) {
        std::cerr << "Failed to initialize audio input\n";
        return 1;
    }
    if (!recorder.startRecording("output.wav")) {
        std::cerr << "Failed to start recording\n";
        return 1;
    }

    std::cout << "Recording for 5 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    recorder.stopRecording();

    std::cout << "Saved output.wav\n";
    return 0;
}