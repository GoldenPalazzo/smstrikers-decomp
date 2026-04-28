#include "Game/Effects/EffectsTemplate.h"
#include "Game/Effects/EmissionController.h"
#include "Game/Effects/EmissionManager.h"
#include "stdlib.h"
#include "NL/nlAVLTree.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlDLRing.h"
#include "NL/nlMath.h"
#include "NL/nlString.h"
#include "Game/Sys/simpleparser.h"

static nlAVLTree<unsigned long, EffectsTemplate*, DefaultKeyCompare<unsigned long> >* pTemplateMap = nullptr;
static unsigned int uSeed = 0x9184EB0C;

struct ColourKey
{
    int index;
    int value;
};

struct ColourKeyListShim
{
    NewAdapter<DLListEntry<ColourKey> > m_Allocator;
    DLListEntry<ColourKey>* m_Head;
};

// /**
//  * Offset/Address/Size: 0x0 | 0x801F29E8 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<ColourKey>, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > > >(DLListEntry<ColourKey>*, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > >*, void (DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > >::*)(DLListEntry<ColourKey>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xB0 | 0x801F29B0 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<ColourKey> >(DLListEntry<ColourKey>**, DLListEntry<ColourKey>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x74 | 0x801F2974 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<ColourKey> >(DLListEntry<ColourKey>**, DLListEntry<ColourKey>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x801F295C | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<ColourKey> >(DLListEntry<ColourKey>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x801F293C | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<ColourKey> >(DLListEntry<ColourKey>*, DLListEntry<ColourKey>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801F2900 | size: 0x3C
//  */
// void nlWalkDLRing<DLListEntry<ColourKey>, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > > >(DLListEntry<ColourKey>*, DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > >*, void (DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > >::*)(DLListEntry<ColourKey>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801F28DC | size: 0x24
//  */
// void DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > >::DeleteEntry(DLListEntry<ColourKey>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801F28B8 | size: 0x24
//  */
// void NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >::Delete(AVLTreeEntry<unsigned long, EffectsTemplate*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x994 | 0x801F2854 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x968 | 0x801F2828 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x93C | 0x801F27FC | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8E4 | 0x801F27A4 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::DeleteValues()
// {
// }

// /**
//  * Offset/Address/Size: 0x8DC | 0x801F279C | size: 0x8
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x184 | 0x801F2044 | size: 0x758
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::PostorderTraversal(AVLTreeEntry<unsigned long, EffectsTemplate*>*, void (AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, EffectsTemplate*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x120 | 0x801F1FE0 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::DestroyTree(void (AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, EffectsTemplate*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xC8 | 0x801F1F88 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x6C | 0x801F1F2C | size: 0x5C
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x48 | 0x801F1F08 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::DeleteEntry(AVLTreeEntry<unsigned long, EffectsTemplate*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801F1EC0 | size: 0x48
//  */
// void AVLTreeBase<unsigned long, EffectsTemplate*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> >, DefaultKeyCompare<unsigned long> >::DeleteValue(AVLTreeEntry<unsigned long, EffectsTemplate*>*)
// {
// }

/**
 * Offset/Address/Size: 0x12D8 | 0x801F1E9C | size: 0x24
 */
float RandomizedValue(float value)
{
    return nlRandomf(value, &uSeed);
}

/**
 * Offset/Address/Size: 0x1260 | 0x801F1E24 | size: 0x78
 */
float RandomizedValue(float base, float range)
{
    float randomOffset = nlRandomf(0.5f * range, &uSeed);
    unsigned int randomSign = nlRandom(0x7FFFFFFF, &uSeed);

    if (randomSign & 1)
    {
        return base + randomOffset;
    }
    else
    {
        return base - randomOffset;
    }
}

/**
 * Offset/Address/Size: 0xFD4 | 0x801F1B98 | size: 0x28C
 * TODO: 84.24% match - stmw r24 vs r25 register allocation mismatch;
 * compiler uses 8 callee-saved regs instead of 7, causing cascading
 * stack frame (0x90 vs 0xa0) and register assignment differences.
 */
static void BlendSpan(nlColour* pColour, int cindex, const ColourKey& k0, const ColourKey& k1)
{
    pColour[k1.index].c[cindex] = k1.value;
    int index = k0.index;
    int value = k0.value;
    int valueDiff = k1.value - value;
    int indexDiff = k1.index - index;
    float step = (float)valueDiff / (float)indexDiff;
    float current = (float)value;
    nlColour* p = pColour + index;
    unsigned char* channel = (unsigned char*)p + cindex;
    while (index < k1.index)
    {
        *channel = current + 0.5f;
        p++;
        current += step;
        index++;
        channel += sizeof(nlColour);
    }
}

void GetColourComponent(SimpleParser* parser, nlColour* pColour, int cindex)
{
    typedef DLListContainerBase<ColourKey, NewAdapter<DLListEntry<ColourKey> > > ColourKeyList;
    char ind[8];
    char val[8];
    ColourKey key;
    ColourKeyListShim keys;
    keys.m_Head = NULL;
    while (true)
    {
        char* token = parser->NextTokenOnLine(true);
        if (token == NULL)
            break;
        char* cp = token;
        int i = 0;
        char* indp = ind;
        while (*cp != ':')
        {
            *indp++ = *cp++;
            i++;
        }
        ind[i] = 0;
        cp++;
        i = 0;
        char* valp = val;
        while ((char)*cp != 0)
        {
            *valp++ = *cp++;
            i++;
        }
        val[i] = 0;
        int index = atoi(ind);
        int value = atoi(val);
        key.index = index;
        key.value = value;
        DLListEntry<ColourKey>* entry = (DLListEntry<ColourKey>*)nlMalloc(0x10, 8, false);
        if (entry != NULL)
        {
            entry->m_next = NULL;
            entry->m_prev = NULL;
            entry->m_data.index = index;
            entry->m_data.value = key.value;
        }
        nlDLRingAddEnd<DLListEntry<ColourKey> >(&keys.m_Head, entry);
    }
    DLListEntry<ColourKey>* start = nlDLRingGetStart<DLListEntry<ColourKey> >(keys.m_Head);
    DLListEntry<ColourKey>* head = keys.m_Head;
    DLListEntry<ColourKey>* current = start;
    while (true)
    {
        if (nlDLRingIsEnd<DLListEntry<ColourKey> >(head, current) || current == NULL)
            current = NULL;
        else
            current = current->m_next;
        if (current == NULL)
            break;
        BlendSpan(pColour, cindex, start->m_data, current->m_data);
        start->m_data.index = current->m_data.index;
        start->m_data.value = current->m_data.value;
    }
    nlWalkDLRing<DLListEntry<ColourKey>, ColourKeyList>(
        keys.m_Head, (ColourKeyList*)&keys, &ColourKeyList::DeleteEntry);
    keys.m_Head = NULL;
}

/**
 * Offset/Address/Size: 0x2A8 | 0x801F0E6C | size: 0xD2C
 */
EffectsTemplate* parse_template(SimpleParser*, bool)
{
    FORCE_DONT_INLINE;
    return nullptr;
}

/**
 * Offset/Address/Size: 0x27C | 0x801F0E40 | size: 0x2C
 */
bool fxLoadTemplateBundle(const char* filename)
{
    unsigned long fileSize;
    void* data = fxLoadEntireFileHigh(filename, &fileSize);
    return fxLoadTemplateBundle(data, fileSize);
}

/**
 * Offset/Address/Size: 0x13C | 0x801F0D00 | size: 0x140
 */
bool fxLoadTemplateBundle(void* data, unsigned long size)
{
    if (data == nullptr)
    {
        return false;
    }

    pTemplateMap = new (nlMalloc(0x14, 8, false)) nlAVLTree<unsigned long, EffectsTemplate*, DefaultKeyCompare<unsigned long> >();

    SimpleParser parser;
    parser.StartParsing((char*)data, size, true);

    for (;;)
    {
        char* token = parser.NextToken(true);
        if (token == nullptr)
        {
            break;
        }

        if (nlStrCmp<char>(token, "begin") == 0)
        {
            EffectsTemplate* template_ptr;
            EffectsTemplate* template_value;
            nlAVLTree<unsigned long, EffectsTemplate*, DefaultKeyCompare<unsigned long> >* tree;
            AVLTreeNode* existingNode;

            template_ptr = parse_template(&parser, false);
            template_value = template_ptr;
            tree = pTemplateMap;

            tree->AddAVLNode((AVLTreeNode**)&tree->m_Root, &template_value->m_uHashID, &template_value, &existingNode, tree->m_NumElements);
            if (existingNode == nullptr)
            {
                tree->m_NumElements++;
            }
        }
        else
        {
            EmissionManager::AddError("EffectsTemplate: unrecognized token '%s'\n", token);
        }
    }

    nlFree(data);
    return true;
}

/**
 * Offset/Address/Size: 0x90 | 0x801F0C54 | size: 0x4C
 */
bool fxUnloadTemplates()
{
    if (pTemplateMap == nullptr)
    {
        return true;
    }

    pTemplateMap->DeleteValues();
    delete pTemplateMap;
    pTemplateMap = nullptr;
    return true;
}

/**
 * Offset/Address/Size: 0x0 | 0x801F0BC4 | size: 0x90
 */
EffectsTemplate* fxGetTemplate(unsigned long key)
{
    EffectsTemplate** resultPtr;
    AVLTreeEntry<unsigned long, EffectsTemplate*>* node;
    int cmpResult;
    bool found;

    for (node = pTemplateMap->m_Root; node != nullptr || (found = false, false);)
    {
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
            if (&resultPtr != nullptr)
            {
                resultPtr = &node->value;
            }
            found = true;
            break;
        }
        else if (cmpResult < 0)
        {
            node = (AVLTreeEntry<unsigned long, EffectsTemplate*>*)node->node.left;
        }
        else
        {
            node = (AVLTreeEntry<unsigned long, EffectsTemplate*>*)node->node.right;
        }
    }

    if (!found)
    {
        return nullptr;
    }

    return *resultPtr;
}

// At the bottom of EffectsTemplate.cpp — REMOVE once real callers exist.
void EffectsTemplate_stub()
{
    NewAdapter<AVLTreeEntry<unsigned long, EffectsTemplate*> > adapter;
    adapter.Delete(0);
}
