---
type: regex
target: {source: file, path: "src/audio.c"}
pattern: 'SDL_Resume(AudioStreamDevice|AudioDevice)\s*\(|SDL_BindAudioStreams?\s*\('
match: contains
---
