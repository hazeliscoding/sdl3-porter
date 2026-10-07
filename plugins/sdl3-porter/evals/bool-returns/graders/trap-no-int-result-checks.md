---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_(Init|SetWindowFullscreen|SetRenderDrawColor|RenderClear)\s*\([^;]*\)\s*(<|<=|>|>=|==|!=)\s*-?[0-9]'
match: not_contains
---
