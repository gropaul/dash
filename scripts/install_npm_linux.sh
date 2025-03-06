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

# Function to install npm (and Node.js if needed)
install_npm() {
    if ! command -v npm &> /dev/null; then
        echo "npm not found. Installing via package manager..."

        if $MUSL_SYSTEM; then
            echo "Installing npm for musl-based system (Alpine)..."
            $SUDO apk update
            # Typically nodejs and npm are separate packages in Alpine
            $SUDO apk add --no-cache nodejs npm
        else
            case "$DISTRO_ID" in
                ubuntu|debian)
                    $SUDO apt-get update
                    $SUDO apt-get install -y nodejs npm
                    ;;
                fedora)
                    $SUDO dnf install -y nodejs npm
                    ;;
                centos|rhel|almalinux|rocky)
                    # CentOS/RHEL often come with older Node.js in default repos.
                    # For newer versions, users might prefer NodeSource or EPEL repos,
                    # but here we do the simplest approach:
                    $SUDO yum install -y nodejs npm
                    ;;
                opensuse*|sles)
                    $SUDO zypper refresh
                    $SUDO zypper install -y nodejs npm
                    ;;
                arch)
                    $SUDO pacman -Sy --noconfirm nodejs npm
                    ;;
                *)
                    # Fallback for distros using the ID_LIKE approach
                    if [[ "$DISTRO_FAMILY" =~ debian ]]; then
                        $SUDO apt-get update
                        $SUDO apt-get install -y nodejs npm
                    elif [[ "$DISTRO_FAMILY" =~ rhel|fedora ]]; then
                        $SUDO yum install -y nodejs npm
                    else
                        echo "Unsupported distribution: $NAME ($ID)"
                        exit 1
                    fi
                    ;;
            esac
        fi
    else
        echo "npm is already installed."
    fi
}

install_npm
