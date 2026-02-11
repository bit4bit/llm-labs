#!/bin/bash
# bin2c.sh - Wrapper for bin2c.rb
# Usage: ./bin2c.sh <input.bin> <array_name>

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

exec ruby "$SCRIPT_DIR/bin2c.rb" "$@"
