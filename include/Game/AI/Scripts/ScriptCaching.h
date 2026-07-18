#ifndef _SCRIPTCACHING_H_
#define _SCRIPTCACHING_H_

#include "NL/nlSingleton.h"
#include "NL/nlAVLTree.h"
#include "Game/AI/FuzzyVariant.h"
#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/msl_tree.h"

extern unsigned char g_bScriptQuestionCachingOn;
extern unsigned char g_bScriptQuestionCachingUseSTD;

typedef std::pair<const unsigned long, FuzzyVariant> ScriptCachePair;
typedef std::map<unsigned long, FuzzyVariant, std::less<unsigned long>, std::allocator<ScriptCachePair> > ScriptCacheMap;
typedef std::__tree<ScriptCachePair, ScriptCacheMap::value_compare, std::allocator<ScriptCachePair> > ScriptCacheTree;

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
    /* 0x28 */ ScriptCacheMap mQuestionCacheMapSTD;
    /* 0x38 */ int mTotalLookups;
    /* 0x3C */ int mCacheHits;
}; // total size: 0x40

inline ScriptQuestionCache::~ScriptQuestionCache()
{
    Clear();
    if ((ScriptCacheTree*)&mQuestionCacheMapSTD)
    {
        if ((ScriptCacheTree*)&mQuestionCacheMapSTD)
        {
            ScriptCacheTree::node* n = *(
                ScriptCacheTree::node**)&((ScriptCacheTree&)mQuestionCacheMapSTD).node_alloc();
            if (n)
            {
                ((ScriptCacheTree&)mQuestionCacheMapSTD).destroy(n);
            }
        }
    }
}

inline void ScriptQuestionCache::Clear()
{
    mQuestionCacheMap.Clear();
    mQuestionCacheMapSTD.tree_.clear();
    mCacheHits = 0;
    mTotalLookups = 0;
}
inline unsigned char ScriptQuestionCache::Lookup(
    unsigned long hash, FuzzyVariant& returnVal, const char* name)
{
    struct MapNodeBase
    {
        void* left;
        void* right;
        void* parent;
    };

    struct MapTree
    {
        unsigned long x0;
        MapNodeBase x4;
    };

    struct MapNode
    {
        MapNodeBase base;
        unsigned long key;
        FuzzyVariant value;
    };

    FuzzyVariant* pValue;

    mTotalLookups++;

    if (g_bScriptQuestionCachingUseSTD)
    {
        MapNode* stdFound = (MapNode*)mQuestionCacheMapSTD.find(hash).ptr_;
        if ((MapNodeBase*)stdFound != &((MapTree*)&mQuestionCacheMapSTD)->x4)
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

inline const FuzzyVariant& ScriptQuestionCache::AddToCache(
    unsigned long key, const FuzzyVariant& variant, const char* name)
{
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            std::pair<const unsigned long, FuzzyVariant>& pair =
                mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(key);
            pair.second = variant;
        }
        else
        {
            AVLTreeNode* existingNode;
            mQuestionCacheMap.AddAVLNode(
                (AVLTreeNode**)&mQuestionCacheMap.m_Root,
                (void*)&key,
                (void*)&variant,
                &existingNode,
                mQuestionCacheMap.m_NumElements);
            if (existingNode == NULL)
            {
                mQuestionCacheMap.m_NumElements++;
            }
        }
    }
    return variant;
}
#endif // _SCRIPTCACHING_H_
