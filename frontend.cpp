#include <iostream>
#include "frontend.h"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <thread>

using namespace std;
using namespace sf;

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
}

string getPosition(int x, int y) {
    return string(1, static_cast<char>(x + 'a')) + string(1, static_cast<char>(y + '1'));
}

auto getSpriteRect(const map<string, Piece>& referenceBoard, const string& position) {
    if (referenceBoard.at(position).white) {
        switch (referenceBoard.at(position).piece) {
            default: return IntRect({0, 0}, {0, 0});
            case 'P': return IntRect({4, 5}, {15, 18});
            case 'N': return IntRect({88, 2}, {18, 20});
            case 'B': return IntRect({31, 2}, {18, 20});
            case 'R': return IntRect({60, 2}, {18, 20});
            case 'Q': return IntRect({143, 1}, {18, 20});
            case 'K': return IntRect({115, 1}, {18, 20});
        }
    }
    switch (referenceBoard.at(position).piece) {
        default: return IntRect({0, 0}, {0, 0});
        case 'P': return IntRect({172, 5}, {15, 18});
        case 'N': return IntRect({256, 2}, {18, 20});
        case 'B': return IntRect({200, 2}, {18, 20});
        case 'R': return IntRect({227, 2}, {18, 20});
        case 'Q': return IntRect({311, 1}, {18, 20});
        case 'K': return IntRect({284, 1}, {18, 20});
    }
}

void displayBoard(const map<string, Piece>& board) {
printBoard(board);
}

int graphicsTest(const map<string, Piece>& printBoard) {
    RenderWindow window(VideoMode({1000, 1000}), "Chess Engine");

    Texture pieceSprites("assets/ChessPieces-Sheet.png");


    bool dragging = false;
    sf::Vector2f dragOffset;

    while (window.isOpen()) {

        window.clear();
        for (int a=1; a<=8; ++a) {
            for (int b=1; b<=8; ++b) {
                const int index = a + b % 2;
                RectangleShape tile({100, 100});
                tile.setPosition({static_cast<float>(b)*100.f, static_cast<float>(a)*100.f});
                if (index % 2 == 0) {
                    tile.setFillColor({234, 233, 210});
                } else {
                    tile.setFillColor({75, 115, 153});
                }

                window.draw(tile);
            }
        }
        for (int a=0; a<8; ++a) {
            for (int b=0; b<8; ++b) {

                Sprite piece(pieceSprites);
                piece.setScale({5, 5});
                piece.setTextureRect(getSpriteRect(printBoard, getPosition(b, a)));
                //piece.setScale({4.8, 4.8});
                piece.setPosition({(static_cast<float>(b + 1)*100.f)-25, (900.f - 100.f*static_cast<float>(a + 1))-25});
                bool dragging = false;

                window.draw(piece);
            }
        }


        CircleShape circle(100.f);

        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();

            if (const auto* mousePressed =
                    event->getIf<sf::Event::MouseButtonPressed>())
            {
                Vector2i mousePosition = Mouse::getPosition(window);
                Vector2f mouseCords = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                if (circle.getGlobalBounds().contains(mouseCords)) {
                    dragging = true;
                    dragOffset = mouseCords - circle.getPosition();
                }

            }

            if (const auto* mouseMoved =
                    event->getIf<sf::Event::MouseMoved>())
            {
                if (dragging)
                {
                    sf::Vector2f mousePos = window.mapPixelToCoords(mouseMoved->position);
                    circle.setPosition(mousePos - dragOffset);
                }
            }

            if (const auto* released =
            event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (released->button == sf::Mouse::Button::Left)
                {
                    dragging = false;
                }
            }

        }

        window.draw(circle);
        window.display();

    }
    return 0;
}
