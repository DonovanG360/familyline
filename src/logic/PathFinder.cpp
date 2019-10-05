#include "PathFinder.hpp"
#include <cstring> //memset()
#include <algorithm>

//#include "ObjectEventEmitter.hpp"

using namespace familyline::logic;

PathFinder::PathFinder(ObjectManager* om)
    : _om(om)
{
    // Start listening for object creation events

    // ObjectEventEmitter::addListener(&this->oel);
}

PathFinder::~PathFinder()
{
    delete[] _pathing_slots;
    //ObjectEventEmitter::removeListenr(&this->oel);
}

void PathFinder::InitPathmap(int w, int h)
{
    _mapWidth = w;
    _mapHeight = h;
    if (_pathing_slots)
        delete[] _pathing_slots;

    _pathing_slots = new unsigned char[w*h];
}

// Object information used by the pathfinder
struct ObjectPathData {
    int size;
    object_id_t ID;
    std::weak_ptr<GameObject> obj;

    ObjectPathData(std::weak_ptr<GameObject> obj, int size, object_id_t ID)
        : obj(obj), size(size), ID(ID)
    {}
};
std::vector<ObjectPathData> opd;
    

void PathFinder::UpdatePathmap(int w, int h, int x, int y)
{
    this->ClearPathmap(w, h, x, y);

    /*
    // Update the object list
    ObjectEvent ev;
    while (this->oel.popEvent(ev)) {
        switch (ev.type) {
        case ObjectCreated: {
            if (ev.to.expired())
                continue;

            Log::GetLog()->InfoWrite("pathfinder", ">> created %d at %.2f %.2f", ev.oid);
            auto sto = std::dynamic_pointer_cast<GameObject>(ev.to.lock());
            opd.emplace_back(sto, 3, ev.oid);
            break;
        }
        
        case ObjectDestroyed:       
            Log::GetLog()->InfoWrite("pathfinder",
                                     ">> destroyed %d", ev.oid);
            opd.erase(
                std::remove_if(std::begin(opd), std::end(opd),
                               [&](const ObjectPathData& op) {
                                   if (op.obj.expired())
                                       return true;
                       
                                   return op.ID == ev.oid;
                               })
                      );
            break;
        
        default:
            continue;
        }
        
    }
    
    for (const auto& opddata : opd) {
        auto sobj = opddata.obj.lock();
        if (!sobj)
            continue;

        auto spos = sobj->getPosition();
        
        int ox = spos.x, oz = spos.z;
        float r = opddata.size;

        for (int y = oz - r; y < oz + r; y++) {
            for (int x = ox - r; x < ox + r; x++) {
                _pathing_slots[y*_mapWidth + x] = 0xff;
            }
        }
    }

    */
}

void PathFinder::ClearPathmap(int w, int h, int x, int y) {
    for (int oy = y; oy < y + h; oy++) {
        for (int ox = x; ox < x + w; ox++) {
            _pathing_slots[oy*_mapWidth + ox] = 0;
        }
    }
}


/**
 * Pathfinder calculations
 *
 **/


NodeList::NodeList( PathFinder* const pf, unsigned char* pathlist, unsigned w)
    : pf(pf), pathlist(pathlist), width(w)
{}

/* Check if node has an obstacle on it */
bool NodeList::isObstacle(glm::vec2 pos) {
    unsigned y = unsigned(pos.y);
    unsigned x = unsigned(pos.x);

    return this->pathlist[x + y * this->width] == 0xff;
}

/*  Get a node from the node definitions
    Makes easier to create a node and ensure that it have the same properties
    (score, etc) for the same paths
*/
PathNode* NodeList::getNode(glm::vec2 pos) {
    auto pi = std::find_if(nodes.begin(), nodes.end(), [pos](PathNode* p) {
        return (p->pos == pos);
    });

    if (pi == nodes.end()) {
        return (this->isObstacle(pos)) ? nullptr : new PathNode(this->pf, pos);
    }
    else {
        return *pi;
    }

}


PathNode::PathNode(PathFinder* const pf, glm::vec2 pos)
    : pf(pf), pos(pos), visited(false)
{
    pf->node_list->nodes.push_back(this);
}

/* Get all possible neighbors for a node */
std::vector<PathNode*> PathNode::getNeighbors()
{
    std::vector<PathNode*> r;
    r.reserve(8);

    for (auto y = -1; y <= 1; y++) {
        for (auto x = -1; x <= 1; x++) {
            if (x == 0 && y == 0)
                continue;

            glm::vec2 neipos = this->pos - glm::vec2(x, y);
            if (neipos.x >= 0 && neipos.y >= 0 && neipos.x < this->pf->_mapWidth &&
                neipos.y < this->pf->_mapHeight) {

                auto n = this->pf->node_list->getNode(neipos);
                if (n)
                    r.push_back(n);
            }
        }
    }
    return r;
}


/* Calculate the heuristic*/
double PathNode::getHeuristic(glm::vec2 end, glm::vec2 start)
{
    return glm::distance(start, end);
}

/**
 * Converts a linked list of pathnodes in a list of paths.
 *
 *  Gets the last node and rebuild the path from there, then revert the path.
 */
std::vector<glm::vec2> PathFinder::BuildPath(PathNode* last) {
    std::list<glm::vec2> paths;

    PathNode* node = last;
    while (node) {
        // Push to the front, so we don't need to reverse it later.
        paths.push_front(node->pos);
        node = node->prev;
    }

    std::vector<glm::vec2> rpaths(paths.size());
    std::copy(paths.begin(), paths.end(), rpaths.begin());
    return rpaths;
}

/*  The path node comparator
    The priority queue will return the node with the lowest score, the one
    that might result in a shorter path
*/
struct PathNodeComparator {
    bool operator()(const PathNode* lhs, const PathNode* rhs) const {
        return lhs->f > rhs->f;
    }
};

std::vector<glm::vec2> PathFinder::FindPath(glm::vec2 start, glm::vec2 end) {

    /* The border of our pathfinder, the nodes here are the most periferical ones */
    std::priority_queue<PathNode*, std::vector<PathNode*>, 
                        PathNodeComparator> frontier;

    auto nstart = node_list->getNode(start);
    nstart->visited = true;
    nstart->f = nstart->getHeuristic(end, nstart->pos);
    frontier.push(nstart);

    /* Add unvisited neighbors in the frontier */
    while (!frontier.empty()) {     
        auto frontnode = frontier.top();
        frontier.pop();

        // Compare final position using a bias, so we don't need to
        // be in an exact integer
        if (glm::abs(frontnode->pos.x - end.x) < 1.0 &&
            glm::abs(frontnode->pos.y - end.y) < 1.0) {
            auto b = this->BuildPath(frontnode);
            node_list->nodes.clear();
            return b;
        }

        if (frontnode->pos == end) {
            auto b = this->BuildPath(frontnode);
            node_list->nodes.clear();
            return b;
        }

        for (const auto neighbor : frontnode->getNeighbors()) {
            if (!neighbor->visited) {
                neighbor->visited = true;

                // Remember the previous node, so we can build the path
                neighbor->prev = frontnode;
                neighbor->f = neighbor->getHeuristic(end, neighbor->pos) + 
                    neighbor->getHeuristic(neighbor->pos, start);

                frontier.push(neighbor);
            }
        }

        
        if (frontier.size() > (size_t)(abs(end.x - start.x) * abs(end.x - start.y) * 10)) {
            Log::GetLog()->Fatal("pathfinder", "would get caught in an infinite loop!\n"
                                 "\t\tMight be a bug in the pathfinder");
            Log::GetLog()->InfoWrite("", "origin:          (%.3f, %.3f)", 
                                     start.x, start.y);
            Log::GetLog()->InfoWrite("", "destiny:         (%.3f, %.3f)", 
                                     end.x, end.y);
            Log::GetLog()->InfoWrite("", "actual location: (%.3f, %.3f)",
                                     frontier.top()->pos.x, frontier.top()->pos.y);
            break;
        }
    
    }

    /* Always have an exit */
    auto b = this->BuildPath(frontier.top());
    node_list->nodes.clear();
    return b;
}



std::vector<glm::vec2> PathFinder::CreatePath(const GameObject& o, glm::vec2 destination)
{
    std::vector<glm::vec2> vec;
    std::list<PathNode*> nodelist;

    float r = 8;//o->GetRadius();

    auto position = o.getPosition();
    glm::vec2 from(position.x, position.z);

    /* Unmap our object */
    this->ClearPathmap(r * 2, r * 2, from.x - r, from.y - r);

    /* If we have an obstruction in the destiny, we change the destiny
     *
     * Change the direction to be more closer to the start
     */
    while (this->_pathing_slots[size_t(destination.x) +
                                size_t(destination.y * this->_mapWidth)] == 0xff) {
        auto directionx = (from.x - destination.x);
        auto directiony = (from.y - destination.y);

        if (directionx < 0) {
            destination.x--;
        }
        else if (directionx > 0) {
            destination.x++;
        }

        if (directiony < 0) {
            destination.y--;
        }
        else if (directiony > 0) {
            destination.y++;
        }
    }

    if (this->node_list)
        delete this->node_list;

    this->node_list = new NodeList( this, this->_pathing_slots, unsigned(this->_mapWidth) );
    return this->FindPath(from, glm::vec2(int(destination.x), int(destination.y)));
}
