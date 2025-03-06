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

install_node_18_nodesource() {
    # Attempt NodeSource 18.x
    if [[ "$DISTRO_ID" =~ ubuntu|debian ]]; then
        $SUDO apt-get update
        $SUDO apt-get install -y curl ca-certificates
        curl -fsSL https://deb.nodesource.com/setup_18.x | $SUDO bash - || return 1
        $SUDO apt-get install -y nodejs || return 1
    elif [[ "$DISTRO_ID" =~ fedora ]]; then
        $SUDO dnf install -y curl
        curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash - || return 1
        $SUDO dnf install -y nodejs || return 1
    elif [[ "$DISTRO_ID" =~ centos|rhel|almalinux|rocky ]]; then
        $SUDO yum install -y curl
        curl -fsSL https://rpm.nodesource.com/setup_18.x | $SUDO bash - || return 1
        $SUDO yum install -y nodejs || return 1
    else
        return 1  # Means "we don’t know how to handle $DISTRO_ID here"
    fi
}

install_node_16_nodesource() {
    # Attempt NodeSource 16.x
    if [[ "$DISTRO_ID" =~ ubuntu|debian ]]; then
        $SUDO apt-get update
        $SUDO apt-get install -y curl ca-certificates
        curl -fsSL https://deb.nodesource.com/setup_16.x | $SUDO bash - || return 1
        $SUDO apt-get install -y nodejs || return 1
    elif [[ "$DISTRO_ID" =~ fedora ]]; then
        $SUDO dnf install -y curl
        curl -fsSL https://rpm.nodesource.com/setup_16.x | $SUDO bash - || return 1
        $SUDO dnf install -y nodejs || return 1
    elif [[ "$DISTRO_ID" =~ centos|rhel|almalinux|rocky ]]; then
        $SUDO yum install -y curl
        curl -fsSL https://rpm.nodesource.com/setup_16.x | $SUDO bash - || return 1
        $SUDO yum install -y nodejs || return 1
    else
        return 1
    fi
}

install_node_via_system_repos() {
    # Fallback to system repositories (likely older)
    if $MUSL_SYSTEM; then
        # Alpine
        $SUDO apk update
        $SUDO apk add --no-cache nodejs npm || return 1
    else
        case "$DISTRO_ID" in
            ubuntu|debian)
                $SUDO apt-get update
                $SUDO apt-get install -y nodejs npm || return 1
                ;;
            fedora)
                $SUDO dnf install -y nodejs npm || return 1
                ;;
            centos|rhel|almalinux|rocky)
                $SUDO yum install -y nodejs npm || return 1
                ;;
            opensuse*|sles)
                $SUDO zypper refresh
                $SUDO zypper install -y nodejs npm || return 1
                ;;
            arch)
                $SUDO pacman -Sy --noconfirm nodejs npm || return 1
                ;;
            *)
                # If ID_LIKE can help us guess
                if [[ "$DISTRO_FAMILY" =~ debian ]]; then
                    $SUDO apt-get update
                    $SUDO apt-get install -y nodejs npm || return 1
                elif [[ "$DISTRO_FAMILY" =~ rhel|fedora ]]; then
                    $SUDO yum install -y nodejs npm || return 1
                else
                    return 1
                fi
                ;;
        esac
    fi
}

check_or_install_node() {
    echo "Trying NodeSource for Node >= 18..."
    if install_node_18_nodesource; then
        echo "Node 18.x installation successful."
        return 0
    else
        echo "Node 18.x installation failed. Trying Node 16.x..."
        if install_node_16_nodesource; then
            echo "Node 16.x installation successful."
            return 0
        else
            echo "Node 16.x installation failed. Falling back to system repos..."
            if install_node_via_system_repos; then
                echo "Installed Node from system repos."
                return 0
            else
                echo "All attempts to install Node.js have failed."
                return 1
            fi
        fi
    fi
}

# If node is installed, check if it's >= 18.14.2. Otherwise, run fallback logic:
REQUIRED_NODE_VERSION="18.14.2"
if command -v node >/dev/null 2>&1; then
    CURRENT_NODE_VERSION="$(node -v 2>/dev/null | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')"
    if [ -n "$CURRENT_NODE_VERSION" ]; then
        # Compare versions with sort -V
        LOWEST="$(printf '%s\n' "$REQUIRED_NODE_VERSION" "$CURRENT_NODE_VERSION" | sort -V | head -n1)"
        if [ "$LOWEST" = "$REQUIRED_NODE_VERSION" ] && [ "$CURRENT_NODE_VERSION" != "$REQUIRED_NODE_VERSION" ]; then
            echo "Detected Node.js $CURRENT_NODE_VERSION which is >= $REQUIRED_NODE_VERSION."
            echo "No new installation needed."
            exit 0
        else
            echo "Detected Node.js $CURRENT_NODE_VERSION which is < $REQUIRED_NODE_VERSION."
            echo "Attempting to install newer or fallback Node."
            check_or_install_node
        fi
    else
        echo "Node found, but version could not be parsed. Reinstalling."
        check_or_install_node
    fi
else
    echo "No node found. Attempting to install Node >= $REQUIRED_NODE_VERSION or fallback..."
    check_or_install_node
fi
