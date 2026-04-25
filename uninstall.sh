#!/bin/bash
# ASLN Uninstaller

set -e

echo "=========================================="
echo "Atomic Super Layer Network Uninstaller"
echo "=========================================="

sudo make uninstall
rm -rf ~/.cache/asln

echo "[*] ASLN removed successfully"
