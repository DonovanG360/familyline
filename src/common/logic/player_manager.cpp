#include <fmt/format.h>
#include <cinttypes>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <common/logger.hpp>
#include <common/logic/ObjectPathManager.hpp>
#include <common/logic/colony.hpp>
#include <common/logic/logic_service.hpp>
#include <common/logic/player_manager.hpp>

/**
 * TODO: create an object that will create other objects.
 *
 * it will be used now to create our test objects in the map, but in the future it will
 * be used to create objects in scripts, or in triggers, or in the start of the game.
 */

using namespace familyline;
using namespace familyline::logic;

/**
 * Add a player here
 *
 * Return its generated ID
 */
int PlayerManager::add(std::unique_ptr<Player> p, bool allocate_id)
{
    auto& pi = players_.emplace_back(
        (uintptr_t)p.get() / 1 + (((uintptr_t)players_.size() * 16384)), std::move(p));

    if (allocate_id) pi.player->code_ = pi.id;

    return pi.id;
}

/**
 * Get a player from the player manager
 *
 * Returns the player object.
 * Remember that this object is owned by the player manager.
 * In C++, you are the borrow checker.
 */
std::optional<Player*> PlayerManager::get(int id) { return this->getPlayerFromID(id); }

std::optional<Player*> PlayerManager::getPlayerFromID(int id)
{
    auto p =
        std::find_if(players_.begin(), players_.end(), [&](PlayerInfo& pi) { return pi.id == id; });

    if (p == players_.end()) {
        return std::optional<Player*>();
    }

    return std::optional<Player*>(p->player.get());
}

/**
 * Iterate between the players, allows the game interface to iterate on the player
 * list
 */
void PlayerManager::iterate(PlayerCallback c)
{
    std::for_each(players_.begin(), players_.end(), [&](auto& p) { c(p.player.get()); });
}

/**
 * Push an action
 */
void PlayerManager::pushAction(unsigned int id, PlayerInputType type)
{
    auto duration   = std::chrono::system_clock::now().time_since_epoch();
    uint64_t micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    auto& log = LoggerService::getLogger();
    log->write(
        "player-manager", LogType::Debug, "push action of player %08x on tick %d", id, _tick);

    PlayerInputAction a;
    a.playercode = id;
    a.tick       = _tick;
    a.timestamp  = micros;
    a.type       = type;

    actions_.push(a);
}

auto getValidSelections(const std::vector<std::weak_ptr<GameObject>>& selections)
{
    // TODO: PLEASE use ranges instead of this.
    std::vector<std::weak_ptr<GameObject>> temp_selections;
    std::vector<std::shared_ptr<GameObject>> valid_selections;

    std::copy_if(
        selections.begin(), selections.end(), std::back_inserter(temp_selections), [](auto& sel) {
            if (sel.expired()) return false;

            return true;
        });

    std::transform(
        temp_selections.begin(), temp_selections.end(), std::back_inserter(valid_selections),
        [](auto& sel) { return sel.lock(); });

    return valid_selections;
}

/**
 * Adds a listener to the player input action event listeners
 *
 * Returns the ID
 */
int PlayerManager::addListener(PlayerListenerHandler h)
{
    PlayerHandlerInfo phi;
    phi.id      = player_input_listeners_.size() + 1;
    phi.handler = h;

    player_input_listeners_.push_back(phi);
    return phi.id;
}

/// This will allow us to use std::visit with multiple variants at once, a thing
/// that should be part of C++20.
/// There are two locations that I use this. I hope this gets suggested for C++23
/// or C++26
template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

void writeSelectDebugInfo(auto& log, auto str, const SelectAction& a)
{
    switch (a.objects.size()) {
        case 0:
            log->write(
                "player-manager", LogType::Info, "%s: SelectAction { objects=0, deselecting all } ",
                str);
            break;
        case 1:
            log->write(
                "player-manager", LogType::Info, "%s: SelectAction { objects=[%lld] } ", str,
                a.objects[0]);
            break;
        case 2:
            log->write(
                "player-manager", LogType::Info, "%s: SelectAction { objects=[%lld, %lld] } ", str,
                a.objects[0], a.objects[1]);
            break;
        case 3:
            log->write(
                "player-manager", LogType::Info, "%s: SelectAction { objects=[%lld, %lld, %lld] } ",
                str, a.objects[0], a.objects[1], a.objects[2]);
            break;
        default:
            log->write(
                "player-manager", LogType::Info,
                "%s: SelectAction { objects=[%lld, %lld, %lld... (%zu objects)] } ", str,
                a.objects[0], a.objects[1], a.objects[2], a.objects.size());
            break;
    }
}

void PlayerManager::processAction(const PlayerInputAction& pia, ObjectManager& om)
{
    fmt::memory_buffer out;
    format_to(out, "action of player {:x} at tick {:d}", pia.playercode, pia.tick);

    assert(olm != nullptr);
    assert(pf != nullptr);

    auto& log = LoggerService::getLogger();

    char str[96];
    sprintf(str, "receive action of player %" PRIx64 " at tick %" PRId32, pia.playercode, pia.tick);

    auto player        = this->getPlayerFromID(pia.playercode);
    bool invalidAction = false;

    if (!player.has_value()) {
        log->write("player-manager", LogType::Fatal, "invalid player ID (%" PRIx64 ")!", pia.playercode);
        return;
    }

    std::visit(
        overload{
            [&](const CommandInput& a) {
                /**
                 * The attack command is the only command treated specially.
                 */
                log->write(
                    "player-manager", LogType::Info, "%s: CommandInput { command=%s } ", str,
                    a.commandName.c_str());

                if (a.commandName == "attack") {
                    auto* objectID = std::get_if<object_id_t>(&a.param);
                    if (!objectID) {
                        invalidAction = true;
                        return;
                    }

                    auto attacker_w = (*player)->getSelections().at(0);
                    auto attackee_o = om.get(*objectID);

                    if (!attacker_w.expired() && attackee_o.has_value()) {
                        auto attacker = attacker_w.lock();
                        auto attackee = (*attackee_o);

                        if (attacker->getColonyComponent().has_value() &&
                            attacker->getColonyComponent()->owner.has_value() &&
                            attacker->getColonyComponent()->owner->get().isOfPlayer(*(*player))) {
                            auto& atkManager = LogicService::getAttackManager();
                            atkManager->doRegister(
                                attacker->getID(), attacker->getAttackComponent().value());
                            atkManager->doRegister(
                                attackee->getID(), attackee->getAttackComponent().value());
                            atkManager->startAttack(attacker->getID(), attackee->getID());
                        }
                    }

                } else {
                    auto selections       = (*player)->getSelections();
                    auto valid_selections = getValidSelections(selections);

                    for (auto& s : valid_selections) {
                        if (s->hasCommand(a.commandName)) {
                            switch (a.param.index()) {
                                case 0: s->runCommand(a.commandName, {}); break;
                                case 1: {
                                    auto object = std::get<object_id_t>(a.param);
                                    s->runCommand(a.commandName, {object});
                                    break;
                                }
                                case 2: {
                                    auto pos = std::get<std::array<int, 2>>(a.param);
                                    s->runCommand(
                                        a.commandName,
                                        {(unsigned long long)pos[0], (unsigned long long)pos[1]});
                                    break;
                                }
                            }
                        }
                    }
                }
            },
            [&](const SelectAction& a) {
                writeSelectDebugInfo(log, str, a);

                (*player)->clearSelection();

                if (a.objects.size() > 0) {
                    auto obj = om.get(a.objects[0]);
                    if (!obj) {
                        invalidAction = true;
                    } else {
                        (*player)->pushToSelection(a.objects[0], *obj);
                    }
                }
            },
            [&](const ObjectMove& a) {
                log->write(
                    "player-manager", LogType::Info, "%s: ObjectMove { xPos=%.2f, yPos=%.2f } ",
                    str, a.xPos, a.yPos);

                auto selections       = (*player)->getSelections();
                auto valid_selections = getValidSelections(selections);

                for (auto& s : valid_selections) {
                    // Only move components that are of the player.
                    if (s->getColonyComponent().has_value() &&
                        s->getColonyComponent()->owner.has_value() &&
                        s->getColonyComponent()->owner->get().isOfPlayer(*(*player))) {
                        auto path    = pf->CreatePath(*s.get(), glm::vec2(a.xPos, a.yPos));
                        glm::vec2 lp = path.back();
                        log->write(
                            "human-player", LogType::Debug, "moved to %.2fx%.2f", lp.x, lp.y);

                        ObjectPathManager::getInstance()->AddPath(s.get(), path);
                    }
                }
            },
            [&](const CameraMove& a) {
                log->write(
                    "player-manager", LogType::Info,
                    "%s: CameraMove { dx=%.2f, dy=%.2f, dZoom=%.2f} ", str, a.deltaX, a.deltaY,
                    a.deltaZoom);

                auto optcam = (*player)->getCamera();
                if (optcam.has_value()) {
                    glm::vec3 mov(a.deltaX, 0, a.deltaY);

                    (*optcam)->AddPosition(mov);
                    (*optcam)->AddLookAt(mov);
                    (*optcam)->AddZoomLevel(a.deltaZoom);
                }
            },
            [&](const CameraRotate& a) {
                log->write(
                    "player-manager", LogType::Info,
                    "%s: CameraRotate { angle=%.3f rad} ", str, a.radians);

                auto optcam = (*player)->getCamera();
                if (optcam.has_value()) {                    
                    (*optcam)->AddRotation(glm::vec3(0, 1, 0), a.radians);
                }                
            },
            [&](const CreateEntity& a) {
                log->write(
                    "player-manager", LogType::Info,
                    "%s: CreateEntity { type=%s, xPos=%d, yPos=%d } ", str, a.type.c_str(),
                    a.xPos, a.yPos);

                auto& of  = LogicService::getObjectFactory();
                auto nobj = std::dynamic_pointer_cast<GameObject>(
                    of->getObject(a.type.c_str(), 0, 0, 0));

                if (!nobj) {
                    log->write(
                        "player-manager", LogType::Error, "building type %s not found",
                        a.type.c_str());
                    invalidAction = false;
                    return;
                }

                auto height = (*player)->terr_.getHeightFromCoords(glm::vec2(a.xPos, a.yPos));
                glm::vec3 buildpos(a.xPos, height, a.yPos);
                nobj->setPosition(buildpos);

                auto cobjID = om.add(std::move(nobj));
                auto ncobj  = om.get(cobjID).value();

                assert(ncobj->getPosition().x == buildpos.x);
                assert(ncobj->getPosition().z == buildpos.z);

                this->render_add_callback(ncobj);

                olm->doRegister(ncobj);
                colony_add_callback(ncobj, (*player)->getCode());
                olm->notifyCreation(cobjID);
                
            },
            [&](const AddSelectAction& a) {}, [&](const CreateSelectGroup& a) {},
            [&](const SelectGroup& a) {}, [&](const AddSelectGroup& a) {},
            [&](const RemoveSelectGroup& a) {}},
        pia.type);

    if (invalidAction) {
        log->write("player-manager", LogType::Fatal, "last action had an invalid parameter!");
        log->write("player-manager", LogType::Fatal, "This WILL cause issues later!");
    }
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
 * Gets a multimap of ID=>playername, so you can easily discover the
 * ID of a certain player name, or a name of a player who has a
 * certain ID
 */
std::multimap<int, std::string> PlayerManager::getPlayerNames()
{
    std::multimap<int, std::string> r;

    std::for_each(players_.begin(), players_.end(), [&r](PlayerInfo& i) {
        r.emplace(i.id, std::string{i.player->getName()});
    });
    return r;
}

/**
 * Run the input handlers and pop the event from the input action
 * queue
 */
void PlayerManager::run(GameContext& gctx)
{
    _tick = gctx.tick;

    while (!actions_.empty()) {
        PlayerInputAction& a = actions_.front();
        this->processAction(a, *gctx.om);

        for (auto h : player_input_listeners_) {
            h.handler(a);
        }

        actions_.pop();
    }
}
