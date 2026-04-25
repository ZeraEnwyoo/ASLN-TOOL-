#!/bin/bash
# ASLN Installer

set -e

echo "=========================================="
echo "Atomic Super Layer Network Installer"
echo "=========================================="

# Check dependencies
echo "[*] Checking dependencies..."

for cmd in gcc make curl; do
    if ! command -v $cmd &> /dev/null; then
        echo "[-] Missing: $cmd"
        echo "[*] Install with: sudo apt install $cmd (Ubuntu/Debian)"
        echo "[*] Or: sudo pacman -S $cmd (Arch Linux)"
        exit 1
    fi
done

# Compile
echo "[*] Compiling ASLN..."
make clean
make

# Install
echo "[*] Installing to /usr/local/bin..."
sudo make install

echo "[*] Done!"
echo ""
echo "Usage: asln help"
echo "      asln connect nasa"
echo "      asln status"
echo ""
echo "Powered by Atomic Super Layer Network Community"
