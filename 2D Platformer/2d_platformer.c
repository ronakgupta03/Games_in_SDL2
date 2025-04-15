#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h> // Add SDL_image library
#include <stdio.h>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GROUND_HEIGHT = 50;
const float START_X = 200;
const float START_Y = 100;

// Global variables
SDL_Rect camera = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
int score = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;

// Textures for player and enemies
SDL_Texture *playerTexture = NULL;
SDL_Texture *enemyTexture = NULL;

// Type definitions
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
    bool facingLeft; // Direction enemy is facing
} Enemy;

Enemy enemies[MAX_ENEMIES] = {
    {600, 420, 40, 40, 1.0f, false},
    {900, 420, 40, 40, -1.0f, true},
    {1300, 420, 40, 40, 0.8f, false}
};

#define MAX_PLATFORMS 10
SDL_Rect platforms[MAX_PLATFORMS] = {
    {250, 450, 120, 20},
    {400, 350, 150, 20},
    {600, 250, 100, 20},
    {100, 200, 180, 20},
    {300, 150, 130, 20},
    {980, 500,  50, 20},
    {1130, 380, 50, 20},
    {1330, 280, 50, 20},
    {1550, 150, 50, 20},
    {1700, 500, 50, 20}
};

typedef struct {
    float x, y;
    float w, h;
    float vx, vy;
    bool onGround;
    bool facingLeft; // Direction player is facing
} Player;

SDL_Rect goal = { 1700, 420, 40, 60 }; // far right side of map

// Function prototypes
void resetGame(Player* player);
bool initSDL();
bool loadMedia();
void cleanupSDL();
void handleInput(Player* player, bool* running);
void updatePhysics(Player* player);
void checkCollisions(Player* player);
void checkEnemyCollisions(Player* player);
void checkGoalCollision(Player* player);
void checkFallDetection(Player* player);
void updateCamera(Player player);
void renderScene(Player player);
void displayMessage(const char* message, SDL_Color color);
SDL_Texture* loadTexture(const char* path);

// Reset the game
void resetGame(Player* player) {
    player->x = START_X;
    player->y = START_Y;
    player->vx = 0;
    player->vy = 0;
    player->onGround = false;
    player->facingLeft = false;

    score = 0;

    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].collected = false;
    }

    // Reset enemies to original positions
    enemies[0] = (Enemy){600, 420, 40, 40, 1.0f, false};
    enemies[1] = (Enemy){900, 420, 40, 40, -1.0f, true};
    enemies[2] = (Enemy){1300, 420, 40, 40, 0.8f, false};
}

// Load a texture from file
SDL_Texture* loadTexture(const char* path) {
    SDL_Texture* newTexture = NULL;
    
    // Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    
    // Create texture from surface pixels
    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (newTexture == NULL) {
        printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
    }
    
    // Get rid of old loaded surface
    SDL_FreeSurface(loadedSurface);
    
    return newTexture;
}

// Load media (images)
bool loadMedia() {
    // Load player texture
    playerTexture = loadTexture("assets/1.png");
    if (playerTexture == NULL) {
        printf("Failed to load player texture!\n");
        return false;
    }
    
    // Load enemy texture
    enemyTexture = loadTexture("assets/2.png");
    if (enemyTexture == NULL) {
        printf("Failed to load enemy texture!\n");
        return false;
    }
    
    return true;
}

// Initialize SDL
bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Log("SDL_image could not initialize! SDL_image Error: %s", IMG_GetError());
        return false;
    }

    if (TTF_Init() < 0) {
        SDL_Log("TTF Init failed: %s", TTF_GetError());
        return false;
    }

    font = TTF_OpenFont("fonts/TTF/ARIAL.TTF", 24);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
        return false;
    }

    return true;
}

// Clean up SDL resources
void cleanupSDL() {
    // Free loaded textures
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(enemyTexture);
    playerTexture = NULL;
    enemyTexture = NULL;

    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Handle user input
void handleInput(Player* player, bool* running) {
    SDL_Event event;
    const float moveSpeed = 5.0f;
    const float jumpStrength = -12.0f;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = false;
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        player->vx = -moveSpeed;
        player->facingLeft = true;
    } else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        player->vx = moveSpeed;
        player->facingLeft = false;
    } else {
        player->vx = 0;
    }

    if (keys[SDL_SCANCODE_SPACE] && player->onGround) {
        player->vy = jumpStrength;
        player->onGround = false;
    }
}

// Update physics
void updatePhysics(Player* player) {
    const float gravity = 0.5f;
    
    player->vy += gravity;
    player->x += player->vx;
    player->y += player->vy;
    
    // Reset ground state for this frame
    player->onGround = false;
    
    // Update enemy positions
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x += enemies[i].vx;

        // Update enemy facing direction based on velocity
        if (enemies[i].vx > 0) {
            enemies[i].facingLeft = false;
        } else if (enemies[i].vx < 0) {
            enemies[i].facingLeft = true;
        }

        // Basic left-right patrol (adjust these bounds per enemy)
        if (enemies[i].x < 500 || enemies[i].x > 1500) {
            enemies[i].vx *= -1;
        }
    }
    
    // Prevent player from going off left edge
    if (player->x <= 0) player->x = 0;
}

// Check collisions with ground and platforms
void checkCollisions(Player* player) {
    // Check collision with ground
    float groundY = WINDOW_HEIGHT - GROUND_HEIGHT - player->h;
    if (player->y >= groundY && 
        player->x + player->w > 0 && 
        player->x < WINDOW_WIDTH) {
        player->y = groundY;
        player->vy = 0;
        player->onGround = true;
    }

    // Check collision with platforms
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        SDL_Rect *plat = &platforms[i];

        if (player->x + player->w > plat->x &&
            player->x < plat->x + plat->w &&
            player->y + player->h >= plat->y &&
            player->y + player->h <= plat->y + plat->h &&
            player->vy >= 0) {
            player->y = plat->y - player->h;
            player->vy = 0;
            player->onGround = true;
        }
    }

    // Check collision with coins
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].collected) continue;

        if (player->x + player->w > coins[i].x &&
            player->x < coins[i].x + coins[i].w &&
            player->y + player->h > coins[i].y &&
            player->y < coins[i].y + coins[i].h) {
            coins[i].collected = true;
            score++;
            SDL_Log("Coin collected! Score: %d", score);
        }
    }
}

// Check collisions with enemies
void checkEnemyCollisions(Player* player) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (player->x + player->w > enemies[i].x &&
            player->x < enemies[i].x + enemies[i].w &&
            player->y + player->h > enemies[i].y &&
            player->y < enemies[i].y + enemies[i].h) {
            
            displayMessage("Game Over! Hit by enemy!", (SDL_Color){255, 0, 0});
            resetGame(player);
            break;
        }
    }
}

// Display message on screen
void displayMessage(const char* message, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Surface* messageSurface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
    SDL_Rect messageRect = {
        WINDOW_WIDTH / 2 - messageSurface->w / 2,
        WINDOW_HEIGHT / 2 - messageSurface->h / 2,
        messageSurface->w,
        messageSurface->h
    };
    
    SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);  // Wait 1 second

    SDL_FreeSurface(messageSurface);
    SDL_DestroyTexture(messageTexture);
}

// Check if player reached the goal
void checkGoalCollision(Player* player) {
    if (player->x + player->w > goal.x &&
        player->x < goal.x + goal.w &&
        player->y + player->h > goal.y &&
        player->y < goal.y + goal.h) {
        
        displayMessage("You win!", (SDL_Color){255, 255, 0});
        resetGame(player);
    }
}

// Check if player fell off the map
void checkFallDetection(Player* player) {
    if (player->y > WINDOW_HEIGHT + 100) {
        displayMessage("Game Over! You fell!", (SDL_Color){255, 0, 0});
        resetGame(player);
    }
}

// Update camera position
void updateCamera(Player player) {
    camera.x = (int)(player.x + player.w / 2) - WINDOW_WIDTH / 2;
    camera.y = (int)(player.y + player.h / 2) - WINDOW_HEIGHT / 2;
    
    // Clamp camera
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
}

// Render the scene
void renderScene(Player player) {
    // Clear screen with sky blue
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
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

    // Draw player using texture
    SDL_Rect playerRect = {
        (int)(player.x - camera.x),
        (int)(player.y - camera.y),
        (int)player.w, (int)player.h
    };
    
    // Flip texture based on direction
    SDL_RendererFlip flip = player.facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, playerTexture, NULL, &playerRect, 0.0, NULL, flip);

    // Draw coins
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

    // Draw score
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);

    SDL_Color textColor = { 255, 255, 255 }; // white
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    // Draw enemies using texture
    for (int i = 0; i < MAX_ENEMIES; i++) {
        SDL_Rect enemyRect = {
            enemies[i].x - camera.x,
            enemies[i].y - camera.y,
            enemies[i].w,
            enemies[i].h
        };
        
        // Flip enemy texture based on direction
        SDL_RendererFlip enemyFlip = enemies[i].facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, enemyTexture, NULL, &enemyRect, 0.0, NULL, enemyFlip);
    }

    // Draw goal
    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255); // blue flag
    SDL_Rect goalDraw = {
        goal.x - camera.x,
        goal.y - camera.y,
        goal.w,
        goal.h
    };
    SDL_RenderFillRect(renderer, &goalDraw);

    // Present the renderer
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (!initSDL()) {
        return 1;
    }
    
    // Load media (textures)
    if (!loadMedia()) {
        return 1;
    }
    
    // Create player
    Player player = {START_X, START_Y, 100, 100, 0, 0, false, false};
    
    // Game loop
    bool running = true;
    while (running) {
        // Handle input
        handleInput(&player, &running);
        
        // Update physics
        updatePhysics(&player);
        
        // Handle collisions
        checkCollisions(&player);
        checkEnemyCollisions(&player);
        checkGoalCollision(&player);
        checkFallDetection(&player);
        
        // Update camera
        updateCamera(player);
        
        // Render scene
        renderScene(player);
        
        // Cap frame rate
        SDL_Delay(16); // ~60 FPS
    }
    
    // Clean up resources
    cleanupSDL();
    
    return 0;
}