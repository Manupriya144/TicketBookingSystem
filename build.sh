#!/bin/bash
echo "============================================"
echo "  CineBook - Movie Ticket Booking System"
echo "  Group 24 - University of Ruhuna"
echo "============================================"
echo

mkdir -p build && cd build

echo "[1/3] Configuring..."
cmake .. || { echo "CMake failed. Install Qt and CMake."; exit 1; }

echo "[2/3] Building..."
cmake --build . --config Release || { echo "Build failed."; exit 1; }

echo "[3/3] Done!"
echo "Run: ./build/MovieTicketBookingSystem"
echo "Admin: admin / admin123 | User: user1 / user123"
