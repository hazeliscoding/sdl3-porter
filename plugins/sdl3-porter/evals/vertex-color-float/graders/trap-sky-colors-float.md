---
type: regex
target: {source: file, path: "src/main.c"}
pattern: '\{\s*\{[^{}]*\}\s*,\s*\{\s*([2-9][0-9]|1[0-9][0-9]|2[0-4][0-9])\s*,'
match: not_contains
---
