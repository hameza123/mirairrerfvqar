#!/bin/bash

echo "=== Building DLR for x86_64 ==="

mkdir -p release

# Build for x86_64
gcc -Os -D BOT_ARCH=\"x86_64\" -D X64 -Wl,--gc-sections -fdata-sections -ffunction-sections \
    -e __start -nostartfiles -static main.c -o release/dlr.x86_64

if [ $? -eq 0 ]; then
    echo "[OK] dlr.x86_64 built"
    ls -lh release/dlr.x86_64
else
    echo "[FAIL] Build failed"
fi