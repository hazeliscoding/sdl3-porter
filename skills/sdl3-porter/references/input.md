# Input traps

Read this when the project uses the mouse, the keyboard, text input, joysticks or gamepads. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime. The guidance sections after the traps are changes no headless fixture can reproduce, so check them by reading the code.

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

**Source:** SDL `docs/README-migration.md`, `SDL_joystick.h` section: "SDL_JoystickID has changed from Sint32 to Uint32, with an invalid ID being 0", "Rather than iterating over joysticks using device index, there is a new function SDL_GetJoysticks()" and "SDL_AttachVirtualJoystick() now returns the joystick instance ID instead of a device index". `SDL_gamecontroller.h` section: "The SDL_EVENT_GAMEPAD_ADDED event now provides the joystick instance ID in the which member". The shared counter comes from SDL's source at `release-3.4.18`, not from the docs: joystick drivers take IDs from `SDL_GetNextObjectID()` in `src/SDL_utils.c`, which `src/video/SDL_video.c`, `src/timer/SDL_timer.c` and the sensor and haptic drivers use too.

**Fixture:** `fixtures/gamepad-index-vs-id/`.

## text-input-off

**What compiles:**

```c
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_TEXT_INPUT) {    /* never arrives */
        SDL_strlcat(name, event.text.text, sizeof name);
    }
}
```

**What breaks:** SDL2 turned text input on when video started, on every platform except the 3DS and PSP, so many programs read text events without ever calling `SDL_StartTextInput`. SDL3 starts with text input off, and keeps it per window. Until the app calls `SDL_StartTextInput(window)`, typing still produces key events, but no `SDL_EVENT_TEXT_INPUT` or `SDL_EVENT_TEXT_EDITING` events. Name entry, chat boxes and consoles stay empty. Code that did call `SDL_StartTextInput()` in SDL2 doesn't compile until it passes a window, so it gets a second look anyway.

**How to find it:** search the ported sources for text events, and for the calls that control text input:

```
SDL_EVENT_TEXT_(INPUT|EDITING)|\.text\.text|\.edit\.text
SDL_(Start|Stop)TextInput|SDL_TextInputActive
```

If the project reads text events, check that it calls `SDL_StartTextInput` for each window that should receive them, before the user types. A project that never called `SDL_StartTextInput` in SDL2 relied on the default, and needs the call added.

**Fix:** turn text input on for the window while the app wants text, and off again afterwards:

```c
SDL_StartTextInput(window);    /* when a text field gets focus */
/* ... */
SDL_StopTextInput(window);     /* when it loses focus */
```

The migration guide warns that starting text input may show an input method editor (IME) and skip key events, so prefer turning it on only around text fields. To keep SDL2's always-on behavior, call `SDL_StartTextInput(window)` once after creating the window.

**Source:** SDL `docs/README-migration.md`, `SDL_keyboard.h` section: "Text input is no longer automatically enabled when initializing video, you should call SDL_StartTextInput() when you want to receive text input and call SDL_StopTextInput() when you are done. Starting text input may shown an input method editor (IME) and cause key up/down events to be skipped, so should only be enabled when the application wants text input." SDL's `src/events/SDL_keyboard.c` at `release-3.4.18` drops text and editing events unless `SDL_TextInputActive` is true for the focused window.

**Fixture:** `fixtures/text-input-off/`.

## mouse-logical-coords

**What compiles:**

```c
SDL_SetRenderLogicalPresentation(renderer, 320, 240, SDL_LOGICAL_PRESENTATION_LETTERBOX);
/* ... */
case SDL_EVENT_MOUSE_BUTTON_DOWN:
    click_at(event.button.x, event.button.y);    /* window coordinates, not logical */
    break;
```

**What breaks:** in SDL2, a renderer with a logical size from `SDL_RenderSetLogicalSize` rewrote mouse, wheel and touch events into logical coordinates before the app saw them. SDL3 doesn't, so those events arrive in window coordinates. Code that hit-tests clicks against things drawn in logical coordinates misses as soon as the window isn't at the logical size: in a 640×480 window showing 320×240, a click on a button at (120, 120) arrives as (240, 240). Relative motion isn't scaled either.

**How to find it:** search the ported sources for a logical size, and for coordinates read from mouse, wheel and touch events:

```
SDL_SetRenderLogicalPresentation|SDL_RenderSetLogicalSize
\b(button|motion|wheel|tfinger)\.(x|y|xrel|yrel|mouse_x|mouse_y)\b
```

If the project sets a logical size, find every place it compares an event's coordinates with positions in logical space: buttons, menus, tiles, aiming. `SDL_GetMouseState` was never converted, even in SDL2, so code that already converts its result is fine.

**Fix:** convert each event before using its coordinates:

```c
case SDL_EVENT_MOUSE_BUTTON_DOWN:
    SDL_ConvertEventToRenderCoordinates(renderer, &event);
    click_at(event.button.x, event.button.y);
    break;
```

For a point that doesn't come from an event, use `SDL_RenderCoordinatesFromWindow`.

**Source:** SDL `docs/README-migration.md`, `SDL_render.h` section: "Mouse and touch events are no longer filtered to change their coordinates, instead you can call SDL_ConvertEventToRenderCoordinates() to explicitly map event coordinates into the rendering viewport." `SDL_hints.h` section: "SDL_HINT_MOUSE_RELATIVE_SCALING - mouse coordinates are no longer automatically scaled by the SDL renderer". SDL2's conversion is `SDL_RendererEventWatch` in `src/render/SDL_render.c` at `release-2.32.10`, which rescaled mouse, wheel and touch events while a logical size was set.

**Fixture:** `fixtures/mouse-logical-coords/`.

## Nintendo face buttons

**What changed:** SDL2 reported the face buttons of Nintendo controllers by label by default (`SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS` was on), so `SDL_CONTROLLER_BUTTON_A` was the button printed A, on the right of a Switch controller. The rename scripts turn A/B/X/Y into `SDL_GAMEPAD_BUTTON_SOUTH`/`EAST`/`WEST`/`NORTH`, which are positions, and SDL3 ignores the hint. On a Nintendo controller, a port that confirms with `SOUTH` now confirms with the bottom button, printed B, and the button printed A cancels. Xbox and PlayStation layouts are unchanged.

**How to find it:** search the ported sources for the face buttons:

```
SDL_GAMEPAD_BUTTON_(SOUTH|EAST|WEST|NORTH)\b
```

If the project handles them, it has this change.

**Fix:** the guide recommends a setting that swaps South and East, defaulting to swapped when `SDL_GetGamepadButtonLabel(gamepad, SDL_GAMEPAD_BUTTON_SOUTH)` is `SDL_GAMEPAD_BUTTON_LABEL_B`, which keeps SDL2's behavior on Nintendo controllers. Whether to add that is the owner's call, so report it under `Needs a human:` instead of changing the controls on your own.

**Source:** SDL `docs/README-migration.md`, `SDL_gamecontroller.h` section: "The gamepad face buttons have been renamed from A/B/X/Y to North/South/East/West to indicate that they are positional rather than hardware-specific. [...] The hint SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS is ignored [...] Applications should provide a way for users to swap between South/East as their accept/cancel buttons", followed by example code. SDL2's default is in `SDL_hints.h` at `release-2.32.10`: "The default value is "1"."

## Mouse wheel values

**What changed:** SDL2's `event.wheel.y` was an `Sint32` count of whole scroll steps. SDL added up the small amounts that touchpads and smooth-scrolling wheels send until they made a whole step, and put the raw amount in `preciseY`. SDL3 drops `preciseX` and `preciseY` and makes `x` and `y` the raw `float` amounts. Code that does one thing per event where `y > 0`, such as zoom one level or select the next item, now does it for every fraction, so one swipe on a touchpad scrolls through a whole list. Code that stores `y` in an `int` drops the fractions, so slow scrolling does nothing. A wheel that clicks in whole steps behaves as before.

**How to find it:** search the ported sources for wheel amounts:

```
\.wheel\.(x|y)\b
```

**Fix:** for one step per wheel click, read `event.wheel.integer_x` and `event.wheel.integer_y`, which SDL3 adds up into whole steps the way SDL2's `x` and `y` were. They exist since SDL 3.2.12, so a project that must build against an older SDL3 has to add up `y` itself. For smooth scrolling, keep `y` and use it as a `float`.

**Source:** SDL `SDL_MouseWheelEvent` in `SDL_events.h` at `release-3.4.18` ([wiki](https://wiki.libsdl.org/SDL3/SDL_MouseWheelEvent)): `float y`, and `Sint32 integer_y`, "The amount scrolled vertically, accumulated to whole scroll "ticks" (added in 3.2.12)". SDL2's struct, in `SDL_events.h` at `release-2.32.10`, has `Sint32 y` and `float preciseY`, and `SDL_SendMouseWheel` in `src/events/SDL_mouse.c` added up the whole steps.

## Gamepad rumble

**What changed:** SDL2 programs often rumbled a gamepad through the haptic API: `SDL_HapticOpenFromJoystick`, then `SDL_HapticRumbleInit` and `SDL_HapticRumblePlay`. The rename scripts keep that code, as `SDL_OpenHapticFromJoystick`, `SDL_InitHapticRumble` and `SDL_PlayHapticRumble`. In SDL3, gamepads that can only rumble aren't haptic devices any more, so `SDL_OpenHapticFromJoystick` fails for them, and a program that treats that as "this pad can't rumble" never rumbles.

**How to find it:** search the ported sources for haptic calls:

```
SDL_(OpenHapticFromJoystick|IsJoystickHaptic|InitHapticRumble|PlayHapticRumble|HapticRumbleSupported)\s*\(
```

**Fix:** rumble through the gamepad, and keep the haptic API for devices with real force feedback:

```c
SDL_RumbleGamepad(gamepad, (Uint16)(strength * 0xFFFF), (Uint16)(strength * 0xFFFF), duration_ms);
```

`SDL_PlayHapticRumble` took a strength from 0 to 1, and `SDL_RumbleGamepad` takes one for each motor, from 0 to 0xFFFF. For a joystick that isn't a gamepad, use `SDL_RumbleJoystick`.

**Source:** SDL `docs/README-migration.md`, `SDL_haptic.h` section: "Gamepads with simple rumble capability no longer show up in the SDL haptics interface, instead you should use SDL_RumbleGamepad()."
