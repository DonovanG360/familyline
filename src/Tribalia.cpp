
/***
    Tribalia main file

    Copyright 2016 Arthur M.

***/

#include <cstdio>
#include <cstdlib>

#include "Log.hpp"
#include "logic/HumanPlayer.hpp"

#include "graphical/Renderer.hpp"

#include "EnviroDefs.h"

using namespace Tribalia;
using namespace Tribalia::Logic;
using namespace Tribalia::Graphics;

int main(int argc, char const *argv[]) {
    Log::GetLog()->SetFile(stdout);
    Log::GetLog()->Write("Tribalia v%s", VERSION);
    Log::GetLog()->Write("built %s by %s ", __DATE__, USERNAME);

    if (COMMIT > 0)
        Log::GetLog()->Write("Built commit %07x", COMMIT);

    ObjectManager* om = nullptr;
    Renderer* rndr = nullptr;

    try {
        om = new ObjectManager{};
        rndr = new Renderer{};
    } catch (renderer_exception& re) {
        fprintf(stderr, "Rendering error: %s [%d]",
            re.what(), re.code);
        exit(EXIT_FAILURE);
    }

    GameContext gctx;
    gctx.om = om;

    HumanPlayer hp = HumanPlayer{"Arthur"};

    bool player = false;
    do {
        player = true;
    } while (player);

    printf("Exited.\n");

    return 0;
}
