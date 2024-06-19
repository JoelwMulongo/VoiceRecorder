#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <fstream>
#include <string>

class VoiceRecorder {
public:
    VoiceRecorder();
    ~VoiceRecorder();
    bool initialize();
    void startRecording(const std::string& filename);
    void stopRecording();

private:
    HWAVEIN hWaveIn;
    WAVEFORMATEX waveFormat;
    std::ofstream outFile;
    static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    WaveHeader waveHeader;
};
