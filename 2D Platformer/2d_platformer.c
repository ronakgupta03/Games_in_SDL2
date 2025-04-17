#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GROUND_HEIGHT = 75;
const float START_X = 200;
const float START_Y = 100;
const int LEVEL_WIDTH = 2000; 

// Global variables
SDL_Rect camera = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
int score = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;

// Textures for player and enemies
SDL_Texture *playerTexture = NULL;
SDL_Texture *enemyTexture = NULL;
SDL_Texture *coinTexture = NULL; 
SDL_Texture *terrainTexture = NULL;
SDL_Texture *platformTexture = NULL;
SDL_Texture *bgTexture = NULL;
SDL_Texture *goalTexture = NULL;

// Type definitions
#define MAX_COINS 10
typedef struct {
    float x, y;
    int w, h;
    bool collected;
    int frame;
    int frameTimer;
    int frameDelay;
    int totalFrames;
    int frameWidth, frameHeight;
} Coin;

Coin coins[MAX_COINS] = {
    {250, 400,60,60, false,0,0,8,17,16,16},
    {430, 300,60,60, false,0,0,8,17,16,16},
    {620, 200,60,60, false,0,0,8,17,16,16},
    {120, 100,60,60, false,0,0,8,17,16,16},
    {310, 860,60,60, false,0,0,8,17,16,16},
    {750, 400,60,60, false,0,0,8,17,16,16},
    {980, 360,60,60, false,0,0,8,17,16,16},
    {1130, 240,60,60, false,0,0,8,17,16,16},
    {1330, 150,60,60, false,0,0,8,17,16,16},
    {1550, 100,60,60, false,0,0,8,17,16,16},
};

#define MAX_ENEMIES 3
typedef struct {
    float x, y;
    int w, h;
    float vx;
    bool facingLeft;
    int frame;             // Animation frame
    int frameTimer;        // Timer for animation
    int frameDelay;        // Animation speed
    int totalFrames;       // Total animation frames
    float patrolStart;     // Patrol boundary - start
    float patrolEnd;       // Patrol boundary - end
} Enemy;

Enemy enemies[MAX_ENEMIES] = {
    {600, 420, 40, 40, 1.0f, false, 0, 0, 6, 9, 500, 700},
    {900, 420, 40, 40, -1.0f, true, 0, 0, 6, 9, 800, 1000},
    {1300, 420, 40, 40, 0.8f, false, 0, 0, 6, 9, 1200, 1400}
};

#define MAX_PLATFORMS 10
typedef struct {
    SDL_Rect rect;
    bool isActive;
} Platform;

Platform platforms[MAX_PLATFORMS] = {
    {{250, 450, 120, 20}, true},
    {{400, 350, 150, 20}, true},
    {{600, 250, 100, 20}, true},
    {{100, 250, 180, 20}, true},
    {{300, 150, 130, 20}, true},
    {{980, 500, 50, 20}, true},
    {{1130, 380, 50, 20}, true},
    {{1330, 280, 50, 20}, true},
    {{1550, 150, 50, 20}, true},
    {{1700, 500, 50, 20}, true}
};

typedef struct {
    float x, y;
    float w, h;
    float vx, vy;
    bool onGround;
    bool facingLeft;
    int frame;
    int frameDelay;
    int frameTimer;
    int totalFrames;
    int frameWidth, frameHeight;
} Player;

SDL_Rect goal = { 1700, 420, 70, 90 };

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
    player->frame = 0;
    player->frameDelay = 6;
    player->frameTimer = 0;
    player->totalFrames = 12;
    player->frameWidth = 32;
    player->frameHeight = 32;
    player->w = 50;
    player->h = 50;

    score = 0;

    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].collected = false;
        coins[i].frame = 0;
        coins[i].frameTimer = 0;
        coins[i].frameDelay = 8;
        coins[i].totalFrames = 6;
        coins[i].frameWidth = 32;
        coins[i].frameHeight = 32;
    }

    // Reset enemies to original positions with animation properties
    enemies[0] = (Enemy){600, 420, 40, 40, 1.0f, false, 0, 0, 6, 9, 500, 700};
    enemies[1] = (Enemy){900, 420, 40, 40, -1.0f, true, 0, 0, 6, 9, 800, 1000};
    enemies[2] = (Enemy){1300, 420, 40, 40, 0.8f, false, 0, 0, 6, 9, 1200, 1400};
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
    playerTexture = loadTexture("assets/Pixel Adevnture/Main Characters/Virtual Guy/Run (32x32).png");
    if (!playerTexture) {
        printf("Failed to load player texture! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    
    goalTexture = loadTexture("assets/Pixel Adevnture/Items/Checkpoints/End/End (Idle).png");
    if (!goalTexture) {
        printf("Failed to load Flag texture! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    bgTexture = loadTexture("assets/Pixel Adevnture/Background/Blue.png");
    if (!bgTexture) {
        printf("Failed to load Background texture! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // Load terrain texture
    terrainTexture = loadTexture("assets/Terrain (16x16).png");
    if (!terrainTexture) {
        printf("Failed to load terrain texture! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // Load enemy texture (with animation frames)
    enemyTexture = loadTexture("assets/Pixel Adevnture/Enemies/BlueBird/Flying (32x32).png");
    if (!enemyTexture) {
        printf("Failed to load enemy texture! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    // Load coin texture
    coinTexture = loadTexture("assets/Pixel Adevnture/Items/Fruits/Apple.png");
    if (!coinTexture) {
        printf("Failed to load coin texture! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    
    // Load platform texture
    platformTexture = loadTexture("assets/Pixel Adevnture/Terrain/Terrain (16x16).png");
    if (!platformTexture) {
        printf("Failed to load platform texture! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    return true;
}


bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

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
        "2D Platformer",
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


void cleanupSDL() {
    SDL_DestroyTexture(terrainTexture);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(coinTexture);
    SDL_DestroyTexture(platformTexture);
    terrainTexture = NULL;
    playerTexture = NULL;
    enemyTexture = NULL;
    coinTexture = NULL;
    platformTexture = NULL;

    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


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

    if ((keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_SPACE]) && player->onGround) {
        player->vy = jumpStrength;
        player->onGround = false;
    }
}


void updatePhysics(Player* player) {
    const float gravity = 0.5f;

    if (!player->onGround) { //gravity
        player->vy += gravity;
    }

    player->x += player->vx;
    player->y += player->vy;


    player->onGround = false;

   
    if (player->vx != 0) {   //player
        player->frameTimer++;
        if (player->frameTimer >= player->frameDelay) {
            player->frame++;
            if (player->frame >= player->totalFrames) {
                player->frame = 0;
            }
            player->frameTimer = 0;
        }
    } else {
        player->frame = 0;  
    }

    
    for (int i = 0; i < MAX_ENEMIES; i++) {  //enemy
        enemies[i].x += enemies[i].vx;

        if (enemies[i].vx < 0) {
            enemies[i].facingLeft = false;
        } else if (enemies[i].vx > 0) {
            enemies[i].facingLeft = true;
        }


        enemies[i].frameTimer++;
        if (enemies[i].frameTimer >= enemies[i].frameDelay) {
            enemies[i].frame++;
            if (enemies[i].frame >= enemies[i].totalFrames) {
                enemies[i].frame = 0;
            }
            enemies[i].frameTimer = 0;
        }

        
        if (enemies[i].x <= enemies[i].patrolStart || enemies[i].x >= enemies[i].patrolEnd) {
            enemies[i].vx *= -1;
        }
    }


    if (player->x <= 0) player->x = 0;
    if (player->x >= LEVEL_WIDTH - player->w) player->x = LEVEL_WIDTH - player->w;

   
    for (int i = 0; i < MAX_COINS; i++) {  // Coin Animation
        if (!coins[i].collected) {
            coins[i].frameTimer++;
            if (coins[i].frameTimer >= coins[i].frameDelay) {
                coins[i].frame++;
                if (coins[i].frame >= coins[i].totalFrames) {
                    coins[i].frame = 0;
                }
                coins[i].frameTimer = 0;
            }
        }
    }
}

// Check collisions 
void checkCollisions(Player* player) {
    
    float groundY = WINDOW_HEIGHT - GROUND_HEIGHT - player->h;
   
    if (player->y >= groundY && player->x < 800 ) {
        player->y = groundY;
        player->vy = 0;
        player->onGround = true;
    }

    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].isActive) continue;
        
        SDL_Rect *plat = &platforms[i].rect;

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
    SDL_Delay(1000);  

    SDL_FreeSurface(messageSurface);
    SDL_DestroyTexture(messageTexture);
}


void checkGoalCollision(Player* player) {
    if (player->x + player->w > goal.x &&
        player->x < goal.x + goal.w &&
        player->y + player->h > goal.y &&
        player->y < goal.y + goal.h) {
        
        displayMessage("You win!", (SDL_Color){255, 255, 0});
        resetGame(player);
    }
}


void checkFallDetection(Player* player) {
    if (player->y > WINDOW_HEIGHT + 100) {
        displayMessage("Game Over! You fell!", (SDL_Color){255, 0, 0});
        resetGame(player);
    }
}


void updateCamera(Player player) {
    camera.x = (int)(player.x + player.w / 2) - WINDOW_WIDTH / 2;
    camera.y = (int)(player.y + player.h / 2) - WINDOW_HEIGHT / 2;
    
    
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > LEVEL_WIDTH - WINDOW_WIDTH) camera.x = LEVEL_WIDTH - WINDOW_WIDTH;
}


void renderScene(Player player) {
    
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

   
    SDL_Rect groundSrcRect = { 0, 0, 32, 32 }; // The grass+dirt tile
    SDL_Rect dirtSrcRect = { 0, 16, 32, 16 };  // Just the dirt part

    
    int startX = camera.x / 32;                       
    int endX = (camera.x + WINDOW_WIDTH) / 32 + 1;    
    int groundX = 0;                                  
    int groundEndX = 800 / 32;;              

    //ground
    for (int i = startX; i <= endX; i++) {
        
        if (i >= groundX && i < groundEndX) {
            
            SDL_Rect groundDestRect = {
                (i * 32) - camera.x,
                WINDOW_HEIGHT - GROUND_HEIGHT - camera.y,
                32,
                32
            };
            SDL_RenderCopy(renderer, terrainTexture, &groundSrcRect, &groundDestRect);
            
            // dirt tiles
            for (int j = 1; j < (GROUND_HEIGHT / 16); j++) {
                SDL_Rect dirtDestRect = {
                    (i * 32) - camera.x,
                    WINDOW_HEIGHT - GROUND_HEIGHT + (j * 16) - camera.y,
                    32,
                    16
                };
                SDL_RenderCopy(renderer, terrainTexture, &dirtSrcRect, &dirtDestRect);
            }
        }
    }

    
    SDL_Rect platformSrcRect = { 96, 0, 16, 16 }; 

    
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].isActive) continue;
        
        SDL_Rect platRect = {
            platforms[i].rect.x - camera.x,
            platforms[i].rect.y - camera.y,
            platforms[i].rect.w,
            platforms[i].rect.h
        };
        
        
        int tilesNeeded = platforms[i].rect.w / 16;
        for (int j = 0; j < tilesNeeded; j++) {
            SDL_Rect tileDestRect = {
                platforms[i].rect.x - camera.x + (j * 16),
                platforms[i].rect.y - camera.y,
                16,
                platforms[i].rect.h
            };
            SDL_RenderCopy(renderer, platformTexture, &platformSrcRect, &tileDestRect);
        }
    }

    //player
    SDL_Rect playerDestRect = {
        (int)(player.x - camera.x),
        (int)(player.y - camera.y),
        (int)player.w, (int)player.h
    };
    

    SDL_Rect playerSrcRect = {
        player.frame * player.frameWidth,
        0,
        player.frameWidth,
        player.frameHeight
    };
    
    
    SDL_RendererFlip flip = player.facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    
    
    SDL_RenderCopyEx(renderer, playerTexture, &playerSrcRect, &playerDestRect, 0.0, NULL, flip);

    
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].collected) continue;

        SDL_Rect coinDestRect = {
            coins[i].x - camera.x,
            coins[i].y - camera.y,
            coins[i].w,
            coins[i].h
        };
        
        SDL_Rect coinSrcRect = {
            coins[i].frame * coins[i].frameWidth,
            0,
            coins[i].frameWidth,
            coins[i].frameHeight
        };
        
        SDL_RenderCopy(renderer, coinTexture, &coinSrcRect, &coinDestRect);
    }

    // enemy animation
    for (int i = 0; i < MAX_ENEMIES; i++) {
        SDL_Rect enemyDestRect = {
            enemies[i].x - camera.x,
            enemies[i].y - camera.y,
            enemies[i].w,
            enemies[i].h
        };
        
        
        SDL_Rect enemySrcRect = {
            enemies[i].frame * 32,  
            0,
            32,                      
            32
        };
        
        
        SDL_RendererFlip enemyFlip = enemies[i].facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, enemyTexture, &enemySrcRect, &enemyDestRect, 0.0, NULL, enemyFlip);
    }

   
    SDL_Rect goalDraw = {
        goal.x - camera.x,
        goal.y - camera.y,
        goal.w,
        goal.h
    };
    SDL_RenderCopy(renderer, goalTexture, NULL, &goalDraw);
    

    
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);

    SDL_Color textColor = { 255, 255, 255 }; 
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {

    if (!initSDL()) {   
        return 1;
    }
    
    
    if (!loadMedia()) {
        return 1;
    }
    
    
    Player player = {START_X, START_Y, 50, 50, 0, 0, false, false};
    
    
    resetGame(&player);
   

    bool running = true;
    while (running) {
        
        handleInput(&player, &running);
        
       
        updatePhysics(&player);
        
        
        checkCollisions(&player);
        checkEnemyCollisions(&player);
        checkGoalCollision(&player);
        checkFallDetection(&player);
        
        
        updateCamera(player);
        
        
        renderScene(player);
        
        
        SDL_Delay(16); 
    }
    
    
    cleanupSDL();
    
    return 0;
}