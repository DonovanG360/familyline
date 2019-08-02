/***
    Macros for turn your life easier when adding objects

    Copyright (C) 2016, 2018 Arthur M
***/
#ifndef OBJECTMACROS_H
#define OBJECTMACROS_H


#include "../logic/Building.hpp"
#include "../logic/Unit.hpp"
#include "../graphical/asset_manager.hpp"
#include "../graphical/gfx_service.hpp"

#include <memory>

#define DEF_MESH(dasset) this->mesh = nullptr /*std::dynamic_pointer_cast<Mesh>(familyline::graphics::GFXService::getAssetManager()->getAsset(dasset)) */

/*  Build a game object, generically.
    So, your object always needs to have a constructor like (oid, x, y ,z) */
#define BUILD_GAME_OBJECT_PTR(object, oid, x, y, z) \
    new ##object (oid, x, y, z)



#define CLONE_MACRO_H(classname) \
    virtual GameObject* clone(glm::vec3 position);


#define CLONE_MACRO_CPP(classname)              \
    GameObject* classname ::clone(glm::vec3 position) { \
    auto a = new classname ();                     \
    a->position = position;                        \
    return a;                              \
    }

#endif /* end of include guard: OBJECTMACROS_H */
