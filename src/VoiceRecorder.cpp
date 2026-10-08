#include "VoiceRecorder.h"

#include <iostream>

VoiceRecorder::VoiceRecorder() {
    format.wFormatTag      = WAVE_FORMAT_PCM;
    format.nChannels       = 1;
    format.nSamplesPerSec  = kSampleRate;
    format.wBitsPerSample  = 16;
    format.nBlockAlign     = format.nChannels * format.wBitsPerSample / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.cbSize          = 0;
}

VoiceRecorder::~VoiceRecorder() {
    stopRecording();
    if (hEvent) CloseHandle(hEvent);
}

bool VoiceRecorder::initialize() {
    if (waveInGetNumDevs() == 0) {
        std::cerr << "No audio input devices found\n";
        return false;
    }
    if (!hEvent) {
        hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);  // auto-reset
    }
    return hEvent != nullptr;
}

bool VoiceRecorder::startRecording(const std::string& filename) {
    if (running || !hEvent) return false;

    outFile.open(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Could not open " << filename << " for writing\n";
        return false;
    }
    dataBytes = 0;
    writeHeader();

    MMRESULT r = waveInOpen(&hWaveIn, WAVE_MAPPER, &format,
                            reinterpret_cast<DWORD_PTR>(hEvent), 0, CALLBACK_EVENT);
    if (r != MMSYSERR_NOERROR) {
        std::cerr << "waveInOpen failed (error " << r << ")\n";
        hWaveIn = nullptr;
        outFile.close();
        return false;
    }

    for (int i = 0; i < kBufferCount; ++i) {
        buffers[i].assign(kBufferBytes, 0);
        headers[i] = WAVEHDR{};                 // zero every field
        headers[i].lpData = buffers[i].data();
        headers[i].dwBufferLength = kBufferBytes;
        waveInPrepareHeader(hWaveIn, &headers[i], sizeof(WAVEHDR));
        waveInAddBuffer(hWaveIn, &headers[i], sizeof(WAVEHDR));
    }

    running = true;
    worker = std::thread(&VoiceRecorder::captureLoop, this);
    waveInStart(hWaveIn);
    return true;
}

void VoiceRecorder::stopRecording() {
    if (!running) return;

    running = false;
    SetEvent(hEvent);                       // wake the worker so it exits
    if (worker.joinable()) worker.join();   // from here on only this thread touches the file

    waveInStop(hWaveIn);
    waveInReset(hWaveIn);                   // returns all buffers, including the partial one
    drainBuffers(false);                    // write the remaining data, don't re-queue

    for (auto& h : headers) {
        waveInUnprepareHeader(hWaveIn, &h, sizeof(WAVEHDR));
    }
    waveInClose(hWaveIn);
    hWaveIn = nullptr;

    finalizeHeader();
    outFile.close();
}

void VoiceRecorder::captureLoop() {
    while (running) {
        WaitForSingleObject(hEvent, 100);
        if (!running) break;
        drainBuffers(true);
    }
}

void VoiceRecorder::drainBuffers(bool requeue) {
    for (auto& h : headers) {
        if (h.dwFlags & WHDR_DONE) {
            outFile.write(h.lpData, h.dwBytesRecorded);
            dataBytes += h.dwBytesRecorded;
            if (requeue) {
                waveInAddBuffer(hWaveIn, &h, sizeof(WAVEHDR));
            }
        }
    }
}

void VoiceRecorder::writeHeader() {
    const uint32_t zero = 0;
    const uint32_t fmtSize = 16;
    outFile.write("RIFF", 4);
    outFile.write(reinterpret_cast<const char*>(&zero), 4);      // RIFF size, patched later
    outFile.write("WAVE", 4);
    outFile.write("fmt ", 4);
    outFile.write(reinterpret_cast<const char*>(&fmtSize), 4);
    outFile.write(reinterpret_cast<const char*>(&format), 16);   // 16 bytes only, NOT the 18-byte struct
    outFile.write("data", 4);
    outFile.write(reinterpret_cast<const char*>(&zero), 4);      // data size, patched later
}

void VoiceRecorder::finalizeHeader() {
    const uint32_t riffSize = 36 + dataBytes;
    outFile.seekp(4, std::ios::beg);
    outFile.write(reinterpret_cast<const char*>(&riffSize), 4);
    outFile.seekp(40, std::ios::beg);
    outFile.write(reinterpret_cast<const char*>(&dataBytes), 4);
}