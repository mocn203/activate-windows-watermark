@echo off
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Administrative privileges required.
    echo Please right-click this script and select "Run as administrator".
    pause
    exit /b 1
)

echo [1/3] Reading OEM product key from UEFI BIOS...
for /f "tokens=*" %%k in ('powershell -NoProfile -Command "(Get-CimInstance SoftwareLicensingService).OA3xOriginalProductKey"') do set OEM_KEY=%%k

if "%OEM_KEY%"=="" (
    echo [ERROR] No embedded OEM key found in BIOS.
    pause
    exit /b 1
)

echo Found embedded OEM key: %OEM_KEY%
echo.
echo [2/3] Installing OEM product key via slmgr /ipk...
cscript //nologo %windir%\system32\slmgr.vbs /ipk %OEM_KEY%

echo [3/3] Activating Windows via slmgr /ato...
cscript //nologo %windir%\system32\slmgr.vbs /ato

echo.
echo Restarting Windows Explorer...
taskkill /f /im explorer.exe >nul 2>&1
start explorer.exe

echo.
echo [DONE] Windows license restored and activated successfully.
pause
