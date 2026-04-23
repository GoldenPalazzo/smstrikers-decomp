#ifndef _GAME_H_
#define _GAME_H_

#include "NL/nlMath.h"
#include "NL/nlList.h"
#include "Game/Player.h"

#include "Game/GameTweaks.h"
#include "Game/ScriptTuning.h"

static void DestroyPowerups();
static void DestroyGame();
static void CreateGame();

class Clock;
class BaseTarget
{
public:
    BaseTarget();
    ~BaseTarget();
};

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
    cGame();
    virtual ~cGame();
    void DoPerfectPassSlowDown();
    float GetNormalizedGameTime();
    float GetGameTime();
    void ResetForKickOff();
    static void PostResetCallback(unsigned long, unsigned long);
    void BeginGame(bool, bool);
    void CheckForGoal();
    void BlowUpPowerups(const nlVector3&, float);
    void ResetPowerups(bool);
    void ResetBowser();
    void ResetBowserTimer(float);
    void PreUpdate(float);
    void Update(float);
    void SetPotentialScorer(cPlayer*);
    void ChangeGameState(eGameState);
    void InitGameState(eGameState);
    bool IsThoughtAllowed(unsigned long);
    bool AbortPendingThought(unsigned long);
    void SetDifficulty(eDifficultyID, eDifficultyID, eDifficultyID);

    inline bool IsGameplayOrOvertime()
    {
        return (m_eGameState == GS_GAMEPLAY || m_eGameState == GS_OVERTIME);
    }

    inline eGameState GetGameState() const { return m_eGameState; }

    /* 0x04 */ GameTweaks* m_pGameTweaks;
    /* 0x08 */ FuzzyTweaks* m_pFuzzyTweaks;
    /* 0x0C */ Clock* m_pGameClock;
    /* 0x10 */ bool m_bBallInNet;
    /* 0x14 */ Clock* m_pPostResetClock;
    /* 0x18 */ Clock* m_pPostGameDoneClock;
    /* 0x1C */ float m_fGameDuration;
    /* 0x20 */ int m_nLastTeamToScore;
    /* 0x24 */ eGameState m_eGameState;
    /* 0x28 */ BaseTarget* m_pTarget;
    /* 0x2C */ cPlayer* m_pScorer;
    /* 0x30 */ cPlayer* m_pAssister;
    /* 0x34 */ cPlayer* m_pTeamTouch[2];
    /* 0x3C */ cPlayer** m_pRandomPlayersArray;
    /* 0x40 */ bool mbCaptainShotToScoreOn;
    /* 0x41 */ bool mIsPure;
    /* 0x42 */ bool mInSuddenDeath;
    /* 0x44 */ Timer mBowserTimer;
    /* 0x48 */ f32 mfCheatTilt;
    /* 0x4C */ nlListContainer<unsigned long> mThoughtsQueue;
    /* 0x58 */ int mThoughtsAllowedThisUpdate;
}; // total size: 0x5C

extern cGame* g_pGame;

// class ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long>>>
// {
// public:
//     void DeleteEntry(ListEntry<unsigned long>*);
// };

// class GameTweaks
// {
// public:
//     ~GameTweaks();
// };

// class nlSingleton<ScriptQuestionCache>
// {
// public:
//     void DestroyInstance();
// };

// class ScriptQuestionCache
// {
// public:
//     void Clear();
//     ~ScriptQuestionCache();
// };

// class nlAVLTreeSlotPool<unsigned long, FuzzyVariant, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTreeSlotPool();
// };

// class AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void AllocateEntry(void*, void*);
//     void CompareKey(void*, AVLTreeNode*);
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CastUp(AVLTreeNode*) const;
//     void PostorderTraversal(AVLTreeEntry<unsigned long, FuzzyVariant>*, void (AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, FuzzyVariant>*));
//     void DestroyTree(void (AVLTreeBase<unsigned long, FuzzyVariant, BasicSlotPool<AVLTreeEntry<unsigned long, FuzzyVariant>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, FuzzyVariant>*));
//     void Clear();
//     void ~AVLTreeBase();
// };

// class Config
// {
// public:
//     void TagValuePair::Get<BasicString<char, Detail::TempStringAllocator>>() const;
//     void Get<BasicString<char, Detail::TempStringAllocator>>(const char*, BasicString<char, Detail::TempStringAllocator>);
// };

// class operator==<char, Detail
// {
// public:
//     void TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&, const char*);
// };

// class std
// {
// public:
//     void allocator<std::pair<const unsigned long, FuzzyVariant>>::destroy(std::pair<const unsigned long, FuzzyVariant>*);
//     void allocator<std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::node>::deallocate(std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::node*, unsigned long);
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::__tree(const std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare&, const std::allocator<std::pair<const unsigned long, FuzzyVariant>>&);
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::clear();
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::alloc();
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::node_alloc();
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::destroy(std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::node*);
// };

// class GoalScoredData
// {
// public:
//     void GetID();
// };

// class cCameraManager
// {
// public:
//     void GetCamera<GameplayCamera>(eCameraType);
// };

// class nlWalkList<ListEntry<unsigned long>, ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long>>>>(ListEntry<unsigned long>*, ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long>>>*, void (ListContainerBase<unsigned long, NewAdapter<ListEntry<unsigned long>>>
// {
// public:
//     void *)(ListEntry<unsigned long>*));
// };

// class Metrowerks
// {
// public:
//     void details::compressed_pair_imp<std::allocator<std::pair<const unsigned long, FuzzyVariant>>, unsigned long, 1>::first();
//     void details::compressed_pair_imp<std::allocator<std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::node>, std::__red_black_tree<1>::anchor, 1>::first();
// };

#endif // _GAME_H_
