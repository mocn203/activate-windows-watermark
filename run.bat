@echo off
setlocal
cd /d "%~dp0"

if not exist activate-windows.exe (
    echo [INFO] activate-windows.exe not found. Building binary...
    call build.bat
    if not exist activate-windows.exe exit /b 1
)

start "" "activate-windows.exe" %*
echo [OK] Watermark started in background.
echo Hotkey: Ctrl + Alt + W to toggle visibility.
