#include <stdio.h>
#include <limits.h>

#define PLAYER 'X'
#define COMPUTER 'O'

char board[3][3] = {
    {' ', ' ', ' '},
    {' ', ' ', ' '},
    {' ', ' ', ' '}
};

// Function to print the board
void printBoard() {
    printf("\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf(" %c ", board[i][j]);
            if (j < 2) printf("|");
        }
        printf("\n");
        if (i < 2) printf("---+---+---\n");
    }
    printf("\n");
}

// Function to check if a player has won
int evaluate() {
    for (int row = 0; row < 3; row++) {
        if (board[row][0] == board[row][1] && board[row][1] == board[row][2]) {
            if (board[row][0] == COMPUTER) return 10;
            else if (board[row][0] == PLAYER) return -10;
        }
    }
    for (int col = 0; col < 3; col++) {
        if (board[0][col] == board[1][col] && board[1][col] == board[2][col]) {
            if (board[0][col] == COMPUTER) return 10;
            else if (board[0][col] == PLAYER) return -10;
        }
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        if (board[0][0] == COMPUTER) return 10;
        else if (board[0][0] == PLAYER) return -10;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        if (board[0][2] == COMPUTER) return 10;
        else if (board[0][2] == PLAYER) return -10;
    }
    return 0;
}

// Function to check if the move is available
int isMovesLeft() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ')
                return 1;
    return 0;
}

// Function to check if the opponent can win and return the blocking move
int blockMove() {
    for (int row = 0; row < 3; row++) {
        if (board[row][0] == board[row][1] && board[row][0] == PLAYER && board[row][2] == ' ') return 2;
        if (board[row][1] == board[row][2] && board[row][1] == PLAYER && board[row][0] == ' ') return 0;
        if (board[row][0] == board[row][2] && board[row][0] == PLAYER && board[row][1] == ' ') return 1;
    }
    for (int col = 0; col < 3; col++) {
        if (board[0][col] == board[1][col] && board[0][col] == PLAYER && board[2][col] == ' ') return 6 + col;
        if (board[1][col] == board[2][col] && board[1][col] == PLAYER && board[0][col] == ' ') return col;
        if (board[0][col] == board[2][col] && board[0][col] == PLAYER && board[1][col] == ' ') return 3 + col;
    }
    if (board[0][0] == board[1][1] && board[0][0] == PLAYER && board[2][2] == ' ') return 8;
    if (board[1][1] == board[2][2] && board[1][1] == PLAYER && board[0][0] == ' ') return 0;
    if (board[0][2] == board[1][1] && board[0][2] == PLAYER && board[2][0] == ' ') return 6;
    if (board[1][1] == board[2][0] && board[1][1] == PLAYER && board[0][2] == ' ') return 2;
    return -1; // No move to block
}

// Function to find the best move for the computer
void findBestMove() {
    int block = blockMove(); // Check if we need to block the opponent
    if (block != -1) {
        board[block / 3][block % 3] = COMPUTER;  // Block the opponent
        return;
    }

    // If no blocking move is found, place in the center if available
    if (board[1][1] == ' ') {
        board[1][1] = COMPUTER;
        return;
    }

    // Otherwise, place on any available spot (randomly)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == ' ') {
                board[i][j] = COMPUTER;
                return;
            }
        }
    }
}

// Function for the player's move
void playerMove() {
    int row, col;
    while (1) {
        printf("Enter your move (row and column: 0-2 0-2): ");
        scanf("%d %d", &row, &col);
        if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ') {
            board[row][col] = PLAYER;
            break;
        } else {
            printf("Invalid move, try again.\n");
        }
    }
}

int main() {
    printf("Tic-Tac-Toe: You (X) vs Computer (O)\n");
    printBoard();

    while (1) {
        playerMove();
        printBoard();
        if (evaluate() == -10) {
            printf("You win!\n");
            break;
        }
        if (!isMovesLeft()) {
            printf("It's a draw!\n");
            break;
        }
        findBestMove();
        printBoard();
        if (evaluate() == 10) {
            printf("Computer wins!\n");
            break;
        }
        if (!isMovesLeft()) {
            printf("It's a draw!\n");
            break;
        }
    }
    return 0;
}
