#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include <random>
#include <algorithm>
#include "rectangle.h"

//define window dimensions
constexpr int WINDOW_WIDTH{700};
constexpr int WINDOW_HEIGHT{800};
#define green_color SDL_Color{55, 207, 54, 255}
#define orange_color SDL_Color{250, 147, 40, 255}

// Vertical scrolling grid related variables
constexpr float VERTICAL_SCROLLING_SPEED{ 100 };
constexpr int VERTICAL_SCROLLING_GRID_SQUARE_SIZE{ 20 };
std::vector<Rectangle> scrollingGrid;

// Animation related variables
float DT{ 0.01f };
Uint64 startTime{ 0 };
Uint64 endTime{ 0 };

//define SDL Window related variables
SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};
SDL_Event currentEvent;
SDL_Color backgroundColor{255, 255, 255, 255};

Rectangle player(50.f, 50.f, {WINDOW_WIDTH / 2, WINDOW_HEIGHT-50.f}, green_color);





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
} enemy1{glm::vec2(50.f, 50.f), {WINDOW_WIDTH/3, -25.f}, orange_color},
    enemy2{glm::vec2(50.f, 50.f), {WINDOW_WIDTH/3, -25.f}, orange_color},
    enemy3{glm::vec2(50.f, 50.f), {WINDOW_WIDTH/3, -25.f}, orange_color};



bool quit{false};

float mouseX{-1.0f}, mouseY{-1.0f};

float displayScale{1.0f};


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

    //Try to initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    } else {
        //Try to create the window and renderer
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "SDL Hello World Example",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
        } else {
            //Apply global display scaling to renderer
            SDL_SetRenderScale(renderer, displayScale, displayScale);

            //Set background color
            SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                                   backgroundColor.a);

            //Apply background color
            SDL_RenderClear(renderer);
        }
    }

    return success;
}

void processEvents() {
    //Check for events in queue
    while (SDL_PollEvent(&currentEvent)) {
        //User requests quit
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        //Mouse event -> pressed button
        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT) {
                SDL_GetMouseState(&mouseX, &mouseY);
                mouseX /= displayScale;
                mouseY /= displayScale;
                std::cout << "Left mouse click => " << "x: " << mouseX << ", y: " << mouseY << std::endl;
            }
        }

        //Mouse event -> mouse movement
        if (currentEvent.type == SDL_EVENT_MOUSE_MOTION) {
            if (const SDL_MouseButtonFlags mouseButtons = SDL_GetMouseState(nullptr, nullptr); mouseButtons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
                SDL_GetMouseState(&mouseX, &mouseY);
                mouseX /= displayScale;
                mouseY /= displayScale;
                std::cout << "Mouse move while left clicking => " << "x: " << mouseX << ", y: " << mouseY << std::endl;
            }
        }

        //Keyboard event
        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            switch (currentEvent.key.key) {
                case SDLK_UP:
                    std::cout << "UP arrow key was pressed" << std::endl;
                    break;
                case SDLK_DOWN:
                    std::cout << "DOWN arrow key was pressed" << std::endl;
                    break;
                case SDLK_LEFT:
                    std::cout << "LEFT arrow key was pressed" << std::endl;
                    break;
                case SDLK_RIGHT:
                    std::cout << "RIGHT arrow key was pressed" << std::endl;
                    break;
                case SDLK_R:
                    std::cout << "R key was pressed" << std::endl;
                    break;
                case SDLK_S:
                    std::cout << "S key was pressed" << std::endl;
                    break;
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                default:
                    break;
            }
        }
    }
}

void animate() {

    // Scrolling grid
    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);

        gridRectangle.pos += gridRectangle.vel * DT;

        if (gridRectangle.pos.y > (WINDOW_HEIGHT + gridRectangle.height / 2.0f)) {
            gridRectangle.pos.y -= gridRectangle.height * scrollingGrid.size();
        }
    }

    enemy1.update(DT);
    enemy2.update(DT);
    enemy3.update(DT);
}


void drawFrame() {
    //Clear the background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    //Draw scene
    
    //Draw scrolling grid

    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.draw(renderer);
    }

    player.draw(renderer);
    enemy1.rect.draw(renderer);
    enemy2.rect.draw(renderer);
    enemy3.rect.draw(renderer);

    //Update window
    SDL_RenderPresent(renderer);
}

void cleanup() {
    //Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    //Destroy window
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    //Quit SDL
    SDL_Quit();
}

int main() {
    //Initialize window
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }
    std::vector<int> valori = {0, 1, 2};

    std::random_device rd;
    std::mt19937 gen(rd());

    std::shuffle(valori.begin(), valori.end(), gen);

    enemy1.rect.pos = glm::vec2({WINDOW_WIDTH / 3*(valori[0]+0.5),  - 50.f});
    enemy2.rect.pos = glm::vec2({WINDOW_WIDTH / 3*(valori[1]+0.5  ), - 50.f});
    enemy3.rect.pos = glm::vec2({WINDOW_WIDTH / 3*(valori[2]+0.5  ),  - 50.f});

    std::random_device rd1;
    std::mt19937 gen1(rd1());

    std::uniform_int_distribution<int> distrib(50, 150);
    enemy1.velocity = distrib(gen1);
    enemy2.velocity = distrib(gen1);
    enemy3.velocity = distrib(gen1);

    SDL_zero(currentEvent);

	initScollingRectangles();

    //Game loop
    while (!quit) {
        startTime = SDL_GetTicks();

        processEvents();
        animate();
        drawFrame();

        endTime = SDL_GetTicks();
        DT = (endTime - startTime) / 1000.0f;
    }

    cleanup();
    return 0;
}
