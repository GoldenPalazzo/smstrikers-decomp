#include "Game/AI/AILoader.h"

#include "NL/nlMemory.h"

#include "Game/Ball.h"
#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/CharacterLifecycle.h"

AILoader TheAILoader;

/**
 * Offset/Address/Size: 0x0 | 0x800056C0 | size: 0x50
 */
bool AILoader::StartLoad(LoadingManager*)
{
    FakeBallWorld* temp_ret_2;
    cBall* temp_r3;
    cBall* temp_ret;
    cBall* var_r0;
    u32 var_r4;

    g_pBall = new (nlMalloc(0xACU, 8U, 0)) cBall();

    FakeBallWorld::Init(g_pBall);

    CreateCharacters();

    return true;
}
