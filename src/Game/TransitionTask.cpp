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
#include "Game/Render/GraphicsLoader.h"
#include "Game/NisPlayer.h"
#include "Game/ReplayChoreo.h"
#include "Game/ReplayManager.h"
#include "Game/Render/Jumbotron.h"
#include "Game/Render/CrowdManager.h"
#include "Game/Render/NPCLoader.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim/pnBlender.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "Game/SAnim/pnFeather.h"
#include "Game/Render/SidelineExplodable.h"
#include "Game/BeginFrameTask.h"
#include "Game/WorldManager.h"
#include "Game/ObjectBlur.h"
#include "Game/Camera/CameraMan.h"
#include "Game/CameraLoader.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/AI/AILoader.h"
#include "Game/Loader/LoadingManager.h"
#include "Game/World/WorldLoader.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/AudioScriptEventMgr.h"
#include "Game/Audio/AudioStream.h"
#include "Game/Audio/CrowdMood.h"
#include "Game/Audio/StreamTrack.h"
#include "Game/CharacterTemplate.h"
#include "Game/Sys/PlatStream.h"
#include "Game/Debug/TimeRegions.h"
#include "Game/Render/ElectricFence.h"
#include "Game/Render/Presentation.h"
#include "Game/Game.h"
#include "Game/Goalie.h"
#include "Game/Drawable/DrawableModel.h"
#include "Game/SH/SHLoading.h"
#include "Game/SH/SHPause.h"
#include "Game/FE/feScene.h"
#include "Game/DB/UserOptions.h"
#include "NL/nlMemory.h"
#include "NL/MemAlloc.h"
#include "NL/nlConfig.h"
#include "NL/platpad.h"
#include "NL/gl/gl.h"
#include "NL/gl/glView.h"
#include "NL/glx/glxSwap.h"
#include "NL/plat/plataudio.h"
#include "NL/gl/glMemory.h"
#include "NL/nlLocalization.h"
#include "Game/GameSceneManager.h"
#include "Game/FE/feMusic.h"
#include "Game/FE/feAnimModelManager.h"
#include "Game/FE/FELoader.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/SH/SHSaveLoad.h"
#include "Game/SH/SHMainMenu.h"
#include "Game/SH/SHCupHub.h"
#include "Game/Sys/debug.h"
#include "dolphin/os.h"

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
static unsigned long long s_FontResourceMark = -1;
extern unsigned char gSebringLoadPackageToVirtualMemory;
extern PhysicsLoader ThePhysicsLoader;

void glx_SetFog(int);
bool fxParticleShutdown();
bool fxUnloadGroups();
bool fxUnloadTemplates();
void glResourceRelease(unsigned long long);

extern nlLocalization::nlLanguage g_Language;
extern bool g_e3_Build;

void PrintAvailableARAMMemory();
void InitializeGameObjectLighting();
void AIEventHandler(Event*, void*);
void GoalieEventHandler(Event*, void*);

extern GraphicsLoader TheGraphicsLoader;
extern FELoader TheFELoader;
extern AudioLoader TheAudioLoader;
extern NPCLoader TheNPCLoader;
extern AILoader TheAILoader;
extern CameraLoader TheCameraLoader;

namespace Detail
{
class SwitchToStartScreenLoader : public Loader
{
public:
    virtual bool StartLoad(LoadingManager*);
    virtual bool Update() { return false; }
    virtual const char* GetName() { return "SwitchToStartScreenLoader"; }
};
static SwitchToStartScreenLoader switchToStartScreen;
} // namespace Detail

// /**
//  * Offset/Address/Size: 0x0 | 0x801731FC | size: 0x10
//  */
// void 0x8028D338..0x8028D33C | size : 0x4
// {
// }

/**
 * Offset/Address/Size: 0xC | 0x801731F8 | size: 0x4
 */
// void TransitionTask::Run(float)
// {
// }

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
    const char* fontFileName1 = "fot-rodinprob18";
    const char* fontFileName2 = "cepoitalic24";
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

    nlSNPrintf(bundlePath1, 64, "art/fe/fonts/%sfonttext18.res", langCode);
    nlSNPrintf(fontName1, 64, "fe/fonts/%sfonttext18", langCode);
    nlSNPrintf(bundlePath2, 64, "art/fe/fonts/%sfontheading24.res", langCode);
    nlSNPrintf(fontName2, 64, "fot-rodinprob18", langCode);

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
        nlSingleton<FEResourceManager>::s_pInstance->Run(1.0f);
        nlSingleton<FESceneManager>::s_pInstance->Update(1.0f);
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
 * TODO: 96.85% match - "---\n" string uses sda21 addressing in scratch vs ha/lo in target (linker section placement)
 */
void TransitionTask::InitializeFEState()
{
    m_TransitionState = eTS_Initializing;

    tDebugPrintManager::Print(DC_MEMORY, "--- InitializeFEState ---\n");
    tDebugPrintManager::Print(DC_MEMORY, "Total free memory: %d\n", StandardAllocator.TotalFreeMemory());
    tDebugPrintManager::Print(DC_MEMORY, "Largest free block: %d\n", StandardAllocator.LargestFreeBlock());
    tDebugPrintManager::Print(DC_MEMORY, "---\n");

    s_FontResourceMark = glResourceMark();
    ((nlLocalization*)g_pLocalization)->Load(g_Language, false);
    EnableAutoPressed();

    if (nlSingleton<FontManager>::s_pInstance == NULL)
    {
        nlSingleton<FontManager>::s_pInstance = new (nlMalloc(sizeof(FontManager), 8, false)) FontManager();
    }

    LoadFonts();

    if (nlSingleton<FEResourceManager>::s_pInstance == NULL)
    {
        nlSingleton<FEResourceManager>::s_pInstance = new (nlMalloc(sizeof(FEResourceManager), 8, false)) FEResourceManager();
    }

    nlSingleton<FEResourceManager>::s_pInstance->Initialize();
    nlSingleton<FEResourceManager>::s_pInstance->LoadPermanentResourceBundle("art/fe/fepermanent.res");

    if (nlSingleton<FESceneManager>::s_pInstance == NULL)
    {
        nlSingleton<FESceneManager>::s_pInstance = new (nlMalloc(sizeof(FESceneManager), 8, false)) FESceneManager();
    }

    nlSingleton<FESceneManager>::s_pInstance->m_uDefaultRenderView = 31;

    nlSingleton<GameInfoManager>::s_pInstance->OnPostGameState();

    if (nlSingleton<GameSceneManager>::s_pInstance == NULL)
    {
        nlSingleton<GameSceneManager>::s_pInstance = new (nlMalloc(sizeof(GameSceneManager), 8, false)) GameSceneManager();
    }

    if (nlSingleton<FEAnimModelManager>::s_pInstance == NULL)
    {
        nlSingleton<FEAnimModelManager>::s_pInstance = new (nlMalloc(sizeof(FEAnimModelManager), 8, false)) FEAnimModelManager();
    }

    nlSingleton<FEAnimModelManager>::s_pInstance->Initialize();

    FEMusic::ResetCurrentFEStreamHash();

    static bool gAlreadyBooted = false;

    if (!gAlreadyBooted)
    {
        gAlreadyBooted = true;
        if (SaveLoadScene::IsIOEnabled())
        {
            SaveLoadScene* scene = (SaveLoadScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_SHOULD_LOAD_OR_SAVE, (ScreenMovement)0, false);
            scene->mNextScene = SCENE_POPUP_MENU;
        }
        else
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, (ScreenMovement)0, false);
        }
    }
    else
    {
        AudioLoader::LoadFE(true);

        if (g_e3_Build)
        {
            for (int i = 0; i < 4; i++)
            {
                nlSingleton<GameInfoManager>::s_pInstance->SetPlayingSide(i, -1);
            }
        }

        if (nlSingleton<GameInfoManager>::s_pInstance->IsInDemoMode())
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_TITLE, (ScreenMovement)0, false);
            nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_DEMO);
        }
        else if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupOrTournamentMode())
        {
            GameInfoManager* pGameInfo = nlSingleton<GameInfoManager>::s_pInstance;
            if (pGameInfo->IsInTournamentMode())
            {
                pGameInfo->IncreaseGameNumber(true);
                while (pGameInfo->GetCurrentRoundNumber() != -5)
                {
                    if (pGameInfo->DetermineNextMatchups(0x1b))
                        break;
                    pGameInfo->IncreaseRoundNumber();
                }

                CupHubScene* scene = (CupHubScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_TOURNAMENT_STANDINGS, (ScreenMovement)0, false);
                scene->mDoAutoSave = true;
                FEMusic::StartStreamIfDifferent(4);
            }
            else
            {
                pGameInfo->OnPostCupGameState();
                if (nlSingleton<GameInfoManager>::s_pInstance->IsInRegularCupMode())
                {
                    FEMusic::StartStreamIfDifferent(2);
                }
                else if (nlSingleton<GameInfoManager>::s_pInstance->IsInSuperCupMode())
                {
                    FEMusic::StartStreamIfDifferent(3);
                }
            }
        }
        else
        {
            if (nlSingleton<GameInfoManager>::s_pInstance->mGoToChooseCaptains)
            {
                nlSingleton<GameInfoManager>::s_pInstance->mGoToChooseCaptains = false;
                nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_FRIENDLY);
                nlSingleton<GameInfoManager>::s_pInstance->ResetPlayingSides();
                nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CHOOSE_CAPTAINS, (ScreenMovement)0, false);
            }
            else
            {
                nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, (ScreenMovement)0, false);
                SHMainMenu::mSnapMenuIntoPosition = true;
                SHMainMenu::mLastMenuItem = 0;
            }
            FEMusic::StartStreamIfDifferent(0);
        }
    }

    tDebugPrintManager::Print(DC_MEMORY, "-- Memory upon Exiting InitializeFEState \n");
    tDebugPrintManager::Print(DC_MEMORY, "Free Memory: %u\n", StandardAllocator.TotalFreeMemory());
    tDebugPrintManager::Print(DC_MEMORY, "Largest Free Block: %u\n", StandardAllocator.LargestFreeBlock());
    tDebugPrintManager::Print(DC_MEMORY, "-----------------------------------------\n\n");

    m_TransitionState = eTS_InState;
}
