#include "Game/AI/Scripts/CommonScript.h"

#include "Game/Team.h"
#include "Game/Player.h"
#include "Game/AI/Fielder.h"
#include "Game/Ball.h"
#include "Game/Field.h"

extern "C" double fabs(double);

float InBetweenMyNetAnd(cFielder*, cFielder*);
float InBetweenMyNetAnd(cFielder*, cBall*);
float AbleToInterceptBall(cPlayer*);
float AbleToInterceptBallForSwapController(cFielder*);
float ClosingTo(cPlayer*, cBall*);
float CloseToBall(cPlayer*);
float GoalieType(cPlayer*);
float StrategicBallOwner(cFielder*);
float BallOwner(cPlayer*);
float BallOwnerT(cTeam*);
float UserControlled(cFielder*);
float Defensive(cTeam*);
float Offensive(cTeam*);
float InOffensiveZone(cPlayer*);
float FarToMyNet(cPlayer*);
float FarToTheirNet(cPlayer*);
float FarToBall(cPlayer*);
float ReceivingPass(cFielder*);
float ReceivingVolleyPass(cPlayer*);
float NormalizeVal(float fromVal, float fromMin, float fromMax);

extern cBall* g_pScriptBall;
extern cTeam* g_pScriptCurrentTeam;
extern cTeam* g_pScriptOtherTeam;

// /**
//  * Offset/Address/Size: 0x654 | 0x8007A56C | size: 0x80
//  */
// void std::__red_black_tree<1>::rotate_left(std::__red_black_tree<1>::node_base*, std::__red_black_tree<1>::node_base*&)
// {
// }

// /**
//  * Offset/Address/Size: 0x5D4 | 0x8007A4EC | size: 0x80
//  */
// void std::__red_black_tree<1>::rotate_right(std::__red_black_tree<1>::node_base*, std::__red_black_tree<1>::node_base*&)
// {
// }

// /**
//  * Offset/Address/Size: 0x3E4 | 0x8007A2FC | size: 0x1F0
//  */
// void std::__red_black_tree<1>::balance_insert(std::__red_black_tree<1>::node_base*, std::__red_black_tree<1>::node_base*)
// {
// }

// /**
//  * Offset/Address/Size: 0x234 | 0x8007A14C | size: 0x1B0
//  */
// void std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::insert_node_at(std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::node*, bool, bool, const std::pair<const unsigned long, FuzzyVariant>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1D0 | 0x8007A0E8 | size: 0x64
//  */
// void std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::find<unsigned long>(const unsigned long&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80079F18 | size: 0x1D0
//  */
// void std::__tree<std::pair<const unsigned long, FuzzyVariant>, std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::value_compare, std::allocator<std::pair<const unsigned long, FuzzyVariant>>>::find_or_insert<unsigned long, FuzzyVariant>(const unsigned long&)
// {
// }

struct StdMapNodeBase
{
    void* left;
    void* right;
    void* parent;
};

struct StdMapTree
{
    unsigned long x0;
    StdMapNodeBase x4;
};

struct StdMapNode
{
    StdMapNodeBase base;
    unsigned long key;
    FuzzyVariant value;
};

extern "C" void __find(StdMapNode** outNode, void* tree, const unsigned long* key);

/**
 * Offset/Address/Size: 0xE4 | 0x80079D64 | size: 0x1B4
 * TODO: Remaining diff is std::tree find call symbol (__find wrapper vs templated std::__tree::find<Ul>)
 */
unsigned char ScriptQuestionCache::Lookup(unsigned long hash, FuzzyVariant& returnVal, const char* name)
{
    FuzzyVariant* pValue;
    StdMapNode* stdNode;

    mTotalLookups++;

    if (g_bScriptQuestionCachingUseSTD)
    {
        __find(&stdNode, &mQuestionCacheMapSTD, &hash);

        StdMapNode* stdFound = stdNode;
        if ((StdMapNodeBase*)stdFound != &((StdMapTree*)&mQuestionCacheMapSTD)->x4)
        {
            mCacheHits++;
            returnVal = stdFound->value;
            return 1;
        }
    }
    else
    {
        AVLTreeEntry<unsigned long, FuzzyVariant>* node = mQuestionCacheMap.m_Root;
        unsigned long key = hash;
        unsigned char found;

        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
            {
                cmpResult = 0;
            }
            else if (key < node->key)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                if (&pValue != NULL)
                {
                    pValue = &node->value;
                }
                found = 1;
                goto found_done;
            }
            if (cmpResult < 0)
            {
                node = (AVLTreeEntry<unsigned long, FuzzyVariant>*)node->node.left;
            }
            else
            {
                node = (AVLTreeEntry<unsigned long, FuzzyVariant>*)node->node.right;
            }
        }

        found = 0;

    found_done:

        if (found)
        {
            mCacheHits++;
            returnVal = *pValue;
            return 1;
        }
    }

    return 0;
}

// Stub for find_or_insert result (std::pair<const unsigned long, FuzzyVariant> in map)
struct FuzzyMapPair
{
    unsigned long key;
    FuzzyVariant value;
};

extern "C" FuzzyMapPair* __find_or_insert(void* tree, const unsigned long* key);

/**
 * Offset/Address/Size: 0x0 | 0x80079C80 | size: 0xE4
 */
const FuzzyVariant& ScriptQuestionCache::AddToCache(unsigned long key, const FuzzyVariant& variant, const char* name)
{
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            // TODO: Implement all this std stuff..
            FuzzyMapPair* pair = __find_or_insert(&mQuestionCacheMapSTD, &key);
            pair->value = variant;
        }
        else
        {
            AVLTreeNode* existingNode;
            mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&mQuestionCacheMap.m_Root, (void*)&key, (void*)&variant, &existingNode, mQuestionCacheMap.m_NumElements);
            if (existingNode == NULL)
            {
                mQuestionCacheMap.m_NumElements++;
            }
        }
    }
    return variant;
}

/**
 * Offset/Address/Size: 0x0 | 0x80079B54 | size: 0xE4
 */
// FuzzyVariant::FuzzyVariant(const FuzzyVariant&)
// {
// }

/**
 * Offset/Address/Size: 0xF1B0 | 0x80079380 | size: 0x7D4
 * TODO: 89.38% match - 4-byte stack offset shift from funcAddr scheduling,
 * __find vs find<Ul> symbol diff, r31 register allocation for hash
 */
FuzzyVariant Fuzzy::GetStrategicBallCarrier(cTeam* TheTeam)
{
    FuzzyVariant bestValue;
    float confidence = 1.0f;
    float bestConfidence = 0.0f;

    FuzzyVariant fvTeam(TheTeam);
    unsigned long hash = (unsigned long)GetStrategicBallCarrier + ((Variant*)&fvTeam)->GetHash();
    FuzzyVariant fvTeam2(TheTeam);

    ScriptQuestionCache* cache = ScriptQuestionCache::Instance();
    cache->mTotalLookups++;

    unsigned char lookupFound = 0;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        StdMapNode* stdNode;
        __find(&stdNode, &cache->mQuestionCacheMapSTD, &hash);
        StdMapNode* stdFound = stdNode;
        if ((StdMapNodeBase*)stdFound != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = stdFound->value;
            lookupFound = 1;
        }
    }
    else
    {
        AVLTreeEntry<unsigned long, FuzzyVariant>* node = cache->mQuestionCacheMap.m_Root;
        unsigned long key = hash;
        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
                cmpResult = 0;
            else if (key < node->key)
                cmpResult = -1;
            else
                cmpResult = 1;
            if (cmpResult == 0)
            {
                if (&pValue != NULL)
                    pValue = &node->value;
                lookupFound = 1;
                goto found_done;
            }
            if (cmpResult < 0)
                node = (AVLTreeEntry<unsigned long, FuzzyVariant>*)node->node.left;
            else
                node = (AVLTreeEntry<unsigned long, FuzzyVariant>*)node->node.right;
        }
    found_done:
        if (lookupFound)
        {
            cache->mCacheHits++;
            bestValue = *pValue;
        }
    }

    if (lookupFound)
    {
        unsigned long hashCopy1 = hash;
        if (g_bScriptQuestionCachingOn)
        {
            if (g_bScriptQuestionCachingUseSTD)
            {
                FuzzyMapPair* pair = __find_or_insert(&cache->mQuestionCacheMapSTD, &hashCopy1);
                pair->value = bestValue;
            }
            else
            {
                AVLTreeNode* existingNode1;
                cache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&cache->mQuestionCacheMap.m_Root, (void*)&hashCopy1, (void*)&bestValue, &existingNode1, cache->mQuestionCacheMap.m_NumElements);
                if (existingNode1 == NULL)
                    cache->mQuestionCacheMap.m_NumElements++;
            }
        }
        return bestValue;
    }

    for (int i = 0; i < 4; i++)
    {
        cFielder* fielder = TheTeam->GetFielder(i);
        float score = StrategicBallOwner(fielder);
        float complement = 1.0f - score;
        float minVal = (score <= complement) ? score : complement;
        float maxVal = (score >= complement) ? score : complement;
        float ratio = minVal / maxVal;
        if (score > 0.0f)
        {
            SaveConfidence sc(&confidence);
            if (confidence > score)
                confidence = score;
            if (confidence < score && score < 0.5f)
                confidence = (float)confidence * ratio;
            if (confidence > bestConfidence)
            {
                bestConfidence = confidence;
                bestValue = FuzzyVariant((cPlayer*)fielder);
            }
        }
    }

    bestValue.Confidence = bestConfidence;
    unsigned long hashCopy2 = hash;
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            FuzzyMapPair* pair = __find_or_insert(&cache->mQuestionCacheMapSTD, &hashCopy2);
            pair->value = bestValue;
        }
        else
        {
            AVLTreeNode* existingNode2;
            cache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&cache->mQuestionCacheMap.m_Root, (void*)&hashCopy2, (void*)&bestValue, &existingNode2, cache->mQuestionCacheMap.m_NumElements);
            if (existingNode2 == NULL)
                cache->mQuestionCacheMap.m_NumElements++;
        }
    }

    return bestValue;
}

/**
 * Offset/Address/Size: 0xE9DC | 0x80078BAC | size: 0x7D4
 * TODO: 92.39% match - stack layout +4 offset, r30/r31 register swap for hash
 * vs bestValue ptr, hash computation ordering (funcAddr after GetHash), all
 * from -inline deferred inlining Lookup/AddToCache/SaveConfidence
 */
FuzzyVariant Fuzzy::GetBestBallInterceptor(cTeam* TheTeam)
{
    FuzzyVariant bestValue;
    float confidence = 1.0f;
    float bestConfidence = 0.0f;

    FuzzyVariant fvTeam(TheTeam);
    unsigned long hash = (unsigned long)GetBestBallInterceptor + ((Variant*)&fvTeam)->GetHash();
    FuzzyVariant fvTeam2(TheTeam);

    if (ScriptQuestionCache::Instance()->Lookup(hash, bestValue, "GetBestBallInterceptor"))
    {
        ScriptQuestionCache::Instance()->AddToCache(hash, bestValue, "GetBestBallInterceptor");
        return bestValue;
    }

    for (int i = 0; i < 4; i++)
    {
        cFielder* fielder = TheTeam->GetFielder(i);
        float score = AbleToInterceptBall((cPlayer*)fielder);
        float complement = 1.0f - score;
        float minVal = (score <= complement) ? score : complement;
        float maxVal = (score >= complement) ? score : complement;
        float ratio = minVal / maxVal;

        if (score > 0.0f)
        {
            SaveConfidence sc(&confidence);
            if (confidence > score)
                confidence = score;
            if (confidence < score && score < 0.5f)
                confidence = (float)confidence * ratio;
            if (confidence > bestConfidence)
            {
                bestConfidence = confidence;
                bestValue = FuzzyVariant((cPlayer*)fielder);
            }
        }
    }

    bestValue.Confidence = bestConfidence;
    ScriptQuestionCache::Instance()->AddToCache(hash, bestValue, "GetBestBallInterceptor");
    return bestValue;
}

/**
 * Offset/Address/Size: 0xE40C | 0x800785DC | size: 0x5D0
 * TODO: 95.26% match - stack offset diffs for FuzzyVariant temps and SRP
 * slot from -inline deferred vs -inline auto compilation
 */
FuzzyVariant Fuzzy::GetSwapControllerScore(cPlayer* ThePlayer)
{
    FuzzyVariant bestValue;

    FuzzyVariant fvPlayer((cPlayer*)ThePlayer);
    ((Variant*)&fvPlayer)->GetHash();

    FuzzyVariant fvPlayer2((cPlayer*)ThePlayer);

    unsigned char flag = 0;
    float weightedSum = 0.0f;
    float totalWeight = 0.0f;
    float passWeight = 0.0f;

    cFielder* passTarget = (cFielder*)g_pBall->GetPassTargetFielder();

    if (ReceivingPass(passTarget) && passTarget != (cFielder*)ThePlayer)
    {
        cTeam* targetTeam = passTarget != NULL ? passTarget->m_pTeam : NULL;
        cTeam* playerTeam = ThePlayer != NULL ? ThePlayer->m_pTeam : NULL;

        if (playerTeam != targetTeam)
        {
            flag = 1;
            if (ReceivingVolleyPass((cPlayer*)passTarget))
            {
                passWeight = 2.0f;
            }
            else
            {
                passWeight = 1.5f;
            }
        }
    }

    cTeam* team = ThePlayer != NULL ? ThePlayer->m_pTeam : NULL;
    if (team->GetNumAssignedControllers() > 1)
    {
        team = ThePlayer != NULL ? ThePlayer->m_pTeam : NULL;
        if (BallOwnerT(team) && ThePlayer->m_eClassType == FIELDER)
        {
            Fuzzy::GoodToShoot((cFielder*)ThePlayer);
            float weight = 0.5f;
            flag = 0;
            totalWeight += weight;
            weightedSum += weight * bestValue.mData.f;
        }
    }

    if (flag)
    {
        float dt = 0.1f;
        float px = ThePlayer->m_v3Position.f.x + dt * ThePlayer->m_v3Velocity.f.x;
        float tx = passTarget->m_v3Position.f.x + dt * passTarget->m_v3Velocity.f.x;
        float pz = ThePlayer->m_v3Position.f.y + dt * ThePlayer->m_v3Velocity.f.y;
        float tz = passTarget->m_v3Position.f.y + dt * passTarget->m_v3Velocity.f.y;
        float dx = px - tx;
        float dy = pz - tz;
        float dist = nlSqrt(dx * dx + dy * dy, true);
        float maxDist = 2.0f * cField::mv3FieldPosition.f.x;
        float range = 0.5f * maxDist;
        float normalized = NormalizeVal(dist, range, 0.0f);
        weightedSum += normalized * passWeight;
        totalWeight += passWeight;
    }

    if (ThePlayer->m_eClassType == FIELDER)
    {
        float intercept = AbleToInterceptBallForSwapController((cFielder*)ThePlayer);
        weightedSum += intercept;
        totalWeight += 1.0f;
        float offensive = InOffensiveZone(ThePlayer);
        float defense = 1.0f - offensive;
        team = ThePlayer != NULL ? ThePlayer->m_pTeam : NULL;
        float defResult = Defensive(team);
        if (!(defResult <= defense))
            defResult = defense;
        if (defResult)
        {
            float inBetween = InBetweenMyNetAnd((cFielder*)ThePlayer, g_pBall);
            float weight = 0.175f;
            weightedSum += weight * inBetween;
            totalWeight += weight;
        }
        team = ThePlayer != NULL ? ThePlayer->m_pTeam : NULL;
        float notBallOwner = 1.0f - BallOwnerT(team);
        if (notBallOwner)
            ClosingTo(ThePlayer, g_pBall);
    }

    float result = 0.0f;
    if (totalWeight > 0.0f)
        result = weightedSum / totalWeight;

    unsigned char isIdle = 0;
    float absVal = (float)fabs((double)ThePlayer->m_v3ScreenPosition.f.x);
    if (absVal <= 1.0f)
    {
        absVal = (float)fabs((double)ThePlayer->m_v3ScreenPosition.f.y);
        if (absVal <= 1.0f)
            isIdle = 1;
    }

    FuzzyVariant fvResult(result);
    bestValue = fvResult;
    bestValue.Confidence = 1.0f;
    return bestValue;
}

/**
 * Offset/Address/Size: 0xDC78 | 0x80077E48 | size: 0x794
 * TODO: 98.36% match - context-induced diffs: fcmpu operand order (6x),
 * FuzzyVariant ctor f29/.sdata scheduling (2x), fmuls/fmadds operand order (2x)
 */
FuzzyVariant Fuzzy::ShouldIStrafeBall(cFielder* TheFielder)
{
    FuzzyVariant bestValue;

    float confidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();

    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    if (StrategicBallOwner(TheFielder) == 0.0f)
    {
        if (BallOwner(g_pScriptCurrentTeam->GetGoalie()) != 0.0f || BallOwner(g_pScriptOtherTeam->GetGoalie()) != 0.0f)
        {
            confidence = 1.0f;
            FuzzyVariant fvResult(confidence);
            bestValue = fvResult;
        }
        else if (UserControlled(TheFielder) == 0.0f && TheFielder->m_fDesiredSpeed < 0.1f)
        {
            confidence = 1.0f;
            FuzzyVariant fvResult(1.0f);
            bestValue = fvResult;
        }
        else
        {
            cTeam* team = TheFielder != NULL ? TheFielder->m_pTeam : NULL;
            if (Defensive(team) != 0.0f)
            {
                confidence = 1.0f;
                float farToMyNet = FarToMyNet(TheFielder);
                float inBetween = InBetweenMyNetAnd(TheFielder, g_pScriptBall);
                float a = (1.0f - farToMyNet) * 0.5f;
                FuzzyVariant fvResult(a + inBetween * 0.5f);
                bestValue = fvResult;
            }
            else
            {
                team = TheFielder != NULL ? TheFielder->m_pTeam : NULL;
                if (Offensive(team) != 0.0f)
                {
                    confidence = 1.0f;
                    float farToTheirNet = FarToTheirNet(TheFielder);
                    FuzzyVariant fvResult(1.0f - farToTheirNet);
                    bestValue = fvResult;
                }
                else
                {
                    confidence = 1.0f;
                    float farToBall = FarToBall(TheFielder);
                    FuzzyVariant fvResult(1.0f - farToBall);
                    bestValue = fvResult;
                }
            }
        }
    }

    bestValue.Confidence = confidence;
    return bestValue;
}

/**
 * Offset/Address/Size: 0xD92C | 0x80077AFC | size: 0x34C
 * TODO: 90.9% match - MWCC store scheduling: mType/mData stores placed after ExtraData.Reset()
 * bctrl instead of before. Same issue in all 3 FuzzyVariant template ctor inlines.
 */
FuzzyVariant Fuzzy::ShouldIStrafeMark(cFielder* TheFielder)
{
    FuzzyVariant bestValue;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();

    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    cFielder* mark = TheFielder != NULL ? TheFielder->m_pMark : NULL;
    float inBetween = InBetweenMyNetAnd(TheFielder, mark);

    FuzzyVariant fvResult(inBetween);

    bestValue = fvResult;
    bestValue.Confidence = 1.0f;

    return bestValue;
}

/**
 * Offset/Address/Size: 0xD188 | 0x80077358 | size: 0x7A4
 */
FuzzyVariant Fuzzy::ShouldIMarkBallOwner(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0xC144 | 0x80076314 | size: 0x1044
 */
FuzzyVariant Fuzzy::ShouldIAttemptOneTimer(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0xB89C | 0x80075A6C | size: 0x8A8
 */
FuzzyVariant Fuzzy::GetBestLooseBallPassTarget(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0xAC34 | 0x80074E04 | size: 0xC68
 */
FuzzyVariant Fuzzy::GetBestPassTarget(cPlayer*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0xA138 | 0x80074308 | size: 0xAFC
 */
FuzzyVariant Fuzzy::GoodPassTargetFrom(cFielder*, cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x95A0 | 0x80073770 | size: 0xB98
 */
FuzzyVariant Fuzzy::GetBestHitTarget(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x8D80 | 0x80072F50 | size: 0x820
 */
/**
 * Offset/Address/Size: 0x798 | 0x80072F50 | size: 0x620
 * TODO: 85.68% match - f29 register allocation for fBranchRatio (stuck in f3),
 *       stmw r27 vs stw r28-r31 (stack frame 0x180 vs 0x170),
 *       r28 constant pre-load scheduling before bctrl
 */
FuzzyVariant Fuzzy::GetPassDirection(cPlayer* pFromPlayer, cPlayer* pTargetPlayer)
{
    extern float CloseToTheirNet(cPlayer*);
    extern float InDefensiveZone(cPlayer*);
    extern float InOffensiveZone(cPlayer*);
    extern float FarTo(cPlayer*, cPlayer*);
    extern float FGREATER(float, float);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = CloseToTheirNet(pTargetPlayer);
    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
            fConfidence = fConfidence * fBranchRatio;
        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            bestValue = FuzzyVariant(2);
        }
        if (fFalseConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            float inDefensiveZone = FGREATER(InDefensiveZone(pTargetPlayer), 0.7f);
            float inOffensiveZone = FGREATER(InOffensiveZone(pTargetPlayer), 0.5f);
            float fTrueConfidence = (inOffensiveZone >= inDefensiveZone) ? inOffensiveZone : inDefensiveZone;
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
                    fConfidence = fConfidence * fBranchRatio;
                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(0);
                }
                if (fFalseConfidence > 0.0f)
                {
                    SaveConfidence PushDOM(&fConfidence);
                    float fTrueConfidence = FarTo(pFromPlayer, pTargetPlayer);
                    float fFalseConfidence = 1.0f - fTrueConfidence;
                    float fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);
                    if (fTrueConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM(&fConfidence);
                        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
                            fConfidence = fConfidence * fBranchRatio;
                        if (fConfidence > fBestConfidence)
                        {
                            fBestConfidence = fConfidence;
                            bestValue = FuzzyVariant(3);
                        }
                        if (fFalseConfidence > 0.0f)
                        {
                            SaveConfidence PushDOM(&fConfidence);
                            fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
                            if ((fConfidence < fFalseConfidence) && (fFalseConfidence < 0.5f))
                                fConfidence = fConfidence * fBranchRatio;
                            if (fConfidence > fBestConfidence)
                            {
                                fBestConfidence = fConfidence;
                                bestValue = FuzzyVariant(1);
                            }
                        }
                    }
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;
    return bestValue;
}

/**
 * Offset/Address/Size: 0x801C | 0x800721EC | size: 0xD64
 */
FuzzyVariant Fuzzy::GoodToShoot(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x71F4 | 0x800713C4 | size: 0xE28
 */
FuzzyVariant Fuzzy::GoodToChipShot(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x57C4 | 0x8006F994 | size: 0x1A30
 */
FuzzyVariant Fuzzy::GetBestPassReceiveAction(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x3FD0 | 0x8006E1A0 | size: 0x17F4
 */
FuzzyVariant Fuzzy::GetBestLooseBallAction(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x2CB4 | 0x8006CE84 | size: 0x131C
 */
FuzzyVariant Fuzzy::GetBestWindupShotAction(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x1A48 | 0x8006BC18 | size: 0x126C
 */
FuzzyVariant Fuzzy::GetPowerupToUseForPassReceiveDefence(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x1620 | 0x8006B7F0 | size: 0x428
 * TODO: 99.64% match - remaining diffs are stack offset mismatches from FuzzyVariant copy ctor inlining
 */
FuzzyVariant Fuzzy::GetPowerupToUseForWindupDefence(cFielder* TheFielder)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();

    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    FuzzyVariant usePowerup = Fuzzy::GetPowerupToUseForPassReceiveDefence(TheFielder);

    float fTrueConfidence = usePowerup.Confidence;
    float fFalseConfidence = 1.0f - fTrueConfidence;

    float minC = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float maxC = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = minC / maxC;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            bestValue = usePowerup;
        }
    }

    bestValue.Confidence = fBestConfidence;
    return bestValue;
}

/**
 * Offset/Address/Size: 0xE64 | 0x8006B034 | size: 0x7BC
 */
FuzzyVariant Fuzzy::InDanger(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x3B4 | 0x8006A584 | size: 0xAB0
 */
FuzzyVariant Fuzzy::InDangerDelayed(cFielder*)
{
    FORCE_DONT_INLINE;
    return FuzzyVariant();
}

/**
 * Offset/Address/Size: 0x0 | 0x8006A1D0 | size: 0x3B4
 */
/**
 * TODO: 87.8% match - Same MWCC store scheduling quirk as ShouldIStrafeMark
 * (mType/mData stores after ExtraData.Reset bctrl in template ctor inlines)
 */
FuzzyVariant Fuzzy::GoalieAndGonnaPickupBall(cPlayer* ThePlayer)
{
    FuzzyVariant bestValue;

    FuzzyVariant fvPlayer((cPlayer*)ThePlayer);
    ((Variant*)&fvPlayer)->GetHash();

    FuzzyVariant fvPlayer2((cPlayer*)ThePlayer);

    float ableToIntercept = AbleToInterceptBall(ThePlayer);
    float closingTo = ClosingTo(ThePlayer, g_pScriptBall);
    float closeToBall = CloseToBall(ThePlayer);
    float goalieType = GoalieType(ThePlayer);

    closingTo = (closingTo <= ableToIntercept) ? closingTo : ableToIntercept;
    closeToBall = (closeToBall <= closingTo) ? closeToBall : closingTo;
    if (goalieType <= closeToBall)
        closeToBall = goalieType;

    FuzzyVariant fvResult(closeToBall);

    bestValue = fvResult;

    return bestValue;
}
