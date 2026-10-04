---
type: regex
target: {source: file, path: "src/audio.c"}
pattern: 'SDL_MIX_MAXVOLUME|SDL_MixAudio\s*\([^;]*,\s*([2-9]|[1-9][0-9]+)(\s*/\s*[0-9]+)?\s*\)\s*;'
match: not_contains
---
