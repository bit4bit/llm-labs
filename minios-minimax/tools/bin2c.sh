#!/bin/bash
# bin2c.sh - Convert binary file to C array
# Usage: ./bin2c.sh <input.bin> <array_name>

set -e

if [ $# -ne 2 ]; then
    echo "Usage: $0 <input.bin> <array_name>" >&2
    echo "Example: $0 hello.bin hello_bin" >&2
    exit 1
fi

INPUT="$1"
ARRAY_NAME="$2"

if [ ! -f "$INPUT" ]; then
    echo "Error: Input file '$INPUT' not found" >&2
    exit 1
fi

# Get file size
if [ "$(uname)" = "Darwin" ]; then
    SIZE=$(stat -f%z "$INPUT")
else
    SIZE=$(stat -c%s "$INPUT")
fi

# Generate C array declaration
echo "/* Auto-generated from $INPUT */"
echo "/* Size: $SIZE bytes */"
echo ""
echo "#include <stdint.h>"
echo ""
echo "uint8_t ${ARRAY_NAME}[] = {"

# Convert binary to hex bytes using xxd or od
if command -v xxd >/dev/null 2>&1; then
    # Use xxd if available (cleaner output)
    xxd -i < "$INPUT" | grep -v '^unsigned' | grep -v '^};' | sed 's/^//'
else
    # Fallback to od + formatting
    od -An -tx1 -v "$INPUT" | \
    sed 's/ \+/ /g' | \
    sed 's/^ //' | \
    while read line; do
        first=1
        for byte in $line; do
            if [ $first -eq 1 ]; then
                printf "  0x%s" "$byte"
                first=0
            else
                printf ", 0x%s" "$byte"
            fi
        done
        printf ",\n"
    done | sed '$ s/,$//'
fi

echo "};"
echo ""
echo "uint32_t ${ARRAY_NAME}_size = sizeof(${ARRAY_NAME});"

# Print success message to stderr
echo "Generated C array '${ARRAY_NAME}' with $SIZE bytes" >&2
