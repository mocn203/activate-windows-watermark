@echo off
setlocal
cd /d "%~dp0"

echo [1/2] Compiling Activate Windows Watermark Overlay...
where g++ >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] g++ compiler not found in PATH!
    echo Please ensure MinGW-w64 or GCC is installed.
    pause
    exit /b 1
)

g++ -O2 -s -mwindows -municode main.cpp -o activate-windows.exe -lgdiplus -lgdi32 -luser32 -lshell32 -lcomctl32 -lole32

if %ERRORLEVEL% equ 0 (
    echo [2/2] Build successful: activate-windows.exe
    dir activate-windows.exe | findstr /i "activate-windows.exe"
) else (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

endlocal
