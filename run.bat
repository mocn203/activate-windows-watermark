@echo off
setlocal
cd /d "%~dp0"

if not exist activate-windows.exe (
    echo [INFO] activate-windows.exe not found, building first...
    call build.bat
    if not exist activate-windows.exe exit /b 1
)

echo Starting Activate Windows Watermark...
start "" "activate-windows.exe" %*
echo [OK] Watermark started in background!
echo Press Ctrl + Alt + W to toggle hide/show, or right-click the Windows icon in System Tray.
