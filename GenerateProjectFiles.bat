@echo off
setlocal

rem ------------------------------------------------------------
rem  Generate Visual Studio project files for ET.uproject
rem  Engine path can be overridden by setting UE_ROOT beforehand.
rem ------------------------------------------------------------

set "PROJECT_DIR=%~dp0"
set "PROJECT_FILE=%PROJECT_DIR%ET.uproject"

if not defined UE_ROOT set "UE_ROOT=D:\EpicGames\UE_5.8"

set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"

if not exist "%PROJECT_FILE%" (
    echo [Error] Project file not found: "%PROJECT_FILE%"
    goto :fail
)

if not exist "%BUILD_BAT%" (
    echo [Error] Unreal Engine not found: "%BUILD_BAT%"
    echo         Set UE_ROOT to your engine install folder and try again.
    goto :fail
)

echo Engine : %UE_ROOT%
echo Project: %PROJECT_FILE%
echo.

call "%BUILD_BAT%" -projectfiles -project="%PROJECT_FILE%" -game -rocket -progress
if errorlevel 1 goto :fail

echo.
echo Project files generated successfully.
endlocal
pause
exit /b 0

:fail
echo.
echo Failed to generate project files.
endlocal
pause
exit /b 1
