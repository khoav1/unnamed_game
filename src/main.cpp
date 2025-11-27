#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_timer.h"
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>
#include <random>
#include <string>
#include <vector>

constexpr uint32_t kGameWidth = 40;
constexpr uint32_t kGameHeight = 30;
constexpr uint32_t kBlockSizeInPixels = 20;
constexpr uint32_t kChickenSizeInPixels = 30;
constexpr uint32_t kWindowWidth = (kGameWidth * kBlockSizeInPixels);
constexpr uint32_t kWindowHeight = (kGameHeight * kBlockSizeInPixels);

struct Chicken {
    SDL_Vertex vertices[3];
    uint32_t velocity_ms;
    uint64_t last_step;
    uint32_t health;
};

struct Bullet {
    float xpos;
    float ypos;
    uint64_t last_step;
    bool is_collided = false;
};

struct Aircraft {
    float xpos;
    float ypos;
    std::vector<Bullet> bullets;
    uint32_t bullet_length = kBlockSizeInPixels / 2;
    uint32_t bullet_velocity_ms = 20;
};

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Aircraft aircraft;
    std::vector<Chicken> chickens;
    uint32_t chicken_spawn_rate_ms = 5000;
    uint32_t last_chicken_spawn;
};

SDL_AppResult LogErrAndFail(const std::string msg) {
    const std::string msg_with_err = msg + "\n%s";
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg_with_err.c_str(), SDL_GetError());
    return SDL_APP_FAILURE;
}

static int RandomIntInRange(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

static void AddBullet(Aircraft *aircraft) {
    Bullet bullet = {aircraft->xpos + aircraft->bullet_length,
                     aircraft->ypos - aircraft->bullet_length, SDL_GetTicks()};
    aircraft->bullets.push_back(bullet);
}

static void AddChicken(AppState *app) {
    const SDL_FColor color = {79.0f, 79.0f, 79.0f};
    const uint32_t half_size = kChickenSizeInPixels / 2;
    const float random_xpos =
        float(RandomIntInRange(half_size, kWindowWidth - half_size));
    const float random_ypos = float(RandomIntInRange(0, kWindowHeight / 2));
    const uint32_t random_velocity_ms = RandomIntInRange(10, 150);
    Chicken chicken = {{}, random_velocity_ms, SDL_GetTicks(), 10};
    chicken.vertices[0] = {.position = {random_xpos, random_ypos},
                           .color = color};
    chicken.vertices[1] = {.position = {random_xpos - half_size,
                                        random_ypos + kChickenSizeInPixels},
                           .color = color};
    chicken.vertices[2] = {.position = {random_xpos + half_size,
                                        random_ypos + kChickenSizeInPixels},
                           .color = color};
    app->chickens.push_back(chicken);
    app->last_chicken_spawn = SDL_GetTicks();
    SDL_Log("chickens: %lu", app->chickens.size());
}

// run once at startup
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_SetAppMetadata("unnamed_game", "1.0", "org.libsdl.unnamed_game")) {
        return LogErrAndFail("Failed to set app metadata");
    }
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        return LogErrAndFail("Couldn't init SDL");
    }
    AppState *app = new AppState();
    if (!app) {
        return LogErrAndFail("Couldn't allocate resources for app state");
    }
    *appstate = app;
    app->aircraft.xpos = (kWindowWidth / 2.0f) - kBlockSizeInPixels;
    app->aircraft.ypos = kWindowHeight - 2 * kBlockSizeInPixels;
    AddChicken(app);
    if (!SDL_CreateWindowAndRenderer("unnamed_game", kWindowWidth,
                                     kWindowHeight, SDL_WINDOW_RESIZABLE,
                                     &app->window, &app->renderer)) {
        return LogErrAndFail("Failed to create window/renderer");
    }
    SDL_SetRenderLogicalPresentation(app->renderer, kWindowWidth, kWindowHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    return SDL_APP_CONTINUE;
}

static SDL_AppResult HandleKeyDownEvent(AppState *app,
                                        const SDL_Scancode key_code) {
    Aircraft *ctx = &app->aircraft;
    const float xpos = ctx->xpos;
    const float ypos = ctx->ypos;
    switch (key_code) {
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q:
            return SDL_APP_SUCCESS;
        case SDL_SCANCODE_SPACE:
            AddBullet(ctx);
            break;
        case SDL_SCANCODE_LEFT: {
            const float new_pos = ctx->xpos - kBlockSizeInPixels;
            ctx->xpos = (new_pos < 0) ? 0 : new_pos;
            break;
        }
        case SDL_SCANCODE_RIGHT: {
            const float new_pos = ctx->xpos + kBlockSizeInPixels;
            const float barrier = kWindowWidth - kBlockSizeInPixels;
            ctx->xpos = (new_pos > barrier) ? barrier : new_pos;
            break;
        }
        case SDL_SCANCODE_UP: {
            const float new_pos = ctx->ypos - kBlockSizeInPixels;
            ctx->ypos = (new_pos < 0) ? 0 : new_pos;
            break;
        }
        case SDL_SCANCODE_DOWN: {
            const float new_pos = ctx->ypos + kBlockSizeInPixels;
            const float barrier = kWindowHeight - kBlockSizeInPixels;
            ctx->ypos = (new_pos > barrier) ? barrier : new_pos;
            break;
        }
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *app = (AppState *)appstate;
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            return HandleKeyDownEvent(app, event->key.scancode);
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_FRect GetAircraftSprite(const AppState *app) {
    SDL_FRect rect;
    SDL_SetRenderDrawColor(app->renderer, 7, 54, 66, SDL_ALPHA_OPAQUE);
    rect.x = app->aircraft.xpos;
    rect.y = app->aircraft.ypos;
    rect.w = rect.h = kBlockSizeInPixels;
    return rect;
}

// run once per frame
SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *app = (AppState *)appstate;
    Aircraft *aircraft = &app->aircraft;
    const uint64_t now = SDL_GetTicks();

    SDL_SetRenderDrawColor(app->renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    SDL_FRect aircraft_sprite = GetAircraftSprite(app);
    SDL_RenderFillRect(app->renderer, &aircraft_sprite);

    while ((now - app->last_chicken_spawn) >= app->chicken_spawn_rate_ms) {
        AddChicken(app);
    }

    for (int i = 0; i < app->chickens.size(); i++) {
        for (int j = 0; j < app->aircraft.bullets.size(); j++) {
            const Chicken chicken = app->chickens[i];
            const Bullet bullet = app->aircraft.bullets[j];
            bool vcheck = bullet.xpos >= chicken.vertices[1].position.x &&
                          bullet.xpos <= chicken.vertices[2].position.x;
            bool hcheck = bullet.ypos <= chicken.vertices[1].position.y;
            if (vcheck && hcheck) {
                app->chickens[i].health--;
                app->aircraft.bullets[j].is_collided = true;
                break;
            }
        }
    }

    std::vector<int> removed_chickens;
    for (int i = 0; i < app->chickens.size(); i++) {
        if (app->chickens[i].vertices[0].position.y >= kWindowHeight ||
            app->chickens[i].health <= 0) {
            removed_chickens.push_back(i);
            continue;
        }

        while ((now - app->chickens[i].last_step) >=
               app->chickens[i].velocity_ms) {
            app->chickens[i].vertices[0].position.y++;
            app->chickens[i].vertices[1].position.y++;
            app->chickens[i].vertices[2].position.y++;
            app->chickens[i].last_step += app->chickens[i].velocity_ms;
        }
        SDL_RenderGeometry(app->renderer, nullptr, app->chickens[i].vertices, 3,
                           nullptr, 0);
    }

    SDL_SetRenderDrawColor(app->renderer, 20, 20, 20, SDL_ALPHA_OPAQUE);
    std::vector<int> removed_bullets;
    for (int i = 0; i < aircraft->bullets.size(); i++) {
        if (aircraft->bullets[i].ypos < 0 || aircraft->bullets[i].is_collided) {
            removed_bullets.push_back(i);
            continue;
        }
        while ((now - aircraft->bullets[i].last_step) >=
               aircraft->bullet_velocity_ms) {
            aircraft->bullets[i].ypos -= 4;
            aircraft->bullets[i].last_step += aircraft->bullet_velocity_ms;
        }
        SDL_RenderLine(app->renderer, aircraft->bullets[i].xpos,
                       aircraft->bullets[i].ypos - (kBlockSizeInPixels / 2.0f),
                       aircraft->bullets[i].xpos, aircraft->bullets[i].ypos);
    }

    for (int index : removed_chickens) {
        app->chickens.erase(app->chickens.begin() + index);
    }

    for (int index : removed_bullets) {
        aircraft->bullets.erase(aircraft->bullets.begin() + index);
    }

    SDL_RenderPresent(app->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != nullptr) {
        AppState *app = (AppState *)appstate;
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        app->aircraft.bullets.clear();
        app->chickens.clear();
        delete app;
    }
    SDL_Log("app quit successfully");
    SDL_Quit();
}
