---
type: regex
target: {source: file, path: "src/main.c"}
pattern: '#include\s*[<"](?:SDL2/)?SDL[_a-z0-9]*\.h[>"]'
match: not_contains
---
