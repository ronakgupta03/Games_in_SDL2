#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PLAYER 'X'
#define COMPUTER 'O'

char board[3][3] = {
    {' ', ' ', ' '},
    {' ', ' ', ' '},
    {' ', ' ', ' '}
};

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

int checkWin(char symbol) {
    for (int i = 0; i < 3; i++)
        if (board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol)
            return 1;
    for (int i = 0; i < 3; i++)
        if (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol)
            return 1;
    if (board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol)
        return 1;
    if (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol)
        return 1;
    return 0;
}

// Try to find a winning move for the given symbol
int tryToWin(char symbol, int *bestRow, int *bestCol) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == ' ') {
                board[i][j] = symbol;
                int won = checkWin(symbol);
                board[i][j] = ' ';  // Undo
                if (won) {
                    *bestRow = i;
                    *bestCol = j;
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Make a random move
void playRandom(int *bestRow, int *bestCol) {
    int emptyCells[9][2];
    int count = 0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ') {
                emptyCells[count][0] = i;
                emptyCells[count][1] = j;
                count++;
            }
    if (count > 0) {
        int choice = rand() % count;
        *bestRow = emptyCells[choice][0];
        *bestCol = emptyCells[choice][1];
    }
}

// AI move: Win > Block > Random
void findBestMove() {
    int row, col;

    // 1. Can I win?
    if (tryToWin(COMPUTER, &row, &col)) {
        board[row][col] = COMPUTER;
        return;
    }

    // 2. Can the player win next? Block them
    if (tryToWin(PLAYER, &row, &col)) {
        board[row][col] = COMPUTER;
        return;
    }

    // 3. Play randomly
    playRandom(&row, &col);
    board[row][col] = COMPUTER;
}

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
    srand(time(0));
    printf("Tic-Tac-Toe: You (X) vs Computer (O)\n");
    printBoard();

    while (1) {
        playerMove();
        printBoard();
        if (checkWin(PLAYER)) {
            printf("You win!\n");
            break;
        }
        if (!isMovesLeft()) {
            printf("It's a draw!\n");
            break;
        }

        findBestMove();
        printBoard();
        if (checkWin(COMPUTER)) {
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
