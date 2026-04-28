#include "Game/Render/Presentation.h"
#include "Game/FixedUpdateTask.h"
#include "Game/ParticleUpdateTask.h"
#include "Game/WorldManager.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/Audio/StreamTrack.h"
#include "NL/gl/gl.h"
#include "NL/nlFile.h"
#include "NL/nlDebug.h"
#include "NL/nlString.h"

extern AudioStreamTrack::TrackManagerBase* g_pTrackManager;

enum eGameState
{
    GS_NONE = -1,
    GS_PRE_GAME = 0,
    GS_KICKOFF = 1,
    GS_POST_GOAL = 2,
    GS_END_GAME = 3,
    GS_GAMEPLAY = 4,
    GS_OVERTIME = 5,
};

class cGame
{
public:
    char _pad[0x24];
    eGameState m_eGameState;
    void ChangeGameState(eGameState);
};

extern cGame* g_pGame;

class NisPlayer : public InterpreterCore
{
public:
    static NisPlayer* Instance();
    void SetExtraNameFilter(const char*);
    void Reset();
};

class Wiper
{
public:
    static Wiper& Instance();
    void Reset();
};

class ReplayChoreo : public InterpreterCore
{
public:
    static ReplayChoreo& Instance();
    void Reset();

    u8 _pad[0x2D0 - 0x24];
    int mNumHighlights; // offset 0x2D0, size 0x4
};

class ReplayManager
{
public:
    static ReplayManager* Instance();
    void Flush();
};

class GameInfoManager : public nlSingleton<GameInfoManager>
{
public:
    u8 _pad[0x4959];
    bool mIsInStrikers101Mode;
};

class GoalOverlay
{
public:
    void SetHighlightNumber(int);
    void DoMatchEndOverlay();
    void DoCupWinOverlay();
};

extern unsigned long cupTrophyHash;
char trophyFileName[0xFF];
static const char* idleFun = "Idle";
static bool loopPresentation;

enum Type
{
    _BOOL = 0,
    _INT = 1,
    _FLOAT = 2,
    _STRING = 3,
};

union Value
{
    const char* s;
    int i;
    bool b;
    float f;
};

struct TagValuePair
{
    const char* tag;
    Type type;
    Value value;
};

class Config
{
public:
    static Config& Global();
    bool Exists(const char*) const;
    TagValuePair& FindTvp(const char*);
    void Set(const char*, const char*);
};

template <typename To, typename From>
To LexicalCast(const From&);

/**
 * Offset/Address/Size: 0x68 | 0x80127308 | size: 0x8
 */
template <>
const char* LexicalCast<const char*, const char*>(const char* const& from)
{
    return from;
}

/**
 * Offset/Address/Size: 0x2C | 0x801272CC | size: 0x24
 */
template <>
const char* LexicalCast<const char*, int>(const int& from)
{
    nlBreak();
    return 0;
}

/**
 * Offset/Address/Size: 0x8 | 0x801272A8 | size: 0x24
 */
template <>
const char* LexicalCast<const char*, float>(const float& from)
{
    nlBreak();
    return 0;
}

/**
 * Offset/Address/Size: 0x50 | 0x801272F0 | size: 0x18
 */
template <>
const char* LexicalCast<const char*, bool>(const bool& from)
{
    return from ? "true" : "false";
}

int nlSNPrintf(char*, unsigned long, const char*, ...);

/**
 * Offset/Address/Size: 0x3DC | 0x801272A0 | size: 0x8
 */
u32 NISData::GetID()
{
    return 0x1a5;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x801267BC | size: 0xAE4
//  */
// void Presentation::DoFunctionCall(unsigned int)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80126750 | size: 0x6C
//  */
// void 0x801267BC..0x801272A0 | size: 0xAE4
// {
// }

Presentation::Presentation()
    : InterpreterCore(10)
{
    mByPassWasSkipped = false;
    mSkipPressed = false;
    mInsideByPass = false;
    mByPassing = false;
    mWaitingForCharacterDirectionSince = 0.0f;
    mTimeInFunction = 0.0f;
    mDisplayLetterBox = 0.0f;
    mLetterBoxDuration = 0.0f;
    mLetterBoxEnabled = false;
    mOverlayDelay = 0.0f;
    mOverlayDisplayLength = 0.0f;
    mOverlayDisplayed = false;
    mOverlayToDisplay = SCENE_INVALID;
    mInterruptWipe = NULL;
    mUseInterruptWipe = NULL;
    mQueuedFunction = NULL;
    mGoalQuality = HIGHLIGHT_QUALITY_EMPTY;
    unsigned long fileSize = 0;
    void* bc = nlLoadEntireFile("presentation.bc", &fileSize, 0x20, AllocateStart);
    LoadByteCode(bc);
    nlStrNCpy<char>(mCurrentFunction, idleFun, 64);
    mIsAllowedToSkip[0] = true;
    mIsAllowedToSkip[1] = true;
    mIsAllowedToSkip[2] = true;
    mIsAllowedToSkip[3] = true;
}

/**
 * Offset/Address/Size: 0x1E58 | 0x8012663C | size: 0x114
 */
Presentation& Presentation::Instance()
{
    static Presentation instance;
    return instance;
}

/**
 * Offset/Address/Size: 0x1D18 | 0x801264FC | size: 0x140
 */
void ReadTrophyTexture(void* data, unsigned long size, void* userData)
{
    Presentation& inst = Presentation::Instance();
    inst.mTrophyTextureLoaded = true;
    glEndLoadTextureBundle(data, size);
}

/**
 * Offset/Address/Size: 0x1CA8 | 0x8012648C | size: 0x70
 */
void ReadTrophyModel(void* data, unsigned long size, void* userData)
{
    unsigned long localVar = 0;
    glModel* model = glEndLoadModel(data, size, &localVar);

    int localVar2 = 0;
    WorldManager::s_World->LoadGeometry(model, localVar, true, true, &cupTrophyHash, &localVar2, true);

    DrawableObject* obj = WorldManager::s_World->FindDrawableObject(cupTrophyHash);
    obj->m_uObjectFlags &= ~1;
}

/**
 * Offset/Address/Size: 0x1848 | 0x8012602C | size: 0x460
 * TODO: 67.90% match - -inline deferred codegen diff: target has destructor flag pattern
 * (3 zero-inits + flag-checked cleanup), inline dtor after bl __as__ vs deferred cleanup,
 * extra mr r31,r3 after each LexicalCast (cupName in saved register vs target r3 pass-through)
 */
void Presentation::LoadTrophyModel()
{
    extern bool IsPossibleCupMatch__15GameInfoManagerCFv(const GameInfoManager*);
    extern int GetTrophyTypeByCurrentMode__15GameInfoManagerCFv(const GameInfoManager*);
    extern const char* GetThrophyModelName__F11eTrophyType(int);

    cupTrophyHash = 0;

    bool hasCupOverride = Config::Global().Exists("gimme_cup_trophy");
    if (!hasCupOverride)
    {
        if (!IsPossibleCupMatch__15GameInfoManagerCFv(nlSingleton<GameInfoManager>::s_pInstance))
        {
            return;
        }
    }

    cupTrophyHash = 1;

    BasicString<char, Detail::TempStringAllocator> trophyName;
    const char* cupName = NULL;
    if (hasCupOverride)
    {
        BasicString<char, Detail::TempStringAllocator> prefix("Gameplay/");
        Config& cfg = Config::Global();
        TagValuePair& tvp = cfg.FindTvp("gimme_cup_trophy");

        if (tvp.tag == NULL)
        {
            cfg.Set("gimme_cup_trophy", "FlowerCup");
            cupName = "FlowerCup";
        }
        else if (tvp.type == _BOOL)
        {
            cupName = LexicalCast<const char*, bool>(tvp.value.b);
        }
        else if (tvp.type == _INT)
        {
            cupName = LexicalCast<const char*, int>(tvp.value.i);
        }
        else if (tvp.type == _FLOAT)
        {
            cupName = LexicalCast<const char*, float>(tvp.value.f);
        }
        else if (tvp.type == _STRING)
        {
            cupName = LexicalCast<const char*, const char*>(tvp.value.s);
        }

        trophyName = prefix.Append(cupName);
    }
    else
    {
        trophyName = BasicString<char, Detail::TempStringAllocator>(
            GetThrophyModelName__F11eTrophyType(
                GetTrophyTypeByCurrentMode__15GameInfoManagerCFv(
                    nlSingleton<GameInfoManager>::s_pInstance)));
    }

    nlSNPrintf(trophyFileName, 0xFF, "%s.glg", trophyName.c_str());
    glBeginLoadModel(trophyFileName, ReadTrophyModel, NULL);
}

/**
 * Offset/Address/Size: 0x14EC | 0x80125CD0 | size: 0x35C
 */
void Presentation::Finish()
{
    if ((strcmp("PlayHighlight", mCurrentFunction) == 0 || loopPresentation) && mByPassWasSkipped == false)
    {
        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);

        if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, "PlayHighlight") != 0)
        {
            mQueuedFunction = "PlayHighlight";
        }
        else
        {
            nlStrNCpy<char>(mCurrentFunction, "PlayHighlight", 64);
            mSkipPressed = false;
            mInsideByPass = false;
            mByPassing = false;
            mInterruptWipe = 0;
            mUseInterruptWipe = 0;
            mTimeInFunction = 0.0f;

            NisPlayer::Instance()->SetExtraNameFilter("");
            CallFunction(nlStringHash("PlayHighlight"));
        }
    }
    else
    {
        if (mCurrentFunction == strstr(mCurrentFunction, "frame"))
        {
            if (g_pGame->m_eGameState != GS_END_GAME)
            {
                g_pGame->ChangeGameState(GS_KICKOFF);
            }
        }

        if (mQueuedFunction == 0)
        {
            bool duringEndOfGamePresentation = false;
            if (nlStrCmp<char>("ImplGameEnd", mCurrentFunction) == 0 || nlStrCmp<char>("GameEndNoSuddenDeath", mCurrentFunction) == 0 || nlStrCmp<char>("GoalSuddenDeath", mCurrentFunction) == 0 || nlStrCmp<char>("PlayHighlight", mCurrentFunction) == 0 || nlStrCmp<char>("PlayCupThrophy", mCurrentFunction) == 0)
            {
                duringEndOfGamePresentation = true;
            }

            if (duringEndOfGamePresentation)
            {
                g_pEventManager->CreateValidEvent(3, 0x14);
                nlTaskManager::SetNextState(1);
            }
            else
            {
                if (nlStrCmp<char>(mCurrentFunction, "GameBegin") == 0)
                {
                    g_pGame->ChangeGameState(GS_KICKOFF);
                }
                nlTaskManager::SetNextState(2);
            }
        }
    }

    FixedUpdateTask::mTimeScale = 1.0f;
    const char* functionName = idleFun;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, functionName) != 0)
    {
        mQueuedFunction = functionName;
    }
    else
    {
        nlStrNCpy<char>(mCurrentFunction, functionName, 64);
        mSkipPressed = false;
        mInsideByPass = false;
        mByPassing = false;
        mInterruptWipe = 0;
        mUseInterruptWipe = 0;
        mTimeInFunction = 0.0f;

        NisPlayer::Instance()->SetExtraNameFilter("");
        CallFunction(nlStringHash(functionName));
    }

    functionName = mQueuedFunction;
    if (functionName)
    {
        mQueuedFunction = 0;
        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);

        if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, functionName) != 0)
        {
            mQueuedFunction = functionName;
        }
        else
        {
            nlStrNCpy<char>(mCurrentFunction, functionName, 64);
            mSkipPressed = false;
            mInsideByPass = false;
            mByPassing = false;
            mInterruptWipe = 0;
            mUseInterruptWipe = 0;
            mTimeInFunction = 0.0f;

            NisPlayer::Instance()->SetExtraNameFilter("");
            CallFunction(nlStringHash(functionName));
        }
    }

    if (strcmp(mCurrentFunction, idleFun) == 0)
    {
        ReplayChoreo::Instance().Reset();
    }
}

/**
 * Offset/Address/Size: 0xE70 | 0x80125654 | size: 0x67C
 */
void Presentation::Update(float)
{
}

/**
 * Offset/Address/Size: 0xDBC | 0x801255A0 | size: 0xB4
 */
bool Presentation::DuringEndOfGamePresentation() const
{
    bool result = false;
    if (nlStrCmp<char>("ImplGameEnd", mCurrentFunction) == 0 || nlStrCmp<char>("GameEndNoSuddenDeath", mCurrentFunction) == 0 || nlStrCmp<char>("GoalSuddenDeath", mCurrentFunction) == 0 || nlStrCmp<char>("PlayHighlight", mCurrentFunction) == 0 || nlStrCmp<char>("PlayCupThrophy", mCurrentFunction) == 0)
    {
        result = true;
    }
    return result;
}

/**
 * Offset/Address/Size: 0xCF0 | 0x801254D4 | size: 0xCC
 */
void Presentation::Call(const char* functionName, const char* nisFilter)
{
    FixedUpdateTask::mTimeScale = 1.0f;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, functionName) != 0)
    {
        mQueuedFunction = functionName;
        return;
    }

    nlStrNCpy<char>(mCurrentFunction, functionName, 64);
    mSkipPressed = false;
    mInsideByPass = false;
    mByPassing = false;
    mInterruptWipe = 0;
    mUseInterruptWipe = 0;
    mTimeInFunction = 0.0f;

    NisPlayer::Instance()->SetExtraNameFilter(nisFilter);
    CallFunction(nlStringHash(functionName));
}
/**
 * Offset/Address/Size: 0x57C | 0x80124D60 | size: 0x774
 */
void Presentation::EventHandler(Event*)
{
}

/**
 * Offset/Address/Size: 0x3D0 | 0x80124BB4 | size: 0x1AC
 */
void Presentation::PlayOverlay(const char* name, float delay, float length)
{
    if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
    {
        return;
    }

    if (mOverlayDisplayed)
    {
        nlSingleton<OverlayManager>::s_pInstance->SetVisible(mOverlayToDisplay, false, false);
    }

    mOverlayDisplayed = false;
    mOverlayToDisplay = SCENE_INVALID;
    mOverlayDisplayLength = 0.0f;
    mOverlayDelay = 0.0f;

    if (nlStrCmp<char>("goal", name) == 0)
    {
        mOverlayToDisplay = OVERLAY_GOAL;
        mOverlayDelay = delay;
        mOverlayDisplayLength = length;
        mOverlayDisplayed = false;
        return;
    }

    if (nlStrCmp<char>("highlight", name) == 0)
    {
        mOverlayToDisplay = OVERLAY_GOAL;
        mOverlayDelay = delay;
        mOverlayDisplayLength = length;
        mOverlayDisplayed = false;
        GoalOverlay* scene = (GoalOverlay*)nlSingleton<OverlayManager>::s_pInstance->GetScene(mOverlayToDisplay);
        scene->SetHighlightNumber(ReplayChoreo::Instance().mNumHighlights);
        return;
    }

    if (nlStrCmp<char>("end", name) == 0)
    {
        mOverlayToDisplay = OVERLAY_GOAL;
        mOverlayDelay = delay;
        mOverlayDisplayLength = length;
        mOverlayDisplayed = false;
        GoalOverlay* scene = (GoalOverlay*)nlSingleton<OverlayManager>::s_pInstance->GetScene(mOverlayToDisplay);
        scene->DoMatchEndOverlay();
        return;
    }

    if (nlStrCmp<char>("cup", name) == 0)
    {
        mOverlayToDisplay = OVERLAY_GOAL;
        mOverlayDelay = delay;
        mOverlayDisplayLength = length;
        mOverlayDisplayed = false;
        GoalOverlay* scene = (GoalOverlay*)nlSingleton<OverlayManager>::s_pInstance->GetScene(mOverlayToDisplay);
        scene->DoCupWinOverlay();
    }
}

/**
 * Offset/Address/Size: 0x36C | 0x80124B50 | size: 0x64
 */
void Presentation::StopOverlay()
{
    if (mOverlayDisplayed)
    {
        nlSingleton<OverlayManager>::s_pInstance->SetVisible(mOverlayToDisplay, false, false);
    }
    mOverlayDisplayed = false;
    mOverlayToDisplay = SCENE_INVALID;
    mOverlayDisplayLength = 0.0f;
    mOverlayDelay = 0.0f;
}

/**
 * Offset/Address/Size: 0x19C | 0x80124980 | size: 0x1D0
 * TODO: 99.04% match - sp+0x08/0x10 stack offset swap for Function0<void> copy
 * and Function<FnVoidVoid> temporaries. MWCC internal allocation order difference.
 */
void CupWinStingerDone()
{
    AudioStreamTrack::TrackManagerBase* pMgr = g_pTrackManager;
    AudioStreamTrack::StreamTrack* pTrack = pMgr->GetTrack(nlStringLowerHash("Music"));
    {
        Function0<void> emptyCallback;
        emptyCallback.mTag = EMPTY;
        pTrack->m_IdleCallback = Function<FnVoidVoid>(emptyCallback);
    }
    pTrack->PlayStream(nlStringLowerHash("STAD_Intro"), 0.5f, true, 500, 500, "Stadium", Audio::MasterVolume::VG_Special);
}

/**
 * Offset/Address/Size: 0x60 | 0x80124844 | size: 0x13C
 */
void Presentation::Reset()
{
    mIsAllowedToSkip[0] = true;
    mIsAllowedToSkip[1] = true;
    mIsAllowedToSkip[2] = true;
    mIsAllowedToSkip[3] = true;

    FixedUpdateTask::mTimeScale = 1.0f;
    const char* functionName = idleFun;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, functionName) != 0)
    {
        mQueuedFunction = functionName;
    }
    else
    {
        nlStrNCpy<char>(mCurrentFunction, functionName, 64);
        mSkipPressed = false;
        mInsideByPass = false;
        mByPassing = false;
        mInterruptWipe = 0;
        mUseInterruptWipe = 0;
        mTimeInFunction = 0.0f;

        NisPlayer::Instance()->SetExtraNameFilter("");
        CallFunction(nlStringHash(functionName));
    }

    mQueuedFunction = 0;
    mOverlayDisplayed = false;
    if (mOverlayDisplayed)
    {
        nlSingleton<OverlayManager>::s_pInstance->SetVisible(mOverlayToDisplay, false, false);
    }
    mOverlayDisplayed = false;
    mOverlayToDisplay = SCENE_INVALID;
    mOverlayDisplayLength = 0.0f;
    mOverlayDelay = 0.0f;

    Wiper::Instance().Reset();
    NisPlayer::Instance()->Reset();
    ReplayChoreo::Instance().Reset();
    ReplayManager::Instance()->Flush();
}
