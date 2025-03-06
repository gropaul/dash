@echo off
set "PYTHON_URL=https://www.python.org/ftp/python/3.11.2/python-3.11.2-embed-amd64.zip"
set "PYTHON_DIR=%CD%\python_embed"

echo Downloading Python...
powershell -Command "Invoke-WebRequest -Uri %PYTHON_URL% -OutFile python.zip"

echo Extracting Python...
powershell -Command "Expand-Archive -Path python.zip -DestinationPath %PYTHON_DIR%"

del python.zip
echo Python installed in %PYTHON_DIR%