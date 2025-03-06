#!/bin/bash
set -e

# -------------------------
# Configuration
# -------------------------
NODE_VERSION="v18.14.2"        # Which Node version to download from nodejs.org
INSTALL_DIR="./node_install"   # Where to extract the portable Node
DOWNLOAD_DIR="./node_download" # Where to save the tarball before extraction

# ----------------------------------------------------
# 0) Root check & distribution detection
# ----------------------------------------------------
if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
else
    if command -v sudo &>/dev/null; then
        SUDO="sudo"
    else
        echo "Error: sudo not found and not running as root."
        exit 1
    fi
fi

# OS release
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
    DISTRO_ID="unknown"
fi

# Check if musl or glibc
if ldd --version 2>&1 | grep -q "musl"; then
    echo "Detected musl-based system (Alpine/musl)."
    MUSL_SYSTEM=true
else
    echo "Detected glibc-based system."
    MUSL_SYSTEM=false
fi

# ----------------------------------------------------
# 1) Attempt portable tarball install from nodejs.org
# ----------------------------------------------------
install_node_via_tarball() {
    echo "Attempting to install Node.js $NODE_VERSION via official tarball..."
    # Dependencies: tar, curl (or wget). We'll assume tar is present; we can ensure curl below.

    # Install curl if missing (on Debian/Ubuntu)
    if ! command -v curl &>/dev/null; then
        if [[ "$DISTRO_ID" =~ ubuntu|debian ]]; then
            $SUDO apt-get update
            $SUDO apt-get install -y curl
        elif [[ "$DISTRO_ID" =~ fedora ]]; then
            $SUDO dnf install -y curl
        elif [[ "$DISTRO_ID" =~ centos|rhel|almalinux|rocky ]]; then
            $SUDO yum install -y curl
        elif [ "$DISTRO_ID" = "arch" ]; then
            $SUDO pacman -Sy --noconfirm curl
        elif [ "$DISTRO_ID" = "opensuse" ] || [[ "$DISTRO_ID" =~ sles ]]; then
            $SUDO zypper refresh
            $SUDO zypper install -y curl
        elif $MUSL_SYSTEM; then
            $SUDO apk update
            $SUDO apk add curl
        fi
    fi

    # Architecture detection
    MACHINE="$(uname -m | tr '[:upper:]' '[:lower:]')" # e.g. x86_64, aarch64
    case "$MACHINE" in
        x86_64|amd64)
            ARCH="x64"
            ;;
        *arm64*|*aarch64*)
            ARCH="arm64"
            ;;
        *armv7l*|*armv6l*)
            # Node has specific builds for armv7l, armv6l, etc.
            # We'll guess armv7l for 32-bit ARM:
            ARCH="armv7l"
            ;;
        *)
            echo "Unsupported architecture: $MACHINE"
            return 1
            ;;
    esac

    # If Linux, determine musl vs. glibc in the tarball name
    UNAME_SYS="$(uname | tr '[:upper:]' '[:lower:]')" # "linux" or "darwin"
    if [ "$UNAME_SYS" = "linux" ]; then
        if $MUSL_SYSTEM; then
            # Node official musl builds exist for Node >=16 on x64 & arm64
            TARBALL_OS="linux-musl"
        else
            TARBALL_OS="linux"
        fi
    elif [ "$UNAME_SYS" = "darwin" ]; then
        TARBALL_OS="darwin"
    else
        echo "This script currently supports Linux or Darwin (macOS) only."
        return 1
    fi

    TARBALL_BASENAME="node-$NODE_VERSION-$TARBALL_OS-$ARCH"
    TARBALL_FILE="$TARBALL_BASENAME.tar.gz"
    DOWNLOAD_URL="https://nodejs.org/dist/$NODE_VERSION/$TARBALL_FILE"

    mkdir -p "$DOWNLOAD_DIR" "$INSTALL_DIR"

    echo "Downloading $DOWNLOAD_URL ..."
    if ! curl -fSL "$DOWNLOAD_URL" -o "$DOWNLOAD_DIR/$TARBALL_FILE"; then
        echo "Failed to download $DOWNLOAD_URL"
        return 1
    fi

    echo "Extracting Node.js to $INSTALL_DIR ..."
    tar -xf "$DOWNLOAD_DIR/$TARBALL_FILE" -C "$INSTALL_DIR"

    # Our extracted folder will be: $INSTALL_DIR/node-v18.14.2-linux-x64, for example
    EXTRACTED_DIR="$INSTALL_DIR/$TARBALL_BASENAME"
    NODE_BIN="$EXTRACTED_DIR/bin/node"
    NPM_BIN="$EXTRACTED_DIR/bin/npm"

    if [ ! -x "$NODE_BIN" ] || [ ! -x "$NPM_BIN" ]; then
        echo "Missing node or npm in $EXTRACTED_DIR"
        return 1
    fi

    echo "Portable Node installed to: $EXTRACTED_DIR"
    echo "Added to PATH: $NODE_BIN"
    # add to PATH
    export PATH="$EXTRACTED_DIR/bin:$PATH"


    # Optionally create symlinks in /usr/local/bin or similar:
    # $SUDO ln -sf "$NODE_BIN" /usr/local/bin/node
    # $SUDO ln -sf "$NPM_BIN" /usr/local/bin/npm

    return 0
}

# ----------------------------------------------------
# 2) NodeSource (Node 18.x) as a fallback
# ----------------------------------------------------
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

    echo "Successfully installed Node.js 18.x from NodeSource."
    return 0
}

# ----------------------------------------------------
# 3) Fallback: System repos (older Node)
# ----------------------------------------------------
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

# ----------------------------------------------------
# 4) Main logic
# ----------------------------------------------------
install_npm() {
    # If npm is already on PATH, do nothing
    if command -v npm &>/dev/null; then
        echo "npm is already installed."
        return
    fi

    echo "npm not found. Trying portable tarball method first..."

    if install_node_via_tarball; then
        echo "Installed Node.js via tarball. Done."
    else
        echo "Tarball install failed or is unsupported. Attempting NodeSource next..."
        if install_node18_nodesource; then
            echo "Installed Node.js via NodeSource. Done."
        else
            echo "NodeSource failed or unsupported. Installing from system repos..."
            install_node_from_system
        fi
    fi
}

install_npm
