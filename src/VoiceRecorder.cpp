#include "VoiceRecorder.h"

VoiceRecorder::VoiceRecorder() : hWaveIn(nullptr) {}

VoiceRecorder::~VoiceRecorder() {
    if (hWaveIn) {
        waveInClose(hWaveIn);
    }
}

void VoiceRecorder::startRecording(const std::string& filename) {
    this->filename = filename;

    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 1;
    waveFormat.nSamplesPerSec = 44100;
    waveFormat.nAvgBytesPerSec = 44100 * 2;
    waveFormat.nBlockAlign = 2;
    waveFormat.wBitsPerSample = 16;
    waveFormat.cbSize = 0;

    if (waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormat, (DWORD_PTR)waveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        // Handle error
    }

    waveHeader.lpData = new char[44100 * 2 * 10];  // Buffer for 10 seconds
    waveHeader.dwBufferLength = 44100 * 2 * 10;
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = 0;

    waveInPrepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
    waveInAddBuffer(hWaveIn, &waveHeader, sizeof(WAVEHDR));
    waveInStart(hWaveIn);

    outFile.open(filename, std::ios::binary);

    // Write WAV header placeholder
    outFile.write("RIFF----WAVEfmt ", 16);
    uint32_t fmtSize = 16;
    outFile.write(reinterpret_cast<const char*>(&fmtSize), sizeof(fmtSize));
    outFile.write(reinterpret_cast<const char*>(&waveFormat), sizeof(waveFormat));
    outFile.write("data----", 8);
}

void VoiceRecorder::stopRecording() {
    waveInStop(hWaveIn);
    waveInReset(hWaveIn);

    // Update WAV header
    std::streampos fileSize = outFile.tellp();
    outFile.seekp(4, std::ios::beg);
    uint32_t riffSize = static_cast<uint32_t>(fileSize) - 8;
    outFile.write(reinterpret_cast<const char*>(&riffSize), sizeof(riffSize));

    outFile.seekp(40, std::ios::beg);
    uint32_t dataSize = static_cast<uint32_t>(fileSize) - 44;
    outFile.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

    outFile.close();

    waveInUnprepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
    delete[] waveHeader.lpData;

    waveInClose(hWaveIn);
    hWaveIn = nullptr;
}

void CALLBACK VoiceRecorder::waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (uMsg == WIM_DATA) {
        VoiceRecorder* recorder = reinterpret_cast<VoiceRecorder*>(dwInstance);
        recorder->outFile.write(recorder->waveHeader.lpData, recorder->waveHeader.dwBytesRecorded);
        waveInAddBuffer(hwi, reinterpret_cast<LPWAVEHDR>(dwParam1), sizeof(WAVEHDR));
    }
}
