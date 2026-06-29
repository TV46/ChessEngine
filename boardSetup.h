
#ifndef CHESSENGINE_BOARDSETUP_H
#define CHESSENGINE_BOARDSETUP_H

using namespace std;

#include <map>
#include <string>

struct Piece {
    char piece;
    bool white;
    bool whiteThreat;
    bool blackThreat;
};


#endif //CHESSENGINE_BOARDSETUP_H
