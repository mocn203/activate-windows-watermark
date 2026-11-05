@echo off
setlocal
taskkill /f /im activate-windows.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [OK] activate-windows.exe terminated.
) else (
    echo [INFO] No running instance of activate-windows.exe detected.
)
