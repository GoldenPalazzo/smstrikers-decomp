#include "types.h"
#include "Game/main.h"
#include "NL/gl/glView.h"
#include "NL/gl/glTexture.h"
#include "NL/gl/glState.h"
#include "NL/gl/gl.h"
#include "NL/gl/glAppAttach.h"
#include "NL/gl/glMemory.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Transitions/ModelTransition.h"
#include "NL/nlConfig.h"
#include "NL/nlMain.h"
#include "NL/nlFunction.h"
#include "NL/nlLocalization.h"
#include "NL/nlString.h"
#include "NL/platpad.h"
#include "Game/ProfileTask.h"
#include "Game/Sys/FloatingPointExceptions.h"
#include "Game/Sys/CallStackDumper.h"
#include "Game/Sys/eventman.h"
#include "Game/Sys/gcmemcard.h"
#include "Game/Sys/debug.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/CrowdMood.h"
#include "Game/FE/LidOpenMessage.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feManager.h"
#include "Game/ResetTask.h"
#include "Game/BeginFrameTask.h"
#include "Game/DispatchEventsTask.h"
#include "Game/PlatPadUpdateTask.h"
#include "Game/FixedUpdateTask.h"
#include "Game/WorldUpdateTask.h"
#include "Game/GameRenderTask.h"
#include "Game/FrontEndTask.h"
#include "Game/ParticleUpdateTask.h"
#include "Game/TweakerTask.h"
#include "Game/EndFrameTask.h"
#include "Game/TransitionTask.h"
#include "Game/ComUpdateTask.h"
#include "Game/TestTask.h"
#include "Game/Loader/LoadingManager.h"
#include "Game/GL/GLInventory.h"
#include "Game/Debug/ShapeRender.h"
#include "Game/GameInfo.h"
#include "Game/DB/StatsTracker.h"
#include "Game/DB/SaveLoad.h"
#include "Game/Render/Wiper.h"
#include "Game/Render/depthoffield.h"
#include "Game/Render/FlareHandler.h"
#include "Game/AI/AIPad.h"
#include "Game/OverlayManager.h"
#include "Game/PadActions.h"
#include "Game/Pad/FlickDetection.h"
#include "Game/Replay.h"
#include "Game/ReplayManager.h"
#include "Game/ReplayChoreo.h"
#include "Game/NisPlayer.h"
#include "Game/Render/Presentation.h"
#include "Game/Render/CrowdManager.h"
#include "NL/nlDebug.h"
#include "dolphin/os/OSThread.h"
#include "dolphin/si.h"
#include "dolphin/card.h"

extern u8 g_DoStackWatermarkTests;
extern u8 g_StackWatermarkFiller;
extern u8 g_bFrameSmiler;
extern u8 g_bFrameStatsOnScreen;
extern u8 g_bFrameStatsOnDisk;
extern bool g_bRunSimAndRenderInLockStep;
extern ResetTask resetTask;
extern BeginFrameTask beginFrameTask;
extern DispatchEventsTask dispatchEventsTask;
extern PlatPadUpdateTask platPadUpdateTask;
extern FixedUpdateTask fixedUpdateTask;
extern WorldUpdateTask worldUpdateTask;
extern GameRenderTask gameRenderTask;
extern FrontEndTask frontEndTask;
extern ParticleUpdateTask particleUpdateTask;
extern TweakerTask tweakerTask;
extern EndFrameTask endFrameTask;
extern TransitionTask transitionTask;
extern ComUpdateTask comUpdateTask;
extern TestTask testTask;
extern GLInventory glInventory;
extern ShapeRender g_ShapeRenderer;

namespace Detail
{
template <typename R, typename F>
struct MemFunImpl
{
    F mFuncPtr;
};
} // namespace Detail

static bool g_bProfiling = false;
static bool g_bTweaking = false;
bool g_e3_Build = false;
bool g_Europe = false;
static bool g_bFranticPausing = false;
int g_Language = 0;
static void* g_pTheLoadingManagerTask = nullptr;

ClockUpdateTask clockUpdateTask;
AudioUpdateTask audioUpdateTask;

static void Initialize();

int main(void)
{
    if (g_DoStackWatermarkTests)
    {
        OSClearStack(g_StackWatermarkFiller);
    }

    Initialize();

    fopen("flushfile.txt", "r");

    Config& config = Config::Global();
    bool skipfe = GetConfigBool(config, "skipfe", false);
    nlTaskManager::SetNextState(skipfe ? 2 : 4);

    Config& config2 = Config::Global();
    bool enableFPE = GetConfigBool(config2, "enableFloatingPointExceptions", false);
    if (enableFPE)
    {
        InstallFloatingPointExceptionHandler();
    }

    Config& config3 = Config::Global();
    bool enableCSD = GetConfigBool(config3, "callStackDumper", false);
    if (enableCSD)
    {
        InstallCallStackDumper();
    }

    for (;;)
    {
        nlTaskManager::RunAllTasks();
        UpdateProfile();
    }
}

static void SetupViews()
{
    static eGLView sort_none[] = {
        GLV_Shadow0, GLV_Shadow1, GLV_UnsortedPerspective, GLV_InvisiblePlane, GLV_ElectricFence, GLV_UnsortedOrtho, GLV_ShadowBlend0, GLV_ShadowBlend1, GLV_Debug, GLV_Transitions, GLV_CoPlanar0, GLV_CoPlanar
    };

    static eGLView disabled_views[] = {
        GLV_ShadowBlend0, GLV_ShadowBlend1, GLV_ScreenBlur, GLV_ScreenBlur2
    };

    s32 i;

    for (i = 0; i < GLV_Num; i++)
    {
        glViewSetTarget((eGLView)i, GLTG_Main);
    }

    glViewSetSortMode(GLV_FrontEnd, GLVSort_TransformedDepth);
    glViewSetSortMode(GLV_Anark, GLVSort_Reverse);

    {
        u32 j;
        for (j = 0; j < 12; j++)
        {
            glViewSetSortMode(sort_none[j], GLVSort_None);
        }
    }

    {
        u32 j;
        for (j = 0; j < 4; j++)
        {
            glViewSetEnable(disabled_views[j], false);
        }
    }

    if (!glTextureLoad(glGetTexture("target/warble")))
    {
        glViewSetEnable(GLV_Warble, false);
        glViewSetEnable(GLV_WarbleBlend, false);
    }

    glViewSetDepthClear(GLV_CameraSpace, true);
    glViewSetDepthClear(GLV_Transitions, true);
    glViewSetDepthClear(GLV_Transitions3D, true);
    glViewSetDepthClear(GLV_Anark3D_BG, true);
    glViewSetDepthClear(GLV_Anark3D_FG, true);

    ParticleSystem::ClearViews();
    ParticleSystem::AddView(GLV_Particles);

    ModeledScreenTransition::s_3DView = GLV_Transitions3D;
}

extern void nlRegHandleDVDMessageCB(Function<void(int)>&);
extern void nlRegHandleDVDAllClearCB(Function<void(int)>&);
extern void nlRegCheckForResetFromFSCB(Function<FnVoidVoid>&);
extern void FEEventHandler__14OverlayManagerFP5EventPv(Event*, void*);
extern void FEAudioEventHandler(Event*, void*);
extern void AudioEventHandler__5AudioFP5EventPv(Event*, void*);
extern bool g_BGM_Off__11AudioLoader;
extern void EventHandler__13ReplayManagerFP5EventPv(Event*, void*);
extern void EventHandler__12ReplayChoreoFP5EventPv(Event*, void*);
extern void EventHandler__9NisPlayerFP5EventPv(Event*, void*);
extern void EventHandler__12PresentationFP5EventPv(Event*, void*);
extern void EventHandler__12CrowdManagerFP5EventPv(Event*, void*);

/**
 * Offset/Address/Size: 0x3DC | 0x80173864 | size: 0x1858
 * TODO: 88.1% match - nlStrICmp inlined in target but called as function;
 *       skinning config section generates extra code; register alloc diffs
 */
static void Initialize()
{
    {
        Function<void(int)> dvdMsgCB;
        dvdMsgCB.mTag = FREE_FUNCTION;
        dvdMsgCB.mFreeFunction = DisplayDVDMessageSebring;
        nlRegHandleDVDMessageCB(dvdMsgCB);
    }
    {
        Function<void(int)> dvdClrCB;
        dvdClrCB.mTag = FREE_FUNCTION;
        dvdClrCB.mFreeFunction = DVDAllClearSebring;
        nlRegHandleDVDAllClearCB(dvdClrCB);
    }
    {
        Function<FnVoidVoid> resetCB(Bind<void>(MemFun<ResetTask, void>(&ResetTask::FSCheckForReset), &resetTask));
        nlRegCheckForResetFromFSCB(resetCB);
    }

    nlInit();
    if (!glStartup())
    {
        nlBreak();
    }
    nlSetRandomSeed(OSGetTick(), &nlDefaultSeed);
    if (!glLoadTextureBundle("global.glt"))
    {
        nlBreak();
    }

    Config::Global().LoadFromFile("common.ini");
    Config::Global().LoadFromFile("platform.ini");
    Config::Global().LoadFromFile("locale.ini");
    Config::Global().LoadFromFile("user.ini");

    GetConfigBool(Config::Global(), "DiskAccess", false);
    if (GetConfigBool(Config::Global(), "e3_build", false))
    {
        g_e3_Build = true;
    }
    if (GetConfigBool(Config::Global(), "frantic_pausing", false))
    {
        g_bFranticPausing = true;
    }

    g_bFrameSmiler = GetConfigBool(Config::Global(), "Frame_Smiler", false);
    g_bFrameStatsOnScreen = GetConfigBool(Config::Global(), "Frame_Stats_On_Screen", false);
    g_bFrameStatsOnDisk = GetConfigBool(Config::Global(), "Frame_Stats_On_Disk", false);
    g_bRunSimAndRenderInLockStep = GetConfigBool(Config::Global(), "lockstep", false);

    EventManager::Create(128, 64);
    g_pEventManager->AddEventHandler(FrontEnd::FEEventHandler, NULL, 2);
    g_pEventManager->AddEventHandler(FEEventHandler__14OverlayManagerFP5EventPv, NULL, (u32)-1);
    g_pEventManager->AddEventHandler(FEAudioEventHandler, NULL, 2);
    g_pEventManager->AddEventHandler(AudioEventHandler__5AudioFP5EventPv, NULL, 0x16);

    nlTaskManager::Startup(0x10000);
    tDebugPrintManager::Initialize();
    ClockManager::Initialize();

    AudioLoader::gbDisableAudio = GetConfigBool(Config::Global(), "no_audio", false);
    AudioLoader::gbStream = !GetConfigBool(Config::Global(), "no_stream", false);
    g_BGM_Off__11AudioLoader = GetConfigBool(Config::Global(), "no_bgm", false);
    AudioLoader::gbDisableCrowd = GetConfigBool(Config::Global(), "no_crowd", false);
    AudioLoader::gbDisableReverb = GetConfigBool(Config::Global(), "no_reverb", false);

    CrowdMood::ReadConfig();

    if (!AudioLoader::gbDisableAudio)
    {
        AudioLoader::Initialize();
        AudioLoader::SetupSoundBuffers();
        AudioLoader::LoadFEButtonSoundGroup();
        Audio::InitStreaming();
    }

    g_pEventManager->AddEventHandler(EventHandler__13ReplayManagerFP5EventPv, ReplayManager::Instance(), (u32)-1);
    g_pEventManager->AddEventHandler(EventHandler__12ReplayChoreoFP5EventPv, &ReplayChoreo::Instance(), (u32)-1);
    g_pEventManager->AddEventHandler(EventHandler__9NisPlayerFP5EventPv, NisPlayer::Instance(), (u32)-1);
    g_pEventManager->AddEventHandler(EventHandler__12PresentationFP5EventPv, &Presentation::Instance(), (u32)-1);
    g_pEventManager->AddEventHandler(EventHandler__12CrowdManagerFP5EventPv, &CrowdManager::instance, (u32)-1);

    glResourceMark();
    glInventory.Create();
    InitPads();
    SISetSamplingRate(0);
    PADSetSamplingCallback(VBlankPadUpdate);
    AIPadManager::Startup();
    FEInput::Initialize();
    FlickDetection::Initialize();
    CARDInit();
    MemCard::s_InitDone = true;

    g_pTheLoadingManagerTask = new (nlMalloc(sizeof(LoadingManager), 8, false)) LoadingManager(0x14);

    if (!GetConfigBool(Config::Global(), "DisableComListener", false))
    {
        comUpdateTask.Initialize();
    }

    TransitionTask::sm_pGlobalTask = &transitionTask;
    transitionTask.Initialize(*(LoadingManager*)g_pTheLoadingManagerTask);

    testTask.Initialize();
    nlLocalization::Initialize();

    const char* diskId = (const char*)DVDGetCurrentDiskID();
    if (diskId[0] == 'G' && diskId[1] == '4' && diskId[2] == 'Q' && diskId[3] == 'P')
    {
        u8 lang = (u8)OSGetLanguage();
        switch (lang)
        {
        case 1:
            g_Language = 2;
            break;
        case 2:
            g_Language = 1;
            break;
        case 3:
            g_Language = 3;
            break;
        case 4:
            g_Language = 4;
            break;
        default:
            g_Language = 6;
            break;
        }
        g_Europe = true;
    }
    else if ((s8)diskId[0] == 'G' && diskId[1] == '4' && diskId[2] == 'Q' && diskId[3] == 'J')
    {
        g_Language = 5;
    }
    else if ((s8)diskId[0] == 'G' && diskId[1] == '4' && diskId[2] == 'Q' && diskId[3] == 'E')
    {
        g_Language = 0;
    }
    else
    {
        typedef BasicString<char, Detail::TempStringAllocator> TempString;
        TempString langStr = Config::Global().Get<TempString>("Language", TempString("eng"));
        const char* langCStr = langStr.c_str();

        if (nlStrICmp(langCStr, "eng") == 0) { g_Language = 0; }
        else if (nlStrICmp(langCStr, "jpn") == 0) { g_Language = 5; }
        else if (nlStrICmp(langCStr, "deu") == 0) { g_Language = 2; }
        else if (nlStrICmp(langCStr, "fre") == 0) { g_Language = 1; }
        else if (nlStrICmp(langCStr, "ita") == 0) { g_Language = 4; }
        else if (nlStrICmp(langCStr, "spa") == 0) { g_Language = 3; }
        else if (nlStrICmp(langCStr, "uke") == 0) { g_Language = 6; }
        else if (nlStrICmp(langCStr, "longest") == 0) { g_Language = 7; }
    }

    LoadMemoryCardIconData();

    if (nlSingleton<GameInfoManager>::s_pInstance == NULL)
    {
        nlSingleton<GameInfoManager>::s_pInstance = new (nlMalloc(sizeof(GameInfoManager), 8, false)) GameInfoManager();
    }
    if (nlSingleton<StatsTracker>::s_pInstance == NULL)
    {
        nlSingleton<StatsTracker>::s_pInstance = new (nlMalloc(sizeof(StatsTracker), 8, false)) StatsTracker();
    }

    nlTaskManager::AddTask(&resetTask, 0, -1);
    nlTaskManager::AddTask(&beginFrameTask, 2, -1);
    nlTaskManager::AddTask(&dispatchEventsTask, 0x14, -1);
    nlTaskManager::AddTask(&platPadUpdateTask, 3, -1);
    nlTaskManager::AddTask(&clockUpdateTask, 5, -1);
    nlTaskManager::AddTask(&fixedUpdateTask, 7, -1);
    nlTaskManager::AddTask(&worldUpdateTask, 8, 0x20013);
    nlTaskManager::AddTask(&gameRenderTask, 0xa, 0x20113);
    nlTaskManager::AddTask(&frontEndTask, 0xc, 0x117);
    nlTaskManager::AddTask(&particleUpdateTask, 0xb, 0x20113);
    nlTaskManager::AddTask(&tweakerTask, 0xd, -1);
    nlTaskManager::AddTask(&audioUpdateTask, 0xe, -1);
    nlTaskManager::AddTask(&endFrameTask, 0xf, -1);
    nlTaskManager::AddTask(&transitionTask, 1, -1);
    nlTaskManager::AddTask((nlTask*)g_pTheLoadingManagerTask, 6, -1);

    if (!GetConfigBool(Config::Global(), "DisableComListener", false))
    {
        nlTaskManager::AddTask(&comUpdateTask, 0x10, -1);
    }
    if (GetConfigBool(Config::Global(), "test/enable", false))
    {
        nlTaskManager::AddTask(&testTask, 0x12, -1);
    }

    SetupViews();
    g_ShapeRenderer.Initialize();
    Wiper::Instance().Initialize();
    DepthOfFieldManager::instance.Initialize();
    FlareHandler::instance.Initialize();
    glAppStartup();

    {
        typedef BasicString<char, Detail::TempStringAllocator> TempString;
        extern s32 s_GameplaySkin__14BeginFrameTask;
        extern s32 s_ReplaySkin__14BeginFrameTask;
        TempString skinStr = Config::Global().Get<TempString>("Skinning", TempString("both"));
        TempString replaySkinStr = Config::Global().Get<TempString>("UserReplaySkinning", TempString("blend"));
        if (skinStr == "both") { s_GameplaySkin__14BeginFrameTask = 2; }
        else if (skinStr == "rigid") { s_GameplaySkin__14BeginFrameTask = 0; }
        else { s_GameplaySkin__14BeginFrameTask = 1; }
        if (replaySkinStr == "both") { s_ReplaySkin__14BeginFrameTask = 2; }
        else if (replaySkinStr == "rigid") { s_ReplaySkin__14BeginFrameTask = 0; }
        else { s_ReplaySkin__14BeginFrameTask = 1; }
    }
}

int GetRegion()
{
    static const int g_Region = 1;
    return (int)&g_Region;
}
