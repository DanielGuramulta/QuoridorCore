#ifndef QUORIDORSTATE_H
#define QUORIDORSTATE_H

#include "montecarlonode.h"
#include "gameboard.h"
#include "quoridormove.h"
#include <random>


class QuoridorState : public MonteCarloState
{
public:
    QuoridorState(char me);
    QuoridorState(const QuoridorState &other);

    // MonteCarloState interface
public:
    std::queue<MonteCarloMove *> actions_to_try() const;
    MonteCarloState *nextState(const MonteCarloMove *move) const;
    double rollout() const;
    bool isTerminal() const;
    bool who() const;


    bool playMove(const QuoridorMove *move);
    char checkWinner() const;

    char getTurn() const {return turn;}

    void printBoard();
    int countWinNodes(int targetX) const;


private:



    bool canMove(const node &n, Direction direction) const;
    void setCantMove(node &n, Direction direction);
    void setCanMove(node &n, Direction direction);
    bool canJump(int current, Jump jump) const;
    bool isEnemy(int current, Direction d) const;
    Wall addWall(int x, int y, Orientation orientation);
    Wall removeWall(int x, int y, Orientation orientation);
    bool isWallValid(int x, int y, Orientation orientation);
    char changeTurn();

    /// short path
    template<class Q>
    void enqueue_neighbours(Q &q, const node &current, const Gameboard &board, std::array<bool, 81> &visited, std::array<int, 81> &dist, std::array<int, 81> &pred, int currentDist) const;
    int fastestPath(char who = 'w', bool earlyExit = false) const;
    int fastestPath(int source_node, char who, bool earlyExit = false) const;


    // evaluate
    double evaluate(QuoridorState &s) const;

    // generate Moves
    std::queue<MonteCarloMove *> generateMoves();
    QuoridorMove *pickMove(QuoridorState &s, std::uniform_real_distribution<double> &dist, std::default_random_engine &gen) const;
    int remainingWalls(char p) const { return (p == 'w') ? wnrwalls : bnrwalls; }
    bool forceWall(QuoridorState &s) const;

public:
    int wx, wy; // white on board
    int bx, by; // black on board;
    int wp, bp; // white fast path, black fast path;
    int wnrwalls, bnrwalls;

    char me;
    char turn;

    // TODO maybe longest path also;

    Gameboard board;


    static std::default_random_engine generator;

    int steps;


};

#endif // QUORIDORSTATE_H
