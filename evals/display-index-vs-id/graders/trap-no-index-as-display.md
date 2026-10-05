---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_(GetDisplay\w*|Get\w*DisplayMode\w*)\s*\(\s*(0|i)\s*[,)]'
match: not_contains
---
