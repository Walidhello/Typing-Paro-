#!/bin/sh

# Detect OS and architecture to set correct paths and flags
OS="$(uname -s)"
ARCH="$(uname -m)"

if [ "$OS" = "Darwin" ]; then
    # macOS setup
    if [ "$ARCH" = "arm64" ]; then
        BREW_PATH="/opt/homebrew"
    else
        BREW_PATH="/usr/local"
    fi
    
    COMPILER="clang"
    FLAGS="-I$BREW_PATH/include -L$BREW_PATH/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"
else
    # Linux setup (Ubuntu/Debian/etc.)
    COMPILER="gcc"
    # Linux standard Raylib linking requirements
    FLAGS="-lraylib -lGL -lm -lpthread -ldl -lrt -lX11"
fi

# Compile the project
$COMPILER src/*.c -o game -Iinclude $FLAGS

# Check if compilation succeeded before running
if [ $? -eq 0 ]; then
    echo "Build successful! Running game..."
    ./game
else
    echo "Build failed."
fi