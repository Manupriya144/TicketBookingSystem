@echo off
echo ============================================
echo   CineBook - Movie Ticket Booking System
echo   Group 24 - University of Ruhuna
echo ============================================
echo.

:: Create build directory
if not exist "build" mkdir build
cd build

:: Configure with CMake
echo [1/3] Configuring project...
cmake .. -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed.
    echo Make sure Qt and CMake are installed and in PATH.
    pause
    exit /b 1
)

:: Build
echo.
echo [2/3] Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed.
    pause
    exit /b 1
)

echo.
echo [3/3] Build successful!
echo.
echo To run: build\MovieTicketBookingSystem.exe
echo Default credentials:
echo   Admin:  admin / admin123
echo   User:   user1 / user123
echo.
pause
