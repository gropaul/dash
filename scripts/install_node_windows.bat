@echo off
setlocal

:: Check if Node.js is already installed
where node >nul 2>nul
if %ERRORLEVEL%==0 (
    echo Node.js is already installed.
    exit /b 0
)

:: Specify the Node.js version and download URL
set "NODE_VERSION=18.14.2"
set "NODE_URL=https://nodejs.org/dist/v%NODE_VERSION%/node-v%NODE_VERSION%-win-x64.zip"
set "NODE_ZIP=node-v%NODE_VERSION%-win-x64.zip"
set "NODE_DIR=%CD%\node-v%NODE_VERSION%-win-x64"

echo Downloading Node.js (which includes npm)...
powershell -Command "Invoke-WebRequest -Uri %NODE_URL% -OutFile %NODE_ZIP%"

echo Extracting Node.js...
powershell -Command "Expand-Archive -Path %NODE_ZIP% -DestinationPath %CD%"

del "%NODE_ZIP%"
echo Node.js (and npm) installed in: %NODE_DIR%

echo.
echo To use node/npm from anywhere, add this folder to your system PATH:
echo     %NODE_DIR%
echo.
echo Done!
endlocal
