#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#define WINDOW_TITLE "Neon Tic-Tac-Toe"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 533
#define IMAGE_FLAGS IMG_INIT_PNG
#define GRID_START_X 250  
#define GRID_START_Y 120  
#define CELL_WIDTH 100    
#define CELL_HEIGHT 100 
#define GRID_ROWS 3
#define GRID_COLS 3

struct Game {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    SDL_Texture *X;
    SDL_Texture *O;
    int grid[GRID_ROWS][GRID_COLS];
    bool playerX;
};

void game_cleanup(struct Game *game, int exit_status);
bool sdl_initialize(struct Game *game);
bool loadMedia(struct Game *game);
void render_symbol(SDL_Renderer *renderer, SDL_Texture *texture, int row, int col);
void render_game(struct Game *game);
bool handle_click(struct Game *game, int x, int y);
bool check_winner(struct Game *game);

int main() {
    struct Game game = {
        .window = NULL,
        .renderer = NULL,
        .background = NULL,
        .X = NULL,
        .O = NULL,
        .playerX = true
    };

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            game.grid[row][col] = 0;
        }
    }

    if (sdl_initialize(&game)) {
        game_cleanup(&game, EXIT_FAILURE);
    }
    if (loadMedia(&game)) {
        game_cleanup(&game, EXIT_FAILURE);
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                    
                case SDL_KEYDOWN:
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        running = false;
                    } else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                        for (int row = 0; row < GRID_ROWS; row++) {
                            for (int col = 0; col < GRID_COLS; col++) {
                                game.grid[row][col] = 0;
                            }
                        }
                        game.playerX = true;
                    }
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        handle_click(&game, event.button.x, event.button.y);
                    }
                    break;
            }
        }

       
        render_game(&game);
        
        SDL_Delay(16);
    }

    game_cleanup(&game, EXIT_SUCCESS);
    return 0;
}

bool handle_click(struct Game *game, int x, int y) {

    if (x < GRID_START_X || y < GRID_START_Y || 
        x >= GRID_START_X + CELL_WIDTH * GRID_COLS || 
        y >= GRID_START_Y + CELL_HEIGHT * GRID_ROWS) {
        printf("Click outside grid (%d, %d)\n", x, y);
        return false;
    }
    
    
    int col = (x - GRID_START_X) / CELL_WIDTH;
    int row = (y - GRID_START_Y) / CELL_HEIGHT;
    
    printf("Mouse clicked at (%d, %d)\n", x, y);
    printf("Grid cell clicked: row=%d, col=%d\n", (row+1), (col+1));
    

    if (game->grid[row][col] != 0 || check_winner(game)) {
        printf("Cell already occupied or game is over\n");
        return false;
    }
    

    game->grid[row][col] = game->playerX ? 1 : 2;
    
    
    if (check_winner(game)) {
        printf("Player %c wins!\n", game->playerX ? 'X' : 'O');
    } else {
        
        game->playerX = !game->playerX;
    }
    
    return true;
}

bool check_winner(struct Game *game) {
    // Checks rows
    for (int row = 0; row < GRID_ROWS; row++) {
        if (game->grid[row][0] != 0 && 
            game->grid[row][0] == game->grid[row][1] && 
            game->grid[row][0] == game->grid[row][2]) {
            return true;
        }
    }
    
    // Check columns
    for (int col = 0; col < GRID_COLS; col++) {
        if (game->grid[0][col] != 0 && 
            game->grid[0][col] == game->grid[1][col] && 
            game->grid[0][col] == game->grid[2][col]) {
            return true;
        }
    }
    // Checks Diagonals
    if (game->grid[0][0] != 0 && 
        game->grid[0][0] == game->grid[1][1] && 
        game->grid[0][0] == game->grid[2][2]) {
        return true;
    }
    
    if (game->grid[0][2] != 0 && 
        game->grid[0][2] == game->grid[1][1] && 
        game->grid[0][2] == game->grid[2][0]) {
        return true;
    }
    
    return false;
}

void render_game(struct Game *game) {
    SDL_RenderClear(game->renderer);
    SDL_RenderCopy(game->renderer, game->background, NULL, NULL);
    
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            if (game->grid[row][col] == 1) {
                render_symbol(game->renderer, game->X, row, col);
            } else if (game->grid[row][col] == 2) {
                render_symbol(game->renderer, game->O, row, col);
            }
        }
    }
    
    SDL_RenderPresent(game->renderer);
}

void game_cleanup(struct Game *game, int exit_status) {
    SDL_DestroyTexture(game->O);
    SDL_DestroyTexture(game->X);
    SDL_DestroyTexture(game->background);
    IMG_Quit();
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
    exit(exit_status);
}

bool sdl_initialize(struct Game *game) {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return true;
    }

    int img_init = IMG_Init(IMAGE_FLAGS);
    if ((img_init&IMAGE_FLAGS)!=IMAGE_FLAGS){
        fprintf(stderr, "Error initializing SDL_image: %s\n", IMG_GetError());
        return true;
    }

    game->window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                    SCREEN_HEIGHT, 0);
    if (!game->window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        return true;
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, 0);
    if (!game->renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        return true;
    }

    return false;
}

bool loadMedia(struct Game *game){
    game->background = IMG_LoadTexture(game->renderer, "images/background.png");
    game->X = IMG_LoadTexture(game->renderer, "images/X.png");
    game->O = IMG_LoadTexture(game->renderer, "images/O.png");
    if (!game->background) {
        fprintf(stderr, "Error creating Background: %s\n", IMG_GetError());
        return true;
    }
    if (!game->X) {
        fprintf(stderr, "Error creating X: %s\n", IMG_GetError());
        return true;
    }
    if (!game->O) {
        fprintf(stderr, "Error creating O: %s\n", IMG_GetError());
        return true;
    }

    return false;
}

void render_symbol(SDL_Renderer *renderer, SDL_Texture *texture, int row, int col) {
    SDL_Rect dest = {
        .x = GRID_START_X + col * CELL_WIDTH,
        .y = GRID_START_Y + row * CELL_HEIGHT,
        .w = CELL_WIDTH,
        .h = CELL_HEIGHT
    };

    SDL_RenderCopy(renderer, texture, NULL, &dest);
}
