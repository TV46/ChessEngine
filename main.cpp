#include <iostream>
#include <cmath>
#include <vector>
#include <tuple>

class chessBoard {
private:
    struct Piece {
        char piece;
        bool isWhite;
    };
protected:
    void clear(int x, int y) {
        Piece board[x][y];
    }
public:
    void static move(int startX, int startY, int targetX, int targetY) {
        clear(startX, startY);

    }
};


int main() {

}

