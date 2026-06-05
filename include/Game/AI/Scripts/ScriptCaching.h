#ifndef _SCRIPTCACHING_H_
#define _SCRIPTCACHING_H_

// ScriptQuestionCache::Lookup / AddToCache are defined here (not in CommonScript.cpp)
// so their compile unit is this header, matching the original's weak-symbol
// emission order. This header is not self-contained: it relies on the cache helper
// types (StdMapNode/StdMapTree/FuzzyMapPair), the __find/__find_or_insert decls,
// and the ScriptQuestionCache class being visible before it is included.

/**
 * Offset/Address/Size: 0xE4 | 0x80079D64 | size: 0x1B4
 * TODO: Remaining diff is std::tree find call symbol (__find wrapper vs templated std::__tree::find<Ul>)
 */
inline unsigned char ScriptQuestionCache::Lookup(unsigned long hash, FuzzyVariant& returnVal, const char* name)
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

/**
 * Offset/Address/Size: 0x0 | 0x80079C80 | size: 0xE4
 */
#pragma dont_inline on
inline const FuzzyVariant& ScriptQuestionCache::AddToCache(unsigned long key, const FuzzyVariant& variant, const char* name)
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

#endif // _SCRIPTCACHING_H_
