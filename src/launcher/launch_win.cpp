#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>

// WinMain entry point: Launches Unified Engineering Workbench C++ backend and browser UI
// with zero console flash and zero required terminal interaction.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;

    // Determine application root directory from launcher module location
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        return 1;
    }
    std::wstring dirPath(exePath);
    size_t lastSlash = dirPath.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        dirPath = dirPath.substr(0, lastSlash);
    }

    // Search order for precompiled C++17 native backend daemon
    std::vector<std::wstring> backendCandidates = {
        dirPath + L"\\bin\\workbench_server.exe",
        dirPath + L"\\build\\bin\\Release\\workbench_server.exe",
        dirPath + L"\\build\\bin\\workbench_server.exe",
        dirPath + L"\\workbench_server.exe"
    };

    std::wstring serverExePath;
    for (const auto& candidate : backendCandidates) {
        DWORD attribs = GetFileAttributesW(candidate.c_str());
        if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
            serverExePath = candidate;
            break;
        }
    }

    if (!serverExePath.empty()) {
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE; // Run background daemon without terminal popup
        ZeroMemory(&pi, sizeof(pi));

        std::wstring cmd = L"\"" + serverExePath + L"\" --port 8080";
        std::vector<wchar_t> cmdBuffer(cmd.begin(), cmd.end());
        cmdBuffer.push_back(L'\0');

        if (CreateProcessW(
                NULL,
                cmdBuffer.data(),
                NULL,
                NULL,
                FALSE,
                CREATE_NO_WINDOW,
                NULL,
                dirPath.c_str(),
                &si,
                &pi
            )) {
            // Process started successfully; close handles so it runs autonomously
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            // Give socket server a short moment to bind port 8080
            Sleep(400);

            // Open user's default web browser to the workbench interface
            ShellExecuteW(NULL, L"open", L"http://127.0.0.1:8080", NULL, NULL, SW_SHOWNORMAL);
            return 0;
        }
    }

    // Standalone fallback: If backend daemon is not precompiled, open bundled dist/index.html directly
    std::wstring indexPath = dirPath + L"\\dist\\index.html";
    DWORD htmlAttribs = GetFileAttributesW(indexPath.c_str());
    if (htmlAttribs != INVALID_FILE_ATTRIBUTES && !(htmlAttribs & FILE_ATTRIBUTE_DIRECTORY)) {
        ShellExecuteW(NULL, L"open", indexPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        return 0;
    }

    // Component missing alert
    MessageBoxW(
        NULL,
        L"Unified Engineering Workbench components could not be found.\n\n"
        L"Please verify that 'bin\\workbench_server.exe' and 'dist\\index.html' are present in the application directory.",
        L"PASS Science Works - Launch Error",
        MB_OK | MB_ICONERROR
    );

    return 1;
}
