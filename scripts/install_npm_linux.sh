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

NODE_VERSION="18.14.2"

install_node_18() {
    echo "Installing Node.js $NODE_VERSION or newer..."

    if $MUSL_SYSTEM; then
        # Alpine uses separate 'community' repositories or 'edge' for current Node.
        # 'nodejs-current' package typically points to the latest stable Node branch.
        echo "Installing Node.js >= $NODE_VERSION on Alpine..."
        $SUDO apk update
        # If the Alpine base doesn't have nodejs-current, you may need to enable community repo:
        # $SUDO apk add --no-cache nodejs-current npm
        # Some Alpine versions ship an older nodejs package. For actual guaranteed 18.x,
        # you may need to manually download from nodejs.org or use a tarball.
        $SUDO apk add --no-cache nodejs npm
    else
        case "$DISTRO_ID" in
            ubuntu|debian)
                # Use NodeSource setup_18.x script to ensure we get Node >= 18.x
                $SUDO apt-get update
                $SUDO apt-get install -y curl ca-certificates
                curl -fsSL https://deb.nodesource.com/setup_18.x | $SUDO bash -
                $SUDO apt-get install -y nodejs
                ;;
            fedora)
                $SUDO dnf install -y curl
                curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash -
                $SUDO dnf install -y nodejs
                ;;
            centos|rhel|almalinux|rocky)
                $SUDO yum install -y curl
                curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash -
                $SUDO yum install -y nodejs
                ;;
            opensuse*|sles)
                # openSUSE Tumbleweed typically has newer Node in main repos:
                echo "Installing Node.js >= $NODE_VERSION on openSUSE/SLES..."
                $SUDO zypper refresh
                $SUDO zypper install -y nodejs18 npm18 || {
                    echo "Attempted nodejs18/npm18 from the official repos."
                    echo "If that fails, consider a manual NodeSource or manual install approach."
                    exit 1
                }
                ;;
            arch)
                # Arch generally has very recent packages
                echo "Installing Node.js >= $NODE_VERSION on Arch..."
                $SUDO pacman -Sy --noconfirm nodejs npm
                ;;
            *)
                # If ID_LIKE can help us guess
                if [[ "$DISTRO_FAMILY" =~ debian ]]; then
                    $SUDO apt-get update
                    $SUDO apt-get install -y curl ca-certificates
                    curl -fsSL https://deb.nodesource.com/setup_18.x | $SUDO bash -
                    $SUDO apt-get install -y nodejs
                elif [[ "$DISTRO_FAMILY" =~ rhel|fedora ]]; then
                    $SUDO yum install -y curl
                    curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash -
                    $SUDO yum install -y nodejs
                else
                    echo "Unsupported distribution: $NAME ($ID)"
                    exit 1
                fi
                ;;
        esac
    fi
}

# Check if npm exists, and if node is >= 18.14.2
if command -v node &>/dev/null && command -v npm &>/dev/null; then
    # Extract just the numeric version (e.g., "18.14.2") from something like "v18.14.2"
    INSTALLED_VERSION="$(node -v | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')"
    if [ -z "$INSTALLED_VERSION" ]; then
        echo "Node found, but version check failed. Reinstalling Node >= $NODE_VERSION."
        install_node_18
    else
        # Compare versions using sort -V
        MIN_VERSION="$NODE_VERSION"
        LOWER_VERSION="$(printf '%s\n' "$MIN_VERSION" "$INSTALLED_VERSION" | sort -V | head -n1)"

        if [ "$LOWER_VERSION" = "$MIN_VERSION" ] && [ "$INSTALLED_VERSION" != "$MIN_VERSION" ]; then
            # If the installed version is strictly greater than MIN_VERSION, it's fine
            # If it's equal, also fine. Only reinstall if installed < required
            echo "Installed Node.js is $INSTALLED_VERSION, which is >= $NODE_VERSION."
            echo "No installation needed."
        else
            echo "Installed Node.js version is $INSTALLED_VERSION, which is less than $NODE_VERSION."
            echo "Reinstalling to get Node.js >= $NODE_VERSION."
            install_node_18
        fi
    fi
else
    echo "npm (or node) not found. Installing Node.js >= $NODE_VERSION..."
    install_node_18
fi

echo "Done!"
