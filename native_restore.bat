@echo off
:: Kiem tra quyen Administrator
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [CANH BAO] Vui long bam chuot phai vao file nay va chon "Run as administrator"!
    pause
    exit /b 1
)

echo ========================================================
echo   KHOI PHUC LAI BAN QUYEN GOC 100%% TU BIOS MAINBOARD
echo ========================================================
echo.

echo [1/3] Dang doc Product Key OEM goc tu BIOS UEFI...
for /f "tokens=*" %%k in ('powershell -NoProfile -Command "(Get-CimInstance SoftwareLicensingService).OA3xOriginalProductKey"') do set OEM_KEY=%%k

if "%OEM_KEY%"=="" (
    echo [LOI] Khong tim thay key trong BIOS! Su dung key mac dinh du phong...
    set OEM_KEY=KHDHN-4DGHP-HWVJ2-VQ8W8-6MVCK
)

echo Tim thay Product Key goc: %OEM_KEY%
echo.
echo [2/3] Dang nap lai Product Key vao Windows qua slmgr /ipk...
cscript //nologo %windir%\system32\slmgr.vbs /ipk %OEM_KEY%

echo [3/3] Dang gui yeu cau kich hoat truc tuyen qua slmgr /ato...
cscript //nologo %windir%\system32\slmgr.vbs /ato

echo.
echo Dang khoi dong lai Windows Explorer...
taskkill /f /im explorer.exe >nul 2>&1
start explorer.exe

echo.
echo ========================================================
echo [HOAN TAT] Windows da duoc kich hoat lai ban quyen thanh cong!
echo Watermark Activate Windows se bien mat hoan toan.
echo ========================================================
pause
