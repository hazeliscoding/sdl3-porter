---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_Resume(AudioStreamDevice|AudioDevice)\s*\(|SDL_BindAudioStreams?\s*\('
match: contains
---
