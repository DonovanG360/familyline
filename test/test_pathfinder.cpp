#include <gtest/gtest.h>

#include <algorithm>
#include <common/logic/logic_service.hpp>
#include <common/logic/pathfinder.hpp>
#include <common/logic/terrain.hpp>

#include "utils.hpp"

#include <cmath>

using namespace familyline::logic;

std::vector<bool> createPathmap(int w, int h) { return std::vector<bool>(w * h, false); }

void addObjectToMap(std::vector<bool>& map, int width, int height, const GameObject& o)
{
    EXPECT_EQ(width*height, map.size());
    
    std::vector<int> indices;
    auto pos  = o.getPosition();
    auto size = o.getSize();

    int minx = fmax(0.0, pos.x - (size.x / 2.0));
    int miny = fmax(0.0, pos.z - (size.y / 2.0));
    int maxx = fmin(double(width), pos.x + (size.x / 2.0));
    int maxy = fmin(double(height), pos.z + (size.y / 2.0));

    printf("%.2f %.2f %.2f %.2f\n", pos.x, pos.z, size.x, size.y);
    
    for (auto y = miny; y < maxy; y++) {
        for (auto x = minx; x < maxx; x++) {
            printf("%d %d (%d)\t", y, x, y*width+x);
            indices.push_back(y*width+x);
        }
    }
    
    for (int i = 0; i < map.size(); i++) {
        if (std::find(indices.begin(), indices.end(), i) != indices.end())
            map[i] = true;
    }
}

std::vector<glm::vec2> createPath(Pathfinder& pf, ObjectManager& om, int oid, glm::vec2 end)
{
    auto obj = om.get(oid).value();
    auto pos2d = glm::vec2(obj->getPosition().x, obj->getPosition().z);
    auto size = obj->getSize();

    return pf.findPath(pos2d, end, size);
}

TEST(Pathfinder, CanWalkStraightLine)
{
    ObjectManager om;

    auto atkComp                 = std::optional<AttackComponent>();
    struct object_init objParams = {"test-obj", "Test Object", glm::vec2(3, 3), 100,
                                    100,        false,         [&]() {},        atkComp};

    auto component = make_object(objParams);
    component->setPosition(glm::vec3(10, 1, 10));

    auto id = om.add(std::move(component));

    TerrainFile tf{25, 25};
    Terrain t(tf);

    Pathfinder pf(t);
    auto map = createPathmap(25, 25);

    pf.update(map);

    auto path = createPath(pf, om, id, glm::vec2(20, 10));
    
    EXPECT_EQ(11, path.size());
    ASSERT_EQ(glm::vec2(10, 10), path[0]);
    ASSERT_EQ(glm::vec2(11, 10), path[1]);
    ASSERT_EQ(glm::vec2(12, 10), path[2]);
    ASSERT_EQ(glm::vec2(13, 10), path[3]);
    ASSERT_EQ(glm::vec2(14, 10), path[4]);
    ASSERT_EQ(glm::vec2(15, 10), path[5]);
    ASSERT_EQ(glm::vec2(16, 10), path[6]);
    ASSERT_EQ(glm::vec2(17, 10), path[7]);
    ASSERT_EQ(glm::vec2(18, 10), path[8]);
    ASSERT_EQ(glm::vec2(19, 10), path[9]);
    ASSERT_EQ(glm::vec2(20, 10), path[10]);
}

TEST(Pathfinder, CanWalkDiagonalLine)
{
    ObjectManager om;

    auto atkComp                 = std::optional<AttackComponent>();
    struct object_init objParams = {"test-obj", "Test Object", glm::vec2(3, 3), 100,
                                    100,        false,         [&]() {},        atkComp};

    auto component = make_object(objParams);
    component->setPosition(glm::vec3(10, 1, 10));

    auto id = om.add(std::move(component));

    TerrainFile tf{100, 100};
    Terrain t(tf);

    Pathfinder pf(t);
    auto map = createPathmap(100, 100);
    pf.update(map);

    auto path = createPath(pf, om, id, glm::vec2(22, 22));

    EXPECT_EQ(13, path.size());
    ASSERT_EQ(glm::vec2(10, 10), path[0]);
    ASSERT_EQ(glm::vec2(11, 11), path[1]);
    ASSERT_EQ(glm::vec2(12, 12), path[2]);
    ASSERT_EQ(glm::vec2(13, 13), path[3]);
    ASSERT_EQ(glm::vec2(14, 14), path[4]);
    ASSERT_EQ(glm::vec2(15, 15), path[5]);
    ASSERT_EQ(glm::vec2(16, 16), path[6]);
    ASSERT_EQ(glm::vec2(17, 17), path[7]);
    ASSERT_EQ(glm::vec2(18, 18), path[8]);
    ASSERT_EQ(glm::vec2(19, 19), path[9]);
    ASSERT_EQ(glm::vec2(20, 20), path[10]);
    ASSERT_EQ(glm::vec2(21, 21), path[11]);
    ASSERT_EQ(glm::vec2(22, 22), path[12]);
}

TEST(Pathfinder, CanWalkAroundObstacle)
{
    auto& actionQueue = LogicService::getActionQueue();
    actionQueue->clearEvents();

    const auto& olist = familyline::logic::LogicService::getObjectListener();

    ObjectManager om;

    auto atkComp                 = std::optional<AttackComponent>();
    struct object_init objParams = {"test-obj", "Test Object", glm::vec2(3, 3), 100,
                                    100,        false,         [&]() {},        atkComp};

    auto component = make_object(objParams);
    auto obstacle  = make_object(objParams);

    component->setPosition(glm::vec3(10, 1, 10));
    obstacle->setPosition(glm::vec3(16, 1, 16));

    auto id = om.add(std::move(component));
    auto obsid = om.add(std::move(obstacle));

    actionQueue->processEvents();
    olist->updateObjects();


    TerrainFile tf{100, 100};
    Terrain t(tf);

    Pathfinder pf(t);
    auto map = createPathmap(100, 100);

    addObjectToMap(map, 100, 100, *om.get(obsid).value().get());
    pf.update(map);

    auto path = createPath(pf, om, id, glm::vec2(22, 22));

    int i = 0;
    
    EXPECT_LT(13, path.size());
    ASSERT_EQ(glm::vec2(10, 10), path[0]);
    ASSERT_EQ(glm::vec2(11, 11), path[1]);
    ASSERT_EQ(glm::vec2(12, 11), path[2]);
    ASSERT_NE(glm::vec2(13, 13), path[3]);
    ASSERT_NE(glm::vec2(14, 14), path[4]);
    ASSERT_NE(glm::vec2(15, 15), path[5]);
    ASSERT_NE(glm::vec2(16, 16), path[6]);
    ASSERT_NE(glm::vec2(17, 17), path[7]);
    ASSERT_NE(glm::vec2(18, 18), path[8]);
    ASSERT_NE(glm::vec2(19, 19), path[9]);

    ASSERT_EQ(glm::vec2(21, 21), path[path.size() - 2]);
    ASSERT_EQ(glm::vec2(22, 22), path[path.size() - 1]);
}
