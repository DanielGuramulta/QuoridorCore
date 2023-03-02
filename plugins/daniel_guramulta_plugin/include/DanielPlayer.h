#ifndef Header_qcore_ICPlayer
#define Header_qcore_ICPlayer

#include "Player.h"


#include "montecarlonode.h"
#include "quoridorstate.h"

namespace qplugin
{

class DanielPlayer : public qcore::Player
{
public:
    DanielPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);
    void doNextMove() override;

private:
    char me;
    bool firstMove {false};

    QuoridorState *state;
    MonteCarloTree *gameTree;
};
}

#endif // Header_qcore_ICPlayer
