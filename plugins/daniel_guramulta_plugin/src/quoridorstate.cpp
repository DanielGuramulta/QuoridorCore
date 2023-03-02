#include "quoridorstate.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <cmath>

#define MAX(A, B) (((A) > (B)) ? A : B)

std::default_random_engine QuoridorState::generator = std::default_random_engine(time(NULL));


// c++23 cast to underlying type
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

QuoridorState::QuoridorState(char me)
    : wx(8)
    , wy(4)
    , bx(0)
    , by(4)
    , wnrwalls(10)
    , bnrwalls(10)
    , me(me)
    , turn('w')
    , steps(0)
{

    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            board.nodes[i * 9 + j].x = i;
            board.nodes[i * 9 + j].y = j;

            if (i == 0 && j == 0) // topLeft
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"0101"};
            }
            else if (i == 0 && j == 8) // topRight
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"1001"};
            }
            else if (i == 8 && j == 0) // bottomLeft
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"0110"};
            }
            else if (i == 8 && j == 8) // bottomRight
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"1010"};
            }
            else if (i == 0) // top area
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"1101"};
            }
            else if (j == 8) // right area
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"1011"};
            }
            else if (i == 8) // bottom area
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"1110"};
            }
            else if (j == 0) // left area
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"0111"};
            } else // inside
            {
                board.nodes[i * 9 + j].neighbours = std::bitset<4>{"1111"};
            }
        }
    }
}

QuoridorState::QuoridorState(const QuoridorState &other)
    : wx(other.wx)
    , wy(other.wy)
    , bx(other.bx)
    , by(other.by)
    , wnrwalls(other.wnrwalls)
    , bnrwalls(other.bnrwalls)
    , me(other.me)
    , turn(other.turn)
    , steps(other.steps)
{
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            board.nodes[i * 9 + j].x = i;
            board.nodes[i * 9 + j].y = j;

            board.nodes[i * 9 + j].neighbours = other.board.nodes[i * 9 + j].neighbours;
        }
    }

    for (auto &wall : other.board.walls) {
        board.walls.push_back(wall);
    }
}

std::queue<MonteCarloMove *> QuoridorState::actions_to_try() const
{
    return const_cast<QuoridorState *>(this)->generateMoves();
}

MonteCarloState *QuoridorState::nextState(const MonteCarloMove *move) const
{
    QuoridorState *state = new QuoridorState(*this);
    state->playMove(static_cast<const QuoridorMove*>(move));

//    std::cout << "State fastest path for w = " << state->fastestPath('w') << '\n';
//    std::cout << "State fastest path for b = " << state->fastestPath('b') << '\n';

//    std::cout << "Walls on map: " << board.walls.size() << std::endl;

//    if (20 - wnrwalls - bnrwalls != board.walls.size()) {
//        std::cout << "problem\n";
//    }

    return state;
}

double QuoridorState::rollout() const
{
    double evalLimit =  0.8;

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    QuoridorState s(*this);


    for (int i = 0; i < 50; ++i)
    {
        if (s.isTerminal()) {
            if (s.checkWinner() == 'w') {
                return me == 'w' ? 1.0 : 0.0;
            } else {
                return me == 'b' ? 1.0 : 0.0;
            }
        }
        // second check if we can call who is going to win
        double eval = evaluate(s);
        if (eval <= 1.0 - evalLimit && eval >= evalLimit) {
            break;
        }


        QuoridorMove *m = pickMove(s, dist, generator);

//        std::cout << "play move\n";
        bool noError = s.playMove(m);

        if (!noError) {
//            s.printBoard();
//            std::cout << "Motorin problem habibi!\n";
            break;
        }

        delete m;

    }

    return evaluate(s);
}

bool QuoridorState::isTerminal() const
{
    char winner = checkWinner();
    return winner == 'w' || winner == 'b';
}

bool QuoridorState::who() const
{
    return turn == me;;
}



bool QuoridorState::playMove(const QuoridorMove *move)
{
    if (move == nullptr) return false;
    if (move->t == 'm' && canMove(board.nodes[move->x * 9 + move->y], move->d)) {
        switch (move->p) {
            case 'w':
                if (wx == move->x && wy == move->y) {
                    wx = move->dx;
                    wy = move->dy;
                // reset his dists
                } else return false;
                break;
            case 'b':
                if (bx == move->x && by == move->y) {
                    bx = move->dx;
                    by = move->dy;
                }  else return false;

                // reset his dists
                break;
        }
    }
    else if (move->t == 'j' && canJump(move->x * 9 + move->y, move->j)) {
        switch (move->p) {
            case 'w':
                if (wx == move->x && wy == move->y) {
                    wx = move->dx;
                    wy = move->dy;
                // reset his dists
                } else return false;
                // reset his dists
                break;
            case 'b':
                if (bx == move->x && by == move->y) {
                    bx = move->dx;
                    by = move->dy;
                }  else return false;
                // reset his dists
                break;
        }
    }
    else if (move->t == 'w' && isWallValid(move->x, move->y, move->o)) {
        addWall(move->x, move->y, move->o);
//        std::cout << "Played wall x: " << move->x << " y: " << move->y << " o: " << (move->o == Orientation::Horizontal ? 'h' : 'v') << '\n';
        switch (move->p) {
            case 'w':
                wnrwalls--;
                // reset his dists
                break;
            case 'b':
                bnrwalls--;
                // reset his dists
                break;
        }
    }
    else {
        return false;
    }

    // TODO: change turn;
    changeTurn();
    steps++;

    return true;
}

bool QuoridorState::canMove(const node &n, Direction direction) const
{
    return n.neighbours.test(3 - to_underlying(direction));
}

void QuoridorState::setCantMove(node &n, Direction direction)
{
    n.neighbours.set(3 - to_underlying(direction), false);
}

void QuoridorState::setCanMove(node &n, Direction direction)
{
    n.neighbours.set(3 - to_underlying(direction), true);
}

Wall QuoridorState::addWall(int x, int y, Orientation orientation)
{
    // n1  n2
    // -------
    // n3  n4



    Wall w;
    w.x = x;
    w.y = y;
    w.orientation = orientation;

    if (orientation == Orientation::Horizontal)
    {
        setCantMove(board.nodes[x * 9 + y], Direction::Down);
        setCantMove(board.nodes[x * 9 + y + 1], Direction::Down);



        x+=1; // go down one row

        setCantMove(board.nodes[x * 9 + y], Direction::Up);
        setCantMove(board.nodes[x * 9 + y + 1], Direction::Up);


    }
    // n1 | n2
    //    |
    // n3 | n4
    else if (orientation == Orientation::Vertical) // no need for else if just else but still make it clear
    {

        setCantMove(board.nodes[x * 9 + y], Direction::Right);
        setCantMove(board.nodes[(x + 1) * 9 + y], Direction::Right);


        y+=1;

        setCantMove(board.nodes[x * 9 + y], Direction::Left);
        setCantMove(board.nodes[(x + 1) * 9 + y], Direction::Left);


    }

    board.walls.push_back(w);


    return w;
}

Wall QuoridorState::removeWall(int x, int y, Orientation orientation)
{
    // n1  n2
    // -------
    // n3  n4



    Wall w;
    w.x = x;
    w.y = y;
    w.orientation = orientation;

    if (orientation == Orientation::Horizontal)
    {
        setCanMove(board.nodes[x * 9 + y], Direction::Down);
        setCanMove(board.nodes[x * 9 + y + 1], Direction::Down);



        x+=1; // go down one row

        setCanMove(board.nodes[x * 9 + y], Direction::Up);
        setCanMove(board.nodes[x * 9 + y + 1], Direction::Up);


    }
    // n1 | n2
    //    |
    // n3 | n4
    else if (orientation == Orientation::Vertical) // no need for else if just else but still make it clear
    {

        setCanMove(board.nodes[x * 9 + y], Direction::Right);
        setCanMove(board.nodes[(x + 1) * 9 + y], Direction::Right);


        y+=1;

        setCanMove(board.nodes[x * 9 + y], Direction::Left);
        setCanMove(board.nodes[(x + 1) * 9 + y], Direction::Left);


    }



    board.walls.erase(std::remove_if(board.walls.begin(),
                                     board.walls.end(),
                                     [x, y, orientation](const Wall &w){
        return w.x == x && w.y == y && w.orientation == orientation;
    }));


    return w;
}

bool QuoridorState::canJump(int current, Jump jump) const
{
    int cx = current / 9;
    int cy = current % 9;

    int ex = turn == 'w' ? bx : wx;
    int ey = turn == 'w' ? by : wy;

    if (jump == Jump::Up) {
        if (cx == 0) return false;
        if (cx - 1 == ex && cy == ey && canMove(board.nodes[ex * 9 + ey], Direction::Up) && canMove(board.nodes[current], Direction::Up)) {
            return true;
        }
    } else if (jump == Jump::Down) {
        if (cx == 8) return false;
        if (cx + 1 == ex && cy == ey && canMove(board.nodes[ex * 9 + ey], Direction::Down) && canMove(board.nodes[current], Direction::Down)) {
            return true;
        }
    } else if (jump == Jump::Left) {
        if (cy == 0) return false;
        if (cy - 1 == ey && cx == ex && canMove(board.nodes[ex * 9 + ey], Direction::Left) && canMove(board.nodes[current], Direction::Left)) {
            return true;
        }
    } else if (jump == Jump::Right) {
        if (cy == 8) return false;
        if (cy + 1 == ey && cx == ex && canMove(board.nodes[ex * 9 + ey], Direction::Right) && canMove(board.nodes[current], Direction::Right)) {
            return true;
        }
    } else if (jump == Jump::TopLeft) {
        if (cx == 0 || cy == 0) return false;
        if (cx - 1 == ex && cy == ey && !canMove(board.nodes[ex * 9 + ey], Direction::Up) && canMove(board.nodes[ex * 9 + ey], Direction::Left) && canMove(board.nodes[current], Direction::Up)) {
            return true;
        }
        if (cy - 1 == ey && cx == ex && !canMove(board.nodes[ex * 9 + ey], Direction::Left) && canMove(board.nodes[ex * 9 + ey], Direction::Up) && canMove(board.nodes[current], Direction::Left)) {
            return true;
        }
    } else if (jump == Jump::TopRight) {
        if (cx == 0 || cy == 8) return false;
        if (cx - 1 == ex && cy == ey && !canMove(board.nodes[ex * 9 + ey], Direction::Up) && canMove(board.nodes[ex * 9 + ey], Direction::Right) && canMove(board.nodes[current], Direction::Up)) {
            return true;
        }
        if (cy + 1 == ey && cx == ex && !canMove(board.nodes[ex * 9 + ey], Direction::Right) && canMove(board.nodes[ex * 9 + ey], Direction::Up) && canMove(board.nodes[current], Direction::Right)) {
            return true;
        }
    } else if (jump == Jump::BottomLeft) {
        if (cx == 8 || cy == 0) return false;
        if (cx + 1 == ex && cy == ey && !canMove(board.nodes[ex * 9 + ey], Direction::Down) && canMove(board.nodes[ex * 9 + ey], Direction::Left) && canMove(board.nodes[current], Direction::Down)) {
            return true;
        }
        if (cy - 1 == ey && cx == ex && !canMove(board.nodes[ex * 9 + ey], Direction::Left) && canMove(board.nodes[ex * 9 + ey], Direction::Down) && canMove(board.nodes[current], Direction::Left)) {
            return true;
        }
    } else if (jump == Jump::BottomRight) {
        if (cx == 8 || cy == 8) return false;
        if (cx + 1 == ex && cy == ey && !canMove(board.nodes[ex * 9 + ey], Direction::Down) && canMove(board.nodes[ex * 9 + ey], Direction::Right) && canMove(board.nodes[current], Direction::Down)) {
            return true;
        }
        if (cy + 1 == ey && cx == ex && !canMove(board.nodes[ex * 9 + ey], Direction::Right) && canMove(board.nodes[ex * 9 + ey], Direction::Down) && canMove(board.nodes[current], Direction::Right)) {
            return true;
        }
    }

    return false;
}


bool QuoridorState::isEnemy(int current,  Direction d) const
{
    char p = turn;
    int ex = (p == 'w' ? bx : wx);
    int ey = (p == 'w' ? by : wy);

    int cx = current / 9;
    int cy = current % 9;

    if (d == Direction::Up) {
        return (cx - 1 == ex && cy == ey);
    } else if (d == Direction::Down) {
        return (cx + 1 == ex && cy == ey);
    } else if (d == Direction::Left) {
        return (cx == ex && cy - 1 == ey);
    } else if (d == Direction::Right) {
        return (cx == ex && cy + 1 == ey);
    }
    return false;
}



bool QuoridorState::isWallValid(int x, int y, Orientation orientation)
{
    //check limits

    if (x > 7 || x < 0) return false;
    if (y > 7 || y < 0) return false;

    auto it = std::find_if(board.walls.begin(), board.walls.end(), [x, y, orientation](Wall w){
        if (x == w.x && y == w.y && w.orientation == orientation) return true;
        else return false;
    });

    if (it != board.walls.end()) {
//        std::cout << "found existing wall x:" << x << " y:" << y << " orientation: " << (orientation == Orientation::Horizontal ? 'h' : 'v') << std::endl;
        return false;
    }

    node left = board.nodes[x * 9 + y];
    node right = board.nodes[x * 9 + (y + 1)];

    node bleft = board.nodes[(x + 1) * 9 + y];
    node bright = board.nodes[(x + 1) * 9 + y + 1];

    bool validOnMap = false;

    if (orientation == Orientation::Horizontal)
    {
        if (canMove(left, Direction::Down) && canMove(right, Direction::Down)) {
            if ((canMove(left, Direction::Right) || (left.x == 0)) || (canMove(bleft, Direction::Right) || bleft.x == 8)) {
                validOnMap = true;
            }
        }
    }
    else if (orientation == Orientation::Vertical)
    {
        if ((canMove(left, Direction::Right) || (left.x == 0)) && (canMove(bleft, Direction::Right)  || bleft.x == 8)) {
            if (canMove(left, Direction::Down) || canMove(right, Direction::Down)) {
                validOnMap = true;
            }
        }
    }

    if (validOnMap) {
        addWall(x, y, orientation);
        constexpr bool earlyExit = true;

        bool valid = ((fastestPath('w', earlyExit) != 20000) && (fastestPath('b', earlyExit) != 20000));

        removeWall(x, y, orientation);

        return valid;
    }

    return false;
}

char QuoridorState::changeTurn()
{
    turn = (turn == 'w') ? 'b' : 'w';
    return turn;
}

char QuoridorState::checkWinner() const
{
    if (wx == 0) return 'w';
    if (bx == 8) return 'b';
    return ' ';
}

void QuoridorState::printBoard()
{
    bool format = true;
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (format) {
                if (wx == i && wy == j)
                    std::cout << "  W  ";
                else if (bx == i && by == j)
                    std::cout << "  B  ";
                else
                    std::cout << board.nodes[i * 9 + j].neighbours << ' ';
            } else {
                std::cout << i * 9 + j << ' ';
                if (i == 0 || (i == 1 && j == 0)) std::cout << " ";
            }
            if (j < 8 && !canMove(board.nodes[i * 9 + j], Direction::Right)) {
                std::cout << "| ";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << '\n';
        for (int j = 0; j < 9; ++j) {
            if (j < 8 && !canMove(board.nodes[i * 9 + j], Direction::Right)) {
                if (format)
                    std::cout << "     | ";
                else
                    std::cout << "   | ";
            } else if (j < 8 && !canMove(board.nodes[i * 9 + j], Direction::Down)) {
                if (format)
                    std::cout << "_______";
                else
                    std::cout << "___";
            } else {
                if (format)
                    std::cout << "       ";
                else
                    std::cout << "     ";
            }
        }
        std::cout << '\n';

    }
    std::cout << "Walls on map: " << board.walls.size() << std::endl;
    std::cout << "white remaining: " << wnrwalls << " black remaining: " << bnrwalls << std::endl;
}

int QuoridorState::countWinNodes(int targetX) const
{
    int count = 0;
    for (int j = 0; j < 9; ++j) {
        if (board.nodes[targetX * 9 + j].neighbours != std::bitset<4>{"0000"}) count++;
    }

    return count;
}

int QuoridorState::fastestPath(char who, bool earlyExit) const // todo: add starting point in bfs
{
    int source_node = (who == 'w') ? (wx * 9 + wy) : (bx * 9 + by);
    return fastestPath(source_node, who, earlyExit);
}

int QuoridorState::fastestPath(int source_node, char who, bool earlyExit) const // todo: add starting point in bfs
{

    std::array<bool, 81> visited;
    std::fill(visited.begin(), visited.end(), false);
//    std::priority_queue<node, std::vector<node>, decltype(heuristic)> q(heuristic);
    std::queue<node> q;
    int total_visited = 0;
    std::array<int, 81> dist;
    std::fill(dist.begin(), dist.end(), -1);

    std::array<int, 81> pred;
    std::fill(pred.begin(), pred.end(), -1);

    q.push(board.nodes[source_node]); // starting point //
    // node position is x * 9 + y;
    visited[source_node] = true;
    dist[source_node] = 0;

    node current;
    int targetX = (who == 'w' ? 0 : 8);

    int countTargets = 0;

    while(!q.empty())
    {
        current = q.front();
        int currentDist = dist[current.x * 9 + current.y];
        q.pop();
        total_visited++;
//        if (total_visited > 34) break; // limit after x iterations BAD
//        std::cout << "visiting node [" << current.x << "," << current.y << "]\n";

        if (current.x == targetX){
            countTargets++;
            if (countTargets == countWinNodes(targetX) || earlyExit) break; // TODO store coutnWinNodes in state only compute once
        }

        // early exit we found shortest PATH AVG 3-4 times less visited nodes

        // get neighbours
        enqueue_neighbours(q, current, board, visited, dist, pred, currentDist);

//        std::cout << "queue size: " << q.size() << '\n';
    }


//    std::cout  << "Visited total: " << total_visited << '\n';
    int minNode = -1;
    int minDist = 20000;
    for (int i = 0; i < 9; ++i)
    {
//        std::cout << dist[targetX * 9 + i] << " ";
        if (dist[targetX * 9 + i] < minDist && dist[targetX * 9 + i] != -1) {
            minDist = dist[targetX * 9 + i];
            minNode = targetX * 9 + i;
        }
    }
//    std::cout << '\n';

//    for (int i = 0; i < 9; ++i)
//    {
//        for (int j = 0; j < 9; ++j) {
//            std::cout << dist[i * 9 + j] << " ";
//        }

//        std::cout << '\n';
//    }

//    std::cout << "Computting path to fastest\n";

//    std::vector<int> path;
//    int crawl = minNode;
//    path.push_back(crawl);
//    while (pred[crawl] != -1) {
//        crawl = pred[crawl];
//        path.push_back(crawl);
//    }

//    for (auto it = path.rbegin(); it != path.rend(); ++it) {
////        std::cout << *it << " ";
//    }

//    std::cout << '\n';

    return minDist;

}

template <class Q>
void QuoridorState::enqueue_neighbours(Q &q, const node &current, const Gameboard &board, std::array<bool, 81> &visited, std::array<int, 81> &dist, std::array<int, 81> &pred, int currentDist) const
{
//    std::cout << "enqueue neighbours\n";
    int currNodeValue = current.x * 9 + current.y;

    if (!isEnemy(currNodeValue, Direction::Left) && canMove(current, Direction::Left))
    {
        int p = current.x * 9 + (current.y - 1);
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "left\n";
        }
    }
    if (!isEnemy(currNodeValue, Direction::Right) && canMove(current, Direction::Right))
    {
        int p = current.x * 9 + (current.y + 1);
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "right\n";
        }

    }
    if (!isEnemy(currNodeValue, Direction::Down) && canMove(current, Direction::Down))
    {
        int p = (current.x + 1) * 9 + current.y;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "down\n";

        }
    }
    if (!isEnemy(currNodeValue, Direction::Up) && canMove(current, Direction::Up))
    {
        int p = (current.x - 1) * 9 + current.y;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "up\n";
        }
    }

    if (canJump(currNodeValue, Jump::Up)) {
        int p = (current.x - 2) * 9 + current.y;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "left\n";
        }
    }

    if (canJump(currNodeValue, Jump::Down)) {
        int p = (current.x + 2) * 9 + current.y;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "left\n";
        }
    }

    if (canJump(currNodeValue, Jump::Left)) {
        int p = current.x * 9 + current.y - 2;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "left\n";
        }
    }

    if (canJump(currNodeValue, Jump::Right)) {
        int p = current.x * 9 + current.y + 2;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "left\n";
        }
    }

    if (canJump(currNodeValue, Jump::TopLeft)) {
        int p = (current.x - 1) * 9 + current.y - 1;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "left\n";
        }
    }

    if (canJump(currNodeValue, Jump::TopRight)) {
        int p = (current.x - 1) * 9 + current.y + 1;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "left\n";
        }
    }

    if (canJump(currNodeValue, Jump::BottomLeft)) {
        int p = (current.x + 1) * 9 + current.y - 1;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "bottom left\n";
        }
    }

    if (canJump(currNodeValue, Jump::BottomRight)) {
        int p = (current.x + 1) * 9 + current.y + 1;
        if (!visited[p]) {
            q.push(board.nodes[p]);
            visited[p] = true;
            dist[p] = currentDist + 1;
            pred[p] = currNodeValue;
//            std::cout << "bottom rigth\n";
        }
    }
}

double QuoridorState::evaluate(QuoridorState &s) const
{
    double win = 0.95;

    int wp = s.fastestPath('w');
    int bp = s.fastestPath('b');



    if (s.bnrwalls <= 0 && ((wp + static_cast<int>(s.turn != 'w')) <= bp)) {
        return win;
    }

    if (s.wnrwalls <= 0 && ((bp + static_cast<int>(s.turn != 'b')) <= wp)) {
        return 1.0 - win;
    }

    if (s.bnrwalls <= 1 && s.wnrwalls >= 2 && ((wp + static_cast<int>(s.turn != 'w')) <= bp)) {
        return win - 0.1;
    }

    if (s.wnrwalls <= 1 && s.bnrwalls >= 2 && ((bp + static_cast<int>(s.turn != 'b')) <= wp)) {
        return 1.0 - (win - 0.1);
    }

    double walldif_metric = 0.0;
    double max = s.wnrwalls > s.bnrwalls ? s.wnrwalls : s.bnrwalls;
    if (max > 0) {
        walldif_metric = (s.wnrwalls > s.bnrwalls ? +1: -1) * (static_cast<double>(pow(s.wnrwalls - s.bnrwalls, 2)) / static_cast<double>(pow(max, 2)));
    }

    double pathdif = bp - wp + (s.turn == 'w' ? +0.5:-0.5);
    double pathdif_metric = static_cast<double>(MAX(pathdif, 10.0)) / 10.0;

    double total = 0.5 + 0.2 * walldif_metric + 0.2 * pathdif_metric;

    if (s.me == 'b') return (1.0 - total);

    return total;

}

std::queue<MonteCarloMove *> QuoridorState::generateMoves()
{
    char p = turn;
    int currentX = (p == 'w' ? wx : bx);
    int currentY = (p == 'w' ? wy : by);

    char enemy = (p == 'w' ? 'b' : 'w');

    int currentNode = currentX * 9 + currentY;

    std::queue<MonteCarloMove *> q;

    if (!isEnemy(currentNode, Direction::Up) && canMove(board.nodes[currentNode], Direction::Up)) {
        q.push(new QuoridorMove(currentX, currentY, currentX - 1, currentY, p, 'm', Direction::Up, Jump::None, Orientation::None));
    }
    if (!isEnemy(currentNode, Direction::Down) && canMove(board.nodes[currentNode], Direction::Down)) {
        q.push(new QuoridorMove(currentX, currentY, currentX + 1, currentY, p, 'm', Direction::Down, Jump::None, Orientation::None));

    }
    if (!isEnemy(currentNode, Direction::Left) && canMove(board.nodes[currentNode], Direction::Left)) {
        q.push(new QuoridorMove(currentX, currentY, currentX, currentY - 1, p, 'm', Direction::Left, Jump::None, Orientation::None));

    }
    if (!isEnemy(currentNode, Direction::Right) && canMove(board.nodes[currentNode], Direction::Right)) {
        q.push(new QuoridorMove(currentX, currentY, currentX, currentY + 1, p, 'm', Direction::Right, Jump::None, Orientation::None));
    }

    if (canJump(currentNode, Jump::Up)) {
        q.push(new QuoridorMove(currentX, currentY, currentX - 2, currentY, p, 'j', Direction::None, Jump::Up, Orientation::None));
    }
    if (canJump(currentNode, Jump::Down)) {
        q.push(new QuoridorMove(currentX, currentY, currentX + 2, currentY, p, 'j', Direction::None, Jump::Down, Orientation::None));

    }
    if (canJump(currentNode, Jump::Left)) {
        q.push(new QuoridorMove(currentX, currentY, currentX, currentY - 2, p, 'j', Direction::None, Jump::Left, Orientation::None));

    }
    if (canJump(currentNode, Jump::Right)) {
        q.push(new QuoridorMove(currentX, currentY, currentX, currentY + 2, p, 'j', Direction::None, Jump::Right, Orientation::None));

    }
    if (canJump(currentNode, Jump::TopLeft)) {
        q.push(new QuoridorMove(currentX, currentY, currentX - 1, currentY - 1, p, 'j', Direction::None, Jump::TopLeft, Orientation::None));

    }
    if (canJump(currentNode, Jump::TopRight)) {
        q.push(new QuoridorMove(currentX, currentY, currentX - 1, currentY + 1, p, 'j', Direction::None, Jump::TopRight, Orientation::None));

    }
    if (canJump(currentNode, Jump::BottomLeft)) {
        q.push(new QuoridorMove(currentX, currentY, currentX + 1, currentY - 1, p, 'j', Direction::None, Jump::BottomLeft, Orientation::None));

    }
    if (canJump(currentNode, Jump::BottomRight)) {
        q.push(new QuoridorMove(currentX, currentY, currentX + 1, currentY + 1, p, 'j', Direction::None, Jump::BottomRight, Orientation::None));
    }

//    if (isWallValid(i, j, Orientation::Horizontal)) {
//        q.push(new QuoridorMove(i, j, i, j, p, 'w', Direction::None, Jump::None, Orientation::Horizontal));
//    }
//    if (isWallValid(i, j, Orientation::Vertical)) {
//        q.push(new QuoridorMove(i, j, i, j, p, 'w', Direction::None, Jump::None, Orientation::Vertical));
//    }

    int ourPath = fastestPath(p);
    int enemyPath = fastestPath(enemy);

    int MIN_ENC_FOR_STOPPING_ENEMY_WALLS = 3;

    if (remainingWalls(p) > 0) {
        bool alreadyUsed[8][8][2]{false};
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                for (int k = 0; k < 2; ++k) {
                    Orientation o = (k == 0) ? Orientation::Horizontal : Orientation::Vertical;

                    if (isWallValid(i, j, o)) {
                        QuoridorMove *move = new QuoridorMove(i, j, i, j, p, 'w', Direction::None, Jump::None, o);

                        addWall(i, j, o);

                        int enemyWithWall = fastestPath(enemy);
                        int enemyEnc = enemyWithWall - enemyPath;
                        int usWithWall = fastestPath(p);
                        int ourEnc = usWithWall - ourPath;

                        if (!alreadyUsed[i][j][k] && enemyEnc > ourEnc && enemyWithWall >= 0 && usWithWall >= 0) {
                            alreadyUsed[i][j][k] = true;
                            q.push(move);
                        } else {
                            delete move;
                        } // if (!alreadyUsed[i][j][k] && enemyEnc > ourEnc && enemyWithWall >= 0 && usWithWall >= 0)

                        removeWall(i, j, o);


                        // counter wall
                        if (remainingWalls(enemy) > 0 && ourEnc - enemyEnc >= MIN_ENC_FOR_STOPPING_ENEMY_WALLS) {
                            QuoridorMove *counter = new QuoridorMove(i, j, i, j, p, 'w', Direction::None, Jump::None, (o == Orientation::Horizontal ? Orientation::Vertical : Orientation::Horizontal));

                            if (!alreadyUsed[i][j][1 - k] && isWallValid(i, j, (o == Orientation::Horizontal ? Orientation::Vertical : Orientation::Horizontal) )){
                                alreadyUsed[i][j][1 - k] = true;
                                q.push(counter);
                            } else {
                                delete counter;
                            }

                            if (k == 0) {

                                if (j - 1 >= 0 && !alreadyUsed[i][j - 1][k] && isWallValid(i, j - 1, o)) {
                                    QuoridorMove *counter = new QuoridorMove(i, j - 1, i, j - 1, p, 'w', Direction::None, Jump::None, o );
                                    alreadyUsed[i][j - 1][k] = true;
                                    q.push(counter);
                                }

                                if (j + 1 < 8 && !alreadyUsed[i][j + 1][k] && isWallValid(i, j + 1, o)) {
                                    QuoridorMove *counter = new QuoridorMove(i, j + 1, i, j + 1, p, 'w', Direction::None, Jump::None, o );
                                    alreadyUsed[i][j + 1][k] = true;
                                    q.push(counter);
                                }

                            } else if (k == 1) {
                                if (i - 1 >= 0 && !alreadyUsed[i - 1][j][k] && isWallValid(i - 1, j, o)) {
                                    QuoridorMove *counter = new QuoridorMove(i - 1, j, i - 1, j, p, 'w', Direction::None, Jump::None, o );
                                    alreadyUsed[i - 1][j][k] = true;
                                    q.push(counter);
                                }

                                if (i + 1 < 8 && !alreadyUsed[i + 1][j][k] && isWallValid(i + 1, j, o)) {
                                    QuoridorMove *counter = new QuoridorMove(i + 1, j, i + 1, j, p, 'w', Direction::None, Jump::None, o );
                                    alreadyUsed[i + 1][j][k] = true;
                                    q.push(counter);
                                }
                            }
                        } // if (remainingWalls(enemy) > 0 && ourEnc - enemyEnc >= MIN_ENC_FOR_STOPPING_ENEMY_WALLS)
                    } // sWallValid(i, j, o)
                } // for (int k = 0; k < 2; ++k)
            } // for (int j = 0; j < 8; ++j)
        } // for (int i = 0; i < 8; ++i)
    }


    return q;
}

QuoridorMove * QuoridorState::pickMove(QuoridorState &s, std::uniform_real_distribution<double> &dist, std::default_random_engine &gen) const
{
    double WALL_VS_MOVE_CHANCE = 0.4;
    double BEST_VS_RANDOM_MOVE = 0.8;
    double BEST_WALLMOVE = 0.1;
    double GUIDED_RANDOM_WALL = 0.75;

    char p = s.turn;
    char enemy = (p == 'w' ? 'b' : 'w');

    double turns = s.steps;
    double wall_vs_move_prob = (turns <= 2) ? 0.0
                                            : (turns <= 6) ? (WALL_VS_MOVE_CHANCE / 2) : WALL_VS_MOVE_CHANCE;

    if (s.remainingWalls(p) > 0 && (forceWall(s) || dist(gen) < wall_vs_move_prob)) {

        std::vector<QuoridorMove *> pool;
        pool.reserve(128);
        for (int i = 0; i < 8; ++i) {
           for (int j = 0; j < 8; ++j) {
               if (s.isWallValid(i, j, Orientation::Horizontal)) {
                   pool.push_back(new QuoridorMove(i, j, i, j, p, 'w', Direction::None, Jump::None, Orientation::Horizontal));
               }
               if (s.isWallValid(i, j, Orientation::Vertical)) {
                   pool.push_back(new QuoridorMove(i, j, i, j, p, 'w', Direction::None, Jump::None, Orientation::Vertical));
               }
           }
        }

        std::shuffle(pool.begin(), pool.end(), gen);

        if (dist(gen) < BEST_WALLMOVE) {
           QuoridorMove *bestWallMove = nullptr;
           double maxDiff = 0.0;
           for (QuoridorMove *move : pool) {
               int enemyPath = s.fastestPath(enemy);
               int ourPath = s.fastestPath(p);

               s.addWall(move->x, move->y, move->o);

               int enemyEnc = s.fastestPath(enemy) - enemyPath;
               int ourEnc = s.fastestPath(p) - ourPath ;

               s.removeWall(move->x, move->y, move->o);

               if (enemyPath >= 0 && enemyEnc > 0) {
                   if (ourPath >= 0 && enemyEnc > ourEnc) {
                       if (enemyEnc - ourEnc > maxDiff) {
                           if (bestWallMove) delete bestWallMove;
                           bestWallMove = move;
                           maxDiff = enemyEnc - ourEnc;
                       } else delete move;
                   } else delete move;
               } else delete move;


           } // for (QuoridorMove *move : pool)

           if (bestWallMove) return bestWallMove;
        } // if (dist(gen) < BEST_WALLMOVE)
        else {
            if (dist(gen) < GUIDED_RANDOM_WALL) {
                QuoridorMove *bestWallMove = nullptr;
                bool accepted = false;
                for (QuoridorMove *move : pool) {
                    if (!accepted) {
                        int enemyPath = s.fastestPath(enemy);
                        int ourPath = s.fastestPath(p);

                        s.addWall(move->x, move->y, move->o);

                        int enemyEnc = s.fastestPath(enemy) - enemyPath;
                        int ourEnc = s.fastestPath(p) - ourPath ;

                        s.removeWall(move->x, move->y, move->o);

                        if (enemyPath >= 0 && enemyEnc > 0) {
                            if (ourPath >= 0 && enemyEnc > ourEnc) {
                                bestWallMove = move;
                                accepted = true;
                            } else delete move;
                        } else delete move;

                        continue; // avoid delete
                    }
                    delete move;
                } // for (QuoridorMove *move : pool)

                if (bestWallMove) return bestWallMove;
            }
            else {
                QuoridorMove *bestWallMove = nullptr;
                bool accepted = false;
                for (QuoridorMove *move : pool) {
                    if (!accepted) {
                        bestWallMove = move;
                        accepted = true;
                        continue;
                    }
                    delete move;
                }

                if (bestWallMove) return bestWallMove;

            }

       } // else if (dist(gen) < BEST_WALLMOVE)


    } // if (s.remainingWalls(p) > 0 && (forceWall(s) || dist(gen) < wall_vs_move_prob))



    // play a move
    int currentX = (p == 'w' ? s.wx : s.bx);
    int currentY = (p == 'w' ? s.wy : s.by);

    int currentNode = currentX * 9 + currentY;

    std::vector<MonteCarloMove *> q;

    if (!s.isEnemy(currentNode, Direction::Up) && s.canMove(s.board.nodes[currentNode], Direction::Up)) {
//        std::cout << currentNode << '\n';
//        std::cout << "can move up " << board.nodes[currentNode].neighbours << '\n';
        q.push_back(new QuoridorMove(currentX, currentY, currentX - 1, currentY, p, 'm', Direction::Up, Jump::None, Orientation::None));
    }
    if (!s.isEnemy(currentNode, Direction::Down) && s.canMove(s.board.nodes[currentNode], Direction::Down)) {
//        std::cout << currentNode << '\n';
//        std::cout << "can move up " << board.nodes[currentNode].neighbours << '\n';
        q.push_back(new QuoridorMove(currentX, currentY, currentX + 1, currentY, p, 'm', Direction::Down, Jump::None, Orientation::None));

    }
    if (!s.isEnemy(currentNode, Direction::Left) && s.canMove(s.board.nodes[currentNode], Direction::Left)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX, currentY - 1, p, 'm', Direction::Left, Jump::None, Orientation::None));

    }
    if (!s.isEnemy(currentNode, Direction::Right) && s.canMove(s.board.nodes[currentNode], Direction::Right)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX, currentY + 1, p, 'm', Direction::Right, Jump::None, Orientation::None));
    }

    if (s.canJump(currentNode, Jump::Up)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX - 2, currentY, p, 'j', Direction::None, Jump::Up, Orientation::None));
    }
    if (s.canJump(currentNode, Jump::Down)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX + 2, currentY, p, 'j', Direction::None, Jump::Down, Orientation::None));

    }
    if (s.canJump(currentNode, Jump::Left)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX, currentY - 2, p, 'j', Direction::None, Jump::Left, Orientation::None));

    }
    if (s.canJump(currentNode, Jump::Right)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX, currentY + 2, p, 'j', Direction::None, Jump::Right, Orientation::None));

    }
    if (s.canJump(currentNode, Jump::TopLeft)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX - 1, currentY - 1, p, 'j', Direction::None, Jump::TopLeft, Orientation::None));

    }
    if (s.canJump(currentNode, Jump::TopRight)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX - 1, currentY + 1, p, 'j', Direction::None, Jump::TopRight, Orientation::None));

    }
    if (s.canJump(currentNode, Jump::BottomLeft)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX + 1, currentY - 1, p, 'j', Direction::None, Jump::BottomLeft, Orientation::None));

    }
    if (s.canJump(currentNode, Jump::BottomRight)) {
        q.push_back(new QuoridorMove(currentX, currentY, currentX + 1, currentY + 1, p, 'j', Direction::None, Jump::BottomRight, Orientation::None));
    }

    if (s.remainingWalls(enemy) == 0 || dist(gen) < BEST_VS_RANDOM_MOVE) {

        int min = 20000;
        QuoridorMove *best = nullptr;

        for (auto *move : q) {
            QuoridorMove *m = static_cast<QuoridorMove *>(move);
            int path = s.fastestPath(m->dx * 9 + m->dy, p);
            if (path >= 0 && path < min) {
                min = path;
                if (best) delete best;
                best = m;
            } else {
                delete m;
            }
        }

        if (best) return best;
    } else {
        int r = rand() % q.size();
        for (int i = 0; i < q.size(); ++i) {
            if (i != r) delete q[i];
        }

        return static_cast<QuoridorMove *>(q[r]);
    }

    return nullptr;
}

bool QuoridorState::forceWall(QuoridorState &s) const
{
    char p = s.turn;
    char enemy = (p == 'w' ? 'b' : 'w');



    int ourPath = s.fastestPath(p);
    int ourWalls = s.remainingWalls(p);

    int enemyPath = s.fastestPath(enemy);
    int enemyWalls = s.remainingWalls(enemy);

    if (enemyPath <= 1 && ourPath > enemyPath) return true;
    if (enemyPath <= 2 && ourPath > enemyPath + 1 && ourWalls >= enemyWalls) return true;
    if (enemyPath <= 3 && ourPath > enemyPath + 2 && ourWalls > enemyWalls) return true;

    return false;
}
