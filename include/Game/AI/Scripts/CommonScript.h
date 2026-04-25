#ifndef _COMMONSCRIPT_H_
#define _COMMONSCRIPT_H_

#include "NL/nlSingleton.h"
#include "NL/nlAVLTreeSlotPool.h"
#include "Game/AI/FuzzyVariant.h"
#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/msl_tree.h"

class cTeam;
class cPlayer;
class cFielder;

extern unsigned char g_bScriptQuestionCachingOn;
extern unsigned char g_bScriptQuestionCachingUseSTD;

class Fuzzy
{
public:
    static FuzzyVariant GetStrategicBallCarrier(cTeam*);
    static FuzzyVariant GetBestBallInterceptor(cTeam*);
    static FuzzyVariant GetSwapControllerScore(cPlayer*);
    static FuzzyVariant ShouldIStrafeBall(cFielder*);
    static FuzzyVariant ShouldIStrafeMark(cFielder*);
    static FuzzyVariant ShouldIMarkBallOwner(cFielder*);
    static FuzzyVariant ShouldIAttemptOneTimer(cFielder*);
    static FuzzyVariant GetBestLooseBallPassTarget(cFielder*);
    static FuzzyVariant GetBestPassTarget(cPlayer*);
    static FuzzyVariant GoodPassTargetFrom(cFielder*, cFielder*);
    static FuzzyVariant GetBestHitTarget(cFielder*);
    static FuzzyVariant GetPassDirection(cPlayer*, cPlayer*);
    static FuzzyVariant GoodToShoot(cFielder*);
    static FuzzyVariant GoodToChipShot(cFielder*);
    static FuzzyVariant GetBestPassReceiveAction(cFielder*);
    static FuzzyVariant GetBestLooseBallAction(cFielder*);
    static FuzzyVariant GetBestWindupShotAction(cFielder*);
    static FuzzyVariant GetPowerupToUseForPassReceiveDefence(cFielder*);
    static FuzzyVariant GetPowerupToUseForWindupDefence(cFielder*);
    static FuzzyVariant InDanger(cFielder*);
    static FuzzyVariant InDangerDelayed(cFielder*);
    static FuzzyVariant GoalieAndGonnaPickupBall(cPlayer*);
};

// class FuzzyVariant
// {
// public:
//     FuzzyVariant(const FuzzyVariant&);
// };

class SaveConfidence
{
public:
    SaveConfidence(float* pFloat)
        : m_savedValue(*pFloat)
        , m_pFloat(pFloat)
    {
    }
    ~SaveConfidence()
    {
        *m_pFloat = m_savedValue;
    }

    float m_savedValue;
    float* m_pFloat;
};

#ifndef _SCRIPTQUESTIONCACHE_DEFINED_
#define _SCRIPTQUESTIONCACHE_DEFINED_
class ScriptQuestionCache : public nlSingleton<ScriptQuestionCache>
{
public:
    ScriptQuestionCache()
        : mQuestionCacheMap(16, 16)
    {
    }
    ~ScriptQuestionCache();
    unsigned char Lookup(unsigned long, FuzzyVariant&, const char*);
    const FuzzyVariant& AddToCache(unsigned long, const FuzzyVariant&, const char*);
    void Clear();

    /* 0x00 */ nlAVLTreeSlotPool<unsigned long, FuzzyVariant, DefaultKeyCompare<unsigned long> > mQuestionCacheMap;
    /* 0x28 */ std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > > mQuestionCacheMapSTD;
    /* 0x38 */ int mTotalLookups;
    /* 0x3C */ int mCacheHits;
};
#endif

// class std
// {
// public:
//     void __red_black_tree<1>::rotate_left(std::__red_black_tree<1>::node_base*, std::__red_black_tree<1>::node_base*&);
//     void __red_black_tree<1>::rotate_right(std::__red_black_tree<1>::node_base*, std::__red_black_tree<1>::node_base*&);
//     void __red_black_tree<1>::balance_insert(std::__red_black_tree<1>::node_base*, std::__red_black_tree<1>::node_base*);
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::insert_node_at(std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::node*, bool, bool, const std::pair<const unsigned long, FuzzyVariant>&);
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::find<unsigned long>(const unsigned long&);
//     void __tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::find_or_insert<unsigned long, FuzzyVariant>(const unsigned long&);
// };

#endif // _COMMONSCRIPT_H_
