#ifndef _COMMONSCRIPT_H_
#define _COMMONSCRIPT_H_

#include "NL/nlSingleton.h"
#include "NL/nlAVLTreeSlotPool.h"
#include "Game/AI/FuzzyVariant.h"
#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/msl_tree.h"

#include "Game/AI/Scripts/SaveConfidence.h"

class cTeam;
class cPlayer;
class cFielder;

extern unsigned char g_bScriptQuestionCachingOn;
extern unsigned char g_bScriptQuestionCachingUseSTD;

namespace Fuzzy
{
FuzzyVariant GetStrategicBallCarrier(cTeam*);
FuzzyVariant GetBestBallInterceptor(cTeam*);
FuzzyVariant GetSwapControllerScore(cPlayer*);
FuzzyVariant ShouldIStrafeBall(cFielder*);
FuzzyVariant ShouldIStrafeMark(cFielder*);
FuzzyVariant ShouldIMarkBallOwner(cFielder*);
FuzzyVariant ShouldIAttemptOneTimer(cFielder*);
FuzzyVariant GetBestLooseBallPassTarget(cFielder*);
FuzzyVariant GetBestPassTarget(cPlayer*);
FuzzyVariant GoodPassTargetFrom(cFielder*, cFielder*);
FuzzyVariant GetBestHitTarget(cFielder*);
FuzzyVariant GetPassDirection(cPlayer*, cPlayer*);
FuzzyVariant GoodToShoot(cFielder*);
FuzzyVariant GoodToChipShot(cFielder*);
FuzzyVariant GetBestPassReceiveAction(cFielder*);
FuzzyVariant GetBestLooseBallAction(cFielder*);
FuzzyVariant GetBestWindupShotAction(cFielder*);
FuzzyVariant GetPowerupToUseForPassReceiveDefence(cFielder*);
FuzzyVariant GetPowerupToUseForWindupDefence(cFielder*);
FuzzyVariant InDanger(cFielder*);
FuzzyVariant InDangerDelayed(cFielder*);
FuzzyVariant GoalieAndGonnaPickupBall(cPlayer*);
} // namespace Fuzzy

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
