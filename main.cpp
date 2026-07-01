#include "boardSetup.h"
#include "frontend.h"
#include "evaluations.h"
#include <iostream>
#include <cmath>
#include <vector>

string blackEnPassantAvailable;
string whiteEnPassantAvailable;


map<string, Piece> board;

vector<string> moveHistory;
vector<map<string, Piece>> boardHistory;

// Convert 4 bools to an int and vice versa
int boolMerge(bool one, bool two, bool three, bool four) {
    return (one << 3) | (two << 2) | (three << 1) | (four << 0);
}
tuple<bool, bool, bool, bool> boolSplit(const int input) {
    bool one = (input >> 3) & 1;
    bool two = (input >> 2) & 1;
    bool three = (input >> 1) & 1;
    bool four = (input >> 0) & 1;
    return {one, two, three, four};
}

// Handles failed moves
void failedMove(const string& print) {
    displayBoard(board);
    throw runtime_error(print);
}

// Exits with message if there are multiple possible pieces with no or not enough disambiguation
void disambiguationExit() {
    failedMove("Failed to disambiguate!");
}

//Parse Notation
tuple<string, char, bool, int, char, string> ParseNotation(string input) {
    char promotion;
    char piece = '0';
    int check = 0;
    bool capture = false;
    string position;
    string disambiguation;

    // account for castling
    if (!input.contains("O-O")) {

        int clear = 0;
        // Separate out target location
        for (int i=0; i<=input.size(); ++i) {
            if (input[i] >= '1' && input[i] <= '8') {
                position = input.substr(i - 1, 2);
                clear = i;
            }
        }
        if (clear == 0) {failedMove("Failed to find coordinate");}
        input.erase( clear - 1, 2);
        if (position.empty()) {failedMove("No move location");}

        // Check for promotion
        if (input.contains('=')) {
            promotion = input[input.find('=') + 1];
            input.erase(input.find('='), 2);

            // Verify promotion is a valid piece
            switch (promotion) {
                case 'R':
                case 'N':
                case 'B':
                case 'Q':
                    break;
                default: failedMove("Invalid promotion piece");;
            }
        }

        // Check for piece moved
        for (int i=0; i<input.size(); ++i) {
            if (input[i] != -1 && input[i] >= 65 && input[i] <= 90) {piece = input[i]; input.erase(i, 1); break;}
        }
        if (piece == '0') {piece = 'P';}

        // Check for capture
        if (input.contains('x')) {
            input.erase(input.find('x'), 1);
            capture = true;
        }
    }


    // Check for check or mate
    if (input.contains('#')) {
        input.erase(input.find('#'), 1);
        check = 2;
    } else if (input.contains('+')) {
        input.erase(input.find('+'), 1);
        check = 1;
    }

    if (input.contains("O-O")) {
        position = input;
        piece = 'C';
    } else {
        disambiguation = input;
    }
    if (position[0] < 'a' || position[0] > 'h' || position[1] < '1' || position[1] > '8') {failedMove("Out Of Bounds Coordinate");}

    return {position, piece, capture, check, promotion, disambiguation};
}

// handles empty indexes
void emptyHandler(const bool silent) {
    for (int a=0; a<8; ++a) {
        char y_pos = static_cast<char>('1' + a);
        for (int b=0; b<8; ++b) {
            char x_pos = static_cast<char>('a' + b);

            string index = string(1, x_pos) + string(1, y_pos);
            if (!board.contains(index)) {
                board[index] = {0};
                if (!silent) {
                    cout<<"DEBUG WARNING: Empty index location"<<endl;
                    cin.get();
                }
            }
        }
    }
}

// Reset Board
void reset() {
    board.clear();
    moveHistory.clear();

    board["a1"] = {'R', true};
    board["b1"] = {'N', true};
    board["c1"] = {'B', true};
    board["d1"] = {'Q', true};
    board["e1"] = {'K', true};
    board["f1"] = {'B', true};
    board["g1"] = {'N', true};
    board["h1"] = {'R', true};

    board["a2"] = {'P', true};
    board["b2"] = {'P', true};
    board["c2"] = {'P', true};
    board["d2"] = {'P', true};
    board["e2"] = {'P', true};
    board["f2"] = {'P', true};
    board["g2"] = {'P', true};
    board["h2"] = {'P', true};

    board["a7"] = {'P', false};
    board["b7"] = {'P', false};
    board["c7"] = {'P', false};
    board["d7"] = {'P', false};
    board["e7"] = {'P', false};
    board["f7"] = {'P', false};
    board["g7"] = {'P', false};
    board["h7"] = {'P', false};

    board["a8"] = {'R', false};
    board["b8"] = {'N', false};
    board["c8"] = {'B', false};
    board["d8"] = {'Q', false};
    board["e8"] = {'K', false};
    board["f8"] = {'B', false};
    board["g8"] = {'N', false};
    board["h8"] = {'R', false};

    emptyHandler(true);
}
// Set board to custom
void customBoard() {
    board.clear();
    board["e5"] = {'P', true};
    board["d7"] = {'P', false};

    emptyHandler(true);
}

// adds a piece to a location while also clearing a target location effectively moving the piece
void movePiece(const string& position, char piece, const string& clear, bool white) {

    board[position] = {piece, white};

    if (!clear.empty()) {
        board[clear].piece = 0;
    }
}

// offset location by a set value in either x or y
string offsetLocation(const string& position, int x_off, int y_off) {
    return string(1, static_cast<char>(position[0] + x_off)) + string(1, static_cast<char>(position[1] + y_off));
}

// Parses location and outputs whether there is a piece there of a certain colour (also includes an offset variable)
bool checkLocation(const string& position, char piece, int x_off, int y_off, bool white) {
    if (offsetLocation(position, x_off, y_off)[0] < 'a' ||
        offsetLocation(position, x_off, y_off)[0] > 'h' ||
        offsetLocation(position, x_off, y_off)[1] < '1' ||
        offsetLocation(position, x_off, y_off)[1] > '8') {
        return false;
    }

    if (board.at(offsetLocation(position, x_off, y_off)).piece != 0 &&
        board.at(offsetLocation(position, x_off, y_off)).piece == piece &&
        board.at(offsetLocation(position, x_off, y_off)).white == white) {
        return true;
    }
    return false;
}

// Make disambiguation always have a full position
bool verifyDisambiguation(const string& disambiguation, const string& reference) {
    string disambiguated;

    if (disambiguation.empty()) {
        disambiguated = reference;
    } else {
        char disambig_x = 0;
        char disambig_y = 0;
        if (disambiguation[0] >= 97 && disambiguation[0] <= 104) {
            disambig_x = disambiguation[0];
            if (disambiguation.size() == 2) {
                disambig_y = disambiguation[1];
            }
        } else {
            disambig_y = disambiguation[0];
        }
        if (disambiguation.empty()) {
            disambig_y = 0;
        }

        if (disambig_x == 0) {
            disambiguated =  reference[0] + string(1, disambig_y);
        } else if (disambig_y == 0) {
            disambiguated = string(1, disambig_x) + reference[1];
        } else {disambiguated = disambiguation;}
    }

    if (disambiguated == reference) {
        return true;
    }
    return false;
}

// A combination of the checkLocation anf verifyDisambiguation commands
bool verifyLocation(const string& position, char piece, int x_off, int y_off, bool white, const string& disambiguation) {
    if (checkLocation(position, piece, x_off, y_off, white) && verifyDisambiguation(disambiguation, offsetLocation(position, x_off, y_off))) {
        //debug: cout<<"Debug: "<<offsetLocation(position, x_off, y_off)<<endl;
        return true;
    }
    return false;
}

// Check if a positon can be captured or moved to
bool checkClear(const string& position, bool capture, bool white) {
    if (board.at(position).piece == 0 || (capture && board.at(position).white != white)) {
        return true;
    }
    return false;
}

// All pawn movement logic
string pawnLogic(const string& target, char piece, bool capture, const string& disambiguation, bool white) {

    string position;

    int y_off1 = -1;
    int y_off2 = -2;
    char doubleMove = '4';
    string doubleClear = "3";
    string enPassantClear = "5";

    if (!white) {
        y_off1 = 1;
        y_off2 = 2;
        doubleMove = '5';
        doubleClear = "6";
        enPassantClear = "4";
    }

    if ((!whiteEnPassantAvailable.empty() && !white && whiteEnPassantAvailable == target) || (!blackEnPassantAvailable.empty() && white && blackEnPassantAvailable == target)){
        if (capture) {
            for (int i=-1; i<=1; ++i, ++i) {
                if (verifyLocation(target, piece, i, y_off1, white, disambiguation)) {
                    if (!position.empty()) {disambiguationExit();}

                    if (checkClear(target, capture, white)) {
                        position = offsetLocation(target, i, y_off1);
                    }
                }
            }
            board[string(1, target[0]) + enPassantClear].piece = 0;
            blackEnPassantAvailable = "";
            whiteEnPassantAvailable = "";
        } else {
            failedMove("You must capture on en passant");
        }
    } else {
        for (int i=-1; i<=1; ++i) {
            if (verifyLocation(target, piece, i, y_off1, white, disambiguation)) {
                if ((capture && i != 0) || (i == 0 && !capture)) {
                    if (!position.empty()) {disambiguationExit();}
                    if (checkClear(target, capture, white)) {
                        position = offsetLocation(target, i, y_off1);
                    }
                }
            }
        }
        blackEnPassantAvailable = "";
        whiteEnPassantAvailable = "";
        if (verifyLocation(target, piece, 0, y_off2, white, disambiguation) &&
            !capture && target[1] == doubleMove &&
            board.at(string(1, target[0])+ doubleClear).piece == 0) {

            if (!position.empty()) {disambiguationExit();}

            if (checkClear(target, capture, white)) {
                position = offsetLocation(target, 0, y_off2);
                if (white) {
                    whiteEnPassantAvailable = string(1, target[0]) + doubleClear;
                } else {
                    blackEnPassantAvailable = string(1, target[0]) + doubleClear;
                }
            }
            }
    }



    return position;
}
// All knight movement logic
string knightLogic(const string& target, char piece, bool capture, const string& disambiguation, bool white) {
    string position;
    for (int a=0; a<2; ++a) {
        for (int b=1; b<=2; ++b) {
            for (int c=1; c<=2; ++c) {
                const int x_off = (a + 1) * static_cast<int>(pow(-1, b));
                const int y_off = (2 - a) * static_cast<int>(pow(-1, c));

                if (verifyLocation(target, piece, x_off, y_off, white, disambiguation)) {
                    if (!position.empty()) {disambiguationExit();}
                    if (checkClear(target, capture, white)) {
                        position = offsetLocation(target, x_off, y_off);
                    }
                }
            }
        }
    }

    return position;
}
// All bishop movement logic
string bishopLogic(const string& target, char piece, bool capture, const string& disambiguation, bool white) {
    string position;
    for (int a=0; a<=1; ++a) {
        for (int b=0; b<=1; ++b) {
            int x_increment = static_cast<int>(pow(-1,b));
            int y_increment = static_cast<int>(pow(-1,a));

            for (int c=1; c<=8; ++c) {
                int x_off = c*x_increment;
                int y_off = c*y_increment;

                // Stop check if out of bounds
                if (target[0]+x_off > 'h' || target[1]+y_off > '8' || target[0]+x_off < 'a' || target[1]+y_off < '1') {break;}
                // stop check if it comes in contact with a piece that isn't own rook
                if (board.at(offsetLocation(target, x_off, y_off)).piece != 0 && (board.at(offsetLocation(target, x_off, y_off)).piece != piece || board.at(offsetLocation(target, x_off, y_off)).white != white)) {break;}
                if (verifyLocation(target, piece, x_off, y_off, white, disambiguation)) {
                    if (!position.empty()) {disambiguationExit();}
                    if (checkClear(target, capture, white)) {
                        position = offsetLocation(target, x_off, y_off);
                        break;
                    }
                }
            }
        }
    }


    return position;
}
// All rook movement logic
string rookLogic(const string& target, char piece, bool capture, const string& disambiguation, bool white) {
    string position;

    int x_increment;
    int y_increment;

    for (int a=0; a<=1; ++a) {
        for (int b=0; b<=1; ++b) {
            if (a == 1) {
                x_increment = static_cast<int>(pow(-1,b));
                y_increment = 0;
            } else {
                x_increment = 0;
                y_increment = static_cast<int>(pow(-1,b));
            }
            for (int c=1; c<=8; ++c) {
                int x_off = c*x_increment;
                int y_off = c*y_increment;

                // Stop check if out of bounds
                if (target[0]+x_off > 'h' || target[1]+y_off > '8' || target[0]+x_off < 'a' || target[1]+y_off < '1') {break;}
                // stop check if it comes in contact with a piece that isn't own rook
                if (board.at(offsetLocation(target, x_off, y_off)).piece != 0 && (board.at(offsetLocation(target, x_off, y_off)).piece != piece || board.at(offsetLocation(target, x_off, y_off)).white != white)) {break;}
                if (verifyLocation(target, piece, x_off, y_off, white, disambiguation)) {
                    if (!position.empty()) {disambiguationExit();}
                    if (checkClear(target, capture, white)) {
                        position = offsetLocation(target, x_off, y_off);
                        break;
                    }
                }
            }
        }
    }


    return position;
}
// All queen movement logic
string queenLogic(const string& target, char piece, bool capture, const string& disambiguation, bool white) {
    string position;

    string rook = rookLogic(target, piece, capture, disambiguation, white);
    string bishop = bishopLogic(target, piece, capture, disambiguation, white);

    if (rook.empty() && !bishop.empty()) {
        position = bishop;
    } else if (!rook.empty() && bishop.empty()) {
        position = rook;
    } else {
        failedMove("Queen bishop-rook overlap exception.");
    }

    return position;
}
// All king movement logic
string kingLogic(const string& target, char piece, bool capture, bool white) {
    string position;
    if ((board.at(target).blackSight && white) || (board.at(target).whiteSight && !white)) {
        failedMove("King may not move into check.");
    }

    for (int a=-1; a<=1; ++a) {
        for (int b=-1; b<=1; ++b) {
            const char a_char = static_cast<char>(target[0] + a);
            const char b_char = static_cast<char>(target[1] + b);
            string index = string(1,a_char) + string(1,b_char);
            if (a_char >= 'a' && a_char <= 'h' && b_char >= '1' && b_char <= '8') {
                if (board.at(index).piece != 0 && board.at(index).white == white && board.at(index).piece == piece && (board.at(target).piece == 0 || (board.at(target).white != white && capture))) {
                    position = index;
                }
            }

        }
    }

    return position;
}
// All castling logic
tuple<string, string, string, string> castleLogic(const string& target, bool white, int castleAvailable) {

    auto [whiteShortCastle, whiteLongCastle, blackShortCastle, blackLongCastle] = boolSplit(castleAvailable);

    bool Long = false;
    if (target == "O-O-O") {
        Long = true;
    }

    string castleKing = "e1";
    string shortCastleRook = "h1";
    string longCastleRook = "a1";
    string shortCastleKingTarget = "g1";
    string longCastleKingTarget = "c1";
    string shortCastleRookTarget = "f1";
    string longCastleRookTarget = "d1";
    string longCastleBlocking = "b1";
    if (!white) {

        castleKing = "e8";
        shortCastleRook = "h8";
        longCastleRook = "a8";
        shortCastleKingTarget = "g8";
        longCastleKingTarget = "c8";
        shortCastleRookTarget = "f8";
        longCastleRookTarget = "d8";
        longCastleBlocking = "b8";
    }

    if ((board.at(longCastleBlocking).blackSight && white) || (board.at(longCastleBlocking).whiteSight && !white)) {
        failedMove("King may not move through check");
    }



    string castleKingTarget = shortCastleKingTarget;
    string castleRookTarget = shortCastleRookTarget;
    string castleRook = shortCastleRook;
    if (Long) {
        castleKingTarget = longCastleKingTarget;
        castleRookTarget = longCastleRookTarget;
        castleRook = longCastleRook;
    }

    // ensure the pieces being moved exist
    if (board.at(castleKing).piece == 0 ||
        board.at(castleKing).piece != 'K' ||
        board.at(castleKing).white != white ||
        board.at(castleRook).piece == 0 ||
        board.at(castleRook).piece != 'R' ||
        board.at(castleRook).white != white) {failedMove("Piece not found");}

    if (board.at(castleKingTarget).piece != 0 ||
        board.at(castleRookTarget).piece != 0) {failedMove("Piece Blocking castle");}

    if (Long && board.at(longCastleBlocking).piece != 0) {failedMove("Piece blocking long castle");}

    if (white) {
        if ((Long && !whiteLongCastle) || (!Long && !whiteShortCastle)) {
            failedMove("Castle unavailable");
        }
    } else {
        if ((Long && !blackLongCastle) || (!Long && !blackShortCastle)) {
            failedMove("Castle unavailable");
        }
    }

    if ((board.at(castleKingTarget).blackSight && white) || (board.at(castleKingTarget).whiteSight && !white)) {
        failedMove("King may not move into check");
    }

    return {castleKing, castleKingTarget, castleRook, castleRookTarget};
}

// All pawn threat detection logic
int pawnScan(const string& target, bool white) {

    int y_off = -1;

    if (!white) {
        y_off = 1;
    }
    int count = 0;

    //en passant threat detection code
    if (board.at(target).piece == 'P') {
        if (white && string(1, blackEnPassantAvailable[0]) + '5' == target) {

            for (int i=-1; i<=1; ++i, ++i) {
                if (board.at(offsetLocation(target, i, 0)).piece == 'P' && board.at(offsetLocation(target, i, 0)).white == white) {
                    count++;

                }
            }
        }
        if (!white && string(1, whiteEnPassantAvailable[0]) + '4' == target) {
            for (int i=-1; i<=1; ++i, ++i) {
                if (target[0] + i >= 'a' && target[0] + i <= 'h') {
                    if (board.at(offsetLocation(target, i, 0)).piece == 'P' && board.at(offsetLocation(target, i, 0)).white == white) {
                        count++;

                    }
                }
            }
        }
    }

    // regular pawn attack threat detection code
    for (int i=-1; i<=1; ++i, ++i) {
        if (checkLocation(target, 'P', i, y_off, white)) {
            count++;
        }
    }


    return count;
}
// All knight threat detection logic
int knightScan(const string& target, bool white) {
    int count = 0;
    for (int a=0; a<2; ++a) {
        for (int b=1; b<=2; ++b) {
            for (int c=1; c<=2; ++c) {
                const int x_off = (a + 1) * static_cast<int>(pow(-1, b));
                const int y_off = (2 - a) * static_cast<int>(pow(-1, c));

                if (checkLocation(target, 'N', x_off, y_off, white)) {
                    count++;
                }
            }
        }
    }
    return count;
}
// All bishop threat detection logic
int bishopScan(const string& target, const char piece, bool white) {
    int count = 0;
    for (int a=0; a<=1; ++a) {
        for (int b=0; b<=1; ++b) {
            int x_increment = static_cast<int>(pow(-1,b));
            int y_increment = static_cast<int>(pow(-1,a));

            for (int c=1; c<=8; ++c) {
                int x_off = c*x_increment;
                int y_off = c*y_increment;

                // Stop check if out of bounds
                if (target[0]+x_off > 'h' || target[1]+y_off > '8' || target[0]+x_off < 'a' || target[1]+y_off < '1') {break;}
                // stop check if it comes in contact with a piece that isn't own rook
                if (board.at(offsetLocation(target, x_off, y_off)).piece != 0 && (board.at(offsetLocation(target, x_off, y_off)).piece != piece || board.at(offsetLocation(target, x_off, y_off)).white != white)) {break;}
                if (checkLocation(target, piece, x_off, y_off, white)) {
                    count++;
                }
            }
        }
    }
    return count;
}
// All rook threat detection logic
int rookScan(const string& target, const char piece, bool white) {

    int count = 0;

    int x_increment;
    int y_increment;

    for (int a=0; a<=1; ++a) {
        for (int b=0; b<=1; ++b) {
            if (a == 1) {
                x_increment = static_cast<int>(pow(-1,b));
                y_increment = 0;
            } else {
                x_increment = 0;
                y_increment = static_cast<int>(pow(-1,b));
            }
            for (int c=1; c<=8; ++c) {
                int x_off = c*x_increment;
                int y_off = c*y_increment;

                // Stop check if out of bounds
                if (target[0]+x_off > 'h' || target[1]+y_off > '8' || target[0]+x_off < 'a' || target[1]+y_off < '1') {break;}
                // stop check if it comes in contact with a piece that isn't own rook
                if (board.at(offsetLocation(target, x_off, y_off)).piece != 0 && (board.at(offsetLocation(target, x_off, y_off)).piece != piece || board.at(offsetLocation(target, x_off, y_off)).white != white)) {break;}
                if (checkLocation(target, piece,x_off, y_off, white)) {
                   count++;
                }
            }
        }
    }


    return count;
}
// All queen threat detection logic
int queenScan(const string& target, bool white) {
    return bishopScan(target,'Q', white) + rookScan(target,'Q', white);
}
// All king threat detection logic
int kingScan(const string& target, bool white) {
    int count = 0;

    for (int a=-1; a<=1; ++a) {
        for (int b=-1; b<=1; ++b) {
            const char a_char = static_cast<char>(target[0] + a);
            const char b_char = static_cast<char>(target[1] + b);
            string index = string(1,a_char) + string(1,b_char);
            if (checkLocation(index,'K', 0, 0, white) && index != target) {
                count++;
            }
        }
    }

    return count;
}
// A combination of all the threat detection logic
int threatScan(const string& target, bool white) {
    return pawnScan(target, white) + knightScan(target, white) + bishopScan(target, 'B', white) + rookScan(target, 'R', white) + queenScan(target, white) + kingScan(target, white);
}

map<string, Piece> sightAnalysis(map<string, Piece> simulationBoard) {
    for (int a=7; a>=0; --a) {
        char y_pos = static_cast<char>('1' + a);

        for (int b=0; b<8; ++b) {
            char x_pos = static_cast<char>('a' + b);
            string index = string(1, x_pos) + string(1, y_pos);

            simulationBoard[index].whiteSight = threatScan(index, true);
            simulationBoard[index].blackSight = threatScan(index, false);

        }

    }

    return simulationBoard;

}

map<string, Piece> threatenedAnalysis(map<string, Piece> simulationBoard) {
    bool undefended = false;

    for (int a=7; a>=0; --a) {
        char y_pos = static_cast<char>('1' + a);
        for (int b=0; b<8; ++b) {
            char x_pos = static_cast<char>('a' + b);
            string index = string(1, x_pos) + string(1, y_pos);
            undefended = false;
            if ((simulationBoard.at(index).whiteSight < simulationBoard.at(index).blackSight && simulationBoard.at(index).piece != 0 && simulationBoard.at(index).white) ||
                (simulationBoard.at(index).blackSight < simulationBoard.at(index).whiteSight && simulationBoard.at(index).piece != 0 && !simulationBoard.at(index).white)) {
                undefended = true;
            }
            simulationBoard[index].threatened = undefended;
        }
    }

    return simulationBoard;

}

void printSight(const map<string, Piece>& printBoard) {

    string colour;

    for (int a=7; a>=0; --a) {
        char y_pos = static_cast<char>('1' + a);
        cout<<" "<<y_pos<<" ";
        for (int b=0; b<8; ++b) {
            char x_pos = static_cast<char>('a' + b);
            string index = string(1, x_pos) + string(1, y_pos);

            int whiteThreat = printBoard.at(index).whiteSight;
            int blackThreat = printBoard.at(index).blackSight;
            int threat = whiteThreat - blackThreat;
            if (whiteThreat >= 1 && blackThreat >= 1) {
                colour = "\033[95m";
            } else if (whiteThreat >= 1) {
                colour = "\033[94m";
            }else if (blackThreat  >= 1){
                colour = "\033[91m";
            } else {
                colour = "\033[0m";
            }
            cout<<colour;
            if (threat >= 0) {cout<<" ";}
            cout<<threat<<"\033[0m ";
        }
        cout<<endl<<endl;
    }
    cout<<"    A  B  C  D  E  F  G  H"<<endl<<endl;
}

void printUndefended(const map<string, Piece>& printBoard) {

    string colour;

    for (int a=7; a>=0; --a) {
        char y_pos = static_cast<char>('1' + a);
        cout<<" "<<y_pos<<" ";
        for (int b=0; b<8; ++b) {
            char x_pos = static_cast<char>('a' + b);
            string index = string(1, x_pos) + string(1, y_pos);
            bool undefended = false;
            if (printBoard.at(index).threatened) {
                colour = "\033[91m";
                undefended = true;
            } else {
                colour = "\033[0m";
            }
            cout<<colour<<undefended<<"\033[0m ";
        }
        cout<<endl;
    }
    cout<<"   A B C D E F G H"<<endl<<endl;
}

void writeToBoard(map<string, Piece>& mainBoard, const map<string, Piece>& simulationBoard, const string& pointer) {
    for (int a=7; a>=0; --a) {
        char y_pos = static_cast<char>('1' + a);
        for (int b=0; b<8; ++b) {
            char x_pos = static_cast<char>('a' + b);
            string index = string(1, x_pos) + string(1, y_pos);
            if (pointer == "piece") {mainBoard.at(index).piece = simulationBoard.at(index).piece;}
            else if (pointer == "white") {mainBoard.at(index).white = simulationBoard.at(index).white;}
            else if (pointer == "whiteSight") {mainBoard.at(index).whiteSight = simulationBoard.at(index).whiteSight;}
            else if (pointer == "blackSight") {mainBoard.at(index).blackSight = simulationBoard.at(index).blackSight;}
            else if (pointer == "threatened") {mainBoard.at(index).threatened = simulationBoard.at(index).threatened;}
        }
    }
}

void runAfterMove() {
    printSight(sightAnalysis(board));
    writeToBoard(board, sightAnalysis(board), "whiteSight");
    writeToBoard(board, sightAnalysis(board), "blackSight");

    printUndefended(threatenedAnalysis(board));
    writeToBoard(board, threatenedAnalysis(board), "threatened");
    displayBoard(board);
    cout<<"Control Evaluation: "<<evaluateControl(board)<<endl;
}

// find the piece that is being moved and outputs its position
string findPiece(const string& target, char piece, bool capture, const string& disambiguation, bool white) {
    string position;

    // Protect against capturing nothing
    if (board.at(target).piece == 0 && capture && ((blackEnPassantAvailable.empty() && whiteEnPassantAvailable.empty()) || piece != 'P')) {failedMove("Must not capture an empty square");}

    // reset en passant if a non-pawn is moved
    if (piece != 'P') {
        blackEnPassantAvailable = "";
        whiteEnPassantAvailable = "";
    }

    switch (piece) {
        default: failedMove("Unknown piece");;
        case 'P':
            position = pawnLogic(target, piece, capture, disambiguation, white);
            break;
        case 'R':
            position = rookLogic(target, piece, capture, disambiguation, white);
            break;
        case 'N':
            position = knightLogic(target, piece, capture, disambiguation, white);
            break;
        case 'B':
            position = bishopLogic(target, piece, capture, disambiguation, white);
            break;
        case 'Q':
            position = queenLogic(target, piece, capture, disambiguation, white);
            break;
        case 'K':
            position = kingLogic(target, piece, capture, white);
            break;
    }

    if (position.empty()) {failedMove("No valid piece to move");}
    return position;
}

// Moves a piece and checks to ensure the move is legal
string moveCommand(const string& target, char piece, const bool capture, const string& disambiguation, const bool white, const char promotion, int castleAvailable) {

    string clear;
    char backRank = '8';

    if (!white) {
        backRank = '1';
    }

    if (piece != 'C') {
        clear = findPiece(target, piece, capture, disambiguation, white);

        if (promotion != 0 ) {
            if (piece == 'P' && target[1] == backRank) {
                piece = promotion;

            } else {
                failedMove("Invalid promotion exception");
            }
        }

        movePiece( target, piece, clear, white);
    } else {


        auto [castleKing, castleKingTarget, castleRook, castleRookTarget] = castleLogic(target, white, castleAvailable);
        movePiece(castleKingTarget , 'K', castleKing, white);
        movePiece(castleRookTarget , 'R', castleRook, white);
    }

    runAfterMove();
    return clear;
}


int main() {

    reset();
    emptyHandler(true);

    return graphicsTest(board);

    cout<<endl;

    int castleAvailable = 0b1111;
    bool WHITE = true;
    bool AUTOSWITCH = true;
    string null;
    string input;

    bool whiteShortCastle = true;
    bool whiteLongCastle = true;
    bool blackShortCastle = true;
    bool blackLongCastle = true;

    reset();
    runAfterMove();


    LOOP:{
        emptyHandler(false);

        cout<<endl<<"White = "<<WHITE<<endl<<"Input: ";
        cin>>input;

        if (input == "EXIT") {exit(0);}
        if (input == "RESET") {
            reset();
            WHITE = true;
            runAfterMove();

            whiteShortCastle = true;
            whiteLongCastle  = true;
            blackShortCastle = true;
            blackLongCastle  = true;
            castleAvailable = 0b1111;
            goto LOOP;}
        if (input == "CUSTOM") {
            customBoard(); WHITE = true;
            printSight(sightAnalysis(board));
            displayBoard(board);

            whiteShortCastle = true;
            whiteLongCastle  = true;
            blackShortCastle = true;
            blackLongCastle  = true;
            castleAvailable = 0b1111;
            goto LOOP;}
        if (input == "SWITCH") {WHITE = !WHITE; cout<<"White = "<<WHITE<<endl; goto LOOP;}
        if (input == "AUTOSWITCH") {AUTOSWITCH = !AUTOSWITCH; cout<<"Autoswitch = "<<AUTOSWITCH<<endl; goto LOOP;}
        if (input == "PRINT") {
            for (const string& move : moveHistory) {
                cout <<move<<endl;
            }
            goto LOOP;
        }
        if (input == "UNDO") {
            board = boardHistory.back();
            boardHistory.pop_back();
            moveHistory.pop_back();
            WHITE = !WHITE;
            printSight(sightAnalysis(board));
            displayBoard(board);
            goto LOOP;
        }
        moveHistory.push_back(input);
        boardHistory.push_back(board);
        try {
            auto [POSITION, PIECE, CAPTURE, CHECK, PROMOTION, DISAMBIGUATE] = ParseNotation(input);
            string pieceLocation = moveCommand(POSITION, PIECE, CAPTURE, DISAMBIGUATE, WHITE, PROMOTION, castleAvailable);

            switch (PIECE) {
                default: break;
                case 'C':
                case 'K':
                    if (WHITE) {
                        whiteShortCastle = false;
                        whiteLongCastle  = false;
                    } else {
                        blackShortCastle = false;
                        blackLongCastle  = false;
                    }
                    break;
                case 'R':
                    if (WHITE) {
                        if (pieceLocation == "h1") {
                            whiteShortCastle = false;
                        } else if (pieceLocation == "a1") {
                            whiteLongCastle  = false;
                        }
                    } else {
                        if (pieceLocation == "h8") {
                            blackShortCastle = false;
                        } else if (pieceLocation == "a8") {
                            blackLongCastle  = false;
                        }
                    }
            }
            castleAvailable = boolMerge(whiteShortCastle, whiteLongCastle, blackShortCastle, blackLongCastle);

            if (AUTOSWITCH) {WHITE = !WHITE;}
        } catch (const exception& e) {
            cout<<endl<<"Error: \033[91m"<<e.what()<<"\033[0m"<<endl;
        }


    goto LOOP;
    }
}

