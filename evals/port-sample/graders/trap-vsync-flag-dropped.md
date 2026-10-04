---
type: regex
target: {source: file, path: "src/main.c"}
pattern: 'SDL_SetRenderVSync\s*\([^;]*,\s*(1|-1|SDL_RENDERER_VSYNC_ADAPTIVE)\s*\)|SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER|SDL_HINT_RENDER_VSYNC|"SDL_RENDER_VSYNC"'
match: contains
---
