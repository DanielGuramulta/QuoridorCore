#include "DanielPlayer.h"
#include "QcoreUtil.h"

#include <queue>
#include <sstream>
#include <iomanip>
#include <chrono>


using namespace qcore::literals;
using namespace std::chrono_literals;

namespace
{
   /** Log domain */
   const char * const DOM = "qplugin::IC";

   constexpr auto TM = 4s;
   constexpr auto RF = 1;
}

namespace qplugin
{

   DanielPlayer::DanielPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game)
       : qcore::Player(id, name, game)
    {
        srand(time(NULL));

        // figure out who we are 'w' or 'b'
        me = (getId() == 0 ? 'w' : 'b');

        if (me == 'w') firstMove = true; // we will have first move do not look at last action

        state = new QuoridorState(me);
        gameTree = new MonteCarloTree(new QuoridorState(me));


    }

    void DanielPlayer::doNextMove()
    {
        if (!firstMove) {



            auto lastAction = getBoardState()->getLastAction();
            if (lastAction.actionType == qcore::ActionType::Wall)
            {
                std::cout << "LAst wall: " << (int)lastAction.wallState.position.x << " " << (int)lastAction.wallState.position.y << " " << (lastAction.wallState.orientation ==qcore::Orientation::Horizontal?'h':'v') << '\n';

                Direction d = Direction::None;
                Jump j = Jump::None;
                Orientation o = Orientation::None;

                int x = lastAction.wallState.position.x;
                int y = lastAction.wallState.position.y;

                o = (lastAction.wallState.orientation == qcore::Orientation::Horizontal
                     ? Orientation::Horizontal
                     : Orientation::Vertical);

                if (o == Orientation::Horizontal) {
                    x = x - 1;
                } else if (o == Orientation::Vertical) {
                    y = y - 1;
                }


                QuoridorMove *mv = new QuoridorMove(x,
                                                    y,
                                                    x,
                                                    y,
                                                    (me == 'w' ? 'b' : 'w'),
                                                    'w',
                                                    d, j, o);

                std::cout << "enemy played wall at " << x << " " << y << std::endl;

                std::cout << "Play move for: " << me << std::endl;
                if(!state->playMove(mv)) {
                    std::cout << "failed move for opponenet\n";
                }
                gameTree->advanceTree(mv);

            } else if (lastAction.actionType == qcore::ActionType::Move)
            {
                qcore::Position enemyPos;
                qcore::Position lastEnemyPos;

                if (me == 'w') {
                    enemyPos = getBoardState()->getPlayers(0).at(1).position;
                    lastEnemyPos = qcore::Position(state->bx, state->by);
                    std::cout << "enemy at: " << state->bx << " " << state->by << std::endl;
                } else {
                    enemyPos = getBoardState()->getPlayers(0).at(0).position;
                    lastEnemyPos = qcore::Position(state->wx, state->wy);
                    std::cout << "enemy at: " << state->wx << " " << state->wy << std::endl;
                }

                qcore::Position diff = lastEnemyPos - enemyPos;
                Direction d = Direction::None;
                Jump j = Jump::None;
                Orientation o = Orientation::None;
                if (diff.x == 1 && diff.y == 0) {
                    // up
                    d = Direction::Up;

                } else if (diff.x == -1 && diff.y == 0) {
                    // down
                    d = Direction::Down;

                } else if (diff.x == 0 && diff.y == 1) {
                    // left
                    d = Direction::Left;

                } else if (diff.x == 0 && diff.y == -1) {
                    // right
                    d = Direction::Right;

                } else if (diff.x == 2 && diff.y == 0) {
                    // jump up
                    j = Jump::Up;
                } else if (diff.x == -2 && diff.y == 0) {
                    // jump down
                    j = Jump::Down;
                } else if (diff.x == 0 && diff.y == 2) {
                    // jump left
                    j = Jump::Left;

                } else if (diff.x == 0 && diff.y == -2) {
                    // jump right
                    j = Jump::Right;

                } else if (diff.x == 1 && diff.y == 1) {
                    // jump topleft
                    j = Jump::TopLeft;

                } else if (diff.x == 1 && diff.y == -1) {
                    // jump top rigth
                    j = Jump::TopRight;

                } else if (diff.x == -1 && diff.y == 1) {
                    // jump bottom left
                    j = Jump::BottomLeft;

                } else if (diff.x == -1 && diff.y == -1) {
                    // jump bottom right
                    j = Jump::BottomRight;

                }

                QuoridorMove *mv = new QuoridorMove(lastEnemyPos.x,
                                                      lastEnemyPos.y,
                                                      enemyPos.x,
                                                      enemyPos.y,
                                                      (me == 'w' ? 'b' : 'w'),
                                                      (d != Direction::None ? 'm' : 'j'),
                                                      d, j, o);

                std::cout << "Play move for: " << me << std::endl;
                if(!state->playMove(mv)) {
                    std::cout << "failed move for opponenet\n";
                }
                gameTree->advanceTree(mv);
            }

//            if (me == 'w') {
//                enemyPos = getBoardState()->getPlayers(0).at(1).position;
//            }


            std::cout << "Daniel knows enemy is at: ";
            std::cout << (me == 'w' ? state->bx : state->wx) << " " << (me == 'w' ? state->by : state->wy) << '\n';

        } else firstMove = false;

        gameTree->growTree();

        MonteCarloNode* bestChild = gameTree->selectBestChild();
        const QuoridorMove *mv = static_cast<const QuoridorMove *>(bestChild->getMove());

        std::cout << "Daniel knows enemy is at: ";
        std::cout << (me == 'w' ? state->bx : state->by) << '\n';

        std::cout << "Play move for: " << me << std::endl;
        bool noerror = state->playMove(mv);

        while (!noerror) {
            gameTree->removeBadChild(mv);
            bestChild = gameTree->selectBestChild();
            if (bestChild == nullptr) break;
            mv = static_cast<const QuoridorMove *>(bestChild->getMove());
        }

        if (noerror) {
            gameTree->advanceTree(mv);

            std::cout << "if we see this log we **** up\n";

            if (mv->t == 'm' || mv->t == 'j') {
                // play move
                auto initialState = getBoardState()->getPlayers(0).at(getId()).initialState;
                std::cout << "MOving Daniel to: " << mv->dx << " " << mv->dy << std::endl;
                qcore::Position position(mv->dx, mv->dy);
                move(position.rotate(static_cast<int>(initialState)));

            } else
            {
                std::cout << "adding wall at " << mv->dx << " " << mv->dy << (mv->o == Orientation::Horizontal ? 'h' : 'v') << std::endl;
                // play wall
                auto initialState = getBoardState()->getPlayers(0).at(getId()).initialState;
                qcore::Position position(mv->dx, mv->dy);
                if (mv->o == Orientation::Horizontal) {
                    position = position + qcore::Position(1, 0);
                } else {
                    position = position + qcore::Position(0, 1);

                }

                qcore::WallState wl {position, mv->o == Orientation::Horizontal ? qcore::Orientation::Horizontal : qcore::Orientation::Vertical};
                    placeWall(wl.rotate(static_cast<int>(initialState)));
            }

        }
        else
            {
//                move(qcore::Direction::Right) or move(qcore::Direction::Up);
                Direction d = Direction::None;
                Jump j = Jump::None;
                Orientation o = Orientation::None;
                if (me == 'w') {
                    if (move(qcore::Direction::Up))
                    {
                        d = Direction::Up;
                        QuoridorMove *mv = new QuoridorMove(state->wx,
                                                              state->wy,
                                                              state->wx - 1,
                                                              state->wy,
                                                              me,
                                                              (d != Direction::None ? 'm' : 'j'),
                                                              d, j, o);

                        state->playMove(mv);
                        gameTree->advanceTree(mv);
                        return;
                    }
                } else {
                    if (move(qcore::Direction::Down)) {
                        d = Direction::Down;
                        QuoridorMove *mv = new QuoridorMove(state->wx,
                                                              state->wy,
                                                              state->wx + 1,
                                                              state->wy,
                                                              me,
                                                              (d != Direction::None ? 'm' : 'j'),
                                                              d, j, o);
                        state->playMove(mv);
                        gameTree->advanceTree(mv);
                        return;
                    }
                }

                if (move(qcore::Direction::Right)) {
                    d = Direction::Right;
                    QuoridorMove *mv = new QuoridorMove(state->wx,
                                                          state->wy,
                                                          state->wx,
                                                          state->wy + 1,
                                                          me,
                                                          (d != Direction::None ? 'm' : 'j'),
                                                          d, j, o);
                    state->playMove(mv);
                    gameTree->advanceTree(mv);
                    return;
                }

                if (move(qcore::Direction::Left)) {
                    d = Direction::Left;
                    QuoridorMove *mv = new QuoridorMove(state->wx,
                                                          state->wy,
                                                          state->wx,
                                                          state->wy - 1,
                                                          me,
                                                          (d != Direction::None ? 'm' : 'j'),
                                                          d, j, o);
                    state->playMove(mv);
                    gameTree->advanceTree(mv);
                    return;
                }

                if (me == 'w') {
                    if (move(qcore::Direction::Down))
                    {
                        d = Direction::Down;
                        QuoridorMove *mv = new QuoridorMove(state->wx,
                                                              state->wy,
                                                              state->wx + 1,
                                                              state->wy,
                                                              me,
                                                              (d != Direction::None ? 'm' : 'j'),
                                                              d, j, o);

                        state->playMove(mv);
                        gameTree->advanceTree(mv);
                        return;
                    }
                } else {
                    if (move(qcore::Direction::Up)) {
                        d = Direction::Up;
                        QuoridorMove *mv = new QuoridorMove(state->wx,
                                                              state->wy,
                                                              state->wx - 1,
                                                              state->wy,
                                                              me,
                                                              (d != Direction::None ? 'm' : 'j'),
                                                              d, j, o);
                        state->playMove(mv);
                        gameTree->advanceTree(mv);
                        return;
                    }
                }

            }

    }

} // namespace qplugin
