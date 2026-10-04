#include <SDL.h>
#include <stdio.h>

#define MAX_PLAYERS 4

static SDL_GameController *players[MAX_PLAYERS];
static int player_count;

int main(int argc, char *argv[])
{
    SDL_Window *window;
    int running = 1;
    int i;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Lobby", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* Every controller connected at launch gets a player slot. */
    for (i = 0; i < SDL_NumJoysticks() && player_count < MAX_PLAYERS; i++) {
        if (!SDL_IsGameController(i)) {
            continue;
        }
        printf("Player %d: %s\n", player_count + 1, SDL_GameControllerNameForIndex(i));
        players[player_count] = SDL_GameControllerOpen(i);
        if (players[player_count]) {
            player_count++;
        }
    }
    if (player_count == 0) {
        printf("No controllers connected. Plug one in and restart.\n");
    }

    while (running) {
        SDL_Event event;

        while (SDL_WaitEventTimeout(&event, 100)) {
            switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                for (i = 0; i < player_count; i++) {
                    if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(players[i])) == event.cbutton.which) {
                        printf("Player %d pressed %s\n", i + 1,
                               SDL_GameControllerGetStringForButton((SDL_GameControllerButton)event.cbutton.button));
                    }
                }
                break;
            }
        }
    }

    for (i = 0; i < player_count; i++) {
        SDL_GameControllerClose(players[i]);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
