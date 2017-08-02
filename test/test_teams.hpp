/*  Unit test fixture for team testing
 *
 *  Copyright (C) 2017 Arthur M.
 */

#include <gtest/gtest.h>

#include "logic/City.hpp"
#include "logic/Team.hpp"
#include "TestObject.hpp"

class TeamRelationTest : public ::testing::Test {
protected:
    Tribalia::Logic::TeamCoordinator tc;
    Tribalia::Logic::City *c11, *c12, *c21, *c22;
    
public:
    void SetUp();
    void TearDown();
};
