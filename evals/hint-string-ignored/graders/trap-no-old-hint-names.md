---
type: regex
target: {source: file, path: "src/main.c"}
pattern: '"SDL_(RENDER_SCALE_QUALITY|ALLOW_TOPMOST)"'
match: not_contains
---
