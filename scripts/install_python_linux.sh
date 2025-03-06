#!/bin/bash
set -e

# Detect if running as root (for Docker)
if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
else
    if command -v sudo &> /dev/null; then
        SUDO="sudo"
    else
        echo "Error: sudo not found and not running as root."
        exit 1
    fi
fi

# Detect and print distribution name
if [ -f /etc/os-release ]; then
    . /etc/os-release
    echo "Detected distribution: $NAME ($ID)"
    DISTRO_FAMILY="$ID_LIKE"
    DISTRO_ID="$ID"
elif [ -f /etc/debian_version ]; then
    echo "Detected distribution: Debian (no /etc/os-release)"
    DISTRO_ID="debian"
elif [ -f /etc/redhat-release ]; then
    echo "Detected distribution: Red Hat-based (no /etc/os-release)"
    DISTRO_ID="rhel"
else
    echo "Could not detect Linux distribution."
    exit 1
fi

# Check if musl or glibc
if ldd --version 2>&1 | grep -q "musl"; then
    echo "Detected musl-based system (Alpine/musl)."
    MUSL_SYSTEM=true
else
    echo "Detected glibc-based system."
    MUSL_SYSTEM=false
fi

# Function to install Python
install_python() {
    if ! command -v python3 &> /dev/null; then
        echo "Python not found. Installing via package manager..."

        if $MUSL_SYSTEM; then
            echo "Installing Python for musl-based system (Alpine)..."
            $SUDO apk add --no-cache python3
        else
            case "$DISTRO_ID" in
                ubuntu|debian)
                    $SUDO apt-get update
                    $SUDO apt-get install -y python3
                    ;;
                fedora)
                    $SUDO dnf install -y python3
                    ;;
                centos|rhel|almalinux|rocky)
                    $SUDO yum install -y python3
                    ;;
                opensuse*|sles)
                    $SUDO zypper install -y python3
                    ;;
                arch)
                    $SUDO pacman -Sy --noconfirm python
                    ;;
                *)
                    if [[ "$DISTRO_FAMILY" =~ debian ]]; then
                        $SUDO apt-get update
                        $SUDO apt-get install -y python3
                    elif [[ "$DISTRO_FAMILY" =~ rhel|fedora ]]; then
                        $SUDO yum install -y python3
                    else
                        echo "Unsupported distribution: $NAME ($ID)"
                        exit 1
                    fi
                    ;;
            esac
        fi
    else
        echo "Python is already installed."
    fi
}

install_python
