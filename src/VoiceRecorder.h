#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

class VoiceRecorder {
public:
    VoiceRecorder();
    ~VoiceRecorder();

    VoiceRecorder(const VoiceRecorder&) = delete;
    VoiceRecorder& operator=(const VoiceRecorder&) = delete;

    bool initialize();                                  // checks for a mic, creates the event
    bool startRecording(const std::string& filename);  // false on any failure
    void stopRecording();                               // safe to call more than once

private:
    static constexpr int   kBufferCount = 4;
    static constexpr DWORD kSampleRate  = 44100;
    static constexpr DWORD kBufferBytes = kSampleRate * 2 / 5;  // ~200 ms of 16-bit mono

    void captureLoop();                 // runs on the worker thread
    void drainBuffers(bool requeue);    // write finished buffers to disk
    void writeHeader();
    void finalizeHeader();

    HWAVEIN hWaveIn = nullptr;
    HANDLE  hEvent  = nullptr;
    WAVEFORMATEX format{};
    WAVEHDR headers[kBufferCount]{};
    std::vector<char> buffers[kBufferCount];

    std::ofstream outFile;
    std::thread worker;
    std::atomic<bool> running{false};
    uint32_t dataBytes = 0;
};