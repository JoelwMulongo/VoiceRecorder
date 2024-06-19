#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <fstream>
#include <string>

class VoiceRecorder {
public:
    VoiceRecorder();
    ~VoiceRecorder();

    void startRecording(const std::string& filename);
    void stopRecording();

private:
    HWAVEIN hWaveIn;
    WAVEHDR waveHeader;
    std::ofstream outFile;
    std::string filename;

    static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
};
