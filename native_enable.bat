@echo off
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Administrative privileges required.
    echo Please right-click this script and select "Run as administrator".
    pause
    exit /b 1
)

echo [1/3] Uninstalling current product key via slmgr /upk...
cscript //nologo %windir%\system32\slmgr.vbs /upk

echo [2/3] Clearing key from registry via slmgr /cpky...
cscript //nologo %windir%\system32\slmgr.vbs /cpky

echo [3/3] Restarting Windows Explorer...
taskkill /f /im explorer.exe >nul 2>&1
start explorer.exe

echo.
echo [DONE] Windows is now in Notification Mode. Native DWM watermark is enabled.
echo Run native_restore.bat at any time to restore your genuine license.
pause
