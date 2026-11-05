@echo off
:: Kiem tra quyen Administrator
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [CANH BAO] Vui long bam chuot phai vao file nay va chon "Run as administrator"!
    pause
    exit /b 1
)

echo ========================================================
echo   BAT WATERMARK GOC 100%% CUA WINDOWS (QUA SLMGR)
echo ========================================================
echo.
echo Lenh nay se tam thoi go key Windows de he thong chuyen sang
echo che do "Chua kich hoat" (Notification Mode), tu do DWM goc
echo se tu ve watermark "Activate Windows" len man hinh.
echo.
echo May cua ban da co key OEM luu trong BIOS, hoan toan co the
echo khoi phuc lai ban quyen 100%% bang file "native_restore.bat"!
echo.
set /p confirm="Ban co muon tiep tuc khong? (Y/N): "
if /i "%confirm%" neq "Y" exit /b 0

echo.
echo [1/3] Dang go key tam thoi qua slmgr /upk...
cscript //nologo %windir%\system32\slmgr.vbs /upk

echo [2/3] Xoa cache key trong Registry qua slmgr /cpky...
cscript //nologo %windir%\system32\slmgr.vbs /cpky

echo [3/3] Dang khoi dong lai Windows Explorer de cap nhat DWM...
taskkill /f /im explorer.exe >nul 2>&1
start explorer.exe

echo.
echo ========================================================
echo [HOAN TAT] Watermark goc cua Windows se xuat hien!
echo (Neu chua thay ngay, ban chi can Restart may la se hien).
echo.
echo Khi muon khoi phuc lai ban quyen, hay chay "native_restore.bat"!
echo ========================================================
pause
