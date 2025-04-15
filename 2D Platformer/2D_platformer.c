#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>



const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GROUND_HEIGHT = 50;
const float START_X = 200;
const float START_Y = 100;
SDL_Rect camera = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };

#define MAX_COINS 10

typedef struct {
    float x, y;
    int w, h;
    bool collected;
} Coin;

Coin coins[MAX_COINS] = {
    {250, 420, 20, 20, false},
    {430, 320, 20, 20, false},
    {620, 220, 20, 20, false},
    {120, 120, 20, 20, false},
    {310, 70, 20, 20, false},
    {750, 420, 20, 20, false},
    {980, 380, 20, 20, false},
    {1130, 260, 20, 20, false},
    {1330, 150, 20, 20, false},
    {1550, 100, 20, 20, false},
};

#define MAX_ENEMIES 3

typedef struct {
    float x, y;
    int w, h;
    float vx; // horizontal speed
} Enemy;

Enemy enemies[MAX_ENEMIES] = {
    {600, 420, 40, 40, 1.0f},
    {900, 420, 40, 40, -1.0f},
    {1300, 420, 40, 40, 0.8f}
};


int score = 0;


#define MAX_PLATFORMS 5
SDL_Rect platforms[MAX_PLATFORMS] = {
    {200, 450, 120, 20},
    {400, 350, 150, 20},
    {600, 250, 100, 20},
    {100, 150, 180, 20},
    {300, 100, 130, 20}
};


typedef struct {
    float x, y;
    float w, h;
    float vx, vy;
    bool onGround;
} Player;

SDL_Rect goal = { 1700, 420, 40, 60 }; // far right side of map


int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    if (TTF_Init() < 0) {
        SDL_Log("TTF Init failed: %s", TTF_GetError());
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("fonts/TTF/OpenSans-ExtraBoldItalic.ttf", 24); // Change to your font path
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return 1;
    }


    SDL_Window *window = SDL_CreateWindow(
        "Resizable Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE

    );


    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Player player = {START_X, START_Y, 50, 50, 0, 0, false};

    const float gravity = 0.5f;
    const float moveSpeed = 5.0f;
    const float jumpStrength = -12.0f;

    bool running = true;
    SDL_Event event;

    while (running) {
        // === Input Handling ===
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
            player.vx = -moveSpeed;
        } else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
            player.vx = moveSpeed;
        } else {
            player.vx = 0;
        }

        if (keys[SDL_SCANCODE_SPACE] && player.onGround) {
            player.vy = jumpStrength;
            player.onGround = false;
        }

        // === Physics ===
        player.vy += gravity;
        player.x += player.vx;
        player.y += player.vy;

        // === Ground Collision ===
        player.onGround = false;

        for (int i = 0; i < MAX_ENEMIES; i++) {
            enemies[i].x += enemies[i].vx;

            // Basic left-right patrol (adjust these bounds per enemy)
            if (enemies[i].x < 500 || enemies[i].x > 1500) {
                enemies[i].vx *= -1;
            }
        }


        // Check collision with ground
        float groundY = WINDOW_HEIGHT - GROUND_HEIGHT - player.h;
        if (player.y >= groundY) {
            player.y = groundY;
            player.vy = 0;
            player.onGround = true;
        }

        // Check collision with platforms
        for (int i = 0; i < MAX_PLATFORMS; i++) {
            SDL_Rect *plat = &platforms[i];

            if (
                player.x + player.w > plat->x &&
                player.x < plat->x + plat->w &&
                player.y + player.h >= plat->y &&
                player.y + player.h <= plat->y + plat->h &&
                player.vy >= 0
            ) {
                player.y = plat->y - player.h;
                player.vy = 0;
                player.onGround = true;
            }

        }

        for (int i = 0; i < MAX_COINS; i++) {
            if (coins[i].collected) continue;


            if (
                player.x + player.w > coins[i].x &&
                player.x < coins[i].x + coins[i].w &&
                player.y + player.h > coins[i].y &&
                player.y < coins[i].y + coins[i].h
            ) {
                coins[i].collected = true;
                score++;
                SDL_Log("Coin collected! Score: %d", score);
            }
        }
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (
                player.x + player.w > enemies[i].x &&
                player.x < enemies[i].x + enemies[i].w &&
                player.y + player.h > enemies[i].y &&
                player.y < enemies[i].y + enemies[i].h
            ) {
                SDL_Log("Game Over! You were hit by an enemy.");
                SDL_Delay(2000);  // short pause
                exit(0);          // close the game
            }
        }


        // === Fall Detection ===
        if (player.y > WINDOW_HEIGHT) {
            // Reset position
            player.x = START_X;
            player.y = START_Y;
            player.vx = 0;
            player.vy = 0;
            player.onGround = false;
            SDL_Log("You fell! Resetting player...");
        }
        // Center camera on player
        camera.x = (int)(player.x + player.w / 2) - WINDOW_WIDTH / 2;
        camera.y = (int)(player.y + player.h / 2) - WINDOW_HEIGHT / 2;
        if (camera.y < 0) camera.y = 0;

        // Clamp camera within world bounds
        if (camera.x < 0) camera.x = 0;



        // === Rendering ===
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // sky blue
        SDL_RenderClear(renderer);

        // Draw ground
        SDL_Rect ground = {
            0 - camera.x,
            WINDOW_HEIGHT - GROUND_HEIGHT - camera.y,
            WINDOW_WIDTH,
            GROUND_HEIGHT
        };
        SDL_SetRenderDrawColor(renderer, 50, 200, 70, 255); // green
        SDL_RenderFillRect(renderer, &ground);


        // Draw platforms
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // gray
        for (int i = 0; i < MAX_PLATFORMS; i++) {
            SDL_Rect drawPlat = {
                platforms[i].x - camera.x,
                platforms[i].y - camera.y,
                platforms[i].w,
                platforms[i].h
            };
            SDL_RenderFillRect(renderer, &drawPlat);
        }



        // Draw player
        SDL_Rect playerRect = {
            (int)(player.x - camera.x),
            (int)(player.y - camera.y),
            (int)player.w, (int)player.h
        };
        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); // red
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // gold/yellow

        for (int i = 0; i < MAX_COINS; i++) {
            if (coins[i].collected) continue;

            SDL_Rect coinRect = {
                coins[i].x - camera.x,
                coins[i].y - camera.y,
                coins[i].w,
                coins[i].h
            };
            SDL_RenderFillRect(renderer, &coinRect);
        }

        char scoreText[32];
        sprintf(scoreText, "Score: %d", score);

        SDL_Color textColor = { 255, 255, 255 }; // white
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red

        for (int i = 0; i < MAX_ENEMIES; i++) {
            SDL_Rect enemyRect = {
                enemies[i].x - camera.x,
                enemies[i].y - camera.y,
                enemies[i].w,
                enemies[i].h
            };
            SDL_RenderFillRect(renderer, &enemyRect);
        }

        SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255); // blue flag
        SDL_Rect goalDraw = {
            goal.x - camera.x,
            goal.y - camera.y,
            goal.w,
            goal.h
        };
        SDL_RenderFillRect(renderer, &goalDraw);

        if (
            player.x + player.w > goal.x &&
            player.x < goal.x + goal.w &&
            player.y + player.h > goal.y &&
            player.y < goal.y + goal.h
        ) {
            SDL_Log("🎉 You reached the goal! Final Score: %d", score);
            SDL_Delay(2000);
            exit(0);
        }


        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}