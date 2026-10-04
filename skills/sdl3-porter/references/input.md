# Input traps

Read this when the project uses the keyboard, text input, joysticks or gamepads. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

## gamepad-index-vs-id

**What compiles:**

```c
int count;
SDL_JoystickID *joysticks = SDL_GetJoysticks(&count);    /* replaced SDL_NumJoysticks() */
for (int i = 0; i < count; i++) {
    if (SDL_IsGamepad(i)) {                  /* was SDL_IsGameController(i) */
        pads[n++] = SDL_OpenGamepad(i);      /* was SDL_GameControllerOpen(i) */
    }
}
SDL_free(joysticks);
```

**What breaks:** SDL2 referred to devices that weren't open yet by device index, from 0 to `SDL_NumJoysticks() - 1`. SDL3 has no device indexes. The functions that took one now take an instance ID, and the rename scripts rename them without touching the argument. C converts an `int` index to the unsigned `SDL_JoystickID` without a word. SDL3 never uses 0 as an ID, so index 0 matches nothing, and any other index matches nothing or an unrelated device. The loop opens no gamepad, or the wrong one.

Instance IDs changed too. SDL2 numbered joysticks from 0 with a counter of their own, so code could get away with using an instance ID as an array index. SDL3 takes IDs from one counter shared with windows, timers, sensors and other devices, so they don't start at 0 or count up from it. An array indexed by an instance ID, or by the `which` of a device-added event, which was a device index in SDL2, now overflows or mixes up players.

**How to find it:** search the ported sources for the renamed functions that took a device index in SDL2, and for device-added events:

```
SDL_(IsGamepad|OpenGamepad|OpenJoystick|IsJoystickVirtual|DetachVirtualJoystick|OpenHaptic|OpenSensor)\b
SDL_EVENT_(GAMEPAD|JOYSTICK)_ADDED|\b[cgj]device\.which\b
```

For each call, check where the argument comes from. A loop counter, a count, or a number saved from SDL2 code is an index and needs replacing. An ID from `SDL_GetJoysticks`, `SDL_GetGamepads` or an event's `which` is right. For each `which`, check it's used as an ID and not as an index into an array or a player slot.

**Fix:** loop over the IDs SDL3 returns:

```c
int count;
SDL_JoystickID *gamepads = SDL_GetGamepads(&count);
if (gamepads) {
    for (int i = 0; i < count; i++) {
        pads[n++] = SDL_OpenGamepad(gamepads[i]);
    }
    SDL_free(gamepads);
}
```

To map gamepads to player slots, store the ID in the slot and search the slots for it, or use `SDL_GetGamepadFromID`, instead of indexing by the ID.

**Source:** SDL `docs/README-migration.md`, `SDL_joystick.h` section: "SDL_JoystickID has changed from Sint32 to Uint32, with an invalid ID being 0", "Rather than iterating over joysticks using device index, there is a new function SDL_GetJoysticks()" and "SDL_AttachVirtualJoystick() now returns the joystick instance ID instead of a device index". `SDL_gamecontroller.h` section: "The SDL_EVENT_GAMEPAD_ADDED event now provides the joystick instance ID in the which member". The shared counter comes from SDL's source at `release-3.4.16`, not from the docs: joystick drivers take IDs from `SDL_GetNextObjectID()` in `src/SDL_utils.c`, which `src/video/SDL_video.c`, `src/timer/SDL_timer.c` and the sensor and haptic drivers use too.

**Fixture:** `fixtures/gamepad-index-vs-id/`.
