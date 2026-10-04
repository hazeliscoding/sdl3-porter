#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define MAX_PLAYERS 8

int main(int argc, char *argv[])
{
    SDL_Gamepad *players[MAX_PLAYERS];
    SDL_VirtualJoystickDesc desc;
    SDL_JoystickID virtual_id;
    SDL_JoystickID *gamepads;
    int num_gamepads, i, count = 0;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /* Stands in for a gamepad plugged in before launch. */
    SDL_INIT_INTERFACE(&desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
    desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    virtual_id = SDL_AttachVirtualJoystick(&desc);
    if (virtual_id == 0) {
        fprintf(stderr, "setup: SDL_AttachVirtualJoystick failed: %s\n", SDL_GetError());
        return 1;
    }

    /* Open every connected gamepad, one per player. */
    gamepads = SDL_GetGamepads(&num_gamepads);
    if (!gamepads) {
        fprintf(stderr, "setup: SDL_GetGamepads failed: %s\n", SDL_GetError());
        return 1;
    }
    for (i = 0; i < num_gamepads && count < MAX_PLAYERS; i++) {
        SDL_Gamepad *pad = SDL_OpenGamepad(gamepads[i]);
        if (pad) {
            players[count++] = pad;
        }
    }
    SDL_free(gamepads);

    if (!SDL_GetGamepadFromID(virtual_id)) {
        fprintf(stderr, "gamepad-index-vs-id: a gamepad is connected, but the loop didn't open it (%d opened)\n", count);
        return 1;
    }

    for (i = 0; i < count; i++) {
        SDL_CloseGamepad(players[i]);
    }
    SDL_DetachVirtualJoystick(virtual_id);
    SDL_Quit();
    return 0;
}
