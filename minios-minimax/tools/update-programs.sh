#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== MiniOS Minimax - User Programs Update Script ==="
echo ""

check_command() {
    if command -v "$1" &> /dev/null; then
        echo "[OK] $1: $(command -v $1)"
        return 0
    else
        echo "[MISSING] $1 - Not found"
        return 1
    fi
}

echo "Checking dependencies..."
echo ""

MISSING_DEPS=0

if ! check_command gcc; then
    MISSING_DEPS=1
fi

if ! check_command ld; then
    MISSING_DEPS=1
fi

if ! check_command objcopy; then
    MISSING_DEPS=1
fi

if ! check_command make; then
    MISSING_DEPS=1
fi

if ! check_command ruby; then
    MISSING_DEPS=1
fi

if ! check_command python3; then
    MISSING_DEPS=1
fi

echo ""

if [ $MISSING_DEPS -eq 1 ]; then
    echo "Some dependencies are missing. Installing..."

    if command -v apt-get &> /dev/null; then
        echo "Installing dependencies via apt-get..."
        sudo apt-get update
        sudo apt-get install -y gcc gcc-multilib g++ make python3 binutils
    elif command -v dnf &> /dev/null; then
        echo "Installing dependencies via dnf..."
        sudo dnf install -y gcc gcc-c++ make python3 binutils
    elif command -v pacman &> /dev/null; then
        echo "Installing dependencies via pacman..."
        sudo pacman -S --noconfirm gcc make python3 binutils
    elif command -v yum &> /dev/null; then
        echo "Installing dependencies via yum..."
        sudo yum install -y gcc gcc-c++ make python3 binutils
    else
        echo "ERROR: Could not detect a supported package manager."
        echo "Please install the following dependencies manually:"
        echo "  - gcc (with 32-bit support)"
        echo "  - binutils (ld, objcopy)"
        echo "  - make"
        echo "  - ruby"
        echo "  - python3"
        exit 1
    fi
fi

echo ""
echo "Building user programs..."
echo ""

cd "$SCRIPT_DIR"

if ! ruby build-programs.rb --all; then
    echo "ERROR: Failed to build programs"
    exit 1
fi

echo ""
echo "=== Update Complete ==="
echo ""

echo "Generated files:"
ls -la "$PROJECT_ROOT/programs/generated/"
echo ""
echo "Programs built:"
ls -la "$PROJECT_ROOT/programs/src"/*/*.bin 2>/dev/null || echo "  (no .bin files found)"
echo ""
echo "User programs are ready to be embedded in the kernel."
echo "Run 'make all' to build the complete kernel with programs."
