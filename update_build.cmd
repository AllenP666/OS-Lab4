@echo off
git pull
if not exist build mkdir build
cd build

cmake -G "MinGW Makefiles" ..
if %errorlevel% neq 0 (
    echo Configuration failed!
    pause
    exit /b %errorlevel%
)

cmake --build .
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Build successful!
pause