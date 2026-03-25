#!/bin/bash

echo "=== Building DLR for x86_64 ==="

mkdir -p release

# Build for x86_64 - using standard C library
gcc -static -Os -o release/dlr.x86_64 main3.c

if [ $? -eq 0 ]; then
    echo "[OK] dlr.x86_64 built"
    ls -lh release/dlr.x86_64
    
    # Check if it's static
    file release/dlr.x86_64 | grep static
    
    # Check size
    echo ""
    echo "Binary size: $(du -h release/dlr.x86_64 | cut -f1)"
else
    echo "[FAIL] Build failed"
    exit 1
fi