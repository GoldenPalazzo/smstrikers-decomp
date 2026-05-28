#define FUZZYVARIANT_COPY_CTOR_DECL_ONLY
#include "Game/AI/Scripts/CommonScript.h"
#include "Game/AI/Scripts/ScriptQuestions.h"

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

extern cFielder* g_pScriptCurrentFielder; // size: 0x4, address: 0x803977E0
extern cFielder* g_pScriptCurrentMark;    // size: 0x4, address: 0x803977E4
extern cFielder* g_pScriptBallOwner;
extern cTeam* g_pScriptCurrentTeam;
extern cTeam* g_pScriptOtherTeam;
extern cBall* g_pScriptBall;

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
#pragma dont_inline on
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
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x0 | 0x80079B54 | size: 0xE4
 */
FuzzyVariant::FuzzyVariant(const FuzzyVariant& other)
{
    Reset();
    *this = other;
}

/**
 * Offset/Address/Size: 0xF1B0 | 0x80079380 | size: 0x7D4
 * TODO: 95.35% match - std::tree call symbol remains __find/__find_or_insert
 * versus find<Ul>/find_or_insert<Ul,12FuzzyVariant>, plus residual
 * stack/register drift around hash and cache temporaries
 */
FuzzyVariant Fuzzy::GetStrategicBallCarrier(cTeam* TheTeam)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvTeam(TheTeam);
    volatile unsigned long funcAddrTemp = (unsigned long)GetStrategicBallCarrier;
    unsigned long hash = ((Variant*)&fvTeam)->GetHash();
    hash += funcAddrTemp;
    FuzzyVariant fvTeam2(TheTeam);

    unsigned long hashKey = hash;
    unsigned char lookupFound;
    FuzzyVariant* pValue;

    ScriptQuestionCache* pCache;
    pCache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    pCache->mTotalLookups++;

    if (g_bScriptQuestionCachingUseSTD)
    {
        StdMapNode* stdNode;
        __find(&stdNode, &pCache->mQuestionCacheMapSTD, &hashKey);
        StdMapNode* stdFound = stdNode;
        if ((StdMapNodeBase*)stdFound != &((StdMapTree*)&pCache->mQuestionCacheMapSTD)->x4)
        {
            pCache->mCacheHits++;
            bestValue = stdFound->value;
            lookupFound = 1;
        }
        else
        {
            lookupFound = 0;
        }
    }
    else
    {
        AVLTreeEntry<unsigned long, FuzzyVariant>* node = pCache->mQuestionCacheMap.m_Root;
        while (node != NULL)
        {
            int cmpResult;
            if (hashKey == node->key)
                cmpResult = 0;
            else if (hashKey < node->key)
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

        lookupFound = 0;

    found_done:
        if (lookupFound)
        {
            pCache->mCacheHits++;
            bestValue = *pValue;
        }
    }

    if (lookupFound)
    {
        unsigned long hashCopy1 = hashKey;
        if (g_bScriptQuestionCachingOn)
        {
            if (g_bScriptQuestionCachingUseSTD)
            {
                FuzzyMapPair* pair = __find_or_insert(&pCache->mQuestionCacheMapSTD, &hashCopy1);
                pair->value = bestValue;
            }
            else
            {
                AVLTreeNode* existingNode1;
                pCache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&pCache->mQuestionCacheMap.m_Root, (void*)&hashCopy1, (void*)&bestValue, &existingNode1, pCache->mQuestionCacheMap.m_NumElements);
                if (existingNode1 == NULL)
                    pCache->mQuestionCacheMap.m_NumElements++;
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
            SaveConfidence sc(&fConfidence);
            fConfidence = (fConfidence <= score) ? fConfidence : score;
            if (fConfidence < score && score < 0.5f)
            {
                double d = fConfidence;
                fConfidence = (float)d * ratio;
            }
            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = FuzzyVariant((cPlayer*)fielder);
            }
        }
    }

    bestValue.Confidence = fBestConfidence;
    unsigned long hashCopy2 = hashKey;
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            FuzzyMapPair* pair = __find_or_insert(&pCache->mQuestionCacheMapSTD, &hashCopy2);
            pair->value = bestValue;
        }
        else
        {
            AVLTreeNode* existingNode2;
            pCache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&pCache->mQuestionCacheMap.m_Root, (void*)&hashCopy2, (void*)&bestValue, &existingNode2, pCache->mQuestionCacheMap.m_NumElements);
            if (existingNode2 == NULL)
                pCache->mQuestionCacheMap.m_NumElements++;
        }
    }

    return bestValue;
}

/**
 * Offset/Address/Size: 0xE9DC | 0x80078BAC | size: 0x7D4
 * TODO: 95.47% match - std::tree call symbol remains __find/__find_or_insert
 * versus find<Ul>/find_or_insert<Ul,12FuzzyVariant>, plus residual stack/register
 * allocation drift around hash key and cache temporaries
 */
FuzzyVariant Fuzzy::GetBestBallInterceptor(cTeam* TheTeam)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvTeam(TheTeam);
    volatile unsigned long funcAddrTemp = (unsigned long)GetBestBallInterceptor;
    unsigned long hash = ((Variant*)&fvTeam)->GetHash();
    hash += funcAddrTemp;
    FuzzyVariant fvTeam2(TheTeam);

    unsigned long hashKey = hash;
    unsigned char lookupFound;
    FuzzyVariant* pValue;

    ScriptQuestionCache* pCache;
    pCache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    pCache->mTotalLookups++;

    if (g_bScriptQuestionCachingUseSTD)
    {
        StdMapNode* stdNode;
        __find(&stdNode, &pCache->mQuestionCacheMapSTD, &hashKey);
        StdMapNode* stdFound = stdNode;
        if ((StdMapNodeBase*)stdFound != &((StdMapTree*)&pCache->mQuestionCacheMapSTD)->x4)
        {
            pCache->mCacheHits++;
            bestValue = stdFound->value;
            lookupFound = 1;
        }
    }
    else
    {
        AVLTreeEntry<unsigned long, FuzzyVariant>* node = pCache->mQuestionCacheMap.m_Root;
        while (node != NULL)
        {
            int cmpResult;
            if (hashKey == node->key)
                cmpResult = 0;
            else if (hashKey < node->key)
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

        lookupFound = 0;

    found_done:
        if (lookupFound)
        {
            pCache->mCacheHits++;
            bestValue = *pValue;
        }
    }

    if (lookupFound)
    {
        unsigned long hashCopy1 = hashKey;
        if (g_bScriptQuestionCachingOn)
        {
            if (g_bScriptQuestionCachingUseSTD)
            {
                FuzzyMapPair* pair = __find_or_insert(&pCache->mQuestionCacheMapSTD, &hashCopy1);
                pair->value = bestValue;
            }
            else
            {
                AVLTreeNode* existingNode1;
                pCache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&pCache->mQuestionCacheMap.m_Root, (void*)&hashCopy1, (void*)&bestValue, &existingNode1, pCache->mQuestionCacheMap.m_NumElements);
                if (existingNode1 == NULL)
                    pCache->mQuestionCacheMap.m_NumElements++;
            }
        }
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
            SaveConfidence sc(&fConfidence);
            fConfidence = (fConfidence <= score) ? fConfidence : score;
            if (fConfidence < score && score < 0.5f)
            {
                double d = fConfidence;
                fConfidence = (float)d * ratio;
            }
            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = FuzzyVariant((cPlayer*)fielder);
            }
        }
    }

    bestValue.Confidence = fBestConfidence;
    unsigned long hashCopy2 = hashKey;
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            FuzzyMapPair* pair = __find_or_insert(&pCache->mQuestionCacheMapSTD, &hashCopy2);
            pair->value = bestValue;
        }
        else
        {
            AVLTreeNode* existingNode2;
            pCache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&pCache->mQuestionCacheMap.m_Root, (void*)&hashCopy2, (void*)&bestValue, &existingNode2, pCache->mQuestionCacheMap.m_NumElements);
            if (existingNode2 == NULL)
                pCache->mQuestionCacheMap.m_NumElements++;
        }
    }

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
            float shootScore = Fuzzy::GoodToShoot((cFielder*)ThePlayer).mData.f;
            float weight = 0.5f;
            flag = 0;
            totalWeight += weight;
            weightedSum += shootScore * weight;
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
        defResult = (defResult <= defense) ? defResult : defense;
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
    if (1.0f >= absVal)
    {
        absVal = (float)fabs((double)ThePlayer->m_v3ScreenPosition.f.y);
        if (1.0f >= absVal)
            isIdle = 1;
    }

    bestValue = FuzzyVariant(result);
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
    ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)pFielder);

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
    unsigned long functionAddress = (unsigned long)ShouldIAttemptOneTimer;
    unsigned long hash = ((Variant*)&fvFielder)->GetHash();
    hash += functionAddress;
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    ScriptQuestionCache* cache = ScriptQuestionCache::Instance();
    cache->mTotalLookups++;

    unsigned char lookupFound = 0;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant>::iterator stdIt = cache->mQuestionCacheMapSTD.find(hash);
        if (*(StdMapNodeBase**)&stdIt != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            StdMapNode* stdFound = *(StdMapNode**)&stdIt;
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

    float fInFrontOfNet = 1.0f - InFrontOfTheirNet(TheFielder);
    float fTrueConfidence = FarToTheirNet((cPlayer*)TheFielder);
    if (fInFrontOfNet >= fTrueConfidence)
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

            {
                FuzzyVariant returnValue(1.0f);
                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = returnValue;
                }
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

                float fCloseToGoalie = CloseToTheirGoalie((cPlayer*)TheFielder);
                float fDanger = InDanger(TheFielder).Confidence;
                if (fDanger < fCloseToGoalie)
                {
                    fDanger = fCloseToGoalie;
                }

                float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                fDanger = fNearToNet * 0.5f + fDanger * 0.5f;

                float fGoodToShoot = GoodToShoot(TheFielder).Confidence;

                Goalie* pGoalie = NULL;
                if (TheFielder != NULL)
                {
                    pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
                }
                float fGoalieStunned = Stunned(pGoalie);

                if (fDanger >= fGoodToShoot)
                {
                    fGoodToShoot = fDanger;
                }
                if (fGoalieStunned >= fGoodToShoot)
                {
                    fGoodToShoot = fGoalieStunned;
                }

                {
                    FuzzyVariant returnValue(fGoodToShoot);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = returnValue;
                    }
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
                if (fGoalieStunned >= fDanger)
                {
                    fDanger = fGoalieStunned;
                }

                float fGoodToShoot = GoodToShoot(TheFielder).Confidence;
                fDanger = fGoodToShoot * 0.5f + fDanger * 0.5f;

                {
                    FuzzyVariant returnValue(fDanger);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_GroundOneTimerChance, Shooter(TheFielder));
                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = returnValue;
                    }
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
    unsigned long hash = (unsigned long)GetBestLooseBallPassTarget;
    hash += ((Variant*)&fvFielder)->GetHash();
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

    float fTrueConfidence = InDanger(TheFielder).Confidence;
    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fBranchRatio = ((fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence) / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = (float)fConfidence * fBranchRatio;

        FuzzyVariant theBestPassTarget = GetBestPassTarget((cPlayer*)TheFielder);

        fTrueConfidence = (theBestPassTarget.Confidence <= fConfidence) ? theBestPassTarget.Confidence : fConfidence;
        fFalseConfidence = 1.0f - fTrueConfidence;
        fBranchRatio = ((fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence) / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
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
    unsigned long hash = (unsigned long)GetBestPassTarget;
    hash += ((Variant*)&fvPlayer)->GetHash();
    FuzzyVariant fvPlayer2((cPlayer*)ThePlayer);

    ScriptQuestionCache* cache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    cache->mTotalLookups++;

    unsigned char lookupFound;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant>::iterator stdIt = cache->mQuestionCacheMapSTD.find(hash);
        if (*(StdMapNodeBase**)&stdIt != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = (*(StdMapNode**)&stdIt)->value;
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
                std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy1);
                pair.second = bestValue;
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

    float fTrueConfidence = GoalieType(ThePlayer);
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
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
        }

        for (int i = 0; i < 4; i++)
        {
            cFielder* TeamMate = ThePlayer->m_pTeam->GetFielder(i);
            if (TeamMate != (cFielder*)ThePlayer)
            {
                float fUpfield = UpfieldFrom((cPlayer*)TeamMate, ThePlayer);
                float fNearTo = 1.0f - NearTo(ThePlayer, (cPlayer*)TeamMate);
                float fFarTo = 1.0f - FarTo(ThePlayer, (cPlayer*)TeamMate);
                float fIncapacitated = 1.0f - Incapacitated((cPlayer*)TeamMate);

                fNearTo = (fNearTo <= fUpfield) ? fNearTo : fUpfield;
                fFarTo = (fFarTo <= fNearTo) ? fFarTo : fNearTo;
                fIncapacitated = (fIncapacitated <= fFarTo) ? fIncapacitated : fFarTo;

                fTrueConfidence = fIncapacitated;
                {
                    float fFalseConfidence = 1.0f - fTrueConfidence;
                    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    float fBranchRatio = fMinVal / fMaxVal;

                    if (fTrueConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM2(&fConfidence);

                        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        {
                            double d = fConfidence;
                            fConfidence = (float)d * fBranchRatio;
                        }

                        float fOpen = Open(TeamMate);
                        float fOpenTo = OpenTo(ThePlayer, (cPlayer*)TeamMate);
                        fTrueConfidence = fOpen * 0.5f + fOpenTo * 0.5f;

                        {
                            float fFalseConfidence = 1.0f - fTrueConfidence;
                            float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                            float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                            float fBranchRatio = fMinVal / fMaxVal;

                            if (fTrueConfidence > 0.0f)
                            {
                                SaveConfidence PushDOM3(&fConfidence);

                                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                                {
                                    double d = fConfidence;
                                    fConfidence = (float)d * fBranchRatio;
                                }
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
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
        }

        FuzzyVariant TheFielder((cPlayer*)ThePlayer);

        for (int i = 0; i < 4; i++)
        {
            cFielder* TeamMate = TheFielder.mData.pPlayer->m_pTeam->GetFielder(i);
            if (TeamMate != (cFielder*)TheFielder.mData.pPlayer)
            {
                fTrueConfidence = GoodPassTargetFrom(TeamMate, (cFielder*)TheFielder.mData.pPlayer).Confidence;
                {
                    float fFalseConfidence = 1.0f - fTrueConfidence;
                    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    float fBranchRatio = fMinVal / fMaxVal;

                    if (fTrueConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM2(&fConfidence);

                        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        {
                            double d = fConfidence;
                            fConfidence = (float)d * fBranchRatio;
                        }
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

    bestValue.Confidence = fBestConfidence;

    unsigned long hashCopy2 = hash;
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy2);
            pair.second = bestValue;
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
 * TODO: 92.97% match - remaining diffs are register allocation drift in cache
 *       lookup/add paths and confidence-branch temporaries.
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
    unsigned long hash = ((Variant*)&fvFielder)->GetHash();
    hash += funcAddr;
    FuzzyVariant fvFielder2(TheFielder);

    ScriptQuestionCache* cache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    cache->mTotalLookups++;

    unsigned char lookupFound;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant>::iterator stdIt = cache->mQuestionCacheMapSTD.find(hash);
        if (*(StdMapNodeBase**)&stdIt != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = (*(StdMapNode**)&stdIt)->value;
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
        fTrueConfidence = (fNotFallen <= fTrueConfidence) ? fNotFallen : fTrueConfidence;

        float fFalseConfidence = 1.0f - fTrueConfidence;
        float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = (float)fConfidence * fBranchRatio;

            float fChasingBall = ChasingBall((cPlayer*)theOpponent);
            float fReceivingPass = ReceivingPass(theOpponent);
            float fBallOwner = BallOwner((cPlayer*)theOpponent);

            fReceivingPass = (fReceivingPass >= fChasingBall) ? fReceivingPass : fChasingBall;
            fBallOwner = (fBallOwner >= fReceivingPass) ? fBallOwner : fReceivingPass;

            fTrueConfidence = 1.0f - fBallOwner;
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio2 = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
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
                    fConfidence = (fConfidence <= fTrueConfidence2) ? fConfidence : fTrueConfidence2;
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
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
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
                    fConfidence = (fConfidence <= fTrueConfidence3) ? fConfidence : fTrueConfidence3;
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
    unsigned long hash = (unsigned long)GoodToShoot;
    hash += ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2(TheFielder);

    ScriptQuestionCache* cache = ScriptQuestionCache::Instance();
    cache->mTotalLookups++;

    unsigned char lookupFound;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant>::iterator stdIt = cache->mQuestionCacheMapSTD.find(hash);
        if (*(StdMapNodeBase**)&stdIt != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            StdMapNode* stdFound = *(StdMapNode**)&stdIt;
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
                std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy1);
                pair.second = bestValue;
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

        float fNetOpeness = LikelyToScore(TheFielder);
        float fPlayerDistance = PlayerShotDistance(TheFielder);
        float fPlayerWeighting = g_pGame->m_pGameTweaks->unk2E0;
        float fNetWeighting = g_pGame->m_pGameTweaks->unk2DC;
        float fTotalSum = 1.0f + fNetOpeness * fNetWeighting + fPlayerDistance * fPlayerWeighting;
        float fTotalWeight = 1.0f + fNetWeighting + fPlayerWeighting;
        fNetOpeness = 1.0f;

        if (fTotalWeight > 1.0f)
        {
            fNetOpeness = fTotalSum / fTotalWeight;
        }

        if (fNetOpeness < 0.0f)
        {
            fNetOpeness = 0.0f;
        }
        if (fNetOpeness > 1.0f)
        {
            fNetOpeness = 1.0f;
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
                FuzzyVariant fvResult(fNetOpeness);
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
                FuzzyVariant fvResult(fNearToNet * 0.3f + fNetOpeness * 0.7f);
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
            std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy2);
            pair.second = bestValue;
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
    unsigned long hash = (unsigned long)GoodToChipShot;
    hash += ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2(TheFielder);

    ScriptQuestionCache* cache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    cache->mTotalLookups++;

    unsigned char lookupFound;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant>::iterator stdIt = cache->mQuestionCacheMapSTD.find(hash);
        if (*(StdMapNodeBase**)&stdIt != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = (*(StdMapNode**)&stdIt)->value;
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
                std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy1);
                pair.second = bestValue;
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

    Goalie* pGoalieOnScreen = NULL;
    if (TheFielder != NULL)
    {
        pGoalieOnScreen = (TheFielder != NULL) ? TheFielder->m_pTeam->GetOtherTeam()->GetGoalie() : NULL;
    }

    float fGoalieOffScreen = 1.0f - OnScreen((cPlayer*)pGoalieOnScreen);
    float fFarToNet = FarToTheirNet((cPlayer*)TheFielder);
    fTrueConfidence = (fInFrontOfNet <= fTrueConfidence) ? fInFrontOfNet : fTrueConfidence;
    fTrueConfidence = (fGoalieOffScreen <= fTrueConfidence) ? fGoalieOffScreen : fTrueConfidence;
    fTrueConfidence = (fFarToNet <= fTrueConfidence) ? fFarToNet : fTrueConfidence;

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
        float fPositionWeighting = g_pGame->m_pGameTweaks->unk2E4;
        float fNetWeighting = g_pGame->m_pGameTweaks->unk2E8;
        float fTotalSum = 0.0f;
        float fTotalWeight = 0.0f;

        fTotalWeight += fPositionWeighting;
        fTotalSum += fPositionScore * fPositionWeighting;
        fTotalWeight += fNetWeighting;
        fTotalSum += fNetOpeness * fNetWeighting;

        fPositionScore = 0.0f;
        if (fTotalWeight > 0.0f)
        {
            fPositionScore = fTotalSum / fTotalWeight;
        }

        if (fPositionScore < 0.0f)
        {
            fPositionScore = 0.0f;
        }
        if (fPositionScore > 1.0f)
        {
            fPositionScore = 1.0f;
        }

        Goalie* pGoalieOutOfNet = NULL;
        if (TheFielder != NULL)
        {
            pGoalieOutOfNet = (TheFielder != NULL) ? TheFielder->m_pTeam->GetOtherTeam()->GetGoalie() : NULL;
        }

        float fOutOfNetScore = OutOfNet(pGoalieOutOfNet);

        Goalie* pGoalieStunned = NULL;
        if (TheFielder != NULL)
        {
            pGoalieStunned = (TheFielder != NULL) ? TheFielder->m_pTeam->GetOtherTeam()->GetGoalie() : NULL;
        }

        {
            float fTrueConfidence = Stunned(pGoalieStunned);
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

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant fvResult(OpenToTheirNet(TheFielder) * 0.4f + fOutOfNetScore * 0.6f);
                    bestValue = fvResult;
                }
            }
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
                FuzzyVariant fvResult(fOutOfNetScore * 0.4f + fPositionScore * 0.5f + fNearToNet * 0.2f);
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
                FuzzyVariant fvResult(fOutOfNetScore * 0.4f + fPositionScore * 0.5f + fNearToNet * 0.2f);
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
            std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy2);
            pair.second = bestValue;
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
FuzzyVariant Fuzzy::GetBestPassReceiveAction(cFielder* TheFielder)
{
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern cTeam* g_pScriptCurrentTeam;
    extern float UserControlledT(cTeam*);
    extern float ReceivingPass(cFielder*);
    extern float Captain(cFielder*);
    extern float InOffensiveZone(cPlayer*);
    extern float WideOpen(cFielder*);
    extern float ReceivingVolleyPassDelayed(cPlayer*);
    extern FuzzyVariant GetPowerupToUseForPassReceiveDefence(cFielder*);
    extern float Aggressive(cFielder*);
    extern float CalcSelectChance(float, float);
    extern float NearToBall(cPlayer*);
    extern float ClosingTo(cPlayer*, cPlayer*);
    extern float NearTo(cPlayer*, cPlayer*);
    extern float OnScreen(cPlayer*);
    extern float AtIdealDistanceForTackling(cPlayer*, cPlayer*);
    extern float CloseTo(cPlayer*, cPlayer*);
    extern float InDefensiveZone(cPlayer*);
    extern float BallOwner(cPlayer*);
    extern float OnTheGround(cPlayer*);
    extern FuzzyVariant ShouldIAttemptOneTimer(cFielder*);
    extern float PassReceiveCloseToDone(cFielder*);
    extern float Shooter(cFielder*);
    extern float Passer(cFielder*);
    extern float ReceivingVolleyPass(cPlayer*);
    extern float FarTo(cPlayer*, cPlayer*);
    extern float FGREATER(float, float);
    extern float FLESS(float, float);
    extern FuzzyVariant InDanger(cFielder*);
    extern FuzzyVariant GoodToChipShot(cFielder*);
    extern FuzzyVariant GetBestPassTarget(cPlayer*);
    extern FuzzyVariant InDangerDelayed(cFielder*);
    extern cPlayer* GetClosestOpponentFielder(cPlayer*, nlVector3*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    unsigned long hash = (unsigned long)GetBestPassReceiveAction;
    hash += ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    ScriptQuestionCache* cache = ScriptQuestionCache::Instance();
    cache->mTotalLookups++;

    unsigned char lookupFound;
    FuzzyVariant* pValue;
    cPlayer* pClosestOpponent;

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

    float fTrueConfidence = 1.0f - UserControlledT(g_pScriptCurrentTeam);
    float fReceivingPass = ReceivingPass(TheFielder);
    if (fReceivingPass <= fTrueConfidence)
    {
        fTrueConfidence = fReceivingPass;
    }
    else
    {
        fTrueConfidence = fTrueConfidence;
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

        float fCaptain = Captain(TheFielder);
        float fOffZone = InOffensiveZone((cPlayer*)TheFielder);
        float fWideOpen = WideOpen(TheFielder);
        float fThreshold = fCaptain * 0.1f + (1.0f - fWideOpen) * 0.7f + fOffZone * 0.2f;
        fTrueConfidence = ReceivingVolleyPassDelayed((cPlayer*)TheFielder);
        if (fTrueConfidence <= fThreshold)
        {
        }
        else
        {
            fTrueConfidence = fThreshold;
        }

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

            FuzzyVariant powerupToUse = GetPowerupToUseForPassReceiveDefence(TheFielder);

            fTrueConfidence = powerupToUse.Confidence;
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

                FuzzyVariant returnAction(18);
                returnAction.ExtraData = (Variant&)powerupToUse;
                returnAction.Confidence = 0.0f;
                returnAction.SelectionChance = 1.0f;

                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                returnAction.SelectionChance = CalcSelectChance(pSkillTweaks->Off_PassReceivePowerupChance, Aggressive(TheFielder));

                if (fConfidence > 0.0f)
                {
                    fBestConfidence = fConfidence;
                    bestValue = returnAction;
                }
            }
        }

        pClosestOpponent = GetClosestOpponentFielder((cPlayer*)TheFielder, NULL);

        float fNearBallFielder = NearToBall((cPlayer*)TheFielder);
        float fNearBallOpponent = NearToBall((cPlayer*)pClosestOpponent);
        fTrueConfidence = FGREATER(fNearBallOpponent, fNearBallFielder);

        float fDanger = InDanger(TheFielder).mData.f;
        float fClosingTo = ClosingTo((cPlayer*)TheFielder, (cPlayer*)pClosestOpponent);

        if (fDanger < fTrueConfidence)
        {
            fDanger = fTrueConfidence;
        }
        if (fClosingTo >= fDanger)
        {
            fDanger = fClosingTo;
        }

        float fNearTo = NearTo((cPlayer*)TheFielder, (cPlayer*)pClosestOpponent);
        float fOnScreen = OnScreen((cPlayer*)pClosestOpponent);

        if (fNearTo > fDanger)
        {
            fNearTo = fDanger;
        }
        if (fOnScreen > fNearTo)
        {
            fOnScreen = fNearTo;
        }

        fTrueConfidence = fOnScreen;
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM4(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            float fAtIdeal = AtIdealDistanceForTackling((cPlayer*)TheFielder, (cPlayer*)pClosestOpponent);
            float fClose = CloseTo((cPlayer*)TheFielder, (cPlayer*)pClosestOpponent);
            if (fClose >= fAtIdeal)
            {
                fAtIdeal = fClose;
            }

            float fClosing2 = ClosingTo((cPlayer*)TheFielder, (cPlayer*)pClosestOpponent);
            float fDefZone = InDefensiveZone((cPlayer*)TheFielder);
            fClosing2 = fDefZone * 0.65f + fClosing2 * 0.35f;

            float fNotBallOwner = 1.0f - BallOwner((cPlayer*)TheFielder);
            float fGround = OnTheGround((cPlayer*)TheFielder);

            if (fClosing2 > fAtIdeal)
            {
                fClosing2 = fAtIdeal;
            }
            if (fNotBallOwner > fClosing2)
            {
                fNotBallOwner = fClosing2;
            }
            if (fGround > fNotBallOwner)
            {
                fGround = fNotBallOwner;
            }

            fTrueConfidence = fGround;
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM5(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                FuzzyVariant returnAction(5);
                returnAction.ExtraData = Variant(FT_PLAYER, (cPlayer*)pClosestOpponent);
                returnAction.Confidence = 0.0f;
                returnAction.SelectionChance = 1.0f;

                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                returnAction.SelectionChance = CalcSelectChance(pSkillTweaks->Off_PassReceiveHitChance, Aggressive(TheFielder));

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = returnAction;
                }
            }
        }

        FuzzyVariant oneTimerScore = ShouldIAttemptOneTimer(TheFielder);

        fTrueConfidence = FGREATER(PassReceiveCloseToDone(TheFielder), 0.0f);
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM6(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            fTrueConfidence = oneTimerScore.mData.f;
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM7(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                FuzzyVariant shotAction(24);
                shotAction.ExtraData = Variant(FT_BOOL, false);
                shotAction.SelectionChance = oneTimerScore.SelectionChance;

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = shotAction;
                }
            }
        }

        fTrueConfidence = GoodToChipShot(TheFielder).mData.f * 0.5f + oneTimerScore.mData.f * 0.5f;
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM8(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            FuzzyVariant shotAction(24);
            shotAction.ExtraData = Variant(FT_BOOL, true);
            shotAction.Confidence = 0.0f;
            shotAction.SelectionChance = 1.0f;

            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            shotAction.SelectionChance = CalcSelectChance(pSkillTweaks->Off_ChipShotChance, Shooter(TheFielder));

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = shotAction;
            }
        }

        fTrueConfidence = FGREATER(PassReceiveCloseToDone(TheFielder), 0.3f);
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM9(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            FuzzyVariant bestPassTargetFielder = GetBestPassTarget((cPlayer*)TheFielder);

            FuzzyVariant passAction(13);
            passAction.ExtraData = (Variant&)bestPassTargetFielder;
            passAction.Confidence = 0.0f;
            passAction.SelectionChance = 1.0f;

            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            passAction.SelectionChance = CalcSelectChance(pSkillTweaks->Off_OneTouchGroundPassChance, Passer(TheFielder));

            fTrueConfidence = ReceivingVolleyPass((cPlayer*)TheFielder);
            float fVolleyFalse = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fVolleyFalse) ? fTrueConfidence : fVolleyFalse;
            fMaxVal = (fTrueConfidence >= fVolleyFalse) ? fTrueConfidence : fVolleyFalse;
            float fVolleyRatio = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM10(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fVolleyRatio;
                }

                float fClosing3 = ClosingTo((cPlayer*)pClosestOpponent, (cPlayer*)TheFielder);
                float fNearFar = 1.0f - FarTo((cPlayer*)pClosestOpponent, (cPlayer*)TheFielder);
                if (fNearFar > fClosing3)
                {
                    fNearFar = fClosing3;
                }

                float fDangerDelayed = FGREATER(InDangerDelayed(TheFielder).mData.f, 0.0f);
                float fNotWide = 1.0f - WideOpen(TheFielder);

                if (fDangerDelayed < fNearFar)
                {
                    fDangerDelayed = fNearFar;
                }
                if (fNotWide < fDangerDelayed)
                {
                    fNotWide = fDangerDelayed;
                }

                float fOneTimerLess = FLESS(oneTimerScore.mData.f, 0.5f);
                float fNearTarget = 1.0f - NearTo(bestPassTargetFielder.mData.pPlayer, (cPlayer*)TheFielder);
                float fPassConf = FGREATER(bestPassTargetFielder.Confidence, 0.0f);

                if (fOneTimerLess > fNotWide)
                {
                    fOneTimerLess = fNotWide;
                }
                if (fNearTarget > fOneTimerLess)
                {
                    fNearTarget = fOneTimerLess;
                }
                if (fPassConf > fNearTarget)
                {
                    fPassConf = fNearTarget;
                }

                fTrueConfidence = fPassConf;
                fFalseConfidence = 1.0f - fTrueConfidence;
                fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMinVal / fMaxVal;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM11(&fConfidence);

                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    {
                        fConfidence = (float)fConfidence * fBranchRatio;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = passAction;
                    }
                }
            }

            if (fVolleyFalse > 0.0f)
            {
                SaveConfidence PushDOM12(&fConfidence);

                fConfidence = (fConfidence <= fVolleyFalse) ? fConfidence : fVolleyFalse;
                if (fConfidence < fVolleyFalse && fVolleyFalse < 0.5f)
                {
                    fConfidence = (float)fConfidence * fVolleyRatio;
                }

                float fClosing4 = ClosingTo((cPlayer*)pClosestOpponent, (cPlayer*)TheFielder);
                float fNearFar2 = 1.0f - FarTo((cPlayer*)pClosestOpponent, (cPlayer*)TheFielder);
                if (fNearFar2 > fClosing4)
                {
                    fNearFar2 = fClosing4;
                }

                float fDangerDelayed2 = InDangerDelayed(TheFielder).mData.f;
                if (fDangerDelayed2 < fNearFar2)
                {
                    fDangerDelayed2 = fNearFar2;
                }

                float fNearTarget2 = 1.0f - NearTo(bestPassTargetFielder.mData.pPlayer, (cPlayer*)TheFielder);
                float fPassConf2 = FGREATER(bestPassTargetFielder.Confidence, 0.3f);

                if (fNearTarget2 > fDangerDelayed2)
                {
                    fNearTarget2 = fDangerDelayed2;
                }
                if (fPassConf2 > fNearTarget2)
                {
                    fPassConf2 = fNearTarget2;
                }

                fTrueConfidence = fPassConf2;
                fFalseConfidence = 1.0f - fTrueConfidence;
                fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMinVal / fMaxVal;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM13(&fConfidence);

                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    {
                        fConfidence = (float)fConfidence * fBranchRatio;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = passAction;
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
 * Offset/Address/Size: 0x3FD0 | 0x8006E1A0 | size: 0x17F4
 * TODO: 45.9% match - body restored from prior scratch; large remaining diffs.
 */
FuzzyVariant Fuzzy::GetBestLooseBallAction(cFielder* TheFielder)
{
    FORCE_DONT_INLINE;
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern cTeam* g_pScriptCurrentTeam;
    extern cTeam* g_pScriptOtherTeam;
    extern cBall* g_pScriptBall;
    extern cGame* g_pGame;
    extern float FGREATER(float, float);
    extern float RandomChance(float);
    extern float TimeNearlyOver(cGame*);
    extern float Stalling(cTeam*);
    extern float Losing(cTeam*);
    extern float Difficult(cTeam*);
    extern float NormalizeVal(float, float, float);
    extern float UserControlledT(cTeam*);
    extern float OnMushrooms(cFielder*);
    extern float Ownerless(cBall*);
    extern float Open(cFielder*);
    extern float Pressured(cFielder*);
    extern float FacingSideline(cFielder*);
    extern float Aggressive(cFielder*);
    extern float CalcSelectChance(float, float);
    extern float CloseTo(cPlayer*, cPlayer*);
    extern float AtIdealDistanceForTackling(cPlayer*, cPlayer*);
    extern float SeparatingFrom(cPlayer*, cPlayer*);
    extern float OnScreen(cPlayer*);
    extern float FarToBall(cPlayer*);
    extern float BallOwner(cPlayer*);
    extern float WindingUpForShot(cFielder*);
    extern float ReceivingPassDelayed(cFielder*);
    extern float ChasingBall(cPlayer*);
    extern float InDefensiveZoneOfPlayer(cBall*, cPlayer*);
    extern float NearToBall(cPlayer*);
    extern float AbleToInterceptBall(cPlayer*);
    extern float InDefensiveZone(cPlayer*);
    extern float Marking(cFielder*, cPlayer*);
    extern float GonnaGetBall(cTeam*);
    extern float Captain(cFielder*);
    extern float InOffensiveZone(cPlayer*);
    extern float Shooter(cFielder*);
    extern float Passer(cFielder*);
    extern float ClosingTo(cPlayer*, cPlayer*);
    extern float NearTo(cPlayer*, cPlayer*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    volatile unsigned long funcAddr = (unsigned long)GetBestLooseBallAction;
    unsigned long hash = funcAddr + ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    if (ScriptQuestionCache::Instance()->Lookup(hash, bestValue, NULL))
    {
        ScriptQuestionCache::Instance()->AddToCache(hash, bestValue, NULL);
        return bestValue;
    }

    float fDifficulty = NormalizeVal(Difficult(g_pScriptCurrentTeam), 1.0f, 0.0f);
    float fTrueConfidence = 1.0f - UserControlledT(g_pScriptCurrentTeam);
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
            fConfidence = (float)(double)fConfidence * fBranchRatio;
        }

        float fCanSlide = TheFielder->CanISlideAttack(g_pScriptBall->m_v3Position, g_pScriptBall->m_v3Velocity, NULL) ? 1.0f : 0.0f;

        float fOnMushrooms = OnMushrooms(TheFielder);
        float fOwnerless = 1.0f - Ownerless(g_pScriptBall);
        float fNotOpen = 1.0f - Open(TheFielder);
        float fPressured = Pressured(TheFielder);

        fOwnerless = (fOwnerless >= fOnMushrooms) ? fOwnerless : fOnMushrooms;
        fNotOpen = (fNotOpen >= fOwnerless) ? fNotOpen : fOwnerless;
        if (fPressured >= fNotOpen)
        {
            fNotOpen = fPressured;
        }

        float fOnMush2 = OnMushrooms(TheFielder);
        float fNotFacing = 1.0f - FacingSideline(TheFielder);
        fNotFacing = (fNotFacing >= fOnMush2) ? fNotFacing : fOnMush2;
        fNotFacing = (fNotFacing <= fNotOpen) ? fNotFacing : fNotOpen;
        fCanSlide = (fCanSlide <= fNotFacing) ? fCanSlide : fNotFacing;

        fTrueConfidence = fCanSlide;
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
                fConfidence = (float)(double)fConfidence * fBranchRatio;
            }

            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            float fSelectChance = CalcSelectChance(pSkillTweaks->Def_SlideAttackChance, Aggressive(TheFielder));
            float fStallingGreater = FGREATER(Stalling(g_pScriptOtherTeam), fDifficulty);
            if (fStallingGreater >= fSelectChance)
            {
                fSelectChance = fStallingGreater;
            }

            FuzzyVariant returnAction(15);
            returnAction.Confidence = 0.0f;
            returnAction.SelectionChance = fSelectChance;

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = returnAction;
            }
        }

        cTeam* otherTeam = TheFielder ? ((cPlayer*)TheFielder)->m_pTeam->GetOtherTeam() : NULL;
        FuzzyVariant otherSBC = Fuzzy::GetStrategicBallCarrier(otherTeam);

        float fCloseTo = CloseTo((cPlayer*)TheFielder, otherSBC.mData.pPlayer);
        float fAtIdeal = AtIdealDistanceForTackling((cPlayer*)TheFielder, otherSBC.mData.pPlayer);
        if (fAtIdeal >= fCloseTo)
        {
            fCloseTo = fAtIdeal;
        }
        float fNotSeparating = 1.0f - SeparatingFrom((cPlayer*)TheFielder, otherSBC.mData.pPlayer);
        float fOnScreen2 = OnScreen(otherSBC.mData.pPlayer);
        float fNotFarToBall = 1.0f - FarToBall((cPlayer*)TheFielder);

        fNotSeparating = (fNotSeparating <= fCloseTo) ? fNotSeparating : fCloseTo;
        fOnScreen2 = (fOnScreen2 <= fNotSeparating) ? fOnScreen2 : fNotSeparating;
        fNotFarToBall = (fNotFarToBall <= fOnScreen2) ? fNotFarToBall : fOnScreen2;

        fTrueConfidence = fNotFarToBall;
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
                fConfidence = (float)(double)fConfidence * fBranchRatio;
            }

            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            float fSelectChance = CalcSelectChance(pSkillTweaks->Loose_HeavyAttackChance, Aggressive(TheFielder));
            float fStallingGreater = FGREATER(Stalling(g_pScriptOtherTeam), fDifficulty);
            if (fStallingGreater >= fSelectChance)
            {
                fSelectChance = fStallingGreater;
            }

            FuzzyVariant hitAction(5);
            hitAction.ExtraData = *(Variant*)&otherSBC;
            hitAction.Confidence = 0.0f;
            hitAction.SelectionChance = fSelectChance;

            if (BallOwner(otherSBC.mData.pPlayer) != 0.0f)
            {
                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = hitAction;
                }
            }
            else
            {
                if (WindingUpForShot((cFielder*)otherSBC.mData.pPlayer) != 0.0f)
                {
                    cTeam* fielderTeam = TheFielder ? ((cPlayer*)TheFielder)->m_pTeam : NULL;
                    float fLosing = Losing(fielderTeam);
                    float fTimeNearly = TimeNearlyOver(g_pGame);
                    if (fTimeNearly <= fLosing)
                    {
                        fLosing = fTimeNearly;
                    }

                    float fRandom = RandomChance(0.5f);
                    fRandom = (fRandom >= fLosing) ? fRandom : fLosing;

                    fTrueConfidence = fRandom * 0.5f + 0.5f;
                    fFalseConfidence = 1.0f - fTrueConfidence;
                    fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    fBranchRatio = fMinVal / fMaxVal;

                    if (fTrueConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM4(&fConfidence);

                        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        {
                            fConfidence = (float)(double)fConfidence * fBranchRatio;
                        }

                        if (fConfidence > fBestConfidence)
                        {
                            fBestConfidence = fConfidence;
                            bestValue = hitAction;
                        }
                    }
                }
                else
                {
                    float fRecvPass = ReceivingPassDelayed((cFielder*)otherSBC.mData.pPlayer);
                    float fChasing = ChasingBall(otherSBC.mData.pPlayer);
                    fChasing = (fChasing >= fRecvPass) ? fChasing : fRecvPass;

                    if (fChasing != 0.0f)
                    {
                        cTeam* fielderTeam = TheFielder ? ((cPlayer*)TheFielder)->m_pTeam : NULL;
                        float fLosing = Losing(fielderTeam);
                        float fTimeNearly = TimeNearlyOver(g_pGame);
                        if (fTimeNearly <= fLosing)
                        {
                            fLosing = fTimeNearly;
                        }

                        float fRandom = RandomChance(0.4f);
                        if (fRandom >= fLosing)
                        {
                            fLosing = fRandom;
                        }

                        float fDefZone = InDefensiveZoneOfPlayer(g_pScriptBall, (cPlayer*)TheFielder);
                        cPlayer* pOtherPlayer = otherSBC.mData.pPlayer;
                        float fNearBallFielder = NearToBall((cPlayer*)TheFielder);
                        float fNearBallOther = NearToBall(pOtherPlayer);
                        float fGreater = FGREATER(fNearBallFielder, fNearBallOther);

                        if (fGreater >= fDefZone)
                        {
                            fDefZone = fGreater;
                        }

                        float fAbleToIntercept = AbleToInterceptBall(otherSBC.mData.pPlayer);
                        fTrueConfidence = fDefZone * 0.3f + fAbleToIntercept * 0.3f + fLosing * 0.4f;
                        fFalseConfidence = 1.0f - fTrueConfidence;
                        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                        fBranchRatio = fMinVal / fMaxVal;

                        if (fTrueConfidence > 0.0f)
                        {
                            SaveConfidence PushDOM5(&fConfidence);

                            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                            {
                                fConfidence = (float)(double)fConfidence * fBranchRatio;
                            }

                            if (fConfidence > fBestConfidence)
                            {
                                fBestConfidence = fConfidence;
                                bestValue = hitAction;
                            }
                        }
                    }
                }
            }
        }

        FuzzyVariant powerupToUse = Fuzzy::GetPowerupToUseForPassReceiveDefence(TheFielder);

        FuzzyVariant returnAction2(18);
        returnAction2.ExtraData = *(Variant*)&powerupToUse;
        returnAction2.Confidence = 0.0f;
        returnAction2.SelectionChance = 1.0f;

        SkillTweaks* pSkillTweaks2 = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
        returnAction2.SelectionChance = CalcSelectChance(pSkillTweaks2->Off_PassReceivePowerupChance, Aggressive(TheFielder));

        fTrueConfidence = (powerupToUse.mData.i == 7) ? 1.0f : 0.0f;
        fTrueConfidence = (fTrueConfidence <= powerupToUse.Confidence) ? fTrueConfidence : powerupToUse.Confidence;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM6(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)(double)fConfidence * fBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = returnAction2;
            }
        }

        if (fFalseConfidence > 0.0f)
        {
            SaveConfidence PushDOM7(&fConfidence);

            fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
            if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            {
                fConfidence = (float)(double)fConfidence * fBranchRatio;
            }

            float fNotDefZone = 1.0f - InDefensiveZone(otherSBC.mData.pPlayer);
            float fNotCloseTo = 1.0f - CloseTo((cPlayer*)TheFielder, otherSBC.mData.pPlayer);
            float fNotBestConf = 1.0f - fBestConfidence;

            fNotCloseTo = (fNotCloseTo <= fNotDefZone) ? fNotCloseTo : fNotDefZone;

            fNotBestConf = (fNotBestConf <= fNotCloseTo) ? fNotBestConf : fNotCloseTo;
            fNotBestConf = (powerupToUse.Confidence <= fNotBestConf) ? fNotBestConf : powerupToUse.Confidence;

            fTrueConfidence = fNotBestConf;
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM7b(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)(double)fConfidence * fBranchRatio;
                }

                float fMarking = Marking(TheFielder, otherSBC.mData.pPlayer);
                float fWindingUp = WindingUpForShot((cFielder*)otherSBC.mData.pPlayer);
                if (fWindingUp <= fMarking)
                {
                    fMarking = fWindingUp;
                }

                cTeam* otherTeam2 = TheFielder ? ((cPlayer*)TheFielder)->m_pTeam->GetOtherTeam() : NULL;
                float fGonnaGet = GonnaGetBall(otherTeam2);
                float fDefZone2 = InDefensiveZone((cPlayer*)TheFielder);
                if (fDefZone2 <= fGonnaGet)
                {
                    fGonnaGet = fDefZone2;
                }

                float fAbleTo2 = AbleToInterceptBall((cPlayer*)TheFielder);
                float fCaptain = Captain(TheFielder);
                float fOffZone = InOffensiveZone((cPlayer*)TheFielder);

                fCaptain = (fCaptain <= fAbleTo2) ? fCaptain : fAbleTo2;
                fOffZone = (fOffZone <= fCaptain) ? fOffZone : fCaptain;

                float fIsGoalie = (powerupToUse.mData.i == 8) ? 1.0f : 0.0f;

                fGonnaGet = (fGonnaGet >= fMarking) ? fGonnaGet : fMarking;
                fOffZone = (fOffZone >= fGonnaGet) ? fOffZone : fGonnaGet;
                fIsGoalie = (fIsGoalie >= fOffZone) ? fIsGoalie : fOffZone;

                fTrueConfidence = fIsGoalie;
                fFalseConfidence = 1.0f - fTrueConfidence;
                fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMinVal / fMaxVal;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM8(&fConfidence);

                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    {
                        fConfidence = (float)(double)fConfidence * fBranchRatio;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = returnAction2;
                    }
                }
            }
        }

        FuzzyVariant oneTimerScore = Fuzzy::ShouldIAttemptOneTimer(TheFielder);

        float fCanShoot = TheFielder->CanLooseBallShoot() ? 1.0f : 0.0f;
        fTrueConfidence = (oneTimerScore.mData.f <= fCanShoot) ? oneTimerScore.mData.f : fCanShoot;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM9(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)(double)fConfidence * fBranchRatio;
            }

            FuzzyVariant shotAction(14);
            shotAction.ExtraData = Variant(FT_BOOL, false);
            shotAction.SelectionChance = oneTimerScore.SelectionChance;

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = shotAction;
            }
        }

        fTrueConfidence = Fuzzy::GoodToChipShot(TheFielder).mData.f * 0.5f + oneTimerScore.mData.f * 0.5f;
        float fCanShoot2 = TheFielder->CanLooseBallShoot() ? 1.0f : 0.0f;
        if (fCanShoot2 <= fTrueConfidence)
        {
        }
        else
        {
            fTrueConfidence = fCanShoot2;
        }

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOMa(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)(double)fConfidence * fBranchRatio;
            }

            FuzzyVariant shotAction2(14);
            shotAction2.ExtraData = Variant(FT_BOOL, true);
            shotAction2.Confidence = 0.0f;
            shotAction2.SelectionChance = 1.0f;

            SkillTweaks* pSkillTweaks3 = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            shotAction2.SelectionChance = CalcSelectChance(pSkillTweaks3->Off_ChipShotChance, Shooter(TheFielder));

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = shotAction2;
            }
        }

        FuzzyVariant bestPassTargetFielder = Fuzzy::GetBestLooseBallPassTarget(TheFielder);

        float fClosingTo = ClosingTo(otherSBC.mData.pPlayer, (cPlayer*)TheFielder);
        float fNearTo = NearTo(otherSBC.mData.pPlayer, (cPlayer*)TheFielder);
        if (fNearTo <= fClosingTo)
        {
            fClosingTo = fNearTo;
        }

        float fDanger = Fuzzy::InDangerDelayed(TheFielder).mData.f;
        fDanger = (fDanger >= fClosingTo) ? fDanger : fClosingTo;

        float fNotCloseToPass = 1.0f - CloseTo(bestPassTargetFielder.mData.pPlayer, (cPlayer*)TheFielder);
        float fGreater2 = FGREATER(1.0f - fNotCloseToPass, bestPassTargetFielder.SelectionChance);
        float fCanPass = TheFielder->CanLooseBallPass() ? 1.0f : 0.0f;

        fNotCloseToPass = (fNotCloseToPass <= fDanger) ? fNotCloseToPass : fDanger;
        fGreater2 = (fGreater2 <= fNotCloseToPass) ? fGreater2 : fNotCloseToPass;
        fCanPass = (fCanPass <= fGreater2) ? fCanPass : fGreater2;

        fTrueConfidence = fCanPass;
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOMb(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)(double)fConfidence * fBranchRatio;
            }

            FuzzyVariant passAction(13);
            passAction.ExtraData = *(Variant*)&bestPassTargetFielder;
            passAction.Confidence = 0.0f;
            passAction.SelectionChance = 1.0f;

            SkillTweaks* pSkillTweaks4 = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            passAction.SelectionChance = CalcSelectChance(pSkillTweaks4->Loose_GroundPassChance, Passer(TheFielder));

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = passAction;
            }
        }
    }

    bestValue.Confidence = fBestConfidence;
    ScriptQuestionCache::Instance()->AddToCache(hash, bestValue, NULL);
    return bestValue;
}

/**
 * Offset/Address/Size: 0x2CB4 | 0x8006CE84 | size: 0x131C
 * TODO: 66.74% match - remaining divergence is mainly stack/register layout around FuzzyVariant temporaries
 */
FuzzyVariant Fuzzy::GetBestWindupShotAction(cFielder* TheFielder)
{
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern float LikelyToScore(cFielder*);
    extern float Stunned(Goalie*);
    extern float Invincible(cFielder*);
    extern float Captain(cFielder*);
    extern float OpenToTheirNet(cFielder*);
    extern float Open(cFielder*);
    extern float CalcSelectChance(float, float);
    extern float Aggressive(cFielder*);
    extern float Passer(cFielder*);
    extern float CloseToSideline(cFielder*);
    extern float CloseToTheirGoalie(cPlayer*);
    extern float RepeatingLastDesire(cFielder*, eScriptFielderDesire);
    extern float FLESS(float, float);
    extern float Attacked(cFielder*);
    extern float Deker(cFielder*);
    extern float NearToTheirGoalie(cPlayer*);
    extern float FGREATER(float, float);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    volatile unsigned long funcAddrTemp = (unsigned long)GetBestWindupShotAction;
    unsigned long hash = ((Variant*)&fvFielder)->GetHash();
    hash += funcAddrTemp;
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    unsigned long hashKey = hash;
    unsigned char lookupFound;
    FuzzyVariant* pValue;

    ScriptQuestionCache* cache;
    cache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    cache->mTotalLookups++;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<std::pair<const unsigned long, FuzzyVariant> > >::iterator stdFound = cache->mQuestionCacheMapSTD.find(hashKey);
        if ((StdMapNodeBase*)stdFound.ptr_ != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = ((StdMapNode*)stdFound.ptr_)->value;
            lookupFound = 1;
        }
        else
        {
            lookupFound = 0;
        }
    }
    else
    {
        AVLTreeEntry<unsigned long, FuzzyVariant>* node = cache->mQuestionCacheMap.m_Root;

        while (node != NULL)
        {
            int cmpResult;
            if (hashKey == node->key)
            {
                cmpResult = 0;
            }
            else if (hashKey < node->key)
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
        unsigned long hashCopy1 = hashKey;
        if (g_bScriptQuestionCachingOn)
        {
            if (g_bScriptQuestionCachingUseSTD)
            {
                std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy1);
                pair.second = bestValue;
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

    float fTrueConfidence = LikelyToScore(TheFielder);

    Goalie* pGoalie = NULL;
    if (TheFielder != NULL)
    {
        if (TheFielder != NULL)
        {
            pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
        }
    }

    float fGoalieStunned = Stunned(pGoalie);
    if (fGoalieStunned > fTrueConfidence)
    {
        fTrueConfidence = fGoalieStunned;
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
            FuzzyVariant returnAction(14);
            bestValue = returnAction;
        }
    }

    fTrueConfidence = 1.0f - Invincible(TheFielder);
    fFalseConfidence = 1.0f - fTrueConfidence;
    fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        fTrueConfidence = Captain(TheFielder) * 0.2f;
        fTrueConfidence += (1.0f - OpenToTheirNet(TheFielder)) * 0.2f;
        fTrueConfidence += (1.0f - Open(TheFielder)) * 0.6f;
        fFalseConfidence = 1.0f - fTrueConfidence;
        fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            FuzzyVariant powerupToUse;
            powerupToUse = Fuzzy::GetPowerupToUseForWindupDefence(TheFielder);

            fTrueConfidence = powerupToUse.Confidence;
            fFalseConfidence = 1.0f - fTrueConfidence;
            fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                FuzzyVariant returnAction(18);
                returnAction.ExtraData = (Variant&)powerupToUse;

                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                returnAction.SelectionChance = CalcSelectChance(pSkillTweaks->Off_WindupPowerupChance, Aggressive(TheFielder));

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = returnAction;
                }
            }
        }
    }

    fTrueConfidence = Fuzzy::InDangerDelayed(TheFielder).Confidence;
    fFalseConfidence = 1.0f - fTrueConfidence;
    fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        FuzzyVariant bestPassTargetFielder;
        bestPassTargetFielder = Fuzzy::GetBestPassTarget((cPlayer*)TheFielder);

        fTrueConfidence = FGREATER(bestPassTargetFielder.Confidence, 0.3f);
        fFalseConfidence = 1.0f - fTrueConfidence;
        fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            FuzzyVariant passAction(13);
            passAction.ExtraData = (Variant&)bestPassTargetFielder;

            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            passAction.SelectionChance = CalcSelectChance(pSkillTweaks->Off_WindupPassChance, Passer(TheFielder));

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = passAction;
            }
        }

        float fNotCloseToSideline = 1.0f - CloseToSideline(TheFielder);
        float fNotCloseToTheirGoalie = 1.0f - CloseToTheirGoalie((cPlayer*)TheFielder);
        float fNotRepeatingLastDesire = 1.0f - RepeatingLastDesire(TheFielder, (eScriptFielderDesire)2);
        float fNeedDeke = FLESS(Open(TheFielder), 0.5f);
        float fAttacked = Attacked(TheFielder);

        if (fAttacked < fNeedDeke)
        {
            fAttacked = fNeedDeke;
        }

        if (fNotCloseToTheirGoalie > fNotCloseToSideline)
        {
            fNotCloseToTheirGoalie = fNotCloseToSideline;
        }

        if (fNotRepeatingLastDesire > fNotCloseToTheirGoalie)
        {
            fNotRepeatingLastDesire = fNotCloseToTheirGoalie;
        }

        if (fAttacked > fNotRepeatingLastDesire)
        {
            fAttacked = fNotRepeatingLastDesire;
        }

        fTrueConfidence = fAttacked;
        fFalseConfidence = 1.0f - fTrueConfidence;
        fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            FuzzyVariant returnAction(2);

            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            returnAction.SelectionChance = CalcSelectChance(pSkillTweaks->Off_WindupDekeChance, Deker(TheFielder));

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = returnAction;
            }
        }
    }

    fTrueConfidence = fConfidence;
    float fNearToTheirGoalie = NearToTheirGoalie((cPlayer*)TheFielder);
    if (fTrueConfidence < fNearToTheirGoalie)
    {
        fTrueConfidence = fNearToTheirGoalie;
    }

    float fNoBestConfidence = FGREATER(1.0f - fBestConfidence, 0.5f);
    if (fNoBestConfidence < fTrueConfidence)
    {
        fNoBestConfidence = fTrueConfidence;
    }

    fTrueConfidence = fNoBestConfidence;
    fFalseConfidence = 1.0f - fTrueConfidence;
    fBranchRatio = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fBranchRatio / ((fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence);

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if ((fConfidence < fTrueConfidence) && (fTrueConfidence < 0.5f))
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant returnAction(14);
            bestValue = returnAction;
        }
    }

    bestValue.Confidence = fBestConfidence;

    unsigned long hashCopy2 = hashKey;
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy2);
            pair.second = bestValue;
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
        cTeam* pOtherTeam;
        if (TheFielder != NULL)
        {
            pOtherTeam = TheFielder->m_pTeam->GetOtherTeam();
        }
        else
        {
            pOtherTeam = NULL;
        }
        pOtherGoalie = pOtherTeam->GetGoalie();
    }
    else
    {
        pOtherGoalie = NULL;
    }

    float fNotOtherGoaliePickup = 1.0f - Fuzzy::GoalieAndGonnaPickupBall(pOtherGoalie).mData.f;

    cPlayer* pGoalie;
    if (TheFielder != NULL)
    {
        cTeam* pTeamForGoalie;
        if (TheFielder != NULL)
        {
            pTeamForGoalie = TheFielder->m_pTeam;
        }
        else
        {
            pTeamForGoalie = NULL;
        }
        pGoalie = pTeamForGoalie->GetGoalie();
    }
    else
    {
        pGoalie = NULL;
    }

    float fNotGoaliePickup = 1.0f - Fuzzy::GoalieAndGonnaPickupBall(pGoalie).mData.f;

    fNotUserControlled = (fNotUserControlled <= fTrueConfidence) ? fNotUserControlled : fTrueConfidence;
    fNotOtherGoaliePickup = (fNotOtherGoaliePickup <= fNotUserControlled) ? fNotOtherGoaliePickup : fNotUserControlled;
    fNotGoaliePickup = (fNotGoaliePickup <= fNotOtherGoaliePickup) ? fNotGoaliePickup : fNotOtherGoaliePickup;

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
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
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
                double d = fConfidence;
                fConfidence = (float)d * fLikelyBranchRatio;
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
                double d = fConfidence;
                fConfidence = (float)d * fLikelyBranchRatio;
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
                double d = fConfidence;
                fConfidence = (float)d * fLikelyBranchRatio;
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
                double d = fConfidence;
                fConfidence = (float)d * fLikelyBranchRatio;
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
                double d = fConfidence;
                fConfidence = (float)d * fLikelyBranchRatio;
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
                double d = fConfidence;
                fConfidence = (float)d * fLikelyBranchRatio;
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

        fNotNearBall = (fNotNearBall <= fTrueConfidence) ? fNotNearBall : fTrueConfidence;
        fChasingBall = (fChasingBall <= fNotNearBall) ? fChasingBall : fNotNearBall;

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
                double d = fConfidence;
                fConfidence = (float)d * fBranchRatio;
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
                    double d = fConfidence;
                    fConfidence = (float)d * fBranchRatio;
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
                        double d = fConfidence;
                        fConfidence = (float)d * fLikelyBranchRatio;
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

        fBallOwner = (fBallOwner >= fReceivingPass) ? fBallOwner : fReceivingPass;
        fCaptain = (fCaptain <= fNotInDefensiveZone) ? fCaptain : fNotInDefensiveZone;
        fBallOwner = (fBallOwner <= fCaptain) ? fBallOwner : fCaptain;

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
                double d = fConfidence;
                fConfidence = (float)d * fBranchRatio;
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
                    double d = fConfidence;
                    fConfidence = (float)d * fLikelyBranchRatio;
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

    ((Variant*)&FuzzyVariant((cPlayer*)TheFielder))->GetHash();
    FuzzyVariant((cPlayer*)TheFielder);

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
FuzzyVariant Fuzzy::InDanger(cFielder* TheFielder)
{
    extern float FGREATER(float, float);
    extern float AvoidingPowerups(cFielder*);
    extern float StuckOnSidelines(cFielder*);
    extern float Open(cFielder*);
    extern float Pressured(cFielder*);
    extern float Attacked(cFielder*);

    FuzzyVariant bestValue;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    unsigned long funcAddr = (unsigned long)InDanger;
    unsigned long hash = ((Variant*)&fvFielder)->GetHash();
    hash += funcAddr;
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    ScriptQuestionCache* cache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    cache->mTotalLookups++;

    unsigned char lookupFound;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant>::iterator stdIt = cache->mQuestionCacheMapSTD.find(hash);
        if (*(StdMapNodeBase**)&stdIt != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = (*(StdMapNode**)&stdIt)->value;
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
                std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy1);
                pair.second = bestValue;
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

    float fDanger = FGREATER(1.0f - Open(TheFielder), 0.35f);
    float fOther = AvoidingPowerups(TheFielder);
    float fThird = StuckOnSidelines(TheFielder);
    float fFourth = Pressured(TheFielder);
    float fFifth = Attacked(TheFielder);

    if (fOther < fDanger)
    {
        fOther = fDanger;
    }

    if (fThird < fOther)
    {
        fThird = fOther;
    }

    if (fFourth < fThird)
    {
        fFourth = fThird;
    }

    if (fFifth > fFourth)
    {
        fFourth = fFifth;
    }

    FuzzyVariant fvResult(fFourth);
    bestValue = fvResult;
    bestValue.Confidence = 1.0f;

    unsigned long hashCopy2 = hash;

    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy2);
            pair.second = bestValue;
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
    unsigned long funcAddr = (unsigned long)InDangerDelayed;
    unsigned long hash = ((Variant*)&fvFielder)->GetHash();
    hash += funcAddr;
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    ScriptQuestionCache* cache = nlSingleton<ScriptQuestionCache>::s_pInstance;
    cache->mTotalLookups++;

    unsigned char lookupFound;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        std::map<unsigned long, FuzzyVariant>::iterator stdIt = cache->mQuestionCacheMapSTD.find(hash);
        if (*(StdMapNodeBase**)&stdIt != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = (*(StdMapNode**)&stdIt)->value;
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
                std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy1);
                pair.second = bestValue;
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
    if (fOther < fTrueConfidence)
    {
        fOther = fTrueConfidence;
    }
    fTrueConfidence = fOther;

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
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
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
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
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
                double d = fConfidence;
                fConfidence = (float)d * fBranchRatio;
            }
            static nlVector2 g_vInDangerDelayedMin = { 0.8f, 0.2f };
            static nlVector2 g_vInDangerDelayedMax = { 1.0f, 1.0f };
            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            float fMin = Interpolate(g_vInDangerDelayedMin.f.x, g_vInDangerDelayedMin.f.y, pSkillTweaks->Off_Reaction);
            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            float fMax = Interpolate(g_vInDangerDelayedMax.f.x, g_vInDangerDelayedMax.f.y, pSkillTweaks->Off_Reaction);

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
            std::pair<const unsigned long, FuzzyVariant>& pair = cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(hashCopy2);
            pair.second = bestValue;
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
    m.tree_.find_or_insert<unsigned long, FuzzyVariant>(k);
    std::__red_black_tree<1>::node_base* n = 0;
    std::__red_black_tree<1>::rotate_left(n, n);
    std::__red_black_tree<1>::rotate_right(n, n);
}
