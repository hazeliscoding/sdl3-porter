---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_(IsGamepad|OpenGamepad|OpenJoystick|GetGamepad\w*ForID|GetJoystick\w*ForID)\s*\(\s*i\s*\)'
match: not_contains
---
