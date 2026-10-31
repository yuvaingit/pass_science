@echo off
title PASS Science Works - Unified Engineering Workbench
echo ========================================================
echo   PASS SCIENCE WORKS | Unified Engineering Workbench
echo   Windows 11 x64 One-Click Launcher
echo ========================================================

cd /d "%~dp0"

:: 1. If precompiled backend exists, launch it
if exist "bin\workbench_server.exe" (
    echo [INFO] Launching C++ backend server...
    start "" "bin\workbench_server.exe"
    exit /b 0
)

if exist "build\bin\workbench_server.exe" (
    echo [INFO] Launching C++ backend server...
    start "" "build\bin\workbench_server.exe"
    exit /b 0
)

:: 2. Launch bundled high-fidelity web application directly in user's default browser
echo [INFO] Opening Unified Engineering Workbench interface in default browser...
start "" "%~dp0dist\index.html"
exit /b 0
