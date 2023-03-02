#ifndef QUORIDORMOVE_H
#define QUORIDORMOVE_H

#include "montecarlonode.h"

enum class Direction : int // also present in qcore
{
    Left = 0,
    Right = 1,
    Up = 2,
    Down = 3,
    None
};

enum class Jump : int
{
    Left,
    Right,
    Up,
    Down,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    None
};

enum class Orientation // also present in qcore
{
    Horizontal,
    Vertical,
    None
};


class QuoridorMove : public MonteCarloMove
{
public:
    QuoridorMove(int x, int y, int dx, int dy, char p, char t, Direction d, Jump j, Orientation o);

    // MonteCarloMove interface
public:
    bool operator ==(const MonteCarloMove &other) const;

    int x, y;
    int dx, dy;
    char p; // player
    char t; // type m (d) j (j)
    Direction d;
    Jump j;
    Orientation o;
};

#endif // QUORIDORMOVE_H
