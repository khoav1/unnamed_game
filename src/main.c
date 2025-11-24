#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string.h>

#define WINDOW_WIDTH 800U
#define WINDOW_HEIGHT 600U

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} AppState;

SDL_AppResult log_err_and_fail(char *msg) {
    SDL_strlcat(msg, "\n%s", sizeof(msg));
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg, SDL_GetError());
    return SDL_APP_FAILURE;
}

// run once at startup
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_SetAppMetadata("unnamed_game", "1.0", "org.libsdl.unnamed_game")) {
        return log_err_and_fail("Failed to set app metadata");
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        return log_err_and_fail("Couldn't init SDL");
    }

    AppState *app = SDL_calloc(1, sizeof(AppState));
    if (!app) {
        return log_err_and_fail("Couldn't allocate resources for app state");
    }
    *appstate = app;

    if (!SDL_CreateWindowAndRenderer("unnamed_game", WINDOW_WIDTH,
                                     WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE,
                                     &app->window, &app->renderer)) {
        return log_err_and_fail("Failed to create window/renderer");
    }
    SDL_SetRenderLogicalPresentation(app->renderer, WINDOW_WIDTH, WINDOW_HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

// run once per frame
SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *app = (AppState *)appstate;

    SDL_FRect rect;
    SDL_SetRenderDrawColor(app->renderer, 32, 32, 32, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    SDL_SetRenderDrawColor(app->renderer, 7, 54, 66, SDL_ALPHA_OPAQUE);
    rect.x = rect.y = 100;
    rect.w = 300;
    rect.h = 150;
    SDL_RenderFillRect(app->renderer, &rect);

    SDL_RenderPresent(app->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != NULL) {
        AppState *app = (AppState *)appstate;
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_free(app);
    }
    SDL_Log("app quit successfully");
    SDL_Quit();
}
