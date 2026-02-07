#!/bin/bash
# test-programs.sh - Test user program compilation and verification
# Usage: ./tools/test-programs.sh

set -e

echo "======================================"
echo "MiniOS User Programs - Test Suite"
echo "======================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0

# Helper function to print test results
test_passed() {
    echo -e "${GREEN}✓ PASS${NC}: $1"
    TESTS_PASSED=$((TESTS_PASSED + 1))
}

test_failed() {
    echo -e "${RED}✗ FAIL${NC}: $1"
    TESTS_FAILED=$((TESTS_FAILED + 1))
}

test_info() {
    echo -e "${YELLOW}ℹ INFO${NC}: $1"
}

# Change to project root
cd "$(dirname "$0")/.."

echo "Test 1: Check required files exist"
echo "-----------------------------------"

if [ -f "programs/lib/syscall.h" ]; then
    test_passed "programs/lib/syscall.h exists"
else
    test_failed "programs/lib/syscall.h missing"
fi

if [ -f "programs/lib/stdint.h" ]; then
    test_passed "programs/lib/stdint.h exists"
else
    test_failed "programs/lib/stdint.h missing"
fi

if [ -f "programs/user.ld" ]; then
    test_passed "programs/user.ld exists"
else
    test_failed "programs/user.ld missing"
fi

if [ -f "programs/hello/hello.c" ]; then
    test_passed "programs/hello/hello.c exists"
else
    test_failed "programs/hello/hello.c missing"
fi

if [ -f "tools/bin2c.sh" ]; then
    test_passed "tools/bin2c.sh exists"
else
    test_failed "tools/bin2c.sh missing"
fi

echo ""
echo "Test 2: Build user programs"
echo "-----------------------------------"

if make -C programs clean > /dev/null 2>&1; then
    test_passed "Clean successful"
else
    test_failed "Clean failed"
fi

if make -C programs > /dev/null 2>&1; then
    test_passed "Build successful"
else
    test_failed "Build failed"
    echo "Error output:"
    make -C programs
    exit 1
fi

echo ""
echo "Test 3: Verify binary output"
echo "-----------------------------------"

if [ -f "programs/hello/hello.bin" ]; then
    test_passed "hello.bin created"

    SIZE=$(stat -c%s "programs/hello/hello.bin" 2>/dev/null || stat -f%z "programs/hello/hello.bin" 2>/dev/null)
    test_info "Binary size: $SIZE bytes"

    if [ "$SIZE" -lt 1024 ]; then
        test_passed "Binary size is reasonable (< 1KB)"
    else
        test_failed "Binary size too large (>= 1KB)"
    fi

    if [ "$SIZE" -gt 0 ]; then
        test_passed "Binary is not empty"
    else
        test_failed "Binary is empty"
    fi
else
    test_failed "hello.bin not created"
fi

echo ""
echo "Test 4: Generate C arrays"
echo "-----------------------------------"

if make -C programs generated > /dev/null 2>&1; then
    test_passed "Generated C arrays"
else
    test_failed "Failed to generate C arrays"
fi

if [ -f "programs/generated/hello_bin.c" ]; then
    test_passed "hello_bin.c created"

    # Check if it contains the expected array
    if grep -q "uint8_t hello_bin\[\]" "programs/generated/hello_bin.c"; then
        test_passed "Contains hello_bin array declaration"
    else
        test_failed "Missing hello_bin array declaration"
    fi

    if grep -q "uint32_t hello_bin_size" "programs/generated/hello_bin.c"; then
        test_passed "Contains hello_bin_size variable"
    else
        test_failed "Missing hello_bin_size variable"
    fi
else
    test_failed "hello_bin.c not created"
fi

echo ""
echo "Test 5: Verify binary content"
echo "-----------------------------------"

if [ -f "programs/hello/hello.bin" ]; then
    # Check for syscall instruction (int 0x80 = CD 80)
    if hexdump -C "programs/hello/hello.bin" | grep -q "cd 80"; then
        test_passed "Binary contains syscall instruction (int 0x80)"
    else
        test_failed "Binary missing syscall instruction"
    fi

    # Check for the message string "Hello from C program!"
    if strings "programs/hello/hello.bin" | grep -q "Hello from C program"; then
        test_passed "Binary contains expected message string"
    else
        test_failed "Binary missing expected message string"
    fi

    # Check that first bytes are not all zeros
    FIRST_BYTE=$(hexdump -n 1 -e '"%02X"' "programs/hello/hello.bin")
    if [ "$FIRST_BYTE" != "00" ]; then
        test_passed "Binary starts with non-zero byte"
    else
        test_failed "Binary starts with zero (might be empty)"
    fi
fi

echo ""
echo "Test 6: Test bin2c.sh tool"
echo "-----------------------------------"

if [ -x "tools/bin2c.sh" ]; then
    test_passed "bin2c.sh is executable"

    # Create a test binary
    echo -n -e '\x48\x65\x6C\x6C\x6F' > /tmp/test_minios.bin

    if ./tools/bin2c.sh /tmp/test_minios.bin test_array > /tmp/test_minios_array.c 2>/dev/null; then
        test_passed "bin2c.sh can convert binary to C array"

        if grep -q "uint8_t test_array\[\]" /tmp/test_minios_array.c; then
            test_passed "Generated array has correct name"
        else
            test_failed "Generated array has incorrect name"
        fi

        if grep -q "0x48" /tmp/test_minios_array.c; then
            test_passed "Generated array contains hex values"
        else
            test_failed "Generated array missing hex values"
        fi
    else
        test_failed "bin2c.sh failed to convert binary"
    fi

    # Cleanup
    rm -f /tmp/test_minios.bin /tmp/test_minios_array.c
else
    test_failed "bin2c.sh is not executable"
fi

echo ""
echo "Test 7: Check syscall.h syntax"
echo "-----------------------------------"

# Try to compile a minimal test program
cat > /tmp/test_syscall.c << 'EOF'
#include "programs/lib/syscall.h"

void _start(void) {
    write(1, "test", 4);
    exit(0);
}
EOF

if gcc -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -fno-builtin -nostdinc -I. -c /tmp/test_syscall.c -o /tmp/test_syscall.o 2>/dev/null; then
    test_passed "syscall.h compiles without errors"
    rm -f /tmp/test_syscall.o
else
    test_failed "syscall.h has compilation errors"
fi

rm -f /tmp/test_syscall.c

echo ""
echo "======================================"
echo "Test Results"
echo "======================================"
echo -e "Passed: ${GREEN}${TESTS_PASSED}${NC}"
echo -e "Failed: ${RED}${TESTS_FAILED}${NC}"
echo "Total:  $((TESTS_PASSED + TESTS_FAILED))"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC} ✨"
    echo ""
    echo "Next steps:"
    echo "  1. Include programs/generated/hello_bin.c in your kernel"
    echo "  2. Copy hello_bin[] to user space (0x40000000)"
    echo "  3. Create and run the process"
    echo ""
    exit 0
else
    echo -e "${RED}Some tests failed.${NC} Please review the errors above."
    exit 1
fi
