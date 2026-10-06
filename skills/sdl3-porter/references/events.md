# Event guidance

Read this when the project polls SDL events, which nearly every project does. Nothing here is a trap card: each section is a change that no headless fixture can reproduce, or that shows only in conditions too rare to be worth one, so check it by reading the code.

## Keeping event text past the poll cycle

**What changed:** in SDL2, `event.text.text` and `event.edit.text` were arrays inside the event, so a copy of the event, or a pointer into that copy, kept the text. In SDL3 they're `const char *` pointers to memory SDL owns, and so are `event.drop.data` and `event.drop.source`. SDL frees that memory the next time it pumps events: in the first `SDL_PollEvent` or `SDL_WaitEvent` of the next poll cycle, or in `SDL_PumpEvents`. Copying the event copies only the pointer. A port that keeps one compiles unchanged, then reads freed memory: garbage, text from a later event, or a crash.

**How to find it:** search the ported sources for the fields that point to event memory:

```
\.text\.text|\.edit\.text|\.drop\.(data|source)
```

For each hit, check whether the pointer, or an `SDL_Event` that holds it, outlives the current poll cycle: a global or `static`, a struct field, an event queue or list, or another thread. Using the text inside the `while (SDL_PollEvent(&event))` loop, or copying it there, is fine.

**Fix:** copy the text while handling the event:

```c
case SDL_EVENT_TEXT_INPUT:
    SDL_strlcat(input, event.text.text, sizeof input);
    break;
case SDL_EVENT_DROP_FILE:
    pending_file = SDL_strdup(event.drop.data);    /* SDL_free it after loading */
    break;
```

For events the project queues for later, copy each string when queuing and free the copy after processing.

**Source:** SDL `docs/README-migration.md`, `SDL_events.h` section: "Event memory is now managed by SDL, so you should not free the data in SDL_EVENT_DROP_FILE, and if you want to hold onto the text in SDL_EVENT_TEXT_EDITING and SDL_EVENT_TEXT_INPUT events, you should make a copy of it." When SDL frees it: `src/events/SDL_events.c` at `release-3.4.18`, where `SDL_PumpEventsInternal` starts with `SDL_FreeTemporaryMemory()`, "Free any temporary memory from old events".

## SDL_RegisterEvents failure value

**What changed:** SDL2's `SDL_RegisterEvents` returned `(Uint32)-1` when it couldn't allocate the event types. SDL3's returns 0. The function name and return type didn't change, so an SDL2 check like `if (type == (Uint32)-1)` or `if (type == 0xFFFFFFFF)` ports untouched and never fires. The program then uses event type 0, which no handler matches. SDL only runs out after about 32,000 user event types, so this rarely shows, but the check is dead code.

**How to find it:** search the ported sources for the call, then read the check after it:

```
SDL_RegisterEvents\s*\(
```

**Fix:** check for 0:

```c
Uint32 timer_event = SDL_RegisterEvents(1);
if (timer_event == 0) {
    /* handle the failure */
}
```

**Source:** SDL `docs/README-migration.md`, `SDL_events.h` section: "SDL_RegisterEvents() now returns 0 if it couldn't allocate any user events."
