#!/bin/bash
set -e

if ! command -v python3 &> /dev/null; then
    echo "Python not found. Installing via package manager..."
    if [ -f /etc/debian_version ]; then
        sudo apt-get update
        sudo apt-get install -y python3
    elif [ -f /etc/redhat-release ]; then
        sudo yum install -y python3
    else
        echo "Unsupported Linux distribution"
        exit 1
    fi
else
    echo "Python is already installed."
fi
