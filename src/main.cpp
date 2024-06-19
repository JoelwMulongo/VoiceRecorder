#include "VoiceRecorder.h"

int main() {
    VoiceRecorder recorder;
    if (recorder.initialize()) {
        recorder.startRecording("output.wav");
        // Wait for a few seconds to record
        Sleep(5000); // Record for 5 seconds
        recorder.stopRecording();
    }
    return 0;
}
