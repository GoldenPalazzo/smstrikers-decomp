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

// rotate_left, rotate_right defined in msl_tree.h, instantiated via CommonScript_stub()

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
extern cFielder* g_pScriptCurrentFielder;
extern cFielder* g_pScriptBallOwner;

float Marking(cFielder*, cPlayer*);
float UpfieldFrom(cPlayer*, cPlayer*);
float Incapacitated(cPlayer*);

FuzzyVariant Fuzzy::ShouldIMarkBallOwner(cFielder* pFielder)
{
    FuzzyVariant bestValue;
    float confidence = 1.0f;
    float bestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)pFielder);
    unsigned long hash = (unsigned long)ShouldIMarkBallOwner + ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)pFielder);

    if (ScriptQuestionCache::Instance()->Lookup(hash, bestValue, "ShouldIMarkBallOwner"))
    {
        ScriptQuestionCache::Instance()->AddToCache(hash, bestValue, "ShouldIMarkBallOwner");
        return bestValue;
    }

    float score = 1.0f - Marking(g_pScriptCurrentFielder, g_pScriptBallOwner);
    float complement = 1.0f - score;
    float minVal = (score <= complement) ? score : complement;
    float maxVal = (score >= complement) ? score : complement;
    float ratio = minVal / maxVal;

    if (score > 0.0f)
    {
        SaveConfidence sc1(&confidence);

        confidence = (confidence <= score) ? confidence : score;
        if (confidence < score && score < 0.5f)
        {
            confidence = (float)confidence * ratio;
        }

        cFielder* marker = g_pScriptBallOwner != NULL ? g_pScriptBallOwner->m_pMarker : NULL;
        float upfield = UpfieldFrom((cPlayer*)marker, (cPlayer*)g_pScriptBallOwner);

        marker = g_pScriptBallOwner != NULL ? g_pScriptBallOwner->m_pMarker : NULL;
        float incap = Incapacitated((cPlayer*)marker);

        float combined = (incap >= upfield) ? incap : upfield;
        float notCombined = 1.0f - combined;

        float minVal2 = (combined <= notCombined) ? combined : notCombined;
        float maxVal2 = (combined >= notCombined) ? combined : notCombined;
        float ratio2 = minVal2 / maxVal2;

        if (combined > 0.0f)
        {
            SaveConfidence sc2(&confidence);

            confidence = (confidence <= combined) ? confidence : combined;
            if (confidence < combined && combined < 0.5f)
            {
                confidence = (float)confidence * ratio2;
            }

            if (confidence > 0.0f)
            {
                bestConfidence = confidence;
                FuzzyVariant returnValue(confidence);
                bestValue = returnValue;
            }
        }

        if (notCombined > 0.0f)
        {
            SaveConfidence sc3(&confidence);

            confidence = (confidence <= notCombined) ? confidence : notCombined;
            if (confidence < notCombined && notCombined < 0.5f)
            {
                confidence = (float)confidence * ratio2;
            }

            if (confidence > bestConfidence)
            {
                bestConfidence = confidence;
                FuzzyVariant returnValue(0.0f);
                bestValue = returnValue;
            }
        }
    }

    if (complement > 0.0f)
    {
        SaveConfidence sc4(&confidence);

        confidence = (confidence <= complement) ? confidence : complement;
        if (confidence < complement && complement < 0.5f)
        {
            confidence = (float)confidence * ratio;
        }

        if (confidence > bestConfidence)
        {
            bestConfidence = confidence;
            FuzzyVariant returnValue(0.0f);
            bestValue = returnValue;
        }
    }

    bestValue.Confidence = bestConfidence;
    ScriptQuestionCache::Instance()->AddToCache(hash, bestValue, "ShouldIMarkBallOwner");
    return bestValue;
}

/**
 * Offset/Address/Size: 0xC144 | 0x80076314 | size: 0x1044
 */
FuzzyVariant Fuzzy::ShouldIAttemptOneTimer(cFielder* TheFielder)
{
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern float InFrontOfTheirNet(cFielder*);
    extern float IsPerfectPassInPlay();
    extern float ReceivingVolleyPass(cPlayer*);
    extern float CloseToTheirGoalie(cPlayer*);
    extern float NearToTheirNet(cPlayer*);
    extern float Stunned(Goalie*);
    extern float Shooter(cFielder*);
    extern float CalcSelectChance(float, float);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    unsigned long hash = (unsigned long)ShouldIAttemptOneTimer + ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

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
                lookupFound = 1;
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

        lookupFound = 0;

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

    float fTrueConfidence = FarToTheirNet((cPlayer*)TheFielder);
    float fInFrontOfNet = 1.0f - InFrontOfTheirNet(TheFielder);
    if (fTrueConfidence < fInFrontOfNet)
    {
        fTrueConfidence = fInFrontOfNet;
    }

    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMinVal / fMaxVal;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant returnValue(0.0f);
            bestValue = returnValue;
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        fTrueConfidence = IsPerfectPassInPlay();
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant returnValue(1.0f);
                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                bestValue = returnValue;
            }
        }

        fTrueConfidence = 1.0f - FarToTheirNet((cPlayer*)TheFielder);
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            fTrueConfidence = ReceivingVolleyPass((cPlayer*)TheFielder);
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                Goalie* pGoalie = NULL;
                if (TheFielder != NULL)
                {
                    pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
                }

                float fGoalieStunned = Stunned(pGoalie);
                float fDanger = InDanger(TheFielder).Confidence;
                if (fDanger < fGoalieStunned)
                {
                    fDanger = fGoalieStunned;
                }

                float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                fDanger = GoodToShoot(TheFielder).Confidence;
                if (fDanger < fNearToNet)
                {
                    fDanger = fNearToNet;
                }
                if (fGoalieStunned < fDanger)
                {
                    fDanger = fGoalieStunned;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant returnValue(fDanger);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                    bestValue = returnValue;
                }
            }

            if (fFalseConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);

                fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
                if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                Goalie* pGoalie = NULL;
                if (TheFielder != NULL)
                {
                    pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
                }

                float fGoalieStunned = Stunned(pGoalie);
                float fDanger = InDanger(TheFielder).Confidence;
                if (fDanger < fGoalieStunned)
                {
                    fDanger = fGoalieStunned;
                }
                fDanger = GoodToShoot(TheFielder).Confidence * 0.5f + fDanger * 0.5f;

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant returnValue(fDanger);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_GroundOneTimerChance, Shooter(TheFielder));
                    bestValue = returnValue;
                }
            }
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant returnValue(0.0f);
            bestValue = returnValue;
        }
    }

    bestValue.Confidence = fBestConfidence;

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
 * Offset/Address/Size: 0xB89C | 0x80075A6C | size: 0x8A8
 */
FuzzyVariant Fuzzy::GetBestLooseBallPassTarget(cFielder* TheFielder)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    unsigned long hash = (unsigned long)GetBestLooseBallPassTarget + ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

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
                lookupFound = 1;
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

        lookupFound = 0;

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

    FuzzyVariant dangerValue = InDanger(TheFielder);
    float fTrueConfidence = dangerValue.Confidence;
    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMinVal / fMaxVal;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        if (fConfidence > fTrueConfidence)
            fConfidence = fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = (float)fConfidence * fBranchRatio;

        FuzzyVariant theBestPassTarget = GetBestPassTarget((cPlayer*)TheFielder);

        fTrueConfidence = (theBestPassTarget.Confidence <= fConfidence) ? theBestPassTarget.Confidence : fConfidence;
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            if (fConfidence > fTrueConfidence)
                fConfidence = fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = (float)fConfidence * fBranchRatio;
            if (fConfidence > 0.0f)
            {
                fBestConfidence = fConfidence;
                bestValue = theBestPassTarget;
            }
        }
    }

    bestValue.Confidence = fBestConfidence;

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
 * Offset/Address/Size: 0xAC34 | 0x80074E04 | size: 0xC68
 * TODO: 71.76% match - stack layout/register allocation drift in FuzzyVariant
 * ctor temps and hash setup scheduling around GetHash/funcAddr.
 */
FuzzyVariant Fuzzy::GetBestPassTarget(cPlayer* ThePlayer)
{
    extern float Incapacitated(cPlayer*);
    extern float UpfieldFrom(cPlayer*, cPlayer*);
    extern float FarTo(cPlayer*, cPlayer*);
    extern float NearTo(cPlayer*, cPlayer*);
    extern float Open(cFielder*);
    extern float OpenTo(cPlayer*, cPlayer*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvPlayer((cPlayer*)ThePlayer);
    unsigned long funcAddr = (unsigned long)GetBestPassTarget;
    unsigned long hash = ((Variant*)&fvPlayer)->GetHash() + funcAddr;
    FuzzyVariant fvPlayer2((cPlayer*)ThePlayer);

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
                lookupFound = 1;
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

        lookupFound = 0;

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

    float fTrueConfidence = GoalieType(ThePlayer);
    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMinVal / fMaxVal;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        if (fConfidence > fTrueConfidence)
            fConfidence = fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = (float)fConfidence * fBranchRatio;

        for (int i = 0; i < 4; i++)
        {
            cFielder* TeamMate = ThePlayer->m_pTeam->GetFielder(i);
            if (TeamMate != (cFielder*)ThePlayer)
            {
                float fUpfield = UpfieldFrom(ThePlayer, (cPlayer*)TeamMate);
                float fNearTo = 1.0f - NearTo(ThePlayer, (cPlayer*)TeamMate);
                float fFarTo = 1.0f - FarTo(ThePlayer, (cPlayer*)TeamMate);
                float fIncapacitated = 1.0f - Incapacitated((cPlayer*)TeamMate);

                if (fNearTo > fUpfield)
                    fNearTo = fUpfield;
                if (fFarTo > fNearTo)
                    fFarTo = fNearTo;
                if (fIncapacitated > fFarTo)
                    fIncapacitated = fFarTo;

                fTrueConfidence = fIncapacitated;
                fFalseConfidence = 1.0f - fTrueConfidence;
                fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMinVal / fMaxVal;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM2(&fConfidence);
                    if (fConfidence > fTrueConfidence)
                        fConfidence = fTrueConfidence;
                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = (float)fConfidence * fBranchRatio;

                    float fOpen = Open(TeamMate);
                    float fOpenTo = OpenTo(ThePlayer, (cPlayer*)TeamMate);
                    fTrueConfidence = fOpen * 0.5f + fOpenTo * 0.5f;

                    fFalseConfidence = 1.0f - fTrueConfidence;
                    fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    fBranchRatio = fMinVal / fMaxVal;

                    if (fTrueConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM3(&fConfidence);
                        if (fConfidence > fTrueConfidence)
                            fConfidence = fTrueConfidence;
                        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                            fConfidence = (float)fConfidence * fBranchRatio;
                        if (fConfidence > fBestConfidence)
                        {
                            fBestConfidence = fConfidence;
                            bestValue = FuzzyVariant((cPlayer*)TeamMate);
                        }
                    }
                }
            }
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        if (fConfidence > fFalseConfidence)
            fConfidence = fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = (float)fConfidence * fBranchRatio;

        FuzzyVariant TheFielder((cPlayer*)ThePlayer);

        for (int i = 0; i < 4; i++)
        {
            cFielder* TeamMate = TheFielder.mData.pPlayer->m_pTeam->GetFielder(i);
            if (TeamMate != (cFielder*)TheFielder.mData.pPlayer)
            {
                FuzzyVariant TheTarget = GoodPassTargetFrom(TeamMate, (cFielder*)TheFielder.mData.pPlayer);

                fTrueConfidence = TheTarget.Confidence;
                fFalseConfidence = 1.0f - fTrueConfidence;
                fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMinVal / fMaxVal;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM2(&fConfidence);
                    if (fConfidence > fTrueConfidence)
                        fConfidence = fTrueConfidence;
                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = (float)fConfidence * fBranchRatio;
                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = FuzzyVariant((cPlayer*)TeamMate);
                    }
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;

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
 * Offset/Address/Size: 0xA138 | 0x80074308 | size: 0xAFC
 */
FuzzyVariant Fuzzy::GoodPassTargetFrom(cFielder* TheTargetFielder, cFielder* TheBallOwner)
{
    extern float FGREATER(float, float);
    extern float Incapacitated(cPlayer*);
    extern float AbleToUsePowerup(cFielder*, int);
    extern float Captain(cFielder*);
    extern float PerfectPassCandidateFrom(cFielder*, cFielder*);
    extern float OpenTo(cPlayer*, cPlayer*);
    extern float OnScreen(cPlayer*);
    extern float NearToTheirNet(cPlayer*);
    extern float LastBallOwner(cPlayer*);
    extern float DownfieldFrom(cPlayer*, cPlayer*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = Incapacitated((cPlayer*)TheTargetFielder);
    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant fvResult(0.0f);
            bestValue = fvResult;
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if ((fConfidence < fFalseConfidence) && (fFalseConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        float fOwnerCanShoot = Fuzzy::GoodToShoot(TheBallOwner).mData.f;
        float fTargetCanShoot = Fuzzy::GoodToShoot(TheTargetFielder).mData.f;
        float fNetOpeness = FGREATER(fTargetCanShoot, fOwnerCanShoot);
        float fPlayerDistance = FGREATER(Fuzzy::GoodToShoot(TheTargetFielder).mData.f, 0.3f);
        float fTotalSum = 0.5f * fNetOpeness + 0.5f * fPlayerDistance;

        float fCaptainBonus = 1.0f;
        if (AbleToUsePowerup(TheTargetFielder, 8) != 0.0f && Captain(TheTargetFielder) != 0.0f)
        {
            fCaptainBonus = 2.0f;
        }

        float fPlayerWeighting = PerfectPassCandidateFrom(TheTargetFielder, TheBallOwner);
        float fNetWeighting = OpenTo((cPlayer*)TheBallOwner, (cPlayer*)TheTargetFielder);
        float fTrueConfidence2 = OnScreen((cPlayer*)TheTargetFielder) * 0.15f
                               + fNetWeighting * 0.15f
                               + fPlayerWeighting * 0.5f
                               + fTotalSum * 0.2f;

        float fFalseConfidence2 = 1.0f - fTrueConfidence2;
        float fBranchRatio2 = (fTrueConfidence2 <= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2;
        fBranchRatio2 = fBranchRatio2 / ((fTrueConfidence2 >= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2);

        if (fTrueConfidence2 > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence2) ? fConfidence : fTrueConfidence2;
            if ((fConfidence < fTrueConfidence2) && (fTrueConfidence2 < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio2;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(fConfidence * fCaptainBonus);
                bestValue = fvResult;
            }

            float fTrueConfidence3 = NearToTheirNet((cPlayer*)TheTargetFielder);
            float fFalseConfidence3 = 1.0f - fTrueConfidence3;
            float fBranchRatio3 = (fTrueConfidence3 <= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3;
            fBranchRatio3 = fBranchRatio3 / ((fTrueConfidence3 >= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3);

            if (fTrueConfidence3 > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence3) ? fConfidence : fTrueConfidence3;
                if ((fConfidence < fTrueConfidence3) && (fTrueConfidence3 < 0.5f))
                {
                    fConfidence = (float)fConfidence * fBranchRatio3;
                }

                float fLastBallOwner = LastBallOwner((cPlayer*)TheTargetFielder);
                float fOpenToBallOwner = OpenTo((cPlayer*)TheBallOwner, (cPlayer*)TheTargetFielder);
                float fDownfield = DownfieldFrom((cPlayer*)TheBallOwner, (cPlayer*)TheTargetFielder);

                float fTrueConfidence4 = (fDownfield * 0.2f)
                                       + (fTotalSum * 0.425f)
                                       + (fOpenToBallOwner * 0.2f)
                                       + ((1.0f - fLastBallOwner) * 0.175f);
                float fFalseConfidence4 = 1.0f - fTrueConfidence4;
                float fBranchRatio4 = (fTrueConfidence4 <= fFalseConfidence4) ? fTrueConfidence4 : fFalseConfidence4;
                fBranchRatio4 = fBranchRatio4 / ((fTrueConfidence4 >= fFalseConfidence4) ? fTrueConfidence4 : fFalseConfidence4);

                if (fTrueConfidence4 > 0.0f)
                {
                    SaveConfidence PushDOM(&fConfidence);

                    fConfidence = (fConfidence <= fTrueConfidence4) ? fConfidence : fTrueConfidence4;
                    if ((fConfidence < fTrueConfidence4) && (fTrueConfidence4 < 0.5f))
                    {
                        fConfidence = (float)fConfidence * fBranchRatio4;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        FuzzyVariant fvResult(fConfidence * fCaptainBonus);
                        bestValue = fvResult;
                    }
                }
            }

            if (fFalseConfidence3 > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fFalseConfidence3) ? fConfidence : fFalseConfidence3;
                if ((fConfidence < fFalseConfidence3) && (fFalseConfidence3 < 0.5f))
                {
                    fConfidence = (float)fConfidence * fBranchRatio3;
                }

                float fLastBallOwner = LastBallOwner((cPlayer*)TheTargetFielder);
                float fOpenToBallOwner = OpenTo((cPlayer*)TheBallOwner, (cPlayer*)TheTargetFielder);
                float fDownfield = DownfieldFrom((cPlayer*)TheBallOwner, (cPlayer*)TheTargetFielder);

                float fTrueConfidence4 = (fDownfield * 0.35f)
                                       + (fTotalSum * 0.3f)
                                       + (fOpenToBallOwner * 0.2f)
                                       + ((1.0f - fLastBallOwner) * 0.15f);
                float fFalseConfidence4 = 1.0f - fTrueConfidence4;
                float fBranchRatio4 = (fTrueConfidence4 <= fFalseConfidence4) ? fTrueConfidence4 : fFalseConfidence4;
                fBranchRatio4 = fBranchRatio4 / ((fTrueConfidence4 >= fFalseConfidence4) ? fTrueConfidence4 : fFalseConfidence4);

                if (fTrueConfidence4 > 0.0f)
                {
                    SaveConfidence PushDOM(&fConfidence);

                    fConfidence = (fConfidence <= fTrueConfidence4) ? fConfidence : fTrueConfidence4;
                    if ((fConfidence < fTrueConfidence4) && (fTrueConfidence4 < 0.5f))
                    {
                        fConfidence = (float)fConfidence * fBranchRatio4;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        FuzzyVariant fvResult(fConfidence * fCaptainBonus);
                        bestValue = fvResult;
                    }
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;
    return bestValue;
}

/**
 * Offset/Address/Size: 0x95A0 | 0x80073770 | size: 0xB98
 * TODO: 84.14% match - stack layout/hash scheduling and std::map cache lookup
 *       symbol/inlining differences in the cache path.
 */
FuzzyVariant Fuzzy::GetBestHitTarget(cFielder* TheFielder)
{
    extern float Invincible(cFielder*);
    extern float FallenDown(cFielder*);
    extern float ChasingBall(cPlayer*);
    extern float FarTo(cPlayer*, cPlayer*);
    extern float Facing(cPlayer*, cPlayer*);
    extern float NearTo(cPlayer*, cPlayer*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder(TheFielder);
    unsigned long funcAddr = (unsigned long)GetBestHitTarget;
    unsigned long hash = ((Variant*)&fvFielder)->GetHash() + funcAddr;
    FuzzyVariant fvFielder2(TheFielder);

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
                lookupFound = 1;
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

        lookupFound = 0;

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
        cFielder* theOpponent = g_pScriptOtherTeam->GetFielder(i);

        float fTrueConfidence = 1.0f - Invincible(theOpponent);
        float fNotFallen = 1.0f - FallenDown(theOpponent);
        if (fNotFallen < fTrueConfidence)
        {
            fTrueConfidence = fNotFallen;
        }

        float fFalseConfidence = 1.0f - fTrueConfidence;
        float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            if (fConfidence > fTrueConfidence)
                fConfidence = fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = (float)fConfidence * fBranchRatio;

            float fChasingBall = ChasingBall((cPlayer*)theOpponent);
            float fReceivingPass = ReceivingPass(theOpponent);
            float fBallOwner = BallOwner((cPlayer*)theOpponent);

            if (fChasingBall > fReceivingPass)
            {
                fReceivingPass = fChasingBall;
            }
            if (fReceivingPass > fBallOwner)
            {
                fBallOwner = fReceivingPass;
            }

            fTrueConfidence = 1.0f - fBallOwner;
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio2 = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                if (fConfidence > fTrueConfidence)
                    fConfidence = fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = (float)fConfidence * fBranchRatio2;

                float fTrueConfidence2 = 1.0f - FarTo((cPlayer*)TheFielder, (cPlayer*)theOpponent);
                float fFalseConfidence2 = 1.0f - fTrueConfidence2;
                float fMinVal2 = (fTrueConfidence2 <= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2;
                float fMaxVal2 = (fTrueConfidence2 >= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2;
                float fBranchRatio2 = fMinVal2 / fMaxVal2;

                if (fTrueConfidence2 > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    if (fConfidence > fTrueConfidence2)
                        fConfidence = fTrueConfidence2;
                    if (fConfidence < fTrueConfidence2 && fTrueConfidence2 < 0.5f)
                        fConfidence = (float)fConfidence * fBranchRatio2;
                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = FuzzyVariant(theOpponent);
                    }
                }
            }

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM4(&fConfidence);
                if (fConfidence > fTrueConfidence)
                    fConfidence = fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = (float)fConfidence * fBranchRatio2;

                float fFacing = Facing((cPlayer*)TheFielder, (cPlayer*)theOpponent);
                float fNearTo = NearTo((cPlayer*)TheFielder, (cPlayer*)theOpponent);
                float fTrueConfidence3 = fNearTo * 0.6f + fFacing * 0.3f;
                float fFalseConfidence3 = 1.0f - fTrueConfidence3;
                float fMinVal3 = (fTrueConfidence3 <= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3;
                float fMaxVal3 = (fTrueConfidence3 >= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3;
                float fBranchRatio3 = fMinVal3 / fMaxVal3;

                if (fTrueConfidence3 > 0.0f)
                {
                    SaveConfidence PushDOM5(&fConfidence);
                    if (fConfidence > fTrueConfidence3)
                        fConfidence = fTrueConfidence3;
                    if (fConfidence < fTrueConfidence3 && fTrueConfidence3 < 0.5f)
                        fConfidence = (float)fConfidence * fBranchRatio3;
                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = FuzzyVariant(theOpponent);
                    }
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;

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
FuzzyVariant Fuzzy::GoodToShoot(cFielder* TheFielder)
{
    extern float InFrontOfTheirNet(cFielder*);
    extern float LikelyToScore(cFielder*);
    extern float PlayerShotDistance(cFielder*);
    extern float Stunned(Goalie*);
    extern float OpenToTheirNet(cFielder*);
    extern float CloseToTheirGoalie(cPlayer*);
    extern float NearToTheirNet(cPlayer*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder(TheFielder);
    unsigned long funcAddr = (unsigned long)GoodToShoot;
    unsigned long hash = ((Variant*)&fvFielder)->GetHash() + funcAddr;
    FuzzyVariant fvFielder2(TheFielder);

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
                lookupFound = 1;
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

        lookupFound = 0;

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

    float fTrueConfidence = FarToTheirNet((cPlayer*)TheFielder);
    float fInFrontOfNet = 1.0f - InFrontOfTheirNet(TheFielder);
    if (fTrueConfidence > fInFrontOfNet)
    {
        fTrueConfidence = fInFrontOfNet;
    }

    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant fvResult((1.0f - fConfidence) * 0.5f);
            bestValue = fvResult;
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if ((fConfidence < fFalseConfidence) && (fFalseConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        float fLikelyToScore = LikelyToScore(TheFielder);
        float fPlayerDistance = PlayerShotDistance(TheFielder);
        float fNetWeighting = 0.5f;
        float fPlayerWeighting = 0.5f;
        float fTotalSum = fLikelyToScore * fNetWeighting + fPlayerDistance * fPlayerWeighting;
        float fTotalWeight = fNetWeighting + fPlayerWeighting;
        float fScore = 0.0f;

        if (fTotalWeight > 0.0f)
        {
            fScore = fTotalSum / fTotalWeight;
        }

        if (fScore < 0.0f)
        {
            fScore = 0.0f;
        }
        if (fScore > 1.0f)
        {
            fScore = 1.0f;
        }

        Goalie* pGoalie = NULL;
        if (TheFielder != NULL)
        {
            pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
        }

        float fTrueConfidence2 = Stunned(pGoalie);
        float fFalseConfidence2 = 1.0f - fTrueConfidence2;
        float fBranchRatio2 = (fTrueConfidence2 <= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2;
        fBranchRatio2 = fBranchRatio2 / ((fTrueConfidence2 >= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2);

        if (fTrueConfidence2 > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence2) ? fConfidence : fTrueConfidence2;
            if ((fConfidence < fTrueConfidence2) && (fTrueConfidence2 < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio2;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(OpenToTheirNet(TheFielder));
                bestValue = fvResult;
            }
        }

        if (fFalseConfidence2 > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fFalseConfidence2) ? fConfidence : fFalseConfidence2;
            if ((fConfidence < fFalseConfidence2) && (fFalseConfidence2 < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio2;
            }

            float fTrueConfidence3 = CloseToTheirGoalie((cPlayer*)TheFielder);
            float fFalseConfidence3 = 1.0f - fTrueConfidence3;
            float fBranchRatio3 = (fTrueConfidence3 <= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3;
            fBranchRatio3 = fBranchRatio3 / ((fTrueConfidence3 >= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3);

            if (fTrueConfidence3 > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence3) ? fConfidence : fTrueConfidence3;
                if ((fConfidence < fTrueConfidence3) && (fTrueConfidence3 < 0.5f))
                {
                    fConfidence = (float)fConfidence * fBranchRatio3;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                    FuzzyVariant fvResult(fNearToNet * 0.3f + fScore * 0.7f);
                    bestValue = fvResult;
                }
            }

            if (fFalseConfidence3 > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fFalseConfidence3) ? fConfidence : fFalseConfidence3;
                if ((fConfidence < fFalseConfidence3) && (fFalseConfidence3 < 0.5f))
                {
                    fConfidence = (float)fConfidence * fBranchRatio3;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                    FuzzyVariant fvResult(fNearToNet * 0.3f + fScore * 0.7f);
                    bestValue = fvResult;
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;

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
 * Offset/Address/Size: 0x71F4 | 0x800713C4 | size: 0xE28
 */
FuzzyVariant Fuzzy::GoodToChipShot(cFielder* TheFielder)
{
    extern float ReceivingVolleyPass(cPlayer*);
    extern float InFrontOfTheirNet(cFielder*);
    extern float OnScreen(cPlayer*);
    extern float GoalieOutOfPosition(cFielder*);
    extern float LikelyToScore(cFielder*);
    extern float OutOfNet(Goalie*);
    extern float Stunned(Goalie*);
    extern float OpenToTheirNet(cFielder*);
    extern float CloseToTheirGoalie(cPlayer*);
    extern float NearToTheirNet(cPlayer*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder(TheFielder);
    unsigned long funcAddr = (unsigned long)GoodToChipShot;
    unsigned long hash = ((Variant*)&fvFielder)->GetHash() + funcAddr;
    FuzzyVariant fvFielder2(TheFielder);

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
                lookupFound = 1;
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

        lookupFound = 0;

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

    float fTrueConfidence = ReceivingVolleyPass((cPlayer*)TheFielder);
    float fInFrontOfNet = 1.0f - InFrontOfTheirNet(TheFielder);
    if (fTrueConfidence > fInFrontOfNet)
    {
        fTrueConfidence = fInFrontOfNet;
    }

    Goalie* pGoalieOnScreen = NULL;
    if (TheFielder != NULL)
    {
        pGoalieOnScreen = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
    }

    float fGoalieOffScreen = 1.0f - OnScreen((cPlayer*)pGoalieOnScreen);
    if (fTrueConfidence > fGoalieOffScreen)
    {
        fTrueConfidence = fGoalieOffScreen;
    }

    float fFarToNet = FarToTheirNet((cPlayer*)TheFielder);
    if (fFarToNet > fTrueConfidence)
    {
        fFarToNet = fTrueConfidence;
    }
    fTrueConfidence = fFarToNet;

    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant fvResult(0.0f);
            bestValue = fvResult;
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if ((fConfidence < fFalseConfidence) && (fFalseConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        float fPositionScore = GoalieOutOfPosition(TheFielder);
        float fNetOpeness = LikelyToScore(TheFielder);
        float fPositionWeighting = 0.5f;
        float fNetWeighting = 0.5f;
        float fTotalSum = fPositionScore * fPositionWeighting + fNetOpeness * fNetWeighting;
        float fTotalWeight = fPositionWeighting + fNetWeighting;
        float fScore = 0.0f;

        if (fTotalWeight > 0.0f)
        {
            fScore = fTotalSum / fTotalWeight;
        }

        if (fScore < 0.0f)
        {
            fScore = 0.0f;
        }
        if (fScore > 1.0f)
        {
            fScore = 1.0f;
        }

        Goalie* pGoalieOutOfNet = NULL;
        if (TheFielder != NULL)
        {
            pGoalieOutOfNet = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
        }

        float fOutOfNetScore = OutOfNet(pGoalieOutOfNet);

        Goalie* pGoalieStunned = NULL;
        if (TheFielder != NULL)
        {
            pGoalieStunned = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
        }

        float fTrueConfidence2 = Stunned(pGoalieStunned);
        float fFalseConfidence2 = 1.0f - fTrueConfidence2;
        float fBranchRatio2 = (fTrueConfidence2 <= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2;
        fBranchRatio2 = fBranchRatio2 / ((fTrueConfidence2 >= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2);

        if (fTrueConfidence2 > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence2) ? fConfidence : fTrueConfidence2;
            if ((fConfidence < fTrueConfidence2) && (fTrueConfidence2 < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio2;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(OpenToTheirNet(TheFielder) * 0.4f + fOutOfNetScore * 0.6f);
                bestValue = fvResult;
            }
        }

        if (fFalseConfidence2 > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fFalseConfidence2) ? fConfidence : fFalseConfidence2;
            if ((fConfidence < fFalseConfidence2) && (fFalseConfidence2 < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio2;
            }

            float fTrueConfidence3 = CloseToTheirGoalie((cPlayer*)TheFielder);
            float fFalseConfidence3 = 1.0f - fTrueConfidence3;
            float fBranchRatio3 = (fTrueConfidence3 <= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3;
            fBranchRatio3 = fBranchRatio3 / ((fTrueConfidence3 >= fFalseConfidence3) ? fTrueConfidence3 : fFalseConfidence3);

            if (fTrueConfidence3 > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence3) ? fConfidence : fTrueConfidence3;
                if ((fConfidence < fTrueConfidence3) && (fTrueConfidence3 < 0.5f))
                {
                    fConfidence = (float)fConfidence * fBranchRatio3;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                    FuzzyVariant fvResult(fOutOfNetScore * 0.4f + fScore * 0.5f + fNearToNet * 0.2f);
                    bestValue = fvResult;
                }
            }

            if (fFalseConfidence3 > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fFalseConfidence3) ? fConfidence : fFalseConfidence3;
                if ((fConfidence < fFalseConfidence3) && (fFalseConfidence3 < 0.5f))
                {
                    fConfidence = (float)fConfidence * fBranchRatio3;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                    FuzzyVariant fvResult(fOutOfNetScore * 0.4f + fScore * 0.5f + fNearToNet * 0.2f);
                    bestValue = fvResult;
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;

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
 * TODO: 41.34% match - windup decision tree still diverges in action branches and stack layout
 */
FuzzyVariant Fuzzy::GetBestWindupShotAction(cFielder* TheFielder)
{
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern float InFrontOfTheirNet(cFielder*);
    extern float IsPerfectPassInPlay();
    extern float ReceivingVolleyPass(cPlayer*);
    extern float CloseToTheirGoalie(cPlayer*);
    extern float NearToTheirNet(cPlayer*);
    extern float Stunned(Goalie*);
    extern float Shooter(cFielder*);
    extern float CalcSelectChance(float, float);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    unsigned long hash = (unsigned long)GetBestWindupShotAction + ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

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
                lookupFound = 1;
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

        lookupFound = 0;

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

    float fTrueConfidence = FarToTheirNet((cPlayer*)TheFielder);
    float fInFrontOfNet = 1.0f - InFrontOfTheirNet(TheFielder);
    if (fTrueConfidence < fInFrontOfNet)
    {
        fTrueConfidence = fInFrontOfNet;
    }

    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMinVal / fMaxVal;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant returnValue(0.0f);
            bestValue = returnValue;
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        fTrueConfidence = IsPerfectPassInPlay();
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant returnValue(1.0f);
                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                bestValue = returnValue;
            }
        }

        fTrueConfidence = 1.0f - FarToTheirNet((cPlayer*)TheFielder);
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            fTrueConfidence = ReceivingVolleyPass((cPlayer*)TheFielder);
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                Goalie* pGoalie = NULL;
                if (TheFielder != NULL)
                {
                    pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
                }

                float fGoalieStunned = Stunned(pGoalie);
                float fDanger = InDanger(TheFielder).Confidence;
                if (fDanger < fGoalieStunned)
                {
                    fDanger = fGoalieStunned;
                }

                float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                fDanger = GoodToShoot(TheFielder).Confidence;
                if (fDanger < fNearToNet)
                {
                    fDanger = fNearToNet;
                }
                if (fGoalieStunned < fDanger)
                {
                    fDanger = fGoalieStunned;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant returnValue(fDanger);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                    bestValue = returnValue;
                }
            }

            if (fFalseConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);

                fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
                if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                Goalie* pGoalie = NULL;
                if (TheFielder != NULL)
                {
                    pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
                }

                float fGoalieStunned = Stunned(pGoalie);
                float fDanger = InDanger(TheFielder).Confidence;
                if (fDanger < fGoalieStunned)
                {
                    fDanger = fGoalieStunned;
                }
                fDanger = GoodToShoot(TheFielder).Confidence * 0.5f + fDanger * 0.5f;

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant returnValue(fDanger);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_GroundOneTimerChance, Shooter(TheFielder));
                    bestValue = returnValue;
                }
            }
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant returnValue(0.0f);
            bestValue = returnValue;
        }
    }

    bestValue.Confidence = fBestConfidence;

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
 * Offset/Address/Size: 0x1A48 | 0x8006BC18 | size: 0x126C
 */
FuzzyVariant Fuzzy::GetPowerupToUseForPassReceiveDefence(cFielder* TheFielder)
{
    extern cBall* g_pBall;
    extern cFielder* g_pScriptCurrentFielder;
    extern float OnScreen(cPlayer*);
    extern float UserControlledT(cTeam*);
    extern float LikelyToUsePowerup(cFielder*, int);
    extern float High(cBall*);
    extern float NearToBall(cPlayer*);
    extern float ChasingBall(cPlayer*);
    extern float OnMushrooms(cFielder*);
    extern float InDefensiveZone(cPlayer*);
    extern float Captain(cFielder*);
    extern float ReceivingPass(cFielder*);
    extern float BallOwner(cPlayer*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();

    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    float fTrueConfidence = OnScreen((cPlayer*)TheFielder);

    cTeam* pTeam = (TheFielder != NULL) ? TheFielder->m_pTeam : NULL;
    float fNotUserControlled = 1.0f - UserControlledT(pTeam);

    cPlayer* pOtherGoalie;
    if (TheFielder != NULL)
    {
        if (TheFielder != NULL)
        {
            pOtherGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
        }
    }
    else
    {
        pOtherGoalie = NULL;
    }

    float fNotOtherGoaliePickup = 1.0f - Fuzzy::GoalieAndGonnaPickupBall(pOtherGoalie).Confidence;

    cPlayer* pGoalie;
    if (TheFielder != NULL)
    {
        if (TheFielder != NULL)
        {
            pGoalie = TheFielder->m_pTeam->GetGoalie();
        }
    }
    else
    {
        pGoalie = NULL;
    }

    float fNotGoaliePickup = 1.0f - Fuzzy::GoalieAndGonnaPickupBall(pGoalie).Confidence;

    if (fNotUserControlled > fTrueConfidence)
    {
        fNotUserControlled = fTrueConfidence;
    }

    if (fNotOtherGoaliePickup > fNotUserControlled)
    {
        fNotOtherGoaliePickup = fNotUserControlled;
    }

    if (fNotGoaliePickup > fNotOtherGoaliePickup)
    {
        fNotGoaliePickup = fNotOtherGoaliePickup;
    }

    fTrueConfidence = fNotGoaliePickup;

    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinConfidence = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxConfidence = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMinConfidence / fMaxConfidence;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        float fLikelyConfidence = LikelyToUsePowerup(TheFielder, 0);
        float fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
        float fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        float fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        float fLikelyBranchRatio = fLikelyMin / fLikelyMax;

        if (fLikelyConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

            if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fLikelyBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(0);
                bestValue = fvResult;
            }
        }

        fLikelyConfidence = LikelyToUsePowerup(TheFielder, 1);
        fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
        fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyBranchRatio = fLikelyMin / fLikelyMax;

        if (fLikelyConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

            if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fLikelyBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(1);
                bestValue = fvResult;
            }
        }

        fLikelyConfidence = LikelyToUsePowerup(TheFielder, 2);
        fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
        fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyBranchRatio = fLikelyMin / fLikelyMax;

        if (fLikelyConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

            if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fLikelyBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(2);
                bestValue = fvResult;
            }
        }

        fLikelyConfidence = LikelyToUsePowerup(TheFielder, 3);
        fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
        fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyBranchRatio = fLikelyMin / fLikelyMax;

        if (fLikelyConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

            if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fLikelyBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(3);
                bestValue = fvResult;
            }
        }

        fLikelyConfidence = LikelyToUsePowerup(TheFielder, 4);
        fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
        fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyBranchRatio = fLikelyMin / fLikelyMax;

        if (fLikelyConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

            if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fLikelyBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(4);
                bestValue = fvResult;
            }
        }

        fLikelyConfidence = LikelyToUsePowerup(TheFielder, 5);
        fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
        fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
        fLikelyBranchRatio = fLikelyMin / fLikelyMax;

        if (fLikelyConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

            if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fLikelyBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(5);
                bestValue = fvResult;
            }
        }

        fTrueConfidence = 1.0f - High(g_pBall);
        float fNotNearBall = 1.0f - NearToBall((cPlayer*)TheFielder);
        float fChasingBall = ChasingBall((cPlayer*)TheFielder);

        if (fNotNearBall > fTrueConfidence)
        {
            fNotNearBall = fTrueConfidence;
        }
        if (fChasingBall > fNotNearBall)
        {
            fChasingBall = fNotNearBall;
        }

        fTrueConfidence = fChasingBall;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinConfidence = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxConfidence = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinConfidence / fMaxConfidence;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            fTrueConfidence = 1.0f - OnMushrooms(g_pScriptCurrentFielder);
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinConfidence = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxConfidence = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMinConfidence / fMaxConfidence;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                fLikelyConfidence = LikelyToUsePowerup(TheFielder, 7);
                fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
                fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
                fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
                fLikelyBranchRatio = fLikelyMin / fLikelyMax;

                if (fLikelyConfidence > 0.0f)
                {
                    SaveConfidence PushDOM(&fConfidence);

                    fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

                    if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
                    {
                        fConfidence = (float)fConfidence * fLikelyBranchRatio;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        FuzzyVariant fvResult(7);
                        bestValue = fvResult;
                    }
                }
            }
        }

        float fNotInDefensiveZone = 1.0f - InDefensiveZone((cPlayer*)TheFielder);
        float fCaptain = Captain(TheFielder);
        float fReceivingPass = ReceivingPass(TheFielder);
        float fBallOwner = BallOwner((cPlayer*)TheFielder);

        if (fBallOwner < fReceivingPass)
        {
            fBallOwner = fReceivingPass;
        }

        if (fCaptain > fNotInDefensiveZone)
        {
            fCaptain = fNotInDefensiveZone;
        }

        if (fBallOwner > fCaptain)
        {
            fBallOwner = fCaptain;
        }

        fTrueConfidence = fBallOwner;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinConfidence = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxConfidence = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinConfidence / fMaxConfidence;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            fLikelyConfidence = LikelyToUsePowerup(TheFielder, 8);
            fLikelyFalseConfidence = 1.0f - fLikelyConfidence;
            fLikelyMin = (fLikelyConfidence <= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
            fLikelyMax = (fLikelyConfidence >= fLikelyFalseConfidence) ? fLikelyConfidence : fLikelyFalseConfidence;
            fLikelyBranchRatio = fLikelyMin / fLikelyMax;

            if (fLikelyConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fLikelyConfidence) ? fConfidence : fLikelyConfidence;

                if (fConfidence < fLikelyConfidence && fLikelyConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fLikelyBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant fvResult(8);
                    bestValue = fvResult;
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;
    return bestValue;
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
FuzzyVariant Fuzzy::InDangerDelayed(cFielder* TheFielder)
{
    extern float FGREATER(float, float);
    extern float Interpolate(float, float, float);
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern float Difficult(cTeam*);
    extern float AvoidingPowerups(cFielder*);
    extern float StuckOnSidelines(cFielder*);
    extern float Open(cFielder*);
    extern float Pressured(cFielder*);
    extern float Attacked(cFielder*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    unsigned long hash = (unsigned long)InDangerDelayed + ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

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
                lookupFound = 1;
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

        lookupFound = 0;

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
                {
                    cache->mQuestionCacheMap.m_NumElements++;
                }
            }
        }

        return bestValue;
    }

    float fTrueConfidence = AvoidingPowerups(TheFielder);
    float fOther = StuckOnSidelines(TheFielder);
    if (fTrueConfidence < fOther)
    {
        fTrueConfidence = fOther;
    }

    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMinVal / fMaxVal;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant fvResult(fConfidence);
            bestValue = fvResult;
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        fTrueConfidence = FGREATER(1.0f - Open(TheFielder), 0.2f);

        float fPressure = Pressured(TheFielder);
        float fAttack = Attacked(TheFielder);

        fPressure = (fPressure >= fTrueConfidence) ? fPressure : fTrueConfidence;
        fTrueConfidence = (fAttack >= fPressure) ? fAttack : fPressure;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            float fMin = Interpolate(0.8f, 0.2f, pSkillTweaks->Off_Reaction);
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            float fMax = Interpolate(1.0f, 1.0f, pSkillTweaks->Off_Reaction);

            cTeam* pTeam = TheFielder ? TheFielder->m_pTeam : NULL;
            if (Difficult(pTeam) == 0.0f)
            {
                fMin = 0.9f;
            }

            float fScore = NormalizeVal(fConfidence, fMin, fMax);

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant fvResult(fScore);
                bestValue = fvResult;
            }
        }
    }

    bestValue.Confidence = fBestConfidence;

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
            {
                cache->mQuestionCacheMap.m_NumElements++;
            }
        }
    }

    return bestValue;
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

void CommonScript_stub()
{
    std::map<unsigned long, FuzzyVariant> m;
    unsigned long k = 0;
    m.find(k);
    std::__red_black_tree<1>::node_base* n = 0;
    std::__red_black_tree<1>::rotate_left(n, n);
    std::__red_black_tree<1>::rotate_right(n, n);
}
