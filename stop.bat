@echo off
setlocal
echo Stopping Activate Windows Watermark...
taskkill /f /im activate-windows.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [OK] Watermark stopped!
) else (
    echo [INFO] No running activate-windows.exe process found.
)
