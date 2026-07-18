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

    // void* this_00 = nlMalloc(0xACU, 8U, 0);
    // temp_ret = nlMalloc(0xACU, 8U, 0);
    // temp_r3 = temp_ret;
    // var_r4 = (u32)(u64)temp_ret;
    // var_r0 = temp_r3;
    // if (var_r0 != NULL)
    // {
    //     temp_ret_2 = __ct__5cBallFv(temp_r3);
    //     var_r4 = (u32)(u64)temp_ret_2;
    //     var_r0 = temp_ret_2;
    // }
    // g_pBall = (FakeBallWorld*)var_r0;

    FakeBallWorld::Init(g_pBall);
    // Init__13FakeBallWorldFP5cBall((FakeBallWorld*)var_r0, (cBall*)var_r4);

    CreateCharacters();

    return true;
}
