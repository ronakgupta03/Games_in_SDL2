#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PLAYER 'X'
#define COMPUTER 'O'

// Magic square mapping
int magicSquare[3][3] = {
    {8, 1, 6},
    {3, 5, 7},
    {4, 9, 2}
};

char board[3][3] = {
    {' ', ' ', ' '},
    {' ', ' ', ' '},
    {' ', ' ', ' '}
};

// Stores selected numbers by player and computer
int playerMoves[5], computerMoves[5];
int playerCount = 0, computerCount = 0;

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

int isMovesLeft() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ')
                return 1;
    return 0;
}

int isWinningSet(int moves[], int count) {
    for (int i = 0; i < count; i++)
        for (int j = i + 1; j < count; j++)
            for (int k = j + 1; k < count; k++)
                if (moves[i] + moves[j] + moves[k] == 15)
                    return 1;
    return 0;
}

void getCoordinatesFromMagicValue(int value, int *x, int *y) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (magicSquare[i][j] == value) {
                *x = i;
                *y = j;
                return;
            }
}

int isCellFree(int value) {
    int x, y;
    getCoordinatesFromMagicValue(value, &x, &y);
    return board[x][y] == ' ';
}

void makeMove(char symbol, int value) {
    int x, y;
    getCoordinatesFromMagicValue(value, &x, &y);
    board[x][y] = symbol;
    if (symbol == PLAYER)
        playerMoves[playerCount++] = value;
    else
        computerMoves[computerCount++] = value;
}

void findBestMove() {
    // 1. Try to win
    for (int i = 0; i < computerCount; i++) {
        for (int j = i + 1; j < computerCount; j++) {
            int third = 15 - (computerMoves[i] + computerMoves[j]);
            if (third >= 1 && third <= 9 && isCellFree(third)) {
                makeMove(COMPUTER, third);
                return;
            }
        }
    }

    // 2. Try to block player
    for (int i = 0; i < playerCount; i++) {
        for (int j = i + 1; j < playerCount; j++) {
            int third = 15 - (playerMoves[i] + playerMoves[j]);
            if (third >= 1 && third <= 9 && isCellFree(third)) {
                makeMove(COMPUTER, third);
                return;
            }
        }
    }

    // 3. Take center if available
    if (isCellFree(5)) {
        makeMove(COMPUTER, 5);
        return;
    }

    // 4. Else take any available corner or edge
    int options[] = {2, 4, 6, 8, 1, 3, 7, 9};
    for (int i = 0; i < 8; i++) {
        if (isCellFree(options[i])) {
            makeMove(COMPUTER, options[i]);
            return;
        }
    }
}

void playerMove() {
    int row, col;
    while (1) {
        printf("Enter your move (row and column: 0-2 0-2): ");
        scanf("%d %d", &row, &col);
        if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ') {
            int val = magicSquare[row][col];
            makeMove(PLAYER, val);
            break;
        } else {
            printf("Invalid move, try again.\n");
        }
    }
}

int main() {
    srand(time(0));
    printf("Tic-Tac-Toe (Magic Square): You (X) vs Computer (O)\n");
    printBoard();

    while (1) {
        playerMove();
        printBoard();
        if (isWinningSet(playerMoves, playerCount)) {
            printf("You win!\n");
            break;
        }
        if (!isMovesLeft()) {
            printf("It's a draw!\n");
            break;
        }

        findBestMove();
        printBoard();
        if (isWinningSet(computerMoves, computerCount)) {
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
