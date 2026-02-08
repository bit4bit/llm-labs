#!/usr/bin/env python3
"""
bin2c.py - Convert binary file to C array

Usage: ./bin2c.py <input.bin> <array_name>
"""

import sys
import os


def bin2c(input_file, array_name):
    """Convert binary file to C array format."""

    # Check if file exists
    if not os.path.isfile(input_file):
        print(f"Error: Input file '{input_file}' not found", file=sys.stderr)
        return 1

    # Get file size
    file_size = os.path.getsize(input_file)

    # Read binary data
    with open(input_file, "rb") as f:
        data = f.read()

    # Generate C array header
    print(f"/* Auto-generated from {input_file} */")
    print(f"/* Size: {file_size} bytes */")
    print()
    print("#include <stdint.h>")
    print()
    print(f"uint8_t {array_name}[] = {{")

    # Convert bytes to hex format (16 bytes per line)
    bytes_per_line = 16
    for i in range(0, len(data), bytes_per_line):
        line_bytes = data[i : i + bytes_per_line]
        hex_values = ", ".join(f"0x{b:02X}" for b in line_bytes)

        # Add trailing comma except for last line
        if i + bytes_per_line < len(data):
            print(f"    {hex_values},")
        else:
            print(f"    {hex_values}")

    print("};")
    print()
    print(f"uint32_t {array_name}_size = sizeof({array_name});")

    # Print status to stderr
    print(f"Generated C array '{array_name}' with {file_size} bytes", file=sys.stderr)

    return 0


def main():
    if len(sys.argv) != 3:
        print("Usage: bin2c.py <input.bin> <array_name>", file=sys.stderr)
        print("Example: bin2c.py hello.bin hello_bin", file=sys.stderr)
        return 1

    input_file = sys.argv[1]
    array_name = sys.argv[2]

    return bin2c(input_file, array_name)


if __name__ == "__main__":
    sys.exit(main())
