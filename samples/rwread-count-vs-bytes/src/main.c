#include <SDL.h>
#include <stdio.h>

#define MAX_SCORES 10

typedef struct {
    char name[12];
    Uint32 points;
} Score;

static Score scores[MAX_SCORES];
static int score_count;

static void load_scores(const char *path)
{
    SDL_RWops *rw = SDL_RWFromFile(path, "rb");

    score_count = 0;
    if (!rw) {
        return;
    }
    score_count = (int)SDL_RWread(rw, scores, sizeof(Score), MAX_SCORES);
    SDL_RWclose(rw);
}

static int save_scores(const char *path)
{
    SDL_RWops *rw = SDL_RWFromFile(path, "wb");

    if (!rw) {
        return 0;
    }
    if (SDL_RWwrite(rw, scores, sizeof(Score), (size_t)score_count) != (size_t)score_count) {
        SDL_RWclose(rw);
        return 0;
    }
    SDL_RWclose(rw);
    return 1;
}

/* Keeps the table sorted, best first, and drops whatever falls off the end. */
static void add_score(const char *name, Uint32 points)
{
    int at = score_count;

    while (at > 0 && scores[at - 1].points < points) {
        at--;
    }
    if (at >= MAX_SCORES) {
        return;
    }
    if (score_count < MAX_SCORES) {
        score_count++;
    }
    SDL_memmove(&scores[at + 1], &scores[at], sizeof(Score) * (size_t)(score_count - 1 - at));
    SDL_strlcpy(scores[at].name, name, sizeof(scores[at].name));
    scores[at].points = points;
}

int main(int argc, char *argv[])
{
    const char *name = argc > 1 ? argv[1] : "player";
    Uint32 points = argc > 2 ? (Uint32)SDL_atoi(argv[2]) : 100;
    int i;

    load_scores("scores.dat");
    add_score(name, points);
    if (!save_scores("scores.dat")) {
        fprintf(stderr, "Could not save the high scores: %s\n", SDL_GetError());
        return 1;
    }

    for (i = 0; i < score_count; i++) {
        printf("%2d. %-11s %u\n", i + 1, scores[i].name, (unsigned)scores[i].points);
    }
    return 0;
}
