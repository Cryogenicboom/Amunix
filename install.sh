#!/bin/bash

set -e

echo "      Amunix Installation"
echo "========================================="

# Check for apt package manager
if ! command -v apt >/dev/null 2>&1; then
    echo "Error: This installer currently supports Debian/Ubuntu-based systems only."
    exit 1
fi

echo "Installing dependencies..."
sudo apt update
sudo apt install -y gcc make git

echo "Cloning Amunix repository..."
# do not clone if already cloned
if [ -d "Amunix" ]; then
    echo "Amunix directory already exists. Skipping clone."
else
    git clone https://github.com/Cryogenicboom/Amunix.git
fi

echo "Building project..."
cd Amunix
make

echo "Launching Amunix..."

if [ -f "./AMUNIX" ]; then
    chmod +x ./AMUNIX
    ./AMUNIX
else
    echo "AMUNIX executable was not found."
    echo "Check your Makefile output."
    exit 1
fi