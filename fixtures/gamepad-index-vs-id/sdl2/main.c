#include <SDL.h>
#include <stdio.h>

#define MAX_PLAYERS 8

int main(int argc, char *argv[])
{
    SDL_GameController *players[MAX_PLAYERS];
    SDL_VirtualJoystickDesc desc;
    SDL_JoystickID virtual_id;
    int virtual_index, i, count = 0;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "setup: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /* Stands in for a gamepad plugged in before launch. */
    SDL_zero(desc);
    desc.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
    desc.type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
    desc.naxes = SDL_CONTROLLER_AXIS_MAX;
    desc.nbuttons = SDL_CONTROLLER_BUTTON_MAX;
    virtual_index = SDL_JoystickAttachVirtualEx(&desc);
    if (virtual_index < 0) {
        fprintf(stderr, "setup: SDL_JoystickAttachVirtualEx failed: %s\n", SDL_GetError());
        return 1;
    }
    virtual_id = SDL_JoystickGetDeviceInstanceID(virtual_index);

    /* Open every connected gamepad, one per player. SDL2 takes device indexes here. */
    for (i = 0; i < SDL_NumJoysticks() && count < MAX_PLAYERS; i++) {
        if (SDL_IsGameController(i)) {
            SDL_GameController *pad = SDL_GameControllerOpen(i);
            if (pad) {
                players[count++] = pad;
            }
        }
    }

    if (!SDL_GameControllerFromInstanceID(virtual_id)) {
        fprintf(stderr, "gamepad-index-vs-id: a gamepad is connected, but the loop didn't open it (%d opened)\n", count);
        return 1;
    }

    for (i = 0; i < count; i++) {
        SDL_GameControllerClose(players[i]);
    }
    SDL_JoystickDetachVirtual(virtual_index);
    SDL_Quit();
    return 0;
}
