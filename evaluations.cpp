
#include "evaluations.h"

int evaluateControl(const map<string, Piece>& simulationBoard) {
    int whiteTally = 0;
    int blackTally = 0;

    for (int a=0; a<8; ++a) {
        for (int b=0; b<8; ++b) {
            const string& index = string(1,static_cast<char>('a' + a)) + string(1, static_cast<char>('1' + b));
            if (simulationBoard.at(index).whiteThreat == true) whiteTally++;
            if (simulationBoard.at(index).blackThreat == true) blackTally++;
        }
    }
    return whiteTally - blackTally;
}