#include "Game/Game.h"
#include "Game/Sys/clock.h"
#include "Game/Sys/eventman.h"
#include "Game/Team.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/ScriptAction.h"
#include "Game/AI/Powerups.h"
#include "Game/AI/AISandbox.h"
#include "Game/AI/Scripts/ScriptDefines.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/GameInfo.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Camera/GameplayCam.h"
#include "Game/BasicStadium.h"
#include "Game/CharacterTemplate.h"
#include "Game/DB/StatsTracker.h"
#include "Game/Formation.h"
#include "Game/NisPlayer.h"
#include "NL/nlConfig.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlMath.h"
#include "Game/AI/FilteredRandom.h"
#include "Game/Render/SidelineExplodable.h"
#include "Game/Render/ElectricFence.h"
#include "Game/Render/Presentation.h"
#include "Game/ReplayChoreo.h"
#include "Game/Audio/AudioLoader.h"
#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/utility.h"
#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/msl_memory.h"

extern cTeam* g_pTeams[];
extern PowerupBase* g_pPowerups[];
extern cCharacter* g_pCurrentlyUpdatingCharacter;
extern cTeam* g_pCurrentlyUpdatingTeam;

/**
 * Offset/Address/Size: 0x24 | 0x800401E8 | size: 0x4
 */
void std::allocator<std::pair<const unsigned long, FuzzyVariant> >::destroy(std::pair<const unsigned long, FuzzyVariant>*)
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800401C4 | size: 0x24
//  */
// void std::allocator<std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::node>::deallocate(std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::node*, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x4 | 0x800401C0 | size: 0x4
//  */
// void Metrowerks::details::compressed_pair_imp<std::allocator<std::pair<const unsigned long, FuzzyVariant> >, unsigned long, 1>::first()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800401BC | size: 0x4
 */
namespace std
{

template <class Arg1, class Arg2, class Result>
struct binary_function
{
};

template <class T>
struct less : public binary_function<T, T, bool>
{
};

template <class Key, class Value, class Compare, class Allocator>
class map
{
public:
    class value_compare : public binary_function<Value, Value, bool>
    {
    protected:
        Compare comp;
    };
};

template <int N>
class __red_black_tree
{
public:
    struct anchor
    {
    };
};

template <class T, class Compare, class Allocator>
class __tree
{
public:
    struct node
    {
    };

private:
    Metrowerks::details::compressed_pair_imp<Allocator, unsigned long, 1> alloc_;
    Metrowerks::details::compressed_pair_imp<std::allocator<node>, __red_black_tree<1>::anchor, 1> node_alloc_;

public:
    void alloc();
    std::allocator<node>& node_alloc();
    void clear();
};

} // namespace std

typedef std::pair<const unsigned long, FuzzyVariant> _PairT;
typedef std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<_PairT> > _MapT;
typedef std::__tree<_PairT, _MapT::value_compare, std::allocator<_PairT> > _TreeT;
typedef std::allocator<_TreeT::node> _NodeAllocT;
typedef std::__red_black_tree<1>::anchor _AnchorT;
typedef std::allocator<_PairT> _PairAllocT;

#pragma dont_inline on
namespace Metrowerks
{
namespace details
{
template <>
_NodeAllocT& compressed_pair_imp<_NodeAllocT, _AnchorT, 1>::first()
{
    return *this;
}

template <>
_PairAllocT& compressed_pair_imp<_PairAllocT, unsigned long, 1>::first()
{
    return *this;
}
} // namespace details
} // namespace Metrowerks
#pragma dont_inline reset

static void _dummy_compressed_pair_first()
{
    Metrowerks::details::compressed_pair_imp<_NodeAllocT, _AnchorT, 1> obj;
    obj.first();
    Metrowerks::details::compressed_pair_imp<_PairAllocT, unsigned long, 1> obj2;
    obj2.first();
}

// /**
//  * Offset/Address/Size: 0xAC | 0x80040190 | size: 0x2C
//  */
// void nlListAddEnd<ListEntry<unsigned long> >(ListEntry<unsigned long>**, ListEntry<unsigned long>**, ListEntry<unsigned long>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x8004014C | size: 0x44
//  */
// void nlListRemoveStart<ListEntry<unsigned long> >(ListEntry<unsigned long>**, ListEntry<unsigned long>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800400E4 | size: 0x68
//  */
// void nlWalkList<ListEntry<unsigned long>, ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > > >(ListEntry<unsigned long>*, ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > >*, void (ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > >::*)(ListEntry<unsigned long>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8004006C | size: 0x78
//  */
// void cCameraManager::GetCamera<GameplayCamera>(eCameraType)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80040064 | size: 0x8
//  */
// void GoalScoredData::GetID()
// {
// }

// /**
//  * Offset/Address/Size: 0x420 | 0x80040044 | size: 0x20
//  */
// void std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::__tree(const std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare&, const std::allocator<std::pair<const unsigned long, FuzzyVariant> >&)
// {
// }

// /**
//  * Offset/Address/Size: 0x3D4 | 0x8003FFF8 | size: 0x4C
//  */
// void std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::clear()
// {
// }

/**
 * Offset/Address/Size: 0x3D0 | 0x8003FFF4 | size: 0x4
 */
void std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::alloc()
{
}

/**
 * Offset/Address/Size: 0x3C8 | 0x8003FFEC | size: 0x8
 */
_NodeAllocT& _TreeT::node_alloc()
{
    return (_NodeAllocT&)node_alloc_;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8003FC24 | size: 0x3C8
//  */
// void std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::destroy(std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::node*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8003FBA8 | size: 0x7C
//  */
// void operator== <char, Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&, const char*)
// {
// }

// /**
//  * Offset/Address/Size: 0xA8 | 0x8003FB24 | size: 0x84
//  */
// void Config::TagValuePair::Get<BasicString<char, Detail::TempStringAllocator> >() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8003FA7C | size: 0xA8
//  */
// void Config::Get<BasicString<char, Detail::TempStringAllocator> >(const char*, BasicString<char, Detail::TempStringAllocator>)
// {
// }

// /**
//  * Offset/Address/Size: 0x8F0 | 0x8003F994 | size: 0xE8
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8C4 | 0x8003F968 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x898 | 0x8003F93C | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x890 | 0x8003F934 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x138 | 0x8003F1DC | size: 0x758
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::PostorderTraversal(AVLTreeEntry<unsigned long, FuzzyVariant>*, void (AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, FuzzyVariant>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xD4 | 0x8003F178 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::DestroyTree(void (AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, FuzzyVariant>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x7C | 0x8003F120 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8003F0A4 | size: 0x7C
//  */
// void AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
// {
// }

/**
 * Offset/Address/Size: 0xE0 | 0x8003F064 | size: 0x40
 */
void ScriptQuestionCache::Clear()
{
    mQuestionCacheMap.Clear();
    ((_TreeT&)mQuestionCacheMapSTD).clear();
    mCacheHits = 0;
    mTotalLookups = 0;
}

// /**
//  * Offset/Address/Size: 0x80 | 0x8003F004 | size: 0x60
//  */
// void nlAVLTreeSlotPool<unsigned long, FuzzyVariant, DefaultKeyCompare<unsigned long> >::~nlAVLTreeSlotPool()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8003EF84 | size: 0x80
 */
ScriptQuestionCache::~ScriptQuestionCache()
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8003EF4C | size: 0x38
//  */
// void nlSingleton<ScriptQuestionCache>::DestroyInstance()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8003EEEC | size: 0x60
//  */
// GameTweaks::~GameTweaks()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8003EEC8 | size: 0x24
//  */
// void ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > >::DeleteEntry(ListEntry<unsigned long>*)
// {
// }

/**
 * Offset/Address/Size: 0x2274 | 0x8003E7E8 | size: 0x6E0
 * TODO: 9.4% match - incomplete, needs Config/SkillTweaks/difficulty initialization
 */
/**
 * Offset/Address/Size: 0x1FC4 | 0x8003E7E8 | size: 0x6E0
 * TODO: 69.9% match - StdMapStub missing std::__tree ctor call, AISandbox cmplwi pattern,
 *       ScriptQuestionCache r29/r28 register allocation, stack frame 0x10 too small
 */
void CreateGame()
{
    g_pGame = new (nlMalloc(sizeof(cGame), 8, false)) cGame();
    g_pTeams[0] = new (nlMalloc(sizeof(cTeam), 8, false)) cTeam(0);
    g_pTeams[1] = new (nlMalloc(sizeof(cTeam), 8, false)) cTeam(1);
    cField::Init(g_pTeams[0]->m_pNet, g_pTeams[1]->m_pNet);
    FormationManager::LoadFormationSets();

    if (nlSingleton<AISandbox>::s_pInstance == NULL)
    {
        nlSingleton<AISandbox>::s_pInstance = new (nlMalloc(1, 8, false)) AISandbox();
    }

    if (nlSingleton<ScriptQuestionCache>::s_pInstance == NULL)
    {
        nlSingleton<ScriptQuestionCache>::s_pInstance = new (nlMalloc(0x40, 8, false)) ScriptQuestionCache();
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->GetGameplayOptions().SkillLevel == 0)
    {
        g_pGame->m_fGameDuration = 10800.0f;
    }

    if (Config::Global().Exists("DifficultyOverride"))
    {
        eDifficultyID diff = DIFF_MEDIUM;
        BasicString<char, Detail::TempStringAllocator> defaultStr("Professional");
        BasicString<char, Detail::TempStringAllocator> diffStr = Config::Global().Get<BasicString<char, Detail::TempStringAllocator> >("DifficultyOverride", defaultStr);
        if (diffStr == "Rookie")
        {
            diff = DIFF_EASY;
        }
        else if (diffStr == "Professional")
        {
            diff = DIFF_MEDIUM;
        }
        else if (diffStr == "Superstar")
        {
            diff = DIFF_HARD;
        }
        else if (diffStr == "Legendary")
        {
            diff = DIFF_VERYHARD;
        }
        if (diff != DIFF_DEFAULT)
        {
            g_pTeams[0]->SetDifficulty(diff);
        }
        if (diff != DIFF_DEFAULT)
        {
            g_pTeams[1]->SetDifficulty(diff);
        }
        if (diff != DIFF_DEFAULT)
        {
            SkillTweaks localTweaks;
            localTweaks.Init(diff, false);
            for (int i = 0; i < 2; i++)
            {
                SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
                pTweaks->fShotValue1 = localTweaks.fShotValue1;
                pTweaks->fShotValue2 = localTweaks.fShotValue2;
                pTweaks->fShotValue3 = localTweaks.fShotValue3;
                pTweaks->fShotChance0 = localTweaks.fShotChance0;
                pTweaks->fShotChance1 = localTweaks.fShotChance1;
                pTweaks->fShotChance2 = localTweaks.fShotChance2;
                pTweaks->fShotChance3 = localTweaks.fShotChance3;
                pTweaks->fShotChance4 = localTweaks.fShotChance4;
                pTweaks->fSTSWindupTime = localTweaks.fSTSWindupTime;
                pTweaks->fAttackCarrierDistance = localTweaks.fAttackCarrierDistance;
                pTweaks->fLooseBallChaseDistance = localTweaks.fLooseBallChaseDistance;
                pTweaks->fGoalieCanInterceptPass = localTweaks.fGoalieCanInterceptPass;
            }
        }
    }
    else
    {
        eDifficultyID retVal = nlSingleton<GameInfoManager>::s_pInstance->GetSkillLevelAsDifficultyID();
        eDifficultyID diff0 = nlSingleton<GameInfoManager>::s_pInstance->mCurrentDifficulty[0];
        eDifficultyID diff1 = nlSingleton<GameInfoManager>::s_pInstance->mCurrentDifficulty[1];
        if (diff0 != DIFF_DEFAULT)
        {
            g_pTeams[0]->SetDifficulty(diff0);
        }
        if (diff1 != DIFF_DEFAULT)
        {
            g_pTeams[1]->SetDifficulty(diff1);
        }
        if (retVal != DIFF_DEFAULT)
        {
            SkillTweaks localTweaks;
            localTweaks.Init(retVal, false);
            for (int i = 0; i < 2; i++)
            {
                SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
                pTweaks->fShotValue1 = localTweaks.fShotValue1;
                pTweaks->fShotValue2 = localTweaks.fShotValue2;
                pTweaks->fShotValue3 = localTweaks.fShotValue3;
                pTweaks->fShotChance0 = localTweaks.fShotChance0;
                pTweaks->fShotChance1 = localTweaks.fShotChance1;
                pTweaks->fShotChance2 = localTweaks.fShotChance2;
                pTweaks->fShotChance3 = localTweaks.fShotChance3;
                pTweaks->fShotChance4 = localTweaks.fShotChance4;
                pTweaks->fSTSWindupTime = localTweaks.fSTSWindupTime;
                pTweaks->fAttackCarrierDistance = localTweaks.fAttackCarrierDistance;
                pTweaks->fLooseBallChaseDistance = localTweaks.fLooseBallChaseDistance;
                pTweaks->fGoalieCanInterceptPass = localTweaks.fGoalieCanInterceptPass;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x2118 | 0x8003E68C | size: 0x15C
 */
void DestroyGame()
{
    Config& cfg = Config::Global();
    bool bWriteStats = GetConfigBool(cfg, "write_stats", false);

    if (bWriteStats)
    {
        nlSingleton<StatsTracker>::s_pInstance->WriteStats(g_pGame->m_fGameDuration, -1.0f, NULL);
    }

    nlSingleton<ScriptQuestionCache>::DestroyInstance();

    AISandbox* p = nlSingleton<AISandbox>::s_pInstance;
    if (p != NULL)
    {
        if (p != NULL)
        {
            delete p;
        }
        nlSingleton<AISandbox>::s_pInstance = 0;
    }

    delete g_pTeams[0];
    delete g_pTeams[1];

    g_pTeams[1] = NULL;
    g_pTeams[0] = NULL;

    delete g_pGame;

    g_pGame = NULL;

    FormationManager::UnloadFormationSets();
}

/**
 * Offset/Address/Size: 0x2024 | 0x8003E598 | size: 0xF4
 * TODO: 99.0% match - r28/r29/r30 register rotation for pGame/i/ppPowerups
 */
void DestroyPowerups()
{
    int i;
    cGame* pGame;
    cTeam** ppTeams;
    cTeam* pTeam;

    ppTeams = g_pTeams;
    pGame = g_pGame;

    for (i = 0; i < 2; i++)
    {
        pTeam = ppTeams[i];
        if (pTeam != NULL)
        {
            pTeam->mfPowerupMeter = 0.0f;
        }
    }

    for (i = 0; i < 25; i++)
    {
        PowerupBase* pPowerup = g_pPowerups[i];
        if (pPowerup != NULL)
        {
            pPowerup->Destroy(true);
            g_pPowerups[i] = NULL;
        }
    }

    if (BasicStadium::GetCurrentStadium()->mpNPCManager != NULL)
    {
        if (BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp != NULL)
        {
            if (pGame->mbCaptainShotToScoreOn == false)
            {
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->Hide(true);
            }
        }
    }

    CompactPowerups();
}

/**
 * Offset/Address/Size: 0x1D30 | 0x8003E2A4 | size: 0x2F4
 */
cGame::cGame()
    : m_bBallInNet(false)
    , m_eGameState(GS_NONE)
    , m_pScorer(NULL)
    , m_pAssister(NULL)
    , mbCaptainShotToScoreOn(false)
    , mIsPure(false)
    , mInSuddenDeath(false)
{
    m_pPostResetClock = new (nlMalloc(sizeof(Clock), 8, false)) Clock(0.0f, 2.0f, 1.0f, 2, cGame::PostResetCallback);
    m_pPostResetClock->m_uParam1 = (unsigned long)this;

    m_pGameTweaks = new (nlMalloc(sizeof(GameTweaks), 8, false)) GameTweaks("GameTweaks.ini");
    m_pFuzzyTweaks = new (nlMalloc(sizeof(FuzzyTweaks), 8, false)) FuzzyTweaks("FuzzyTweaks.ini");

    m_fGameDuration = m_pGameTweaks->fGameDuration;

    m_pGameClock = new (nlMalloc(sizeof(Clock), 8, false)) Clock(0.0f, 60000.0f, 1.0f, 2, NULL);
    m_pPostGameDoneClock = new (nlMalloc(sizeof(Clock), 8, false)) Clock(0.0f, 1.4f, 1.0f, 2, NULL);

    m_pTarget = NULL;
    m_pTeamTouch[1] = NULL;
    m_pTeamTouch[0] = NULL;
    m_pRandomPlayersArray = (cPlayer**)nlMalloc(sizeof(cPlayer*) * 10, 8, false);

    Config& cfg = Config::Global();
    mIsPure = GetConfigBool(cfg, "pure_game", false);
    if (GetConfigBool(Config::Global(), "save_stats", false) != false)
    {
        nlSingleton<StatsTracker>::s_pInstance->WriteCurrentlyPlaying();
    }
}

/**
 * Offset/Address/Size: 0x1BA8 | 0x8003E11C | size: 0x188
 */
cGame::~cGame()
{
    nlWalkList(mThoughtsQueue.m_Head,
        static_cast<ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > >*>(&mThoughtsQueue),
        &ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > >::DeleteEntry);
    mThoughtsQueue.m_Head = NULL;
    mThoughtsQueue.m_Tail = NULL;

    delete m_pPostResetClock;
    delete m_pGameClock;
    delete m_pGameTweaks;
    delete m_pFuzzyTweaks;
    delete m_pPostGameDoneClock;
    delete[] m_pRandomPlayersArray;
}

#include "Game/FixedUpdateTask.h"
#include "Game/ParticleUpdateTask.h"
#include "Game/Audio/WorldAudio.h"

namespace Audio
{
extern cWorldSFX gWorldSFX;
}

/**
 * Offset/Address/Size: 0x1B2C | 0x8003E0A0 | size: 0x7C
 */
void cGame::DoPerfectPassSlowDown()
{
    if (g_pBall->mbHyperSTS == 0)
    {
        return;
    }

    GameTweaks* pTweaks;
    pTweaks = g_pGame->m_pGameTweaks;
    FixedUpdateTask::mTimeScale = pTweaks->unk1E4;
    pTweaks = g_pGame->m_pGameTweaks;
    ParticleUpdateTask::SetTimeScale(pTweaks->unk1E4);
    g_pEventManager->CreateValidEvent(0x46, 0x14);
    Audio::gWorldSFX.Play(Audio::REPLAYSFX_CAMERA_ZOOM_OUT, 100.0f, -1.0f, true, 100.0f);
    Audio::FadeFilterToFullStrength();
}

/**
 * Offset/Address/Size: 0x1B18 | 0x8003E08C | size: 0x14
 */
float cGame::GetNormalizedGameTime()
{
    return m_pGameClock->m_fTimer / m_fGameDuration;
}

/**
 * Offset/Address/Size: 0x1B0C | 0x8003E080 | size: 0xC
 */
float cGame::GetGameTime()
{
    return m_pGameClock->m_fTimer;
}

/**
 * Offset/Address/Size: 0x1720 | 0x8003DC94 | size: 0x3EC
 * TODO: 91.98% match - this in r29 vs r30 (register allocation shift due to
 * -inline deferred), nlVector3 stack offsets swapped (sp+0x14/0x20)
 */
void cGame::ResetForKickOff()
{
    cFielder* pBallCarrier;
    g_pEventManager->CreateValidEvent(9, 0x14);
    int i;
    for (i = 0; i < 5; i++)
    {
        m_pRandomPlayersArray[i] = g_pTeams[0]->GetPlayer(i);
    }
    for (i = 0; i < 5; i++)
    {
        m_pRandomPlayersArray[5 + i] = g_pTeams[1]->GetPlayer(i);
    }
    static FilteredRandomRange randgen;
    for (i = 0; i < 10; i++)
    {
        int j = randgen.genrand(10);
        if (j != i)
        {
            cPlayer* temp = m_pRandomPlayersArray[i];
            m_pRandomPlayersArray[i] = m_pRandomPlayersArray[j];
            m_pRandomPlayersArray[j] = temp;
        }
    }
    for (i = 0; i < 2; i++)
    {
        g_pTeams[i]->ResetCharacters();
    }
    nlVector3 position = { 0.0f, 0.0f, 0.0f };
    nlVector3 velocity = { 0.0f, 0.0f, 0.0f };
    if (g_pBall->m_pOwner != NULL)
    {
        g_pBall->m_pOwner->ReleaseBall();
    }
    g_pBall->WarpTo(position);
    g_pBall->SetVelocity(velocity, SPINTYPE_NONE, NULL);
    g_pBall->mbCanDamage = 0;
    g_pBall->mpDamageTarget = NULL;
    m_bBallInNet = false;
    g_pBall->ClearBallEffects();
    g_pBall->HandleBuzzerBeater(-1.0f);
    if (g_pTeams[0] != NULL)
    {
        g_pTeams[0]->mfPowerupMeter = 0.0f;
    }
    if (g_pTeams[1] != NULL)
    {
        g_pTeams[1]->mfPowerupMeter = 0.0f;
    }
    for (i = 0; i < 25; i++)
    {
        if (g_pPowerups[i] != NULL)
        {
            g_pPowerups[i]->Destroy(true);
            g_pPowerups[i] = NULL;
        }
    }
    if (BasicStadium::GetCurrentStadium()->mpNPCManager != NULL)
    {
        if (BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp != NULL)
        {
            if (mbCaptainShotToScoreOn == false)
            {
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->Hide(true);
            }
        }
    }
    m_pScorer = NULL;
    m_pAssister = NULL;
    for (i = 0; i < 2; i++)
    {
        m_pTeamTouch[i] = g_pTeams[i]->GetCaptain();
    }
    Bowser::SetTiltParameters(0.0f);
    nlWalkList(mThoughtsQueue.m_Head, (ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > >*)&mThoughtsQueue, &ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long> > >::DeleteEntry);
    mThoughtsQueue.m_Head = NULL;
    mThoughtsQueue.m_Tail = NULL;
    BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->ActionHide();
    if (mBowserTimer.m_uPackedTime != 0)
    {
        if (mBowserTimer.GetSeconds() < 5.0f)
        {
            mBowserTimer.SetSeconds(5.0f);
        }
    }
    m_pPostResetClock->Reset(0.0f, 1.0f, 0.0f);
    m_pPostResetClock->Start();
    cCameraManager::Remove(eCameraType_MatrixEffect, true);
    GameplayCamera* gpc = cCameraManager::GetCamera<GameplayCamera>(eCameraType_Gameplay);
    if (gpc != NULL)
    {
        gpc->m_ForceNeutralAndNearZoom = true;
    }
    SidelineExplodableManager::DestroyAllActiveFragments(false);
    StopDisplayingElectricFence();
    if (g_pTeams[m_nLastTeamToScore] != NULL)
    {
        pBallCarrier = g_pTeams[!m_nLastTeamToScore]->GetFielder(0);
        if (pBallCarrier->m_pBall == NULL)
        {
            pBallCarrier->PickupBall(g_pBall);
            pBallCarrier->InitActionRunningWB(false);
        }
    }
}

/**
 * Offset/Address/Size: 0x16DC | 0x8003DC50 | size: 0x44
 */
void cGame::PostResetCallback(unsigned long, unsigned long)
{
    g_pEventManager->CreateValidEvent(0xa, 0x14);
    GameplayCamera* pCamera = cCameraManager::GetCamera<GameplayCamera>(eCameraType_Gameplay);
    if (pCamera != nullptr)
    {
        pCamera->m_ForceNeutralAndNearZoom = false;
    }
}

/**
 * Offset/Address/Size: 0x113C | 0x8003D6B0 | size: 0x5A0
 * TODO: 96.40% match - register allocation uses one fewer non-volatile register
 * (stmw r25 vs r24), causing cascading r-diffs and extra g_pTeams reload at end.
 */
void cGame::BeginGame(bool bRematch, bool bStraightToKickoff)
{
    int i;

    if (m_eGameState != GS_PRE_GAME)
    {
        InitGameState(GS_PRE_GAME);
    }

    m_bBallInNet = false;
    mInSuddenDeath = false;

    for (i = 0; i < 5; i++)
    {
        m_pRandomPlayersArray[i] = g_pTeams[0]->GetPlayer(i);
    }
    for (i = 0; i < 5; i++)
    {
        m_pRandomPlayersArray[5 + i] = g_pTeams[1]->GetPlayer(i);
    }
    m_nLastTeamToScore = 1;

    static FilteredRandomRange randgen;
    for (i = 0; i < 10; i++)
    {
        int j = randgen.genrand(10);
        if (j != i)
        {
            cPlayer* temp = m_pRandomPlayersArray[i];
            m_pRandomPlayersArray[i] = m_pRandomPlayersArray[j];
            m_pRandomPlayersArray[j] = temp;
        }
    }

    for (i = 0; i < 2; i++)
    {
        g_pTeams[i]->ResetCharacters();
    }

    nlVector3 velocity = { 0.0f, 0.0f, 0.0f };
    nlVector3 position = { 0.0f, 0.0f, 0.18f };

    if (g_pBall->m_pOwner != NULL)
    {
        g_pBall->m_pOwner->ReleaseBall();
    }
    g_pBall->WarpTo(position);
    g_pBall->SetVelocity(velocity, SPINTYPE_NONE, NULL);
    g_pBall->m_unk_0xA6 = false;
    g_pBall->mpDamageTarget = NULL;
    m_bBallInNet = false;
    g_pBall->ClearBallEffects();
    g_pBall->HandleBuzzerBeater(-1.0f);

    for (i = 0; i < 2; i++)
    {
        cTeam* pTeam = g_pTeams[i];
        if (pTeam != NULL)
        {
            pTeam->ClearAllPowerUps();
            pTeam->ClearCurrentPowerUp();
            pTeam->mfPowerupMeter = 0.0f;
        }
    }

    for (i = 0; i < 25; i++)
    {
        if (g_pPowerups[i] != NULL)
        {
            g_pPowerups[i]->Destroy(true);
            g_pPowerups[i] = NULL;
        }
    }

    if (BasicStadium::GetCurrentStadium()->mpNPCManager != NULL)
    {
        if (BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp != NULL)
        {
            if (!mbCaptainShotToScoreOn)
            {
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->Hide(true);
            }
        }
    }
    BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->ActionReset();

    if (GameInfoManager::s_pInstance->IsTiltingFieldOn() || GameInfoManager::s_pInstance->mIsInStrikers101Mode)
    {
        mBowserTimer.m_uPackedTime = 0;
    }
    else
    {
        if (GetConfigBool(Config::Global(), "bowser_repeat", false))
        {
            g_pGame->m_pGameTweaks->unk308 = 1.0f;
            g_pGame->m_pGameTweaks->unk30C = 4.0f;
            g_pGame->m_pGameTweaks->unk310 = -1.0f;
        }

        GameTweaks* pTweaks_ = g_pGame->m_pGameTweaks;
        if (nlRandomf(1.0f, &nlDefaultSeed) < pTweaks_->unk308)
        {
            GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
            float fMinTime = pTweaks->unk30C;
            float fMaxTime = pTweaks->unk310;

            if (fMinTime < 0.0f)
            {
                fMinTime = 0.0f;
            }
            else if (fMinTime > m_fGameDuration)
            {
                fMinTime = m_fGameDuration - 10.f;
            }

            float fThreshold = 0.0f;
            float fTimeRange = fThreshold;

            if (fMaxTime > fThreshold)
            {
                fTimeRange = m_fGameDuration - fMaxTime - fMinTime;
            }

            if (fTimeRange > fThreshold)
            {
                mBowserTimer.SetSeconds(fMinTime + nlRandomf(fTimeRange, &nlDefaultSeed));
            }
            else
            {
                mBowserTimer.SetSeconds(fMinTime);
            }
        }
        else
        {
            mBowserTimer.m_uPackedTime = 0;
        }
    }

    m_pGameClock->Reset(0.0f, 60000.0f, 1.0f);
    m_pPostGameDoneClock->Reset(0.0f, 1.4f, 1.0f);
    m_pPostGameDoneClock->Stop();

    mfCheatTilt = 0.0f;
    g_pTeams[0]->m_nScore = 0;
    g_pTeams[1]->m_nScore = 0;

    for (i = 0; i < 10; i++)
    {
        g_pCharacters[i]->m_Dirt = 0.0f;
    }

    SidelineExplodableManager::DestroyAllActiveFragments(true);
    Presentation::Instance().Reset();
    ReplayChoreo::Instance().FlushHighlights();

    if (bRematch)
    {
        AudioLoader::ResetForRematch();
    }
    else
    {
        AudioLoader::ResetForNewGame();
    }

    if (bStraightToKickoff)
    {
        if (m_eGameState != GS_KICKOFF)
        {
            InitGameState(GS_KICKOFF);
        }
    }
    else
    {
        Presentation::Instance().Call("GameBegin", "");
    }

    ReplayManager::Instance()->ResetSnapshots();
}

/**
 * Offset/Address/Size: 0xE5C | 0x8003D3D0 | size: 0x2E0
 */
void cGame::CheckForGoal()
{
    struct GoalScoredDataExt
    {
        GoalScoredData data;
        int sideOfInterest;
    };

    int nSide;

    if (g_pBall->GetInNet(nSide))
    {
        if (!m_bBallInNet)
        {
            g_pBall->HandleBuzzerBeater(-1.0f);

            nSide += 1;
            nSide %= 2;
            m_nLastTeamToScore = nSide;

            if (m_eGameState == GS_OVERTIME)
            {
                if (m_eGameState != GS_END_GAME)
                {
                    InitGameState(GS_END_GAME);
                }
            }
            else if (m_eGameState != GS_POST_GOAL)
            {
                InitGameState(GS_POST_GOAL);
            }

            if (m_pScorer != g_pBall->m_pLastTouch && g_pBall->m_pLastTouch->m_eClassType != GOALIE)
            {
                float fDirection = g_pBall->m_pLastTouch->m_pTeam->m_pNet->m_baseLocation.f.x * g_pBall->m_v3Position.f.x;
                if (fDirection >= 0.0f)
                {
                    g_pBall->m_uGoalType = 5;
                }
                else
                {
                    g_pBall->m_uGoalType = 3;
                }

                cPlayer* pPlayer = g_pBall->m_pLastTouch;
                if (m_pScorer != NULL && pPlayer != NULL && m_pScorer != pPlayer && m_pScorer->IsOnSameTeam(pPlayer))
                {
                    m_pAssister = m_pScorer;
                }
                else
                {
                    m_pAssister = NULL;
                }

                m_pScorer = pPlayer;

                if (pPlayer != NULL && pPlayer->m_eClassType == FIELDER)
                {
                    m_pTeamTouch[pPlayer->m_pTeam->m_nSide] = pPlayer;
                }
            }
            else if (m_pScorer != NULL)
            {
                if (nSide != m_pScorer->m_pTeam->m_nSide)
                {
                    g_pBall->m_uGoalType = 5;
                }
            }

            unsigned long uNumGoalsScored;
            if (g_pBall->m_uGoalType == 6)
            {
                World::sbIsHyperShootToScoreRenderingEnabled = false;
                uNumGoalsScored = 2;
            }
            else
            {
                uNumGoalsScored = 1;
            }

            g_pTeams[nSide]->m_nScore += uNumGoalsScored;

            GoalScoredDataExt* pGoalScored = new ((u8*)g_pEventManager->CreateValidEvent(5, 0x3C) + 0x10) GoalScoredDataExt();
            pGoalScored->data.uNumGoalsScored = uNumGoalsScored;
            pGoalScored->data.uTeamIndex = nSide;
            pGoalScored->data.uGoalType = g_pBall->m_uGoalType;
            pGoalScored->data.uIsHyper = g_pBall->m_unk_0xA5;
            pGoalScored->data.v3ShotPosition = g_pBall->m_v3ShotOrigin;
            pGoalScored->data.pScorer = m_pScorer;
            pGoalScored->data.pAssister = m_pAssister;

            if (m_pScorer != NULL && m_pScorer->GetGlobalPad() != NULL)
            {
                pGoalScored->sideOfInterest = m_pScorer->GetGlobalPad()->m_padIndex;
            }
            else
            {
                pGoalScored->sideOfInterest = -1;
            }

            pGoalScored->data.pLastTouch[0] = m_pTeamTouch[0];
            pGoalScored->data.pLastTouch[1] = m_pTeamTouch[1];

            g_pBall->m_uGoalType = 4;
            m_bBallInNet = true;
        }
    }
}

/**
 * Offset/Address/Size: 0xDF0 | 0x8003D364 | size: 0x6C
 * TODO: 98.33% match - posX stays in f2 (target uses f1), cascading to
 * y/z temp register allocation (f1/f2 swap in distance math chain).
 */
void cGame::BlowUpPowerups(const nlVector3& v3ExplosionPosition, float fExplosionRadius)
{
    float posX, posY, posZ;
    fExplosionRadius *= fExplosionRadius;
    posZ = v3ExplosionPosition.f.z;
    posY = v3ExplosionPosition.f.y;
    posX = v3ExplosionPosition.f.x;

    for (int i = 0; i < 25; i++)
    {
        PowerupBase* pPowerup = g_pPowerups[i];
        if (pPowerup != nullptr)
        {
            float dx, dy, dz;
            dy = posY - pPowerup->m_v3Position.f.y;
            dz = posZ - pPowerup->m_v3Position.f.z;
            dx = posX - pPowerup->m_v3Position.f.x;

            if (dx * dx + dy * dy + dz * dz < fExplosionRadius)
            {
                pPowerup->m_bShouldDestroy = true;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xCE4 | 0x8003D258 | size: 0x10C
 */
void cGame::ResetPowerups(bool clearPowerUps)
{
    for (int i = 0; i < 2; i++)
    {
        cTeam* pTeam = g_pTeams[i];
        if (pTeam != nullptr)
        {
            if (clearPowerUps)
            {
                pTeam->ClearAllPowerUps();
                pTeam->ClearCurrentPowerUp();
            }
            pTeam->mfPowerupMeter = 0.0f;
        }
    }

    for (int i = 0; i < 25; i++)
    {
        PowerupBase* pPowerup = g_pPowerups[i];
        if (pPowerup != nullptr)
        {
            pPowerup->Destroy(true);
            g_pPowerups[i] = nullptr;
        }
    }

    if (BasicStadium::GetCurrentStadium()->mpNPCManager != nullptr)
    {
        if (BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp != nullptr)
        {
            if (!mbCaptainShotToScoreOn)
            {
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->Hide(true);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xB00 | 0x8003D074 | size: 0x1E4
 */
void cGame::ResetBowser()
{
    if (GameInfoManager::s_pInstance->IsTiltingFieldOn() || GameInfoManager::s_pInstance->mIsInStrikers101Mode)
    {
        mBowserTimer.m_uPackedTime = 0;
        return;
    }

    if (GetConfigBool(Config::Global(), "bowser_repeat", false))
    {
        g_pGame->m_pGameTweaks->unk308 = 1.0f;
        g_pGame->m_pGameTweaks->unk30C = 4.0f;
        g_pGame->m_pGameTweaks->unk310 = -1.0f;
    }

    GameTweaks* pTweaks_ = g_pGame->m_pGameTweaks;
    if (nlRandomf(1.0f, &nlDefaultSeed) < pTweaks_->unk308)
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        float fMinTime = pTweaks->unk30C;
        float fMaxTime = pTweaks->unk310;

        if (fMinTime < 0.0f)
        {
            fMinTime = 0.0f;
        }
        else if (fMinTime > m_fGameDuration)
        {
            fMinTime = m_fGameDuration - 10.f;
        }

        float fThreshold = 0.0f;
        float fTimeRange = fThreshold;

        if (fMaxTime > fThreshold)
        {
            fTimeRange = m_fGameDuration - fMaxTime - fMinTime;
        }

        if (fTimeRange > fThreshold)
        {
            mBowserTimer.SetSeconds(fMinTime + nlRandomf(fTimeRange, &nlDefaultSeed));
            return;
        }

        mBowserTimer.SetSeconds(fMinTime);
        return;
    }

    mBowserTimer.m_uPackedTime = 0;
}

/**
 * Offset/Address/Size: 0xA80 | 0x8003CFF4 | size: 0x80
 */
void cGame::ResetBowserTimer(float seconds)
{
    if (seconds > 0.0f && !GameInfoManager::s_pInstance->IsTiltingFieldOn() && !GameInfoManager::s_pInstance->mIsInStrikers101Mode)
    {
        mBowserTimer.SetSeconds(seconds);
        return;
    }
    mBowserTimer.m_uPackedTime = 0;
}

/**
 * Offset/Address/Size: 0xA20 | 0x8003CF94 | size: 0x60
 */
void cGame::PreUpdate(float deltaTime)
{
    for (int i = 0; i < 2; i++)
    {
        g_pTeams[i]->PreUpdate(deltaTime);
    }
}

static inline void UpdatePowerUpObjects(float fDeltaT)
{
    for (int i = 0; i < 25; i++)
    {
        if (g_pPowerups[i] != nullptr)
        {
            g_pPowerups[i]->Update(fDeltaT);
        }
    }
}

/**
 * Offset/Address/Size: 0x5A4 | 0x8003CB18 | size: 0x47C
 * TODO: 98.55% match - conversion section f-register shift (f1/f2/f3 rotated),
 *       goalie r3/r4 swap for g_pCharacters base + blt vs bge branch inversion,
 *       end section r27/r28 vs r29/r30 for score variables
 */
void cGame::Update(float deltaTime)
{
    mThoughtsAllowedThisUpdate = 1;

    if (m_pGameClock->m_fTimer >= m_fGameDuration)
    {
        if (!g_pBall->IsBuzzerBeaterSet())
        {
            if (g_pTeams[0]->m_nScore == g_pTeams[1]->m_nScore)
            {
                if (m_eGameState == GS_GAMEPLAY)
                {
                    if (*(volatile eGameState*)&m_eGameState != GS_OVERTIME)
                    {
                        InitGameState(GS_OVERTIME);
                    }

                    nlSingleton<StatsTracker>::s_pInstance->mIsOvertime = true;
                    mInSuddenDeath = true;
                }
            }
            else
            {
                if (m_eGameState != GS_END_GAME)
                {
                    if (m_eGameState != GS_END_GAME)
                    {
                        InitGameState(GS_END_GAME);
                    }

                    nlSingleton<StatsTracker>::s_pInstance->TrackWinner(-1);
                    Audio::FadeFilterFromCurrentToZero();
                    FixedUpdateTask::mTimeScale = 1.0f;
                    ParticleUpdateTask::SetTimeScale(1.0f);
                }
            }
        }
    }

    for (int i = 0; i < 2; i++)
    {
        FuzzyScriptSetCurrentTeam(g_pTeams[i]);
        g_pTeams[i]->Update(deltaTime);
        FuzzyScriptClearGlobals();
    }

    for (int i = 0; i < 10; i++)
    {
        g_pCurrentlyUpdatingCharacter = m_pRandomPlayersArray[i];

        if (m_pRandomPlayersArray[i]->m_eClassType == FIELDER)
        {
            FuzzyScriptSetCurrentFielder((cFielder*)m_pRandomPlayersArray[i]);
        }
        else
        {
            FuzzyScriptSetCurrentTeam(m_pRandomPlayersArray[i]->m_pTeam);
        }

        g_pCurrentlyUpdatingTeam = m_pRandomPlayersArray[i]->m_pTeam;
        m_pRandomPlayersArray[i]->Update(deltaTime);
        FuzzyScriptClearGlobals();
    }

    g_pCurrentlyUpdatingCharacter = nullptr;
    g_pBall->Update(deltaTime);

    if (IsGameplayOrOvertime())
    {
        CheckForGoal();
    }

    UpdatePowerUpObjects(deltaTime);

    if (nlSingleton<GameInfoManager>::s_pInstance->IsTiltingFieldOn())
    {
        float tilt = (float)(g_pTeams[0]->m_nScore - g_pTeams[1]->m_nScore);
        tilt *= 2.0f;

        float clampedLower;
        if (tilt >= -6.0f)
        {
            clampedLower = tilt;
        }
        else
        {
            clampedLower = -6.0f;
        }
        tilt = clampedLower;

        float currentTilt = mfCheatTilt;

        float clampedUpper;
        if (tilt <= 6.0f)
        {
            clampedUpper = tilt;
        }
        else
        {
            clampedUpper = 6.0f;
        }
        tilt = clampedUpper;

        mfCheatTilt = ((cCharacter*)g_pTeams[0])->SeekSpeedExponential(currentTilt, tilt, 2.0f, deltaTime);
        Bowser::SetTiltParameters(mfCheatTilt);
    }

    if (mBowserTimer.m_uPackedTime != 0 && m_eGameState != GS_END_GAME)
    {
        bool bSTSActive = false;
        cFielder* pOwnerFielder = g_pBall->GetOwnerFielder();
        if (pOwnerFielder != nullptr)
        {
            if (g_pBall->GetOwnerFielder()->m_eActionState == ACTION_SHOOT_TO_SCORE)
            {
                bSTSActive = true;
            }
        }

        eGoalieActionState goalieState = (eGoalieActionState)((Goalie*)g_pCharacters[8])->mGoalieActionState;
        cCharacter* pAwayGoalie = g_pCharacters[9];

        for (int goalie = 0; !bSTSActive && goalie < 2; goalie++)
        {
            if (goalieState < GOALIEACTION_PASS)
            {
                if (goalieState >= GOALIEACTION_STS_SETUP)
                {
                    bSTSActive = true;
                }
            }
            goalieState = (eGoalieActionState)((Goalie*)pAwayGoalie)->mGoalieActionState;
        }

        if (!bSTSActive)
        {
            bool bChainChompInactive = BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->IsHidden();
            for (int team = 0; bChainChompInactive && team < 2; team++)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (g_pTeams[team]->GetPowerUpByIndex(i).eType == POWER_UP_CHAIN_CHOMP)
                    {
                        bChainChompInactive = false;
                        break;
                    }
                }
            }
            if (bChainChompInactive)
            {
                if (mBowserTimer.Countdown(deltaTime, 0.0f))
                {
                    BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->ActionInit();
                }
            }
        }
    }

    if (m_pPostGameDoneClock->m_clockState == CLOCK_DONE)
    {
        m_pPostGameDoneClock->Reset(0.0f, 1.4f, 1.0f);
        SidelineExplodableManager::DestroyAllActiveFragments(false);

        int awayScore = g_pTeams[1]->m_nScore;
        int homeScore = g_pTeams[0]->m_nScore;
        NisPlayer* pNisPlayer = NisPlayer::Instance();
        pNisPlayer->mWinnerSide[0] = awayScore > homeScore;

        if (!Presentation::Instance().DuringEndOfGamePresentation())
        {
            NisPlayer::Instance()->Reset();
            Presentation::Instance().Call("GameEndNoSuddenDeath", "");
        }
    }
}

/**
 * Offset/Address/Size: 0x508 | 0x8003CA7C | size: 0x9C
 */
void cGame::SetPotentialScorer(cPlayer* pPlayer)
{
    cPlayer* pOldScorer = m_pScorer;

    if (pOldScorer != nullptr && pPlayer != nullptr && pOldScorer != pPlayer && pOldScorer->IsOnSameTeam(pPlayer))
    {
        m_pAssister = m_pScorer;
    }
    else
    {
        m_pAssister = nullptr;
    }

    m_pScorer = pPlayer;

    if (pPlayer != nullptr && pPlayer->m_eClassType == FIELDER)
    {
        m_pTeamTouch[pPlayer->m_pTeam->m_nSide] = pPlayer;
    }
}

/**
 * Offset/Address/Size: 0x4DC | 0x8003CA50 | size: 0x2C
 */
void cGame::ChangeGameState(eGameState state)
{
    if (state != m_eGameState)
    {
        InitGameState(state);
    }
}

/**
 * Offset/Address/Size: 0x374 | 0x8003C8E8 | size: 0x168
 */
void cGame::InitGameState(eGameState state)
{
    // Check if transitioning from GS_GAMEPLAY to GS_OVERTIME
    if (m_eGameState == GS_GAMEPLAY && state == GS_OVERTIME)
    {
        g_pEventManager->CreateValidEvent(0xC, 0x14);
    }

    // Update the game state
    m_eGameState = state;

    // Handle state-specific logic
    switch (state)
    {
    case GS_KICKOFF:
        m_pGameClock->Stop();
        ResetForKickOff();
        break;

    case GS_PRE_GAME:
        m_pGameClock->Stop();
        break;

    case GS_POST_GOAL:
        m_pGameClock->Stop();
        // Loop through all teams and fielders
        for (int i = 0; i < 2; i++)
        {
            cTeam* pTeam = g_pTeams[i];
            for (int j = 0; j < 4; j++)
            {
                cFielder* pFielder = pTeam->GetFielder(j);
                pFielder->CleanUpPowerupEffect();
                pFielder->EndBlur();
            }
        }
        break;

    case GS_END_GAME:
        m_pPostGameDoneClock->Start();
        m_pGameClock->Stop();
        break;

    case GS_GAMEPLAY:
    case GS_OVERTIME:
        m_pGameClock->Start();
        // Loop through all teams and fielders
        for (int i = 0; i < 2; i++)
        {
            cTeam* pTeam = g_pTeams[i];
            for (int j = 0; j < 4; j++)
            {
                cFielder* pFielder = pTeam->GetFielder(j);
                // End desire if fielder is in WAIT state
                if (pFielder->m_eFielderDesireState == FIELDERDESIRE_WAIT)
                {
                    pFielder->EndDesire(false);
                }
            }
        }
        break;

    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x25C | 0x8003C7D0 | size: 0x118
 */
bool cGame::IsThoughtAllowed(unsigned long thought_id)
{
    bool bAllowedToThink = false;

    if (mThoughtsAllowedThisUpdate > 0)
    {
        ListEntry<unsigned long>* head = mThoughtsQueue.m_Head;
        if (head == NULL)
        {
            bAllowedToThink = true;
        }
        else if (thought_id == head->data)
        {
            ListEntry<unsigned long>* removed = nlListRemoveStart<ListEntry<unsigned long> >(&mThoughtsQueue.m_Head, &mThoughtsQueue.m_Tail);
            unsigned long temp;
            if (&temp != NULL)
            {
                temp = removed->data;
            }
            ::operator delete(removed);
            bAllowedToThink = true;
        }
    }

    if (!bAllowedToThink)
    {
        ListEntry<unsigned long>* node = mThoughtsQueue.m_Head;
        bool bFound = false;
        while (node != NULL)
        {
            if (thought_id == node->data)
            {
                bFound = true;
                break;
            }
            node = node->next;
        }

        if (!bFound)
        {
            ListEntry<unsigned long>* newEntry = (ListEntry<unsigned long>*)nlMalloc(sizeof(ListEntry<unsigned long>), 8, false);
            if (newEntry != NULL)
            {
                newEntry->next = NULL;
                newEntry->data = thought_id;
            }
            nlListAddEnd(&mThoughtsQueue.m_Head, &mThoughtsQueue.m_Tail, newEntry);
        }
    }

    if (bAllowedToThink)
    {
        mThoughtsAllowedThisUpdate--;
    }

    return bAllowedToThink;
}

/**
 * Offset/Address/Size: 0x1A0 | 0x8003C714 | size: 0xBC
 */
bool cGame::AbortPendingThought(unsigned long thoughtHash)
{
    struct ThoughtEntry
    {
        ThoughtEntry* next;
        u32 hash;
    };

    ThoughtEntry* head = (ThoughtEntry*)*(void**)((u8*)this + 0x50);
    ThoughtEntry** pHead = (ThoughtEntry**)((u8*)this + 0x50);
    ThoughtEntry** pTail = (ThoughtEntry**)((u8*)this + 0x54);

    if (head != NULL)
    {
        // Check if the first entry matches
        if (thoughtHash == head->hash)
        {
            ThoughtEntry* newHead;
            if (head == *pTail)
            {
                *pTail = newHead = 0;
            }
            else
            {
                newHead = head->next;
            }

            delete *pHead;
            *pHead = newHead;
        }
        else
        {
            // Search through rest of list
            ThoughtEntry* prev = head;
            ThoughtEntry* curr = head->next;

            while (curr != NULL)
            {
                if (thoughtHash == curr->hash)
                {
                    // Remove this entry
                    prev->next = curr->next;

                    if (curr == *pTail)
                    {
                        *pTail = prev;
                    }

                    delete curr;
                    break;
                }

                prev = curr;
                curr = curr->next;
            }
        }
    }

    return true;
}

/**
 * Offset/Address/Size: 0x0 | 0x8003C574 | size: 0x1A0
 */
void cGame::SetDifficulty(eDifficultyID diff0, eDifficultyID diff1, eDifficultyID diff2)
{
    if (diff0 != DIFF_DEFAULT)
    {
        g_pTeams[0]->SetDifficulty(diff0);
    }

    if (diff1 != DIFF_DEFAULT)
    {
        g_pTeams[1]->SetDifficulty(diff1);
    }

    if (diff2 != DIFF_DEFAULT)
    {
        SkillTweaks skillTweaks;
        SkillTweaks* pSkillTweaks;

        skillTweaks.Init(diff2, false);

        for (int i = 0; i < 2; i++)
        {
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotValue1 = skillTweaks.fShotValue1;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotValue2 = skillTweaks.fShotValue2;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotValue3 = skillTweaks.fShotValue3;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotChance0 = skillTweaks.fShotChance0;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotChance1 = skillTweaks.fShotChance1;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotChance2 = skillTweaks.fShotChance2;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotChance3 = skillTweaks.fShotChance3;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fShotChance4 = skillTweaks.fShotChance4;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fSTSWindupTime = skillTweaks.fSTSWindupTime;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fAttackCarrierDistance = skillTweaks.fAttackCarrierDistance;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fLooseBallChaseDistance = skillTweaks.fLooseBallChaseDistance;
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pTeams[i]->m_nSide);
            pSkillTweaks->fGoalieCanInterceptPass = skillTweaks.fGoalieCanInterceptPass;
        }
    }
}
