#ifndef MONTECARLONODE_H
#define MONTECARLONODE_H

#include <vector>
#include <queue>

class MonteCarloMove
{
public:
    virtual ~MonteCarloMove() = default;
    virtual bool operator==(const MonteCarloMove& other) const = 0;
};



class MonteCarloState
{
public:
    virtual ~MonteCarloState() = default;
    virtual std::queue<MonteCarloMove *> actions_to_try() const = 0;
    virtual MonteCarloState *nextState(const MonteCarloMove *move) const = 0;
    virtual double rollout() const = 0;
    virtual bool isTerminal() const = 0;
    virtual bool who() const = 0;     // true me false opponent
};



class MonteCarloNode
{
public:
    MonteCarloNode(MonteCarloNode *parent, MonteCarloState *state, const MonteCarloMove *move);
    ~MonteCarloNode();

    bool isExpanded() const;
    bool isTerminal() const;

    const MonteCarloMove *getMove() const;
    unsigned int getSize() const;

    void expand();
    void rollout();

    MonteCarloNode* selectBestChild(double c) const;
    MonteCarloNode* advanceTree(const MonteCarloMove *move);
    MonteCarloState* getCurrentState() const;

    void printStats();
    double computeWinrate(bool who) const;

    void removeBadChild(const MonteCarloMove *move);

private:
    void backpropagate(double w, int n);


private:
    bool m_isTerminal;
    unsigned int m_size;
    unsigned int m_nrOfSimulations;
    double m_score;

    MonteCarloNode *m_parent;
    std::vector<MonteCarloNode*> m_children;

    MonteCarloState *m_state;
    const MonteCarloMove *m_move;

    std::queue<MonteCarloMove *> m_untriedMoves; // maybe priorityQueue? or set based on something
};


class MonteCarloTree
{
public:
    MonteCarloTree(MonteCarloState *starting);

    MonteCarloNode *select(double c = 1.41); // UCT
    MonteCarloNode *selectBestChild();

    void growTree(); // implements time limit
    void advanceTree(const MonteCarloMove *move);

    unsigned int getSize() const;
    const MonteCarloState *getCurrentState() const;

    void printStats();

    void removeBadChild(const MonteCarloMove *move);
private:
    MonteCarloNode *m_root;
};



#endif // MONTECARLONODE_H
