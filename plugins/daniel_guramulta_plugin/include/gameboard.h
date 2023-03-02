#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <vector>
#include <bitset>
#include <array>

#include "quoridormove.h"
// define point structure for better handling of coordinates

struct node
{
    // coordonates on actual game board
    int x, y;

    // left, right, up, down -> 0 can not move, 1 can move
    // ie: 1101 can move left, right, CANNOT move up, can move down
    std::bitset<4> neighbours;

    // how to handle if this node is a player
    // bool enemyPlayer; // or better in gameboard

//    node(int x, int y, char *bits)
//        : x{x}, y{y}, neighbours{bits} {}

//    node() = default;
};//    gameboard board = generateGameBoard();

//    addWall(board, 3, 2, Orientation::Horizontal);
//    addWall(board, 3, 2, Orientation::Vertical);

//    printGameBoard(board);

//    bfs_print(board);

struct Path
{
    std::vector<int> path; // node value;

    int score() {return path.size() - 1;}
};

struct Wall // check qcore wallstate
{
    int x, y;
    Orientation orientation;

    // nodes that are affected by these wall
    // could help when trying to determine if a path intersects the given wall
    std::vector<int> nodes;

    int intersects(const Path &p)
    {
        for (int i = 0; i < nodes.size(); ++i) {
            for (int j = 0; j < p.path.size(); ++j) {
                if (nodes[i] == p.path[j])
                    return nodes[i];
            }
        }

        return -1;
    }
};


struct Gameboard
{
    std::array<node, 81> nodes;

    std::pair<int, int> enemyPosition;

    std::vector<Wall> walls;
};

#endif // GAMEBOARD_H
