@echo off
setlocal
set "IDF_PATH=D:\Espressif\frameworks\esp-idf-v5.5.4"
set "IDF_TOOLS_PATH=D:\Espressif"
set "PYTHONNOUSERSITE=True"

echo Setting up ESP-IDF environment...
call "%IDF_PATH%\export.bat" > nul 2>&1

echo.
echo === Building xiaozhi-esp32 ===
echo.
cd /d F:\xiaozhi-esp32

echo Running idf.py build...
idf.py build 2>&1
set EXIT_CODE=%ERRORLEVEL%

echo.
if %EXIT_CODE% equ 0 (
    echo === BUILD SUCCESSFUL ===
) else (
    echo === BUILD FAILED with code %EXIT_CODE% ===
)

exit /b %EXIT_CODE%
