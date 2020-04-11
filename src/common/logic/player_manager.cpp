#include <common/logic/player_manager.hpp>
#include <chrono>

using namespace familyline::logic;

/**
 * Add a player here
 *
 * Return its generated ID
 */
int PlayerManager::add(std::unique_ptr<Player> p)
{
    auto& pi = players_.emplace_back((uintptr_t)p.get() / 1+(((uintptr_t)players_.size()*16384)),
                                     std::move(p));
    return pi.id;
}

/**
 * Push an action
 */
void PlayerManager::pushAction(int id, PlayerInputType type)
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
	uint64_t micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    PlayerInputAction a;
    a.playercode = id;
    a.tick = _tick;
    a.timestamp = micros;
    a.type = type;

    actions_.push(a);
}

/**
 * Adds a listener to the player input action event listeners
 *
 * Returns the ID
 */
int PlayerManager::addListener(PlayerListenerHandler h)
{
    PlayerHandlerInfo phi;
    phi.id = player_input_listeners_.size() + 1;
    phi.handler = h;

    player_input_listeners_.push_back(phi);
    return phi.id;
}

void PlayerManager::processAction(const PlayerInputAction& pia)
{
    fprintf(stderr, "action of player %lu at tick %d | ",
            pia.playercode, pia.tick);

    struct InputVisitor {
        void operator()(BuildAction a) {
            fprintf(stderr, "\033[1m\tBuildAction: typename %s \033[0m\n", a.type_name.c_str());
        }
        void operator()(ObjectSelectAction a) {
            fprintf(stderr, "\033[1m\tObjectSelectAction: id %ld \033[0m\n", a.objectID);
        }
        void operator()(ObjectMoveAction a) {
            fprintf(stderr, "\033[1m\tObjectMoveAction: move selected object to %.2f,%.2f \033[0m\n",
                    a.destX, a.destZ);
        }
        void operator()(ObjectUseAction a) {
            fprintf(stderr, "\033[1m\tObjectUseAction: make selected object use object %ld \033[0m\n",
                    a.useWhat);
        }
        void operator()(ObjectRunAction a) {
            fprintf(stderr, "\033[1m\tObjectRunAction: make selected object run action %s \033[0m\n",
                    a.actionName.c_str());
        }
        void operator()(CameraMove a) {
            fprintf(stderr, "\033[1m\tCameraMove: dx: %.2f, dy: %.2f, dZoom: %.3f \033[0m\n",
                    a.deltaX, a.deltaY, a.deltaZoom);
        }

    };
    std::visit(InputVisitor{}, pia.type);

}


/**
 * Exit was requested by some player
 *
 * This can only be requested by the human player.
 */
bool PlayerManager::exitRequested()
{
    bool r = false;
    
    for (auto& p : players_) {
        r = r || p.player->exitRequested();
    }
    
    return r;
}

/**
 * Generate input from all players
 */
void PlayerManager::generateInput()
{
    for (auto& p : players_) {
        p.player->generateInput();
    }
}

/**
 * Run the input handlers and pop the event from the input action
 * queue
 */
void PlayerManager::run(unsigned int tick)
{
    _tick = tick;
    if (!actions_.empty()) {
        PlayerInputAction& a = actions_.front();
        this->processAction(a);

        for (auto h : player_input_listeners_) {
            h.handler(a);
        }

        actions_.pop();
    }
}
