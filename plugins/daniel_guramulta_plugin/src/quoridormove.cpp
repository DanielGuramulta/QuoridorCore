#include "quoridormove.h"

QuoridorMove::QuoridorMove(int x, int y, int dx, int dy, char p, char t, Direction d, Jump j, Orientation o)
    : x(x), y(y), dx(dx), dy(dy), p(p), t(t), d(d), j(j), o(o)
{

}

//TODO
bool QuoridorMove::operator ==(const MonteCarloMove &other) const
{
    const QuoridorMove m2 = static_cast<const QuoridorMove &>(other);

    return (x == m2.x && y == m2.y
            && dx == m2.dx && dy == m2.dy
            && p == m2.p && t == m2.t
            && t == m2.t && d == m2.d && j == m2.j && o == m2.o);
}
