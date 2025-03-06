#!/bin/bash
set -e

# Detect if running as root
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

##############################################
# Attempt installing Node 18.x (and npm) via NodeSource
##############################################
install_node18_nodesource() {
    echo "Attempting to install Node.js 18.x from NodeSource..."

    if $MUSL_SYSTEM; then
        echo "NodeSource does not publish Alpine packages. Cannot use NodeSource on Alpine."
        return 1
    fi

    case "$DISTRO_ID" in
        ubuntu|debian)
            $SUDO apt-get update
            $SUDO apt-get install -y curl ca-certificates
            # Pipe the NodeSource setup script
            if ! curl -fsSL https://deb.nodesource.com/setup_18.x | $SUDO bash -; then
                return 1
            fi
            $SUDO apt-get install -y nodejs || return 1
            ;;
        fedora)
            $SUDO dnf install -y curl
            if ! curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash -; then
                return 1
            fi
            $SUDO dnf install -y nodejs || return 1
            ;;
        centos|rhel|almalinux|rocky)
            $SUDO yum install -y curl
            if ! curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash -; then
                return 1
            fi
            $SUDO yum install -y nodejs || return 1
            ;;
        opensuse*|sles)
            echo "NodeSource doesn't officially support openSUSE/SLES directly."
            echo "Cannot install Node 18.x via NodeSource on $DISTRO_ID."
            return 1
            ;;
        arch)
            echo "NodeSource doesn't provide scripts for Arch. Cannot use NodeSource."
            return 1
            ;;
        *)
            # If ID_LIKE can help us
            if [[ "$DISTRO_FAMILY" =~ debian ]]; then
                $SUDO apt-get update
                $SUDO apt-get install -y curl ca-certificates
                if ! curl -fsSL https://deb.nodesource.com/setup_18.x | $SUDO bash -; then
                    return 1
                fi
                $SUDO apt-get install -y nodejs || return 1
            elif [[ "$DISTRO_FAMILY" =~ rhel|fedora ]]; then
                $SUDO yum install -y curl
                if ! curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash -; then
                    return 1
                fi
                $SUDO yum install -y nodejs || return 1
            else
                echo "Unsupported distribution for NodeSource: $DISTRO_ID"
                return 1
            fi
            ;;
    esac

    # If everything above succeeds, we return 0
    echo "Successfully installed Node.js 18.x from NodeSource."
    return 0
}

##############################################
# Fallback: Install Node.js (and npm) via system repos
# (Likely older version, but usually guaranteed to work)
##############################################
install_node_from_system() {
    echo "Falling back to installing Node.js/npm from system repos..."
    if $MUSL_SYSTEM; then
        echo "Alpine-based system. Using apk..."
        $SUDO apk update
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
                # Fallback for ID_LIKE
                if [[ "$DISTRO_FAMILY" =~ debian ]]; then
                    $SUDO apt-get update
                    $SUDO apt-get install -y nodejs npm
                elif [[ "$DISTRO_FAMILY" =~ rhel|fedora ]]; then
                    $SUDO yum install -y nodejs npm
                else
                    echo "Unsupported distribution: $DISTRO_ID"
                    exit 1
                fi
                ;;
        esac
    fi
    echo "Installed Node.js/npm from system repositories."
}

##############################################
# Main function to ensure npm is installed
##############################################
install_npm() {
    if command -v npm &>/dev/null; then
        echo "npm is already installed."
        return
    fi

    echo "npm not found. Attempting Node 18 from NodeSource first..."

    if install_node18_nodesource; then
        # Node 18 install succeeded
        :
    else
        echo "NodeSource install failed or not supported. Installing from system repos..."
        install_node_from_system
    fi
}

install_npm
