@echo off
chcp 65001 > nul

:: Find VS 2022 vcvars
set VCVARS="C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist %VCVARS% (
    echo ERROR: vcvars64.bat not found. Make sure C++ workload is installed in Visual Studio.
    pause
    exit /b 1
)

call %VCVARS% > nul 2>&1

echo [1/3] Compiling resource...
rc /fo PinOverlay.res PinOverlay.rc
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: rc failed, building without icon resource
    set RES_FILE=
) else (
    set RES_FILE=PinOverlay.res
)

echo [2/3] Compiling and linking...
cl.exe /nologo /W3 /O2 /EHsc /LD ^
    /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "UNICODE" /D "_UNICODE" ^
    PinOverlay.cpp %RES_FILE% ^
    /Fe:PinOverlay.dll ^
    /link /DEF:PinOverlay.def ^
    kernel32.lib user32.lib shell32.lib shlwapi.lib ole32.lib

if %ERRORLEVEL% NEQ 0 (
    echo BUILD FAILED
    pause
    exit /b 1
)

echo [3/3] Done! PinOverlay.dll built successfully.
echo.
echo Next steps:
echo   1. Run install.ps1 as Administrator
echo   2. Or manually: regsvr32 PinOverlay.dll
echo.
pause
