@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
set "PARTITIONS_CSV=%SCRIPT_DIR%partitions.csv"
set "BAUD=460800"
set "CHIP=esp32"
set "BOOTLOADER_OFFSET=0x1000"

set "PORT="
set "YES=0"
set "ERASE=0"
set "DRYRUN=0"

for %%A in (%*) do (
    if /I "%%~A"=="-y" set "YES=1"
    if /I "%%~A"=="--yes" set "YES=1"
    if /I "%%~A"=="--erase" set "ERASE=1"
    if /I "%%~A"=="--dry-run" set "DRYRUN=1"
    if /I not "%%~A"=="-y" if /I not "%%~A"=="--yes" if /I not "%%~A"=="--erase" if /I not "%%~A"=="--dry-run" if not defined PORT set "PORT=%%~A"
)

echo [INFO] PRM-X2 OLED auto flashing script (esp32dev)

call :find_esptool
if errorlevel 1 goto :fail

set "BOOTLOADER_BIN=%SCRIPT_DIR%bootloader.bin"
set "PARTITIONS_BIN=%SCRIPT_DIR%partitions.bin"
set "LITTLEFS_BIN=%SCRIPT_DIR%littlefs.bin"
set "FIRMWARE_BIN=%SCRIPT_DIR%firmware.bin"
set "PRM_BIN=%SCRIPT_DIR%prm_x2.bin"

if not exist "%PARTITIONS_CSV%" (
    echo [ERROR] Missing partitions.csv: %PARTITIONS_CSV%
    goto :fail
)
if not exist "%BOOTLOADER_BIN%" (
    echo [ERROR] Missing bootloader.bin: %BOOTLOADER_BIN%
    goto :fail
)
if not exist "%PARTITIONS_BIN%" (
    echo [ERROR] Missing partitions.bin: %PARTITIONS_BIN%
    goto :fail
)
if not exist "%LITTLEFS_BIN%" (
    echo [ERROR] Missing littlefs.bin: %LITTLEFS_BIN%
    echo [ERROR] Run: pio run -e esp32dev -t buildfs
    goto :fail
)

set "APP_BIN="
if exist "%FIRMWARE_BIN%" set "APP_BIN=%FIRMWARE_BIN%"
if not defined APP_BIN if exist "%PRM_BIN%" set "APP_BIN=%PRM_BIN%"
if not defined APP_BIN (
    echo [ERROR] Missing app image: firmware.bin or prm_x2.bin
    goto :fail
)

set "APP_OFFSET="
set "FS_OFFSET="
for /f "usebackq tokens=1-5 delims=," %%a in ("%PARTITIONS_CSV%") do (
    set "NAME=%%a"
    set "TYPE=%%b"
    set "SUBTYPE=%%c"
    set "OFFSET=%%d"
    call :trim NAME
    call :trim TYPE
    call :trim SUBTYPE
    call :trim OFFSET

    if not "!NAME!"=="" if not "!NAME:~0,1!"=="#" (
        if /I "!TYPE!"=="app" if /I "!SUBTYPE!"=="factory" set "APP_OFFSET=!OFFSET!"
        if /I "!TYPE!"=="app" if /I "!NAME!"=="factory" set "APP_OFFSET=!OFFSET!"

        if /I "!NAME!"=="storage" set "FS_OFFSET=!OFFSET!"
        if not defined FS_OFFSET if /I "!TYPE!"=="data" if /I "!SUBTYPE!"=="spiffs" set "FS_OFFSET=!OFFSET!"
        if not defined FS_OFFSET if /I "!TYPE!"=="data" if /I "!SUBTYPE!"=="littlefs" set "FS_OFFSET=!OFFSET!"
    )
)

if not defined APP_OFFSET (
    echo [ERROR] Cannot detect factory app offset from partitions.csv
    goto :fail
)
if not defined FS_OFFSET (
    echo [ERROR] Cannot detect storage FS offset from partitions.csv
    goto :fail
)

echo [INFO] App offset: %APP_OFFSET%
echo [INFO] FS  offset: %FS_OFFSET%

if not defined PORT (
    call :select_port
    if errorlevel 1 goto :fail
) else (
    echo [INFO] Using port: %PORT%
)

echo.
echo Flash plan:
echo   %BOOTLOADER_OFFSET% ^> "%BOOTLOADER_BIN%"
echo   0x8000     ^> "%PARTITIONS_BIN%"
echo   %APP_OFFSET% ^> "%APP_BIN%"
echo   %FS_OFFSET% ^> "%LITTLEFS_BIN%"
echo.

if "%YES%"=="0" (
    set /p "CONFIRM=Continue? (y/N): "
    if /I not "!CONFIRM!"=="y" if /I not "!CONFIRM!"=="yes" (
        echo [ERROR] Cancelled.
        goto :fail
    )
)

if "%ERASE%"=="1" (
    echo [INFO] Erasing flash...
    if "%DRYRUN%"=="1" (
      echo [DRY-RUN] esptool --chip %CHIP% --port %PORT% --baud %BAUD% erase_flash
    ) else (
      call :exec_esptool --chip %CHIP% --port %PORT% --baud %BAUD% erase_flash
      if errorlevel 1 goto :fail
    )
)

echo [INFO] Flashing bootloader + partitions + app + littlefs...
if "%DRYRUN%"=="1" (
  echo [DRY-RUN] esptool --chip %CHIP% --port %PORT% --baud %BAUD% write_flash -z %BOOTLOADER_OFFSET% "%BOOTLOADER_BIN%" 0x8000 "%PARTITIONS_BIN%" %APP_OFFSET% "%APP_BIN%" %FS_OFFSET% "%LITTLEFS_BIN%"
) else (
  call :exec_esptool --chip %CHIP% --port %PORT% --baud %BAUD% write_flash -z %BOOTLOADER_OFFSET% "%BOOTLOADER_BIN%" 0x8000 "%PARTITIONS_BIN%" %APP_OFFSET% "%APP_BIN%" %FS_OFFSET% "%LITTLEFS_BIN%"
  if errorlevel 1 goto :fail
)

echo.
echo [INFO] Done.
exit /b 0

:trim
set "%~1=!%~1: =!"
exit /b 0

:select_port
set "PORT_COUNT=0"
echo [INFO] Detected COM ports:
for /f "tokens=1,2,*" %%A in ('reg query HKLM\HARDWARE\DEVICEMAP\SERIALCOMM 2^>nul ^| findstr /I "REG_SZ"') do (
    set "P=%%C"
    set "P=!P: =!"
    set /a PORT_COUNT+=1
    set "PORT_!PORT_COUNT!=!P!"
    echo   [!PORT_COUNT!] !P!
)
if "%PORT_COUNT%"=="0" (
    echo [ERROR] No COM ports found.
    exit /b 1
)
set /p "PORT_IDX=Select port number (1-%PORT_COUNT%): "
for /f "delims=0123456789" %%X in ("%PORT_IDX%") do (
    echo [ERROR] Invalid selection.
    exit /b 1
)
if "%PORT_IDX%"=="" (
    echo [ERROR] Invalid selection.
    exit /b 1
)
if %PORT_IDX% LSS 1 (
    echo [ERROR] Invalid selection.
    exit /b 1
)
if %PORT_IDX% GTR %PORT_COUNT% (
    echo [ERROR] Invalid selection.
    exit /b 1
)
call set "PORT=%%PORT_%PORT_IDX%%%"
echo [INFO] Selected port: %PORT%
exit /b 0

:find_esptool
set "ESP_MODE="
set "ESP_PATH="

if exist "%SCRIPT_DIR%esptool.exe" (
    set "ESP_MODE=local_exe"
    set "ESP_PATH=%SCRIPT_DIR%esptool.exe"
    echo [INFO] Using esptool from package: %ESP_PATH%
    exit /b 0
)

for %%R in ("%LOCALAPPDATA%\Arduino15\packages\esp32\tools\esptool_py" "%APPDATA%\Arduino15\packages\esp32\tools\esptool_py" "%USERPROFILE%\AppData\Local\Arduino15\packages\esp32\tools\esptool_py") do (
    if not defined ESP_MODE if exist "%%~fR" (
        for /f "delims=" %%F in ('dir /s /b "%%~fR\esptool.exe" 2^>nul') do (
            if not defined ESP_MODE (
                set "ESP_MODE=arduino_exe"
                set "ESP_PATH=%%F"
            )
        )
        for /f "delims=" %%F in ('dir /s /b "%%~fR\esptool.py" 2^>nul') do (
            if not defined ESP_MODE (
                set "ESP_MODE=arduino_py"
                set "ESP_PATH=%%F"
            )
        )
    )
)

if defined ESP_MODE (
    echo [INFO] Found esptool in Arduino: %ESP_PATH%
    exit /b 0
)

python -m esptool version >nul 2>nul
if "%ERRORLEVEL%"=="0" (
    set "ESP_MODE=python_module"
    echo [INFO] Found esptool via python module.
    exit /b 0
)

for /f "delims=" %%F in ('where esptool.exe 2^>nul') do (
    if not defined ESP_MODE (
        set "ESP_MODE=path_exe"
        set "ESP_PATH=%%F"
    )
)
if defined ESP_MODE (
    echo [INFO] Found esptool.exe in PATH: %ESP_PATH%
    exit /b 0
)

for /f "delims=" %%F in ('where esptool.py 2^>nul') do (
    if not defined ESP_MODE (
        set "ESP_MODE=path_py"
        set "ESP_PATH=%%F"
    )
)
if defined ESP_MODE (
    echo [INFO] Found esptool.py in PATH: %ESP_PATH%
    exit /b 0
)

echo [ERROR] esptool not found.
echo [ERROR] Install Arduino ESP32 core or: python -m pip install esptool
exit /b 1

:exec_esptool
if "%ESP_MODE%"=="local_exe" (
  "%ESP_PATH%" %*
  exit /b %ERRORLEVEL%
)
if "%ESP_MODE%"=="arduino_exe" (
  "%ESP_PATH%" %*
  exit /b %ERRORLEVEL%
)
if "%ESP_MODE%"=="arduino_py" (
  python "%ESP_PATH%" %*
  exit /b %ERRORLEVEL%
)
if "%ESP_MODE%"=="python_module" (
  python -m esptool %*
  exit /b %ERRORLEVEL%
)
if "%ESP_MODE%"=="path_exe" (
  "%ESP_PATH%" %*
  exit /b %ERRORLEVEL%
)
if "%ESP_MODE%"=="path_py" (
  python "%ESP_PATH%" %*
  exit /b %ERRORLEVEL%
)

echo [ERROR] Unknown esptool mode.
exit /b 1

:fail
exit /b 1
