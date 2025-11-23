#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>

constexpr uint32_t window_width{800};
constexpr uint32_t window_height{600};

struct AppContext {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
};

SDL_AppResult SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

// run once at startup
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return SDL_Fail();
    }

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("title", window_width, window_height,
                                     SDL_WINDOW_RESIZABLE, &window,
                                     &renderer)) {
        return SDL_Fail();
    }

    *appstate = new AppContext{
        .window = window,
        .renderer = renderer,
    };

    SDL_SetRenderVSync(renderer, -1);
    return SDL_APP_CONTINUE;
}

// run once per frame
SDL_AppResult SDL_AppIterate(void *appstate) {
    auto *app = (AppContext *)appstate;
    const std::string message = "Hello Chuc beo";
    int w = 0, h = 0;
    float x, y;
    const float scale = 4.0f;

    SDL_GetRenderOutputSize(app->renderer, &w, &h);
    SDL_SetRenderScale(app->renderer, scale, scale);
    x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * message.length()) /
        2;
    y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(app->renderer, x, y, message.c_str());

    SDL_RenderPresent(app->renderer);
    return app->app_quit;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto *app = (AppContext *)appstate;
    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto *app = (AppContext *)appstate;
    if (app) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }
    SDL_Log("app quit successfully");
    SDL_Quit();
}
