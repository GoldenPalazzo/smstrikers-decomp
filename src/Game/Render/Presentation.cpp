#define BASICSTRING_NO_COPY_REREAD
#include "Game/Render/Presentation.h"
#include "Game/BeginFrameTask.h"
#include "Game/FixedUpdateTask.h"
#include "Game/ParticleUpdateTask.h"
#include "Game/WorldManager.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/AudioStream.h"
#include "Game/Audio/CrowdMood.h"
#include "Game/Audio/StreamTrack.h"
#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Effects/EmissionController.h"
#include "Game/AI/AIPad.h"
#include "Game/CharacterTemplate.h"
#include "Game/Debug/ShapeRender.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feManager.h"
#include "Game/GameInfo.h"
#include "Game/ReplayChoreo.h"
#include "Game/Render/CrowdManager.h"
#include "Game/Render/ElectricFence.h"
#include "Game/Render/Jumbotron.h"
#include "Game/Render/Wiper.h"
#include "NL/gl/gl.h"
#include "NL/nlConfig.h"
#include "NL/nlFile.h"
#include "NL/nlDebug.h"
#include "NL/nlString.h"
#include "NL/globalpad.h"
#include "Game/TrophyInfo.h"

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
    float GetGameTime();
};

extern cGame* g_pGame;

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

template <typename To, typename From>
To LexicalCast(const From&);

/**
 * Offset/Address/Size: 0x68 | 0x80127308 | size: 0x8
 */
template <>
const char* LexicalCast<const char*, const char*>(const char* const& from)
{
    FORCE_DONT_INLINE;
    return from;
}

/**
 * Offset/Address/Size: 0x2C | 0x801272CC | size: 0x24
 */
template <>
const char* LexicalCast<const char*, int>(const int& from)
{
    FORCE_DONT_INLINE;
    nlBreak();
    return 0;
}

/**
 * Offset/Address/Size: 0x8 | 0x801272A8 | size: 0x24
 */
template <>
const char* LexicalCast<const char*, float>(const float& from)
{
    FORCE_DONT_INLINE;
    nlBreak();
    return 0;
}

/**
 * Offset/Address/Size: 0x50 | 0x801272F0 | size: 0x18
 */
template <>
const char* LexicalCast<const char*, bool>(const bool& from)
{
    FORCE_DONT_INLINE;
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

static const char* GetGimmeCupTrophyName()
{
    Config& cfg = Config::Global();
    TagValuePair& tvp = cfg.FindTvp("gimme_cup_trophy");
    if (tvp.tag == NULL)
    {
        cfg.Set("gimme_cup_trophy", "FlowerCup");
        return "FlowerCup";
    }
    else if (tvp.type == _BOOL)
    {
        return LexicalCast<const char*, bool>(tvp.value.b);
    }
    else if (tvp.type == _INT)
    {
        return LexicalCast<const char*, int>(tvp.value.i);
    }
    else if (tvp.type == _FLOAT)
    {
        return LexicalCast<const char*, float>(tvp.value.f);
    }
    else if (tvp.type == _STRING)
    {
        return LexicalCast<const char*, const char*>(tvp.value.s);
    }
    return (const char*)0;
}

/**
 * Offset/Address/Size: 0x1848 | 0x8012602C | size: 0x460
 * TODO: 99.00% match - BasicString temporary flags still shift constructor data from r29/r30/r31 to r28/r29/r30
 */
void Presentation::LoadTrophyModel()
{
    cupTrophyHash = 0;

    bool hasCupOverride = Config::Global().Exists("gimme_cup_trophy");
    if (!hasCupOverride)
    {
        if (!nlSingleton<GameInfoManager>::s_pInstance->IsPossibleCupMatch())
        {
            return;
        }
    }

    cupTrophyHash = 1;

    BasicString<char, Detail::TempStringAllocator> trophyName(
        hasCupOverride
            ? BasicString<char, Detail::TempStringAllocator>("Gameplay/").Append(GetGimmeCupTrophyName())
            : BasicString<char, Detail::TempStringAllocator>(
                  GetThrophyModelName(nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode())));

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
    mTimeInFunction += deltaT;

    if (mDisplayLetterBox > 0.0f)
    {
        mDisplayLetterBox -= deltaT;
        if (mDisplayLetterBox <= 0.0f)
        {
            ReplayChoreo::Instance().SaveHighlight(ReplayChoreo::HIGHLIGHT_QUALITY_SAVE);
            mDisplayLetterBox = 0.0f;
        }
    }

    bool isPaused = false;
    if (!FrontEnd::m_bGameOver && nlTaskManager::m_pInstance->m_CurrState == 1)
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

        ReplayChoreo::Instance().Update(deltaT);
        ReplayCamera::UpdateTweakMode();

        if (!mSkipPressed)
        {
            bool pressedSkip;
            if (nlSingleton<GameInfoManager>::s_pInstance->IsInDemoMode())
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
                    cfg.Set("no_presentation_skip", false);
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
                        goto set_skip_pressed;
                    }
                }

                bool duringEndPresentation = DuringEndOfGamePresentation();

                if (duringEndPresentation & (mTimeInFunction <= 1.2f))
                {
                    pressedSkip = false;
                }
                else
                {
                    if (nlTaskManager::m_pInstance->m_CurrState == 0x100 || nlTaskManager::m_pInstance->m_CurrState == 0x10)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            if (!mIsAllowedToSkip[i])
                            {
                                if (nlStrCmp<char>(mCurrentFunction, "PlayHighlight") != 0)
                                {
                                    continue;
                                }
                            }

                            cGlobalPad* pad = cPadManager::GetPad(i);
                            if (pad != 0)
                            {
                                if (cPadManager::GetPad(i)->JustPressed(7, true))
                                {
                                    pressedSkip = true;
                                    goto set_skip_pressed;
                                }
                            }
                        }
                    }
                    pressedSkip = false;
                }
            }

        set_skip_pressed:
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

            if (DuringEndOfGamePresentation())
            {
                mTimeInFunction = 0.0f;
            }
        }

        if (IsFinished())
        {
            Finish();
        }
    }

    bool isPausedForRender = false;
    if (!FrontEnd::m_bGameOver && nlTaskManager::m_pInstance->m_CurrState == 1)
    {
        isPausedForRender = true;
    }

    float renderDelta;
    if (isPausedForRender)
    {
        renderDelta = 0.0f;
    }
    else
    {
        renderDelta = deltaT;
    }

    Wiper::Instance().Render(renderDelta);

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

    bool pauseOverlay = false;
    if (!FrontEnd::m_bGameOver && nlTaskManager::m_pInstance->m_CurrState == 1)
    {
        pauseOverlay = true;
    }

    if (pauseOverlay)
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

static inline void TriggerParticleEffects()
{
    extern void __dla__FPv(void*);
    extern void __dl__FPv(void*);

    WorldLocal* world = (WorldLocal*)WorldManager::s_World;
    TreeStackLocal* stack = (TreeStackLocal*)nlMalloc(8, 8, false);
    if (stack != 0)
    {
        TreeNodeLocal* node = world->helperRoot;
        stack->nodes = (TreeNodeLocal**)nlMalloc((world->helperCount + 1) * sizeof(TreeNodeLocal*), 8, false);
        stack->count = 0;

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
    }

    static int len;
    static signed char init;
    const char* persistentEffectsTag = "fx_goal_";

    while (stack->count != 0)
    {
        TreeNodeLocal* currentNode = stack->nodes[stack->count - 1];
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

            EffectsGroup* fx = fxGetGroup(fxName);
            if (fx != 0)
            {
                EmissionController* ec = EmissionManager::Create(fx, 0);
                ec->SetPosition(helper->position);
                ((EmissionControllerUserLocal*)ec)->m_uUserData = 0xDEADBEEF;
            }
        }

        stack->count--;
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

    if (stack != 0)
    {
        __dla__FPv(stack->nodes);
        __dl__FPv(stack);
    }
}

/**
 * Offset/Address/Size: 0x57C | 0x80124D60 | size: 0x774
 */
void Presentation::EventHandler(Event* event)
{
    struct GameLocal
    {
        char _pad[0x1C];
        float m_fGameDuration;
    };

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
        TriggerParticleEffects();

        Config& cfg = Config::Global();
        TagValuePair& tvp = cfg.FindTvp("no_presentation");
        bool noPresentation;

        if (tvp.tag == 0)
        {
            cfg.Set("no_presentation", false);
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

        bool takeTheLead;
        bool inSuddenDeath;
        GoalScoredData* gsd;
        bool tiesTheGame;
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
                gsd = (GoalScoredData*)&event->m_data;
            }
        }

        if (gsd != 0)
        {
            NisPlayer::Instance()->mWinnerSide[1] = gsd->uTeamIndex;

            cTeam* team = g_pTeams[gsd->uTeamIndex];
            s32 scoreDiff = team->m_nScore - team->GetOtherTeam()->m_nScore;
            tiesTheGame = (scoreDiff == 0);

            takeTheLead = true;
            if (scoreDiff != 1 && scoreDiff != (s32)gsd->uNumGoalsScored)
            {
                takeTheLead = false;
            }

            inSuddenDeath = (g_pGame->m_eGameState == GS_OVERTIME);

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
                    float duration = ((GameLocal*)g_pGame)->m_fGameDuration;
                    if (g_pGame->GetGameTime() >= duration)
                    {
                        s32 awayScore = g_pTeams[1]->m_nScore;
                        s32 homeScore = g_pTeams[0]->m_nScore;
                        NisPlayer* nisPlayer = NisPlayer::Instance();
                        nisPlayer->mWinnerSide[0] = (awayScore > homeScore);
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

            bool foundTeamPad = false;
            for (s32 i = 0; i < 4; i++)
            {
                cAIPad* aiPad = &AIPadManager::mAIPads[i];
                cPlayer** character = (cPlayer**)g_pCharacters;
                for (s32 c = 0; c < 5; c++)
                {
                    for (s32 k = 0; k < 2; k++)
                    {
                        cPlayer* ch = *character;
                        if (ch->m_pController == aiPad)
                        {
                            if ((s32)gsd->uTeamIndex == ch->m_pTeam->m_nSide)
                            {
                                mIsAllowedToSkip[i] = true;
                                foundTeamPad = true;
                            }
                            else
                            {
                                mIsAllowedToSkip[i] = false;
                            }
                        }
                        character++;
                    }
                }
            }

            if (!foundTeamPad)
            {
                mIsAllowedToSkip[0] = true;
                mIsAllowedToSkip[1] = true;
                mIsAllowedToSkip[2] = true;
                mIsAllowedToSkip[3] = true;
            }
        }
    }

    if (event->m_uEventID == 0xF)
    {
        GoalieSaveData* gsd;

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
                gsd = (GoalieSaveData*)&event->m_data;
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

static inline void SetIdleCallback(AudioStreamTrack::StreamTrack* track, const Function0<void>& f0)
{
    track->m_IdleCallback = Function<FnVoidVoid>(f0);
}

/**
 * Offset/Address/Size: 0x19C | 0x80124980 | size: 0x1D0
 */
void CupWinStingerDone()
{
    AudioStreamTrack::TrackManagerBase* pMgr = g_pTrackManager;
    AudioStreamTrack::StreamTrack* pTrack = pMgr->GetTrack(nlStringLowerHash("Music"));
    {
        Function0<void> emptyCallback;
        emptyCallback.mTag = EMPTY;
        SetIdleCallback(pTrack, emptyCallback);
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

/**
 * Offset/Address/Size: 0x0 | 0x801267BC | size: 0xAE4
 * TODO: 99.46% match - the script-load case and the task-state guard hold their
 * popped values one callee-saved register higher than the target (r27.. vs r26..),
 * shifting the saved-register range; logic and call sequence are otherwise exact.
 */
void Presentation::DoFunctionCall(unsigned int func)
{
    switch (func)
    {
    case 0:
        mByPassWasSkipped = false;
        mInsideByPass = true;
        break;
    case 1:
        DisplayElectricFence();
        break;
    case 2:
        if (mByPassing)
        {
            mByPassWasSkipped = true;
        }
        mInsideByPass = false;
        mByPassing = false;
        break;
    case 3:
        StopWithUndo();
        break;
    case 4:
        m_SP--;
        mInterruptWipe = (const char*)*m_SP;
        break;
    case 5:
        m_SP--;
        Jumbotron::instance.m_AnimationClass = (eJumboType)*m_SP;
        Jumbotron::instance.BeginLoad();
        break;
    case 6:
    {
        m_SP--;
        int arg4 = *m_SP;
        m_SP--;
        int arg3 = *m_SP;
        m_SP--;
        int arg2 = *m_SP;
        m_SP--;
        int arg1 = *m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        if (mByPassing)
        {
            break;
        }
        if (nlStrCmp<char>(name, "trophy") == 0 && !Config::Global().Exists("gimme_cup_trophy") && !nlSingleton<StatsTracker>::s_pInstance->mIsUserCupWinner)
        {
            break;
        }
        NisPlayer::Instance()->Load(name, (NisTarget)arg1, (NisUseStadiumOffset)arg2, (NisUseFilter)arg3, (NisWinnerType)arg4);
        break;
    }
    case 7:
        if (cupTrophyHash == 0)
        {
            break;
        }
        mTrophyTextureLoaded = false;
        trophyFileName[nlStrLen<char>(trophyFileName) - 1] = 't';
        glBeginLoadTextureBundle(trophyFileName, ReadTrophyTexture, g_TrophyTextureLocationInMemory);
        break;
    case 8:
        BeginFrameTask::s_FramerateLocked = 1;
        break;
    case 9:
    {
        m_SP--;
        int arg0 = *m_SP;
        if (mByPassing)
        {
            break;
        }
        {
            unsigned int state = nlTaskManager::m_pInstance->m_CurrState;
            if (state != 0x10)
            {
                bool isKickoff = false;
                if (!FrontEnd::m_bGameOver && state == 1)
                {
                    isKickoff = true;
                }
                if (!isKickoff)
                {
                    nlTaskManager::SetNextState(0x10);
                }
            }
        }
        ReplayChoreo::Instance().StartAutoReplay((ReplayType)arg0);
        if (arg0 == 7)
        {
            nlSingleton<OverlayManager>::s_pInstance->SetCurrentTextOverlaySlide((OverlaySlideName)7);
            nlSingleton<OverlayManager>::s_pInstance->SetVisible((SceneList)0x44, false, true);
            nlSingleton<OverlayManager>::s_pInstance->mIsInHighlights = true;
            if (mOverlayDisplayed)
            {
                nlSingleton<OverlayManager>::s_pInstance->SetVisible(mOverlayToDisplay, false, false);
            }
            mOverlayDisplayed = false;
            mOverlayToDisplay = SCENE_INVALID;
            mOverlayDisplayLength = 0.0f;
            mOverlayDelay = 0.0f;
            PlayOverlay("highlight", 0.5f, 30.0f);
        }
        else
        {
            nlSingleton<OverlayManager>::s_pInstance->SetCurrentTextOverlaySlide((OverlaySlideName)7);
            nlSingleton<OverlayManager>::s_pInstance->SetVisible((SceneList)0x44, true, true);
            nlSingleton<OverlayManager>::s_pInstance->mIsInHighlights = false;
        }
        break;
    }
    case 10:
        if (mByPassing)
        {
            break;
        }
        mWaitingForCharacterDirectionSince = FixedUpdateTask::mSimulationTime;
        NisPlayer::Instance()->PlayCharacterDirection();
        break;
    case 11:
    {
        bool hasOverride = Config::Global().Exists("gimme_cup_trophy");
        if (!hasOverride && !nlSingleton<StatsTracker>::s_pInstance->mIsUserCupWinner)
        {
            break;
        }
        if (hasOverride)
        {
            break;
        }
        PlayOverlay("cup", 0.2f, -15.0f);
        const char* streamName = GetCupStreamName(nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode());
        AudioLoader::StartFEStream(streamName, false, "Music");
        AudioStreamTrack::TrackManagerBase* mgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = mgr->GetTrack(nlStringLowerHash("Music"));
        Function0<void> f0;
        f0.mTag = FREE_FUNCTION;
        f0.mFreeFunction = CupWinStingerDone;
        SetIdleCallback(track, f0);
        break;
    }
    case 12:
    {
        nlSingleton<ScreenTransitionManager>::s_pInstance->m_SelectedTransition = NULL;
        if (ReplayChoreo::Instance().NumHighlights() <= 0)
        {
            break;
        }
        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);
        if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, "PlayHighlight") != 0)
        {
            mQueuedFunction = "PlayHighlight";
            break;
        }
        nlStrNCpy<char>(mCurrentFunction, "PlayHighlight", 64);
        mSkipPressed = false;
        mInsideByPass = false;
        mByPassing = false;
        mInterruptWipe = 0;
        mUseInterruptWipe = 0;
        mTimeInFunction = 0.0f;
        NisPlayer::Instance()->SetExtraNameFilter("");
        CallFunction(nlStringHash("PlayHighlight"));
        break;
    }
    case 13:
        Jumbotron::instance.WaitForLoad();
        Jumbotron::instance.BeginPlaying();
        break;
    case 14:
        if (mByPassing)
        {
            break;
        }
        if (NisPlayer::Instance()->Play())
        {
            unsigned int state = nlTaskManager::m_pInstance->m_CurrState;
            if (state == 0x100)
            {
                break;
            }
            bool isKickoff = false;
            if (!FrontEnd::m_bGameOver && state == 1)
            {
                isKickoff = true;
            }
            if (!isKickoff)
            {
                nlTaskManager::SetNextState(0x100);
            }
        }
        else
        {
            StopWithUndo();
        }
        break;
    case 15:
    {
        float length, delay;
        const char* name;
        m_SP--;
        length = *(float*)m_SP;
        m_SP--;
        delay = *(float*)m_SP;
        m_SP--;
        name = (const char*)*m_SP;
        PlayOverlay(name, delay, length);
        break;
    }
    case 16:
    {
        m_SP--;
        const char* name = (const char*)*m_SP;
        if (mByPassing)
        {
            break;
        }
        Audio::PlayWorldSFXbyStr(name, 100.0f, -1.0f, false, true, NULL, NULL, NULL);
        break;
    }
    case 17:
    {
        float vol;
        const char* name;
        m_SP--;
        vol = *(float*)m_SP;
        m_SP--;
        name = (const char*)*m_SP;
        if (mByPassing)
        {
            break;
        }
        Audio::PlayWorldSFXbyStr(name, vol, -1.0f, false, true, NULL, NULL, NULL);
        break;
    }
    case 18:
        m_SP--;
        m_SP--;
        m_SP--;
        m_SP--;
        break;
    case 19:
    {
        m_SP--;
        const char* Param = (const char*)*m_SP;
        m_SP--;
        const char* Type = (const char*)*m_SP;
        NISData* data = new ((u8*)g_pEventManager->CreateValidEvent(0x56, 0x20) + 0x10) NISData();
        data->Type = Type;
        data->Param = Param;
        break;
    }
    case 20:
        NisPlayer::Instance()->Reset();
        break;
    case 21:
        if (mByPassing)
        {
            break;
        }
        ReplayChoreo::Instance().SaveHighlight((ReplayChoreo::HighlightQuality)mGoalQuality);
        break;
    case 22:
        m_SP--;
        CrowdManager::instance.SetState((eCrowdState)*m_SP, false);
        break;
    case 23:
    {
        m_SP--;
        bool enable = *m_SP;
        if (cupTrophyHash == 0)
        {
            break;
        }
        DrawableObject* obj = WorldManager::s_World->FindDrawableObject(cupTrophyHash);
        if (enable)
        {
            obj->m_uObjectFlags |= 1;
        }
        else
        {
            obj->m_uObjectFlags &= ~1;
        }
        if (enable)
        {
            CrowdMood::AdjustMood(CrowdMood::CM_Positive, 0xa);
            CrowdMood::EnableCrowdDecay(false);
        }
        else
        {
            cupTrophyHash = 0;
            CrowdMood::EnableCrowdDecay(true);
        }
        break;
    }
    case 24:
        Audio::StopStreaming();
        break;
    case 25:
        if (Jumbotron::instance.m_State == 4)
        {
            Jumbotron::instance.StopPlaying();
        }
        break;
    case 26:
        StopOverlay();
        break;
    case 27:
        m_SP--;
        break;
    case 28:
        if (Jumbotron::instance.m_State == 4)
        {
            Jumbotron::instance.StopPlaying();
        }
        Jumbotron::instance.Reset();
        break;
    case 29:
        BeginFrameTask::s_FramerateLocked = 0;
        break;
    case 30:
    {
        m_SP--;
        const char* filter = (const char*)*m_SP;
        if (mByPassing)
        {
            break;
        }
        if (nlSingleton<ScreenTransitionManager>::s_pInstance->m_SelectedTransition == NULL)
        {
            nlSingleton<ScreenTransitionManager>::s_pInstance->SelectRandomTransition(filter);
        }
        if (!ReplayChoreo::Instance().Done())
        {
            StopWithUndo();
        }
        break;
    }
    case 31:
        if (mByPassing)
        {
            break;
        }
        if (mWaitingForCharacterDirectionSince > 0.0f)
        {
            if (FixedUpdateTask::mSimulationTime - mWaitingForCharacterDirectionSince < 1.0f)
            {
                StopWithUndo();
            }
        }
        break;
    case 32:
    {
        m_SP--;
        const char* filter = (const char*)*m_SP;
        if (mByPassing)
        {
            break;
        }
        float threshold = 0.0f;
        if (nlSingleton<ScreenTransitionManager>::s_pInstance->m_SelectedTransition == NULL)
        {
            nlSingleton<ScreenTransitionManager>::s_pInstance->SelectRandomTransition(filter);
        }
        if (nlSingleton<ScreenTransitionManager>::s_pInstance->m_SelectedTransition != NULL)
        {
            threshold = 0.2f + nlSingleton<ScreenTransitionManager>::s_pInstance->GetSelectedTransitionCutTime();
        }
        if (NisPlayer::Instance()->TimeLeft() > threshold)
        {
            StopWithUndo();
        }
        break;
    }
    case 33:
        if (cupTrophyHash == 0)
        {
            break;
        }
        if (mTrophyTextureLoaded)
        {
            break;
        }
        StopWithUndo();
        break;
    case 34:
    {
        m_SP--;
        const char* wipeName = (const char*)*m_SP;
        if (mByPassing)
        {
            break;
        }
        if (mUseInterruptWipe != NULL)
        {
            wipeName = mUseInterruptWipe;
        }
        Wiper::Instance().DoWipe(wipeName);
        if (!Wiper::Instance().CutHasOccured() && Wiper::Instance().WipeInProgress())
        {
            StopWithUndo();
        }
        else
        {
            mUseInterruptWipe = 0;
        }
        break;
    }
    default:
        nlBreak();
        break;
    }
}
