$env:IDF_PATH = "D:\Espressif\frameworks\esp-idf-v5.5.4"
$env:IDF_TOOLS_PATH = "D:\Espressif"
$env:PYTHONNOUSERSITE = "True"

# The ESP-IDF export.bat modifies the environment and then we run idf.py
# We need to do this in a single cmd.exe invocation

$buildScript = @"
@echo off
call "$env:IDF_PATH\export.bat" > nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to initialize ESP-IDF environment
    exit /b 1
)
echo ESP-IDF environment initialized successfully
python --version
echo.
echo Building project...
cd /d F:\xiaozhi-esp32
idf.py build
if %ERRORLEVEL% neq 0 (
    echo === BUILD FAILED ===
    exit /b %ERRORLEVEL%
)
echo === BUILD SUCCESSFUL ===
"@

$buildScript | Out-File -FilePath "$env:TEMP\esp_build_tmp.cmd" -Encoding ASCII

Write-Host "=== xiaozhi-esp32 Build ==="
Write-Host "Initializing ESP-IDF and building..."

& cmd.exe /c "$env:TEMP\esp_build_tmp.cmd" 2>&1

$exitCode = $LASTEXITCODE
Write-Host "Exit code: $exitCode"
exit $exitCode
