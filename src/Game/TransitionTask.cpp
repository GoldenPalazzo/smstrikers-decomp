#include "Game/TransitionTask.h"
#include "Game/Font/fontmanager.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/OverlayManager.h"
#include "Game/FE/feSceneManager.h"
#include "Game/FE/feResourceManager.h"
#include "Game/FE/feManager.h"
#include "Game/GameInfo.h"
#include "Game/DB/StatsTracker.h"
#include "Game/Transitions/ScreenTransitionManager.h"
#include "Game/Sys/eventman.h"
#include "Game/Ball.h"
#include "Game/Physics/Physics.h"
#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Render/FlareHandler.h"
#include "Game/NisPlayer.h"
#include "Game/ReplayChoreo.h"
#include "Game/ReplayManager.h"
#include "Game/Render/Jumbotron.h"
#include "Game/Render/CrowdManager.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim/pnBlender.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "Game/SAnim/pnFeather.h"
#include "Game/Render/SidelineExplodable.h"
#include "Game/BeginFrameTask.h"
#include "Game/WorldManager.h"
#include "Game/ObjectBlur.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/AudioScriptEventMgr.h"
#include "Game/CharacterTemplate.h"
#include "Game/Sys/PlatStream.h"
#include "Game/Debug/TimeRegions.h"
#include "Game/Render/ElectricFence.h"
#include "Game/Game.h"
#include "Game/Drawable/DrawableModel.h"
#include "NL/nlMemory.h"
#include "NL/gl/gl.h"
#include "NL/gl/glView.h"
#include "NL/glx/glxSwap.h"
#include "NL/plat/plataudio.h"

struct LocalizationObj
{
    void* m_pFile;
    void* m_LookupTable;
    void* m_FirstString;
    unsigned long m_Language;
};

extern LocalizationObj* g_pLocalization;

int nlSNPrintf(char*, unsigned long, const char*, ...);

extern unsigned char g_bFrameStatsOnDisk;
static unsigned long long s_FontResourceMark;
extern unsigned char gSebringLoadPackageToVirtualMemory;
extern PhysicsLoader ThePhysicsLoader;

void glx_SetFog(int);
bool fxParticleShutdown();
bool fxUnloadGroups();
bool fxUnloadTemplates();
void glResourceRelease(unsigned long long);

// /**
//  * Offset/Address/Size: 0x0 | 0x801731FC | size: 0x10
//  */
// void 0x8028D338..0x8028D33C | size : 0x4
// {
// }

/**
 * Offset/Address/Size: 0xC | 0x801731F8 | size: 0x4
 */
void TransitionTask::Run(float)
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x801731EC | size: 0xC
//  */
// void TransitionTask::GetName()
// {
// }

// /**
//  * Offset/Address/Size: 0x1BF4 | 0x801731C4 | size: 0x28
//  */
// void Detail::SwitchToStartScreenLoader::StartLoad(LoadingManager*)
// {
// }

/**
 * Offset/Address/Size: 0x1A10 | 0x80172FE0 | size: 0x1E4
 */
void LoadFonts()
{
    const char* fontFileName1 = "game_font.fnt";
    const char* fontFileName2 = "fe_font.fnt";
    char langCode[4] = "eng";

    unsigned long lang = g_pLocalization->m_Language;
    switch (lang)
    {
    case 0:
        langCode[0] = 'f';
        langCode[1] = 'r';
        langCode[2] = 'e';
        break;
    case 1:
        langCode[0] = 'd';
        langCode[1] = 'e';
        langCode[2] = 'u';
        break;
    case 2:
        langCode[0] = 'i';
        langCode[1] = 't';
        langCode[2] = 'a';
        break;
    case 3:
        langCode[0] = 'j';
        langCode[1] = 'p';
        langCode[2] = 'n';
        break;
    case 4:
        langCode[0] = 's';
        langCode[1] = 'p';
        langCode[2] = 'a';
        break;
    case 5:
        langCode[0] = 'u';
        langCode[1] = 'k';
        langCode[2] = 'e';
        break;
    case 6:
        langCode[0] = 'b';
        langCode[1] = 'o';
        langCode[2] = 'b';
        break;
    case 7:
        langCode[0] = 'l';
        langCode[1] = 'n';
        langCode[2] = 'g';
        break;
    case 8:
        break;
    }

    char bundlePath1[64];
    char fontName1[64];
    char bundlePath2[64];
    char fontName2[64];

    nlSNPrintf(bundlePath1, 64, "fonts/%s/game_font.bnl", langCode);
    nlSNPrintf(fontName1, 64, "fonts/%s/game_font", langCode);
    nlSNPrintf(bundlePath2, 64, "fonts/%s/fe_font.bnl", langCode);
    nlSNPrintf(fontName2, 64, "fonts/%s/fe_font", langCode);

    nlSingleton<FontManager>::s_pInstance->LoadFont(bundlePath1, fontName1, fontFileName1);
    nlSingleton<FontManager>::s_pInstance->LoadFont(bundlePath2, fontName2, fontFileName2);
}

/**
 * Offset/Address/Size: 0x19E0 | 0x80172FB0 | size: 0x30
 */
TransitionTask::TransitionTask()
{
    m_pAIHandler = nullptr;
    m_pGoalieHandler = nullptr;
    m_pLoadingManager = nullptr;
    m_TransitionState = eTS_Unknown;
}

/**
 * Offset/Address/Size: 0x19D8 | 0x80172FA8 | size: 0x8
 */
void TransitionTask::Initialize(LoadingManager& loadingManager)
{
    m_pLoadingManager = &loadingManager;
}

/**
 * Offset/Address/Size: 0x10B8 | 0x80172688 | size: 0x920
 */
void TransitionTask::StateTransition(unsigned int, unsigned int)
{
}

/**
 * Offset/Address/Size: 0x888 | 0x80171E58 | size: 0x830
 */
void TransitionTask::InitializeGameState()
{
}

/**
 * Offset/Address/Size: 0x4BC | 0x80171A8C | size: 0x3CC
 */
void TransitionTask::DestroyGameState()
{
    m_TransitionState = eTS_Destroying;

    if (g_bFrameStatsOnDisk)
    {
        WriteFrameRateStatsToFile();
    }

    DestroyTimeRegions();

    while (!nlSingleton<FESceneManager>::s_pInstance->AreAllScenesValid())
    {
        nlServiceFileSystem();
        nlSingleton<FESceneManager>::s_pInstance->Update(0.0f);
        nlSingleton<FEResourceManager>::s_pInstance->Run(0.0f);
    }

    nlSingleton<OverlayManager>::s_pInstance->PopEntireStack();
    nlSingleton<FESceneManager>::s_pInstance->ForceImmediateStackProcessing();

    glxSwapLoading(false, false);

    nlSingleton<OverlayManager>::s_pInstance->Push((SceneList)0x4E, (ScreenMovement)0, false);

    nlSingleton<FESceneManager>::s_pInstance->ForceImmediateStackProcessing();

    while (!nlSingleton<FESceneManager>::s_pInstance->AreAllScenesValid())
    {
        nlServiceFileSystem();
        nlSingleton<FESceneManager>::s_pInstance->Update(0.0f);
        nlSingleton<FEResourceManager>::s_pInstance->Run(0.0f);
    }

    for (int i = 0; i < 2; i++)
    {
        glBeginFrame();
        SetupMatrices();
        nlSingleton<FEResourceManager>::s_pInstance->Run(1.0f / 30.0f);
        nlSingleton<FESceneManager>::s_pInstance->Update(1.0f / 30.0f);
        nlSingleton<FESceneManager>::s_pInstance->RenderActiveScenes();
        glFinish();
        glEndFrame();
        glSendFrame();
    }

    glxSwapLoading(true, false);

    PlatAudio::StopAllStreams();
    AudioScriptEventMgr::Purge();
    FlareHandler::instance.Cleanup();
    NisPlayer::Instance()->Reset();
    ReplayChoreo::Instance().Reset();
    ReplayManager::Instance()->Uninitialize();

    glx_SetFog(-1);

    while (!nlSingleton<FESceneManager>::s_pInstance->AreAllScenesValid())
    {
        nlServiceFileSystem();
        nlSingleton<FESceneManager>::s_pInstance->Update(0.0f);
        nlSingleton<FEResourceManager>::s_pInstance->Run(0.0f);
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode != 0)
    {
        nlSingleton<OverlayManager>::s_pInstance->DestroyMessengerManager();
    }

    nlSingleton<OverlayManager>::s_pInstance->PopEntireStack();

    if (nlSingleton<OverlayManager>::s_pInstance != NULL)
    {
        delete nlSingleton<OverlayManager>::s_pInstance;
        nlSingleton<OverlayManager>::s_pInstance = NULL;
    }

    if (nlSingleton<FESceneManager>::s_pInstance != NULL)
    {
        delete nlSingleton<FESceneManager>::s_pInstance;
        nlSingleton<FESceneManager>::s_pInstance = NULL;
    }

    nlSingleton<FEResourceManager>::s_pInstance->UnloadPermanentResourceBundle();

    if (nlSingleton<FEResourceManager>::s_pInstance != NULL)
    {
        delete nlSingleton<FEResourceManager>::s_pInstance;
        nlSingleton<FEResourceManager>::s_pInstance = NULL;
    }

    if (nlSingleton<FontManager>::s_pInstance != NULL)
    {
        delete nlSingleton<FontManager>::s_pInstance;
        nlSingleton<FontManager>::s_pInstance = NULL;
    }

    nlFree(g_pLocalization->m_pFile);
    glResourceRelease(s_FontResourceMark);

    g_pEventManager->RemoveEventHandler(m_pAIHandler);
    g_pEventManager->RemoveEventHandler(m_pGoalieHandler);
    m_pAIHandler = NULL;
    m_pGoalieHandler = NULL;

    AudioLoader::UnloadInGame();
    BeginFrameTask::s_FramerateLocked = 0;

    DestroyPowerups();
    DestroyCharacters();

    delete g_pBall;
    g_pBall = NULL;

    FakeBallWorld::Destroy();
    cCameraManager::Shutdown();
    EmissionManager::Shutdown();
    fxParticleShutdown();
    fxUnloadGroups();
    fxUnloadTemplates();

    SidelineExplodableManager::DestroyAllActiveFragments(true);
    WorldManager::DestroyWorld();

    SlotPoolBase::BaseFreeBlocks(&SFXPlaySet::m_TrackedSFXSlotPool, 0x24);
    SidelineExplodableManager::CleanUp();
    ThePhysicsLoader.DestroyPhysics();
    FrontEnd::Destroy();

    Jumbotron::instance.Uninitialize();
    CrowdManager::instance.Uninitialize();

    FreeElectricFence();
    DestroyGame();
    BlurManager::Shutdown();
    CleanBoundingBoxCache();

    nlSingleton<StatsTracker>::s_pInstance->DestroyEventHandler();
    glResourceRelease(m_GameResourceMark);

    nlSingleton<ScreenTransitionManager>::s_pInstance->CancelAllTransitions();

    gSebringLoadPackageToVirtualMemory = 0;

    SlotPoolBase::BaseFreeBlocks(&cPN_SAnimController::m_SAnimControllerSlotPool, 0x54);
    SlotPoolBase::BaseFreeBlocks(&cPN_Blender::m_BlenderSlotPool, 0x1c);
    SlotPoolBase::BaseFreeBlocks(&cPN_SingleAxisBlender::m_SingleAxisBlenderSlotPool, 0x28);
    SlotPoolBase::BaseFreeBlocks(&cPN_Feather::m_FeatherSlotPool, 0x30);

    glViewCompact();

    m_TransitionState = eTS_Unknown;
}

/**
 * Offset/Address/Size: 0x0 | 0x801715D0 | size: 0x4BC
 */
void TransitionTask::InitializeFEState()
{
}
