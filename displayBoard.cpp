#include <iostream>
#include "displayBoard.h"

using namespace std;

template <typename Map>
void printBoard(Map& board) {
    string colour = "\033[0m";
    for (int b=7; b>=0; --b) {
        cout <<" "<<b+1<<" ";
        for (int a=0; a<8; ++a) {
            char a_char = static_cast<char>('a' + a);
            char b_char = static_cast<char>('1' + b);
            string index = string(1,a_char) + string(1,b_char);
            if (board.at(index).piece != 0) {
                if (board.at(index).white == true) {
                    colour = "\033[97m";
                } else {
                    colour = "\033[37m";
                }
                cout<<colour<<board.at(index).piece<<"\033[0m ";
            } else {
                cout<<". ";
            }
            if (a_char == 'h') {cout<<endl;}
        }
    }
    cout<<"   A B C D E F G H"<<endl;
    cout<<"test: "<<board.at("a1").whiteThreat<<" : "<<board.at("a1").blackThreat<<endl;
}

void displayBoard(const map<string, Piece>& board) {
printBoard(board);
}
