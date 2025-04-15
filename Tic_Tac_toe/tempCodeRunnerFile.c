
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        if (board[0][0] == COMPUTER) return 10;
        else if (board[0][0] == PLAYER) return -10;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
        if (board[0