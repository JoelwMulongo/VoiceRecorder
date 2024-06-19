#include "VoiceRecorder.h"
#include "WaveHeader.h"
#include <windows.h>
#include <iostream>

bool VoiceRecorder::initialize() {
    // Initialize audio input device
    waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD_PTR)waveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
    return true;
}

void VoiceRecorder::startRecording(const std::string& filename) {
    outFile.open(filename, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(&waveHeader), sizeof(WaveHeader));
    waveInStart(hWaveIn);
}

void VoiceRecorder::stopRecording() {
    waveInStop(hWaveIn);
    waveInClose(hWaveIn);
    outFile.seekp(4, std::ios::beg);
    uint32_t fileSize = outFile.tellp();
    outFile.write(reinterpret_cast<const char*>(&fileSize), 4);
    outFile.seekp(40, std::ios::beg);
    uint32_t dataSize = fileSize - 44;
    outFile.write(reinterpret_cast<const char*>(&dataSize), 4);
    outFile.close();
}

void CALLBACK VoiceRecorder::waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (uMsg == WIM_DATA) {
        WAVEHDR* pWaveHdr = (WAVEHDR*)dwParam1;
        VoiceRecorder* recorder = (VoiceRecorder*)dwInstance;
        recorder->outFile.write(pWaveHdr->lpData, pWaveHdr->dwBytesRecorded);
        waveInAddBuffer(hwi, pWaveHdr, sizeof(WAVEHDR));
    }
}

VoiceRecorder::VoiceRecorder() {
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 1;
    waveFormat.nSamplesPerSec = 44100;
    waveFormat.nAvgBytesPerSec = 44100 * 2;
    waveFormat.nBlockAlign = 2;
    waveFormat.wBitsPerSample = 16;
    waveFormat.cbSize = 0;
    waveHeader.chunkID = 0x46464952; // "RIFF"
    waveHeader.format = 0x45564157; // "WAVE"
    waveHeader.subchunk1ID = 0x20746d66; // "fmt "
    waveHeader.subchunk1Size = 16;
    waveHeader.audioFormat = 1;
    waveHeader.numChannels = 1;
    waveHeader.sampleRate = 44100;
    waveHeader.byteRate = 44100 * 2;
    waveHeader.blockAlign = 2;
    waveHeader.bitsPerSample = 16;
    waveHeader.subchunk2ID = 0x61746164; // "data"
    waveHeader.subchunk2Size = 0;
}

VoiceRecorder::~VoiceRecorder() {
    if (hWaveIn != NULL) {
        waveInClose(hWaveIn);
    }
}
