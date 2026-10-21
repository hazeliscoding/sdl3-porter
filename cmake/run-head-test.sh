#!/bin/bash
set -e
TEST_DIR="$1"
PYTHON_EXE="$2"
FIND_TRAPS_SCRIPT="$3"

# Setup the repo
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"
git init -q
echo "#include <SDL2/SDL.h>
int main() { if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1; SDL_Quit(); return 0; }" > main.c
git add main.c
git config user.name "Test User"
git config user.email "test@example.com"
git commit -q -m "Initial"
echo "#include <SDL2/SDL.h>
int main() { if (!SDL_Init(SDL_INIT_VIDEO)) return 1; SDL_Quit(); return 0; }" > main.c

# Run the search
"$PYTHON_EXE" "$FIND_TRAPS_SCRIPT" "$TEST_DIR"
