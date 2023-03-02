#include "montecarlonode.h"

#include <iostream>
#include <math.h>
#include <algorithm>

#include "quoridorstate.h"

MonteCarloNode::MonteCarloNode(MonteCarloNode *parent, MonteCarloState *state, const MonteCarloMove *move)
    : m_parent(parent), m_state(state), m_move(move), m_score(0.0), m_nrOfSimulations(0), m_size(0)
{
    m_children.reserve(32); // pre allocate
    m_untriedMoves = state->actions_to_try();
    m_isTerminal = state->isTerminal();
}

MonteCarloNode::~MonteCarloNode()
{
    delete m_state;
    delete m_move;

    for (auto *child : m_children)  {
        delete child;
    }

    while (!m_untriedMoves.empty()) {
        delete m_untriedMoves.front();
        m_untriedMoves.pop();
    }
}

bool MonteCarloNode::isExpanded() const
{
    return isTerminal() || m_untriedMoves.empty();
}

bool MonteCarloNode::isTerminal() const
{
    return m_isTerminal;
}

const MonteCarloMove *MonteCarloNode::getMove() const
{
    return m_move;
}

unsigned int MonteCarloNode::getSize() const
{
    return m_size;
}

void MonteCarloNode::expand()
{
    if (m_isTerminal) {
        rollout();
        return;
    } else if (isExpanded()) {
        std::cerr << "Error; cant expand this!\n";
        return;
    }



    MonteCarloMove *nextMove = m_untriedMoves.front();
    m_untriedMoves.pop();

//    bool found = false;

//    std::cout << "finding next untried move " << std::endl;

//    while (!found) {
//        QuoridorState *qs = static_cast<QuoridorState *>(m_state);
//        bool noerror = false;
//        while (!noerror && !m_untriedMoves.empty()) {
//            noerror = qs->playMove(static_cast<QuoridorMove*>(nextMove));

//            if (!noerror) {
//                nextMove = m_untriedMoves.front();
//                m_untriedMoves.pop();
//            }
//        }

//        if (noerror) break;

//        if (qs->playMove(static_cast<QuoridorMove*>(nextMove))) found = true;
//        else if (m_untriedMoves.empty()) m_untriedMoves = m_state->actions_to_try();
//    }




    MonteCarloState *nextState = m_state->nextState(nextMove);
    MonteCarloNode * newNode = new MonteCarloNode(this, nextState, nextMove);

    newNode->rollout();
    m_children.push_back(newNode);
}

void MonteCarloNode::rollout()
{
    double w = m_state->rollout();
    backpropagate(w, 1);
}

MonteCarloNode *MonteCarloNode::selectBestChild(double c) const
{
    if (m_children.empty()) { return nullptr; }
    else if (m_children.size() == 1) return m_children.at(0);
    else {
        double uct, max = -1;
        MonteCarloNode *best = nullptr;
        for (auto node : m_children) {
            double winrate = node->m_score / static_cast<double>(node->m_nrOfSimulations);

            if (!m_state->who()) {
                winrate = 1.0 - winrate;
            }

            if (c > 0.0) {
                uct = winrate + c * sqrt(log(static_cast<double>(m_nrOfSimulations) / static_cast<double>(node->m_nrOfSimulations)));
            } else {
                uct = winrate;
            }

            if (uct > max) {
                max = uct;
                best = node;
            }
        }

        return best;
    }

    return nullptr; // should not get here
}

MonteCarloNode *MonteCarloNode::advanceTree(const MonteCarloMove *move)
{
    MonteCarloNode *next = nullptr;
    for (auto child : m_children) {
        if (*(child->m_move) == *(move)) {
            next = child;
        } else {
            delete child;
        }
    }

    m_children.clear();

    if (next == nullptr) {
        MonteCarloState *nextState = m_state->nextState(move);

        next = new MonteCarloNode(nullptr, nextState, nullptr);
    } else {
        next->m_parent = nullptr;
    }




    return next;
}

MonteCarloState *MonteCarloNode::getCurrentState() const
{
    return m_state;
}

void MonteCarloNode::printStats()
{
 // TODO
}

double MonteCarloNode::computeWinrate(bool who) const
{
    if (who) {
        return m_score / static_cast<double>(m_nrOfSimulations);
    } else {
        return 1.0 - m_score / static_cast<double>(m_nrOfSimulations);
    }

    return 0.0;
}

void MonteCarloNode::removeBadChild(const MonteCarloMove *move)
{
    MonteCarloNode *toRM = nullptr;

    for (auto child : m_children) {
        if (*(child->m_move) == *(move)) {
            toRM = child;
        }
    }

    m_children.erase(std::remove_if(m_children.begin(),
                                  m_children.end(),
                                  [move](const MonteCarloNode * x){ return (*x->m_move) == *(move); }));

    if (toRM) delete toRM;

}

void MonteCarloNode::backpropagate(double w, int n)
{
    m_score += w;
    m_nrOfSimulations += n;
    if (m_parent != nullptr) {
        m_parent->m_size++;
        m_parent->backpropagate(w, n);
    }
}

MonteCarloTree::MonteCarloTree(MonteCarloState *starting)
{
    m_root = new MonteCarloNode(nullptr, starting, nullptr);
}

MonteCarloNode *MonteCarloTree::select(double c)
{
    MonteCarloNode *node = m_root;
    while (!node->isTerminal()) {
        if (!node->isExpanded()) {
            return node;
        } else {
            node = node->selectBestChild(c);
        }
    }

    return node;
}

void MonteCarloTree::removeBadChild(const MonteCarloMove *move)
{
    m_root->removeBadChild(move);
}


MonteCarloNode *MonteCarloTree::selectBestChild()
{
    return m_root->selectBestChild(0.0);
}

void MonteCarloTree::growTree()
{
    MonteCarloNode *node;
    double dt;

    time_t start_t, now_t;
    time(&start_t);
    for (int i = 0; i < 10000; ++i) {
        node = select();

        node->expand();

        time(&now_t);
        dt = difftime(now_t, start_t);
        if (dt > 5.0) {
            break;
        }
    }
}

void MonteCarloTree::advanceTree(const MonteCarloMove *move)
{
    MonteCarloNode *oldRoot = m_root;
    m_root = m_root->advanceTree(move);
    delete oldRoot;
}

unsigned int MonteCarloTree::getSize() const
{
    return m_root->getSize();
}

const MonteCarloState *MonteCarloTree::getCurrentState() const
{
    return m_root->getCurrentState();
}

void MonteCarloTree::printStats()
{

}
