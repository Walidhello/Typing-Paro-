#!/bin/sh

# Detect architecture to set correct Homebrew paths
if [ "$(uname -m)" = "arm64" ]; then
    BREW_PATH="/opt/homebrew"
else
    BREW_PATH="/usr/local"
fi

# Compile the project
# Added '-Iinclude' so the compiler can find enemy.h, game.h, etc.
clang src/*.c -o game \
    -I"$BREW_PATH/include" \
    -Iinclude \
    -L"$BREW_PATH/lib" \
    -lraylib \
    -framework OpenGL \
    -framework Cocoa \
    -framework IOKit \
    -framework CoreVideo

# Check if compilation succeeded before running
if [ $? -eq 0 ]; then
    echo "Build successful! Running game..."
    ./game
else
    echo "Build failed."
fi