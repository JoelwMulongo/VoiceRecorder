// Voice Recorder - simple Windows GUI on top of the VoiceRecorder class.
// Click Record to start, Stop to finish. Files are saved as WAV in
// Documents\VoiceRecordings with a timestamped name.

#include "VoiceRecorder.h"

#include <shlobj.h>
#include <shellapi.h>
#include <cstdio>
#include <string>

enum { ID_TOGGLE = 101, ID_OPEN = 102, ID_TIMER = 103 };

static VoiceRecorder g_recorder;
static bool g_recording = false;
static bool g_ready = false;
static DWORD g_startTick = 0;
static HWND g_btnToggle, g_btnOpen, g_lblTime, g_lblStatus;
static std::string g_dir, g_lastFile;

static std::string makeFilename() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "recording_%04d%02d%02d_%02d%02d%02d.wav",
                  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return buf;
}

static void updateTimeLabel() {
    DWORD secs = (GetTickCount() - g_startTick) / 1000;
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02lu:%02lu", secs / 60, secs % 60);
    SetWindowTextA(g_lblTime, buf);
}

static void toggleRecording(HWND hwnd) {
    if (!g_ready) return;

    if (!g_recording) {
        g_lastFile = g_dir + "\\" + makeFilename();
        if (!g_recorder.startRecording(g_lastFile)) {
            MessageBoxA(hwnd,
                "Could not start recording.\n\nCheck that a microphone is connected and that "
                "microphone access is allowed in Windows Settings > Privacy & security > Microphone.",
                "Voice Recorder", MB_ICONERROR);
            return;
        }
        g_recording = true;
        g_startTick = GetTickCount();
        SetWindowTextA(g_btnToggle, "Stop");
        SetWindowTextA(g_lblStatus, "Recording...");
        SetWindowTextA(g_lblTime, "00:00");
        SetTimer(hwnd, ID_TIMER, 250, nullptr);
    } else {
        g_recorder.stopRecording();
        KillTimer(hwnd, ID_TIMER);
        g_recording = false;
        SetWindowTextA(g_btnToggle, "Record");
        SetWindowTextA(g_lblStatus, ("Saved: " + g_lastFile).c_str());
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HFONT guiFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        HFONT bigFont = CreateFontA(44, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_SWISS, "Segoe UI");

        g_lblTime = CreateWindowA("STATIC", "00:00", WS_CHILD | WS_VISIBLE | SS_CENTER,
                                  20, 15, 320, 55, hwnd, nullptr, nullptr, nullptr);
        g_btnToggle = CreateWindowA("BUTTON", "Record", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    20, 85, 150, 40, hwnd, reinterpret_cast<HMENU>(ID_TOGGLE), nullptr, nullptr);
        g_btnOpen = CreateWindowA("BUTTON", "Open folder", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  190, 85, 150, 40, hwnd, reinterpret_cast<HMENU>(ID_OPEN), nullptr, nullptr);
        g_lblStatus = CreateWindowA("STATIC", "Ready", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                    20, 140, 320, 40, hwnd, nullptr, nullptr, nullptr);

        SendMessage(g_lblTime, WM_SETFONT, reinterpret_cast<WPARAM>(bigFont), TRUE);
        SendMessage(g_btnToggle, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
        SendMessage(g_btnOpen, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
        SendMessage(g_lblStatus, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);

        // Save location: Documents\VoiceRecordings
        char docs[MAX_PATH] = {0};
        if (FAILED(SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, docs))) {
            GetCurrentDirectoryA(MAX_PATH, docs);
        }
        g_dir = std::string(docs) + "\\VoiceRecordings";
        CreateDirectoryA(g_dir.c_str(), nullptr);

        g_ready = g_recorder.initialize();
        if (!g_ready) {
            EnableWindow(g_btnToggle, FALSE);
            SetWindowTextA(g_lblStatus, "No microphone found. Plug one in and restart the app.");
        }
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TOGGLE: toggleRecording(hwnd); break;
        case ID_OPEN:   ShellExecuteA(hwnd, "open", g_dir.c_str(), nullptr, nullptr, SW_SHOWNORMAL); break;
        }
        return 0;
    case WM_TIMER:
        if (wParam == ID_TIMER && g_recording) updateTimeLabel();
        return 0;
    case WM_DESTROY:
        if (g_recording) {
            g_recorder.stopRecording();   // never lose a recording when the window closes
            g_recording = false;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = "VoiceRecorderWnd";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("VoiceRecorderWnd", "Voice Recorder",
                              WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 375, 235,
                              nullptr, nullptr, hInst, nullptr);
    if (!hwnd) return 1;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}