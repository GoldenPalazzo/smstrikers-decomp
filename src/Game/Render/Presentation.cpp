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

class ShapeRender
{
public:
    void DrawRectangle2D(float x, float y, float w, float h, float z, const nlColour& colour, int view) const;
};

extern ShapeRender g_ShapeRenderer;

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

class GameInfoWidescreenProbe
{
public:
    u8 _pad[0x2F00];
    bool mIsWidescreen;
};

/**
 * Offset/Address/Size: 0xE70 | 0x80125654 | size: 0x67C
 */
void Presentation::Update(float deltaT)
{
    extern bool m_bGameOver__8FrontEnd;
    extern bool IsInDemoMode__15GameInfoManagerCFv(const GameInfoManager*);
    extern void Set__6ConfigFPCcb(Config*, const char*, bool);
    extern void Update__12ReplayChoreoFf(ReplayChoreo*, float);
    extern void SaveHighlight__12ReplayChoreoFQ212ReplayChoreo16HighlightQuality(ReplayChoreo*, int);
    extern void UpdateTweakMode__12ReplayCameraFv();
    extern void Render__5WiperFf(Wiper*, float);
    extern void* GetPad__11cPadManagerFi(int);
    extern bool JustPressed__10cGlobalPadFib(void*, int, bool);

    mTimeInFunction += deltaT;

    if (mDisplayLetterBox > 0.0f)
    {
        mDisplayLetterBox -= deltaT;
        if (mDisplayLetterBox <= 0.0f)
        {
            SaveHighlight__12ReplayChoreoFQ212ReplayChoreo16HighlightQuality(&ReplayChoreo::Instance(), 1);
            mDisplayLetterBox = 0.0f;
        }
    }

    bool isPaused = false;
    if (!m_bGameOver__8FrontEnd && nlTaskManager::m_pInstance->m_CurrState == 1)
    {
        isPaused = true;
    }

    if (!isPaused)
    {
        if (nlStrCmp<char>(mCurrentFunction, "GameBegin") == 0)
        {
            if (nlTaskManager::m_pInstance->m_CurrState != 0x100)
            {
                glDiscardFrame(1);
            }
        }

        Run();
        Update__12ReplayChoreoFf(&ReplayChoreo::Instance(), deltaT);
        UpdateTweakMode__12ReplayCameraFv();

        if (!mSkipPressed)
        {
            bool pressedSkip;
            if (IsInDemoMode__15GameInfoManagerCFv(nlSingleton<GameInfoManager>::s_pInstance))
            {
                pressedSkip = false;
            }
            else
            {
                Config& cfg = Config::Global();
                TagValuePair& tvp = cfg.FindTvp("no_presentation_skip");
                bool noPresentationSkip;

                if (tvp.tag == 0)
                {
                    Set__6ConfigFPCcb(&cfg, "no_presentation_skip", false);
                    noPresentationSkip = false;
                }
                else if (tvp.type == _BOOL)
                {
                    noPresentationSkip = LexicalCast<bool, bool>(tvp.value.b);
                }
                else if (tvp.type == _INT)
                {
                    noPresentationSkip = LexicalCast<bool, int>(tvp.value.i);
                }
                else if (tvp.type == _FLOAT)
                {
                    noPresentationSkip = LexicalCast<bool, float>(tvp.value.f);
                }
                else if (tvp.type == _STRING)
                {
                    noPresentationSkip = LexicalCast<bool, const char*>(tvp.value.s);
                }
                else
                {
                    noPresentationSkip = false;
                }

                if (noPresentationSkip)
                {
                    bool trophyShown;
                    if (cupTrophyHash != 0)
                    {
                        trophyShown = (WorldManager::s_World->FindDrawableObject(cupTrophyHash)->m_uObjectFlags & 1) != 0;
                    }
                    else
                    {
                        trophyShown = false;
                    }

                    if (nlStrCmp<char>(mCurrentFunction, "PlayHighlight") != 0 && !trophyShown)
                    {
                        pressedSkip = false;
                        mSkipPressed = pressedSkip;
                        goto done_skip_detect;
                    }
                }

                bool duringEndPresentation = false;
                if (nlStrCmp<char>("ImplGameEnd", mCurrentFunction) == 0
                    || nlStrCmp<char>("GameEndNoSuddenDeath", mCurrentFunction) == 0
                    || nlStrCmp<char>("GoalSuddenDeath", mCurrentFunction) == 0
                    || nlStrCmp<char>("PlayHighlight", mCurrentFunction) == 0
                    || nlStrCmp<char>("PlayCupThrophy", mCurrentFunction) == 0)
                {
                    duringEndPresentation = true;
                }

                if (duringEndPresentation && mTimeInFunction >= 1.2f)
                {
                    pressedSkip = false;
                }
                else
                {
                    if (nlTaskManager::m_pInstance->m_CurrState == 0x100 || nlTaskManager::m_pInstance->m_CurrState == 0x10)
                    {
                        pressedSkip = false;
                        for (int i = 0; i < 4; i++)
                        {
                            if (!mIsAllowedToSkip[i])
                            {
                                if (nlStrCmp<char>(mCurrentFunction, "PlayHighlight") != 0)
                                {
                                    continue;
                                }
                            }

                            void* pad = GetPad__11cPadManagerFi(i);
                            if (pad != 0)
                            {
                                if (JustPressed__10cGlobalPadFib(GetPad__11cPadManagerFi(i), 7, true))
                                {
                                    pressedSkip = true;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        pressedSkip = false;
                    }
                }
            }

            mSkipPressed = pressedSkip;
        }

    done_skip_detect:
        if (mSkipPressed && mInsideByPass)
        {
            mByPassing = true;
            mSkipPressed = false;
            g_pEventManager->CreateValidEvent(0x1C, 0x14);
            mUseInterruptWipe = mInterruptWipe;
            mInterruptWipe = 0;

            bool duringEndPresentation = false;
            if (nlStrCmp<char>("ImplGameEnd", mCurrentFunction) == 0
                || nlStrCmp<char>("GameEndNoSuddenDeath", mCurrentFunction) == 0
                || nlStrCmp<char>("GoalSuddenDeath", mCurrentFunction) == 0
                || nlStrCmp<char>("PlayHighlight", mCurrentFunction) == 0
                || nlStrCmp<char>("PlayCupThrophy", mCurrentFunction) == 0)
            {
                duringEndPresentation = true;
            }

            if (duringEndPresentation)
            {
                mTimeInFunction = 0.0f;
            }
        }

        if (IsFinished())
        {
            Finish();
        }
    }

    float renderDelta;
    if (!m_bGameOver__8FrontEnd && nlTaskManager::m_pInstance->m_CurrState == 1)
    {
        renderDelta = 0.0f;
    }
    else
    {
        renderDelta = deltaT;
    }

    Render__5WiperFf(&Wiper::Instance(), renderDelta);

    if (mLetterBoxEnabled)
    {
        mLetterBoxDuration += 0.05f;
    }
    else
    {
        mLetterBoxDuration -= 0.05f;
    }

    if (mLetterBoxDuration < 0.0f)
    {
        mLetterBoxDuration = 0.0f;
    }

    if (mLetterBoxDuration > 1.0f)
    {
        mLetterBoxDuration = 1.0f;
    }

    if (!((GameInfoWidescreenProbe*)nlSingleton<GameInfoManager>::s_pInstance)->mIsWidescreen)
    {
        if (mLetterBoxDuration > 0.0f)
        {
            nlColour black = { { 0x00, 0x00, 0x00, 0xFF } };
            g_ShapeRenderer.DrawRectangle2D(0.0f, 0.0f, glGetOrthographicWidth(), 38.0f * mLetterBoxDuration, 1.0f, black, GLV_Transitions);
            g_ShapeRenderer.DrawRectangle2D(0.0f, glGetOrthographicHeight() - 38.0f * mLetterBoxDuration, glGetOrthographicWidth(), 38.0f * mLetterBoxDuration, 1.0f, black, GLV_Transitions);
        }
    }

    if (!m_bGameOver__8FrontEnd && nlTaskManager::m_pInstance->m_CurrState == 1)
    {
        return;
    }

    if (mOverlayToDisplay == SCENE_INVALID)
    {
        return;
    }

    if (!mOverlayDisplayed)
    {
        mOverlayDelay -= deltaT;
        if (mOverlayDelay <= 0.0)
        {
            nlSingleton<OverlayManager>::s_pInstance->SetVisible(mOverlayToDisplay, true, false);
            if (mOverlayToDisplay == OVERLAY_GOAL)
            {
                nlSingleton<OverlayManager>::s_pInstance->RestartGoalOverlay();
            }
            mOverlayDelay = 0.0f;
            mOverlayDisplayed = true;
        }
    }
    else if (mOverlayDisplayLength != -15.0f)
    {
        mOverlayDisplayLength -= deltaT;
        if (mOverlayDisplayLength <= 0.0)
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
    }
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
void Presentation::EventHandler(Event* event)
{
    class Team
    {
    public:
        char _pad[0x3C];
        s32 m_nScore;
        Team* GetOtherTeam();
    };

    class Player
    {
    public:
        bool IsCaptain() const;
    };

    struct GameLocal
    {
        char _pad[0x1C];
        float m_fGameDuration;
        s32 m_nLastTeamToScore;
        eGameState m_eGameState;
        float GetGameTime();
        void ChangeGameState(eGameState);
    };

    struct NisPlayerGoalData
    {
        char _pad[0xBD28];
        s32 mWinnerSide[2];
    };

    struct GoalieSaveDataLocal : EventData
    {
        nlVector3 v3BallVelocity;
        void* pGoalie;
        void* pShooter;
        u32 saveType;
        float fWowFactor;
        unsigned int isSTS : 1;
        unsigned int padding : 31;
    };

    struct GoalScoredDataLocal : EventData
    {
        unsigned int uTeamIndex : 8;
        unsigned int uNumGoalsScored : 8;
        unsigned int uGoalType : 15;
        unsigned int uIsHyper : 1;
        nlVector3 v3ShotPosition;
        Player* pScorer;
        Player* pAssister;
        Player* pLastTouch[2];
    };

    struct TreeNodeLocal;

    struct HelperObjectLocal
    {
        u32 m_uHashID;
        float _pad0[12];
        nlVector3 position;
        float _pad1;
        char name[64];
    };

    struct TreeNodeLocal
    {
        TreeNodeLocal* left;
        TreeNodeLocal* right;
        s8 heavy;
        u8 _pad0[3];
        u32 key;
        HelperObjectLocal* value;
    };

    struct WorldLocal
    {
        u8 _pad[0x74];
        TreeNodeLocal* helperRoot;
        u8 _pad2[0x4];
        u32 helperCount;
    };

    struct TreeStackLocal
    {
        TreeNodeLocal** nodes;
        u32 count;
    };

    struct EmissionControllerUserLocal
    {
        u8 _pad[0x78];
        u32 m_uUserData;
    };

    extern Team* g_pTeams[];
    extern unsigned int nlDefaultSeed;
    extern void* fxGetGroup__FPCc(const char*);
    extern void* Create__15EmissionManagerFP12EffectsGroupUs(void*, unsigned short);
    extern void SetPosition__18EmissionControllerFRC9nlVector3(void*, const nlVector3&);
    extern void __dla__FPv(void*);
    extern void __dl__FPv(void*);

    if (g_pGame == 0)
    {
        return;
    }

    if (event->m_uEventID == 8)
    {
        mWaitingForCharacterDirectionSince = 0.0f;
    }

    if (event->m_uEventID == 5)
    {
        WorldLocal* world = (WorldLocal*)WorldManager::s_World;
        TreeStackLocal* stack = (TreeStackLocal*)nlMalloc(8, 8, false);
        if (stack != 0)
        {
            stack->nodes = (TreeNodeLocal**)nlMalloc((world->helperCount + 1) * sizeof(TreeNodeLocal*), 8, false);
            stack->count = 0;

            TreeNodeLocal* node = world->helperRoot;
            if (node != 0)
            {
                while (node->left != 0)
                {
                    stack->nodes[stack->count] = node;
                    stack->count++;
                    node = node->left;
                }

                stack->nodes[stack->count] = node;
                stack->count++;
            }

            static int len;
            static signed char init;
            const char* persistentEffectsTag = "fx_goal_";

            while (stack->count != 0)
            {
                u32 c = stack->count;
                TreeNodeLocal* currentNode = stack->nodes[c - 1];
                HelperObjectLocal* helper = currentNode->value;

                if (!init)
                {
                    len = strlen(persistentEffectsTag);
                    init = 1;
                }

                char fxName[256];
                char* fxStart = strstr(helper->name, persistentEffectsTag);
                if (fxStart)
                {
                    nlStrNCpy<char>(fxName, fxStart + len, 256);
                    char* underscore = strstr(fxName, "_");
                    if (underscore != 0)
                    {
                        *underscore = 0;
                    }

                    void* fx = fxGetGroup__FPCc(fxName);
                    if (fx != 0)
                    {
                        void* ec = Create__15EmissionManagerFP12EffectsGroupUs(fx, 0);
                        SetPosition__18EmissionControllerFRC9nlVector3(ec, helper->position);
                        ((EmissionControllerUserLocal*)ec)->m_uUserData = 0xDEADBEEF;
                    }
                }

                stack->count = c - 1;
                TreeNodeLocal* right = stack->nodes[stack->count]->right;
                if (right != 0)
                {
                    while (right->left != 0)
                    {
                        stack->nodes[stack->count] = right;
                        stack->count++;
                        right = right->left;
                    }

                    stack->nodes[stack->count] = right;
                    stack->count++;
                }
            }

            __dla__FPv(stack->nodes);
            __dl__FPv(stack);
        }

        Config& cfg = Config::Global();
        TagValuePair& tvp = cfg.FindTvp("no_presentation");
        bool noPresentation;

        if (tvp.tag == 0)
        {
            cfg.Set("no_presentation", "false");
            noPresentation = false;
        }
        else if (tvp.type == _BOOL)
        {
            noPresentation = LexicalCast<bool, bool>(tvp.value.b);
        }
        else if (tvp.type == _INT)
        {
            noPresentation = LexicalCast<bool, int>(tvp.value.i);
        }
        else if (tvp.type == _FLOAT)
        {
            noPresentation = LexicalCast<bool, float>(tvp.value.f);
        }
        else if (tvp.type == _STRING)
        {
            noPresentation = LexicalCast<bool, const char*>(tvp.value.s);
        }
        else
        {
            noPresentation = false;
        }

        if (noPresentation)
        {
            g_pGame->ChangeGameState(GS_KICKOFF);
            return;
        }

        GoalScoredDataLocal* gsd;
        s32 id = event->m_data.GetID();
        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            gsd = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x18A)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                gsd = 0;
            }
            else
            {
                gsd = (GoalScoredDataLocal*)&event->m_data;
            }
        }

        if (gsd != 0)
        {
            NisPlayerGoalData* np = (NisPlayerGoalData*)NisPlayer::Instance();
            np->mWinnerSide[1] = gsd->uTeamIndex;

            Team* team = g_pTeams[gsd->uTeamIndex];
            s32 scoreDiff = team->m_nScore - team->GetOtherTeam()->m_nScore;
            bool tiesTheGame = (scoreDiff == 0);

            bool takeTheLead = true;
            if (scoreDiff != 1 && scoreDiff != (s32)gsd->uNumGoalsScored)
            {
                takeTheLead = false;
            }

            GameLocal* game = (GameLocal*)g_pGame;
            bool inSuddenDeath = (game->m_eGameState == GS_OVERTIME);

            bool byCaptain;
            if (gsd->uGoalType == 5)
            {
                byCaptain = gsd->pLastTouch[gsd->uTeamIndex]->IsCaptain();
            }
            else
            {
                byCaptain = gsd->pScorer->IsCaptain();
            }

            const char* filter = "high";
            const char* script = "GoalCaptainCelebration";

            if (tiesTheGame)
            {
                mGoalQuality = HIGHLIGHT_QUALITY_GOAL_EQUALIZER;
            }
            else if (takeTheLead)
            {
                mGoalQuality = HIGHLIGHT_QUALITY_GOAL_INCREASE_DIFF;
            }
            else
            {
                mGoalQuality = HIGHLIGHT_QUALITY_GOAL_DECREASE_DIFF;
            }

            if (!inSuddenDeath && !takeTheLead && !tiesTheGame)
            {
                if (nlRandom(100, &nlDefaultSeed) < 80)
                {
                    filter = "low";
                }
            }

            if (!byCaptain)
            {
                if (nlRandom(100, &nlDefaultSeed) < 80)
                {
                    script = "GoalSidekickCelebration";
                }
            }

            if (nlStrCmp<char>(filter, "low") == 0)
            {
                if (gsd->uGoalType != 6 && !gsd->uIsHyper)
                {
                    if (nlRandom(100, &nlDefaultSeed) < 20)
                    {
                        script = "GoalJumbotron";
                        if (nlRandom(100, &nlDefaultSeed) < 50)
                        {
                            filter = "high";
                        }
                    }
                }
            }

            if (!tiesTheGame)
            {
                if (g_pGame != 0)
                {
                    float duration = game->m_fGameDuration;
                    if (game->GetGameTime() >= duration)
                    {
                        s32 awayScore = g_pTeams[1]->m_nScore;
                        s32 homeScore = g_pTeams[0]->m_nScore;
                        np->mWinnerSide[0] = (awayScore < homeScore);
                        script = "GoalSuddenDeath";
                    }
                }
            }

            FixedUpdateTask::mTimeScale = 1.0f;
            ParticleUpdateTask::SetTimeScale(1.0f);

            if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, script) != 0)
            {
                mQueuedFunction = script;
            }
            else
            {
                nlStrNCpy<char>(mCurrentFunction, script, 64);
                mSkipPressed = false;
                mInsideByPass = false;
                mByPassing = false;
                mInterruptWipe = 0;
                mUseInterruptWipe = 0;
                mTimeInFunction = 0.0f;

                NisPlayer::Instance()->SetExtraNameFilter(filter);
                CallFunction(nlStringHash(script));
            }
        }
    }

    if (event->m_uEventID == 0xF)
    {
        GoalieSaveDataLocal* gsd;

        s32 id = event->m_data.GetID();
        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            gsd = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x13C)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                gsd = 0;
            }
            else
            {
                gsd = (GoalieSaveDataLocal*)&event->m_data;
            }
        }

        if (gsd != 0 && !gsd->isSTS)
        {
            mDisplayLetterBox = 1.0f;
        }
    }
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
