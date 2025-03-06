#!/bin/bash
set -e

if ! command -v python3 &> /dev/null; then
    echo "Python not found. Installing via Homebrew..."
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Installing Homebrew first..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    brew install python
else
    echo "Python is already installed."
fi
