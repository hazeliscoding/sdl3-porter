#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;
    char name[32] = "";
    int running = 1;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Enter your name", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 120, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Type your name and press Enter.\n");
    while (running) {
        SDL_Event event;

        if (!SDL_WaitEvent(&event)) {
            break;
        }
        switch (event.type) {
        case SDL_QUIT:
            running = 0;
            break;
        case SDL_TEXTINPUT:
            SDL_strlcat(name, event.text.text, sizeof(name));
            SDL_SetWindowTitle(window, name);
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_BACKSPACE && name[0] != '\0') {
                name[SDL_strlen(name) - 1] = '\0';
                SDL_SetWindowTitle(window, name);
            } else if (event.key.keysym.sym == SDLK_RETURN) {
                printf("Hello, %s!\n", name);
                running = 0;
            }
            break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
