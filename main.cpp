#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include <random>
#include <algorithm>
#include "rectangle.h"

constexpr int WINDOW_WIDTH{700};
constexpr int WINDOW_HEIGHT{800};
#define green_color SDL_Color{55, 207, 54, 255}
#define orange_color SDL_Color{250, 147, 40, 255}
#define red_color SDL_Color{255, 50, 50, 255}

constexpr float VERTICAL_SCROLLING_SPEED{ 100 };
constexpr int VERTICAL_SCROLLING_GRID_SQUARE_SIZE{ 20 };
std::vector<Rectangle> scrollingGrid;
bool isDragging{false};


struct Projectile {
    glm::vec2 pos;
    glm::vec2 vel;
    float radius{10.f};
    SDL_Color color{red_color};

    void update(float dt) {
        pos += vel * dt;
    }

    void draw(SDL_Renderer* renderer) const {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        int r = static_cast<int>(radius);
        for (int w = -r; w <= r; w++) {
            for (int h = -r; h <= r; h++) {
                if (w * w + h * h <= r * r) {
                    SDL_RenderPoint(renderer, pos.x + w, pos.y + h);
                }
            }
        }
    }
};

std::vector<Projectile> projectiles;
bool isFiring{false};
float fireTimer{0.f};
constexpr float FIRE_INTERVAL{0.3f};

float DT{ 0.01f };
Uint64 startTime{ 0 };
Uint64 endTime{ 0 };

SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};
SDL_Event currentEvent;
SDL_Color backgroundColor{255, 255, 255, 255};

Rectangle player(50.f, 50.f, {WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 50.f}, green_color);

class Enemy {
public:
    Rectangle rect;
    float velocity;

    Enemy(glm::vec2 size, glm::vec2 position, SDL_Color color, float initialVel = 100.f):
        rect(size.x, size.y, position, color),
        velocity(initialVel)
    {}

    void update(float dt) {
        rect.pos.y += velocity * dt;

        if (rect.pos.y > WINDOW_HEIGHT + rect.height) {
            rect.pos.y = -50.f;
        }
    }
};

Enemy enemy1{glm::vec2(50.f, 50.f), {WINDOW_WIDTH/3, -25.f}, orange_color},
      enemy2{glm::vec2(50.f, 50.f), {WINDOW_WIDTH/3, -25.f}, orange_color},
      enemy3{glm::vec2(50.f, 50.f), {WINDOW_WIDTH/3, -25.f}, orange_color};

bool quit{false};
float displayScale{1.0f};

bool mouseMovedThisFrame{false};
float mouseXRel{0.f};
float mouseYRel{0.f};

void initScollingRectangles() {
    int numGridRectangles = (WINDOW_HEIGHT / VERTICAL_SCROLLING_GRID_SQUARE_SIZE) / 2 + 2;

    for (int i = 0; i < numGridRectangles; i++) {
        Rectangle newGridRectangleLeft, newGridRectangleRight;

        newGridRectangleLeft.width = newGridRectangleLeft.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleLeft.pos.x = newGridRectangleLeft.width / 2.0f;
        newGridRectangleLeft.pos.y = WINDOW_HEIGHT - newGridRectangleLeft.height / 2.0f - 2.0f * i * newGridRectangleLeft.height;
        newGridRectangleLeft.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleLeft.color = SDL_Color{ 235, 235, 235, 255 };

        newGridRectangleRight.width = newGridRectangleRight.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleRight.pos.x = WINDOW_WIDTH - newGridRectangleRight.width / 2.0f;
        newGridRectangleRight.pos.y = WINDOW_HEIGHT - newGridRectangleRight.height / 2.0f - 2.0f * i * newGridRectangleRight.height;
        newGridRectangleRight.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleRight.color = SDL_Color{ 235, 235, 235, 255 };

        scrollingGrid.push_back(newGridRectangleLeft);
        scrollingGrid.push_back(newGridRectangleRight);
    }
}

bool initWindow() {
    bool success{true};

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    } else {
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "SDL Space Game",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
            } else {
                SDL_SetRenderScale(renderer, displayScale, displayScale);

                SDL_SetWindowRelativeMouseMode(window, false);

                SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
                SDL_RenderClear(renderer);
            }
    }

    return success;
}

void processEvents() {
    mouseMovedThisFrame = false;
    mouseXRel = 0.f;
    mouseYRel = 0.f;

    while (SDL_PollEvent(&currentEvent)) {
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT) {
                float mX, mY;
                SDL_GetMouseState(&mX, &mY);
                mX /= displayScale;
                mY /= displayScale;

                float halfW = player.width / 2.0f;
                float halfH = player.height / 2.0f;
                if (mX >= player.pos.x - halfW && mX <= player.pos.x + halfW &&
                    mY >= player.pos.y - halfH && mY <= player.pos.y + halfH) {
                    isDragging = true;
                    isFiring = true;
                    }
            }
        }

        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT) {
                isDragging = false;
                isFiring = false;
            }
        }

        if (currentEvent.type == SDL_EVENT_MOUSE_MOTION) {
            if (isDragging) {
                mouseXRel += currentEvent.motion.xrel;
                mouseYRel += currentEvent.motion.yrel;
                mouseMovedThisFrame = true;
            }
        }

        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            if (currentEvent.key.key == SDLK_ESCAPE) {
                quit = true;
            }
        }
    }
}

void animate() {
    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.pos += gridRectangle.vel * DT;

        if (gridRectangle.pos.y > (WINDOW_HEIGHT + gridRectangle.height / 2.0f)) {
            gridRectangle.pos.y -= gridRectangle.height * scrollingGrid.size();
        }
    }

    constexpr float accelerationStrength{ 400.0f };
    constexpr float maxVelocity{ 500.0f };
    constexpr float decelerationStrength{ 800.0f };

    if (mouseMovedThisFrame) {
        glm::vec2 mouseDelta(mouseXRel, mouseYRel);
        float len = glm::length(mouseDelta);
        if (len > 0.001f) {
            glm::vec2 dir = glm::normalize(mouseDelta);
            player.vel += dir * accelerationStrength * DT;
        }
    } else {
        float speed = glm::length(player.vel);
        if (speed > 0.0f) {
            float drop = decelerationStrength * DT;
            if (speed <= drop) {
                player.vel = glm::vec2(0.0f, 0.0f);
            } else {
                player.vel -= glm::normalize(player.vel) * drop;
            }
        }
    }

    float currentSpeed = glm::length(player.vel);
    if (currentSpeed > maxVelocity) {
        player.vel = glm::normalize(player.vel) * maxVelocity;
    }

    player.pos += player.vel * DT;

    float halfW = player.width / 2.0f;
    float halfH = player.height / 2.0f;
    if (player.pos.x < halfW) {
        player.pos.x = halfW;
        player.vel.x = 0;
    }
    if (player.pos.x > WINDOW_WIDTH - halfW) {
        player.pos.x = WINDOW_WIDTH - halfW;
        player.vel.x = 0;
    }
    if (player.pos.y < halfH) {
        player.pos.y = halfH;
        player.vel.y = 0;
    }
    if (player.pos.y > WINDOW_HEIGHT - halfH) {
        player.pos.y = WINDOW_HEIGHT - halfH;
        player.vel.y = 0;
    }

    if (isFiring) {
        fireTimer += DT;
        if (fireTimer >= FIRE_INTERVAL) {
            fireTimer = 0.f;
            projectiles.push_back(Projectile{ player.pos, glm::vec2(0.f, -500.f) });
        }
    } else {
        fireTimer = FIRE_INTERVAL;
    }

    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->update(DT);
        if (it->pos.y + it->radius < 0) {
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }

    enemy1.update(DT);
    enemy2.update(DT);
    enemy3.update(DT);
}

void drawFrame() {
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.draw(renderer);
    }

    for (const auto& proj : projectiles) {
        proj.draw(renderer);
    }

    player.draw(renderer);
    enemy1.rect.draw(renderer);
    enemy2.rect.draw(renderer);
    enemy3.rect.draw(renderer);

    SDL_RenderPresent(renderer);
}

void cleanup() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

int main() {
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    std::vector<int> valori = {0, 1, 2};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(valori.begin(), valori.end(), gen);

    enemy1.rect.pos = glm::vec2(WINDOW_WIDTH / 3 * (valori[0] + 0.5), -50.f);
    enemy2.rect.pos = glm::vec2(WINDOW_WIDTH / 3 * (valori[1] + 0.5), -50.f);
    enemy3.rect.pos = glm::vec2(WINDOW_WIDTH / 3 * (valori[2] + 0.5), -50.f);

    std::random_device rd1;
    std::mt19937 gen1(rd1());
    std::uniform_int_distribution<int> distrib(50, 150);
    enemy1.velocity = distrib(gen1);
    enemy2.velocity = distrib(gen1);
    enemy3.velocity = distrib(gen1);

    SDL_zero(currentEvent);
    initScollingRectangles();

    while (!quit) {
        startTime = SDL_GetTicks();

        processEvents();
        animate();
        drawFrame();

        endTime = SDL_GetTicks();
        DT = (endTime - startTime) / 1000.0f;
        if (DT <= 0.f) DT = 0.001f;
    }

    cleanup();
    return 0;
}