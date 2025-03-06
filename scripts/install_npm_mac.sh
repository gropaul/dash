#!/bin/bash
set -e

if ! command -v npm &> /dev/null; then
    echo "npm not found. Installing via Homebrew..."

    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Installing Homebrew first..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi

    brew install node
else
    echo "npm is already installed."
fi
