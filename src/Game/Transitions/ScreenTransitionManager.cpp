#include "Game/Transitions/ScreenTransitionManager.h"

#include "Game/Transitions/TransitionSequence.h"
#include "Game/Transitions/ColourBlendScreenTransition.h"
#include "Game/Transitions/ScriptedTransition.h"
#include "Game/Transitions/ModelTransition.h"

#include "Game/Sys/simpleparser.h"

#include "string.h"

#include "NL/nlMath.h"
#include "NL/nlString.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x80206908 | size: 0x24
//  */
// void NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >::Delete(AVLTreeEntry<unsigned long, ScreenTransition*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x9EC | 0x802068E4 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::DeleteEntry(AVLTreeEntry<unsigned long, ScreenTransition*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x988 | 0x80206880 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x95C | 0x80206854 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x930 | 0x80206828 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8D0 | 0x802067C8 | size: 0x60
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::DeleteValue(AVLTreeEntry<unsigned long, ScreenTransition*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x878 | 0x80206770 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x81C | 0x80206714 | size: 0x5C
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x814 | 0x8020670C | size: 0x8
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x80205FB4 | size: 0x758
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::PostorderTraversal(AVLTreeEntry<unsigned long, ScreenTransition*>*, void (AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, ScreenTransition*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x58 | 0x80205F50 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::DestroyTree(void (AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, ScreenTransition*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80205EF8 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, ScreenTransition*, NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> >, DefaultKeyCompare<unsigned long> >::DeleteValues()
// {
// }

// /**
//  * Offset/Address/Size: 0x1C0 | 0x80205D3C | size: 0x1BC
//  */
// void Vector<BasicString<char, Detail::TempStringAllocator>, DefaultAllocator>::reserve(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x80205BB4 | size: 0x188
//  */
// void Vector<BasicString<char, Detail::TempStringAllocator>, DefaultAllocator>::insert(BasicString<char, Detail::TempStringAllocator>*, const BasicString<char, Detail::TempStringAllocator>*, const BasicString<char, Detail::TempStringAllocator>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80205B7C | size: 0x38
//  */
// void Vector<BasicString<char, Detail::TempStringAllocator>, DefaultAllocator>::push_back(const BasicString<char, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80205B70 | size: 0xC
//  */
// void BasicString<char, Detail::TempStringAllocator>::BasicString()
// {
// }

/**
 * Offset/Address/Size: 0x9DC | 0x80205ACC | size: 0xA4
 */
ScreenTransitionManager::ScreenTransitionManager()
    : m_pActiveTransition(nullptr)
    , m_TransitionMap()
    , m_eView(GLV_Transitions)
    , m_pCallback(nullptr)
    , m_SelectedTransition(nullptr)
    , m_fCurrentTime(0.0f)
{
    m_Transitions.mData = nullptr;
    m_Transitions.mSize = 0;
    m_Transitions.mCapacity = 0;
    m_Cut = false;
    m_Transitions.reserve(16);
}

// /**
//  * Offset/Address/Size: 0x97C | 0x80205A6C | size: 0x60
//  */
// void nlAVLTree<unsigned long, ScreenTransition*, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
// {
// }

/**
 * Offset/Address/Size: 0x8FC | 0x802059EC | size: 0x80
 * TODO: 99.84% match - remaining mismatch is the call target encoding at the
 * DeleteAllTransitions callsite after forcing out-of-line behavior.
 */
ScreenTransitionManager::~ScreenTransitionManager()
{
    extern void DeleteAllTransitions__23ScreenTransitionManagerFv(ScreenTransitionManager*);

    DeleteAllTransitions__23ScreenTransitionManagerFv(this);
}

/**
 * Offset/Address/Size: 0x7F4 | 0x802058E4 | size: 0x108
 */
void ScreenTransitionManager::Render(float dt)
{
    if (m_pActiveTransition)
    {
        m_pActiveTransition->Update(dt);

        if (m_pActiveTransition->IsFinished())
        {
            m_pActiveTransition->Cancel();
            m_pActiveTransition = 0;

            if (m_pCallback)
            {
                m_pCallback->TransitionFinished();
            }
        }
        else
        {
            m_fCurrentTime += dt;

            if (m_pCallback)
            {
                float curProgress = m_fCurrentLength;
                float progress = 0.0f;
                if (curProgress > 0.0f)
                {
                    progress = m_fCurrentTime / curProgress;
                }

                m_pCallback->TransitionProgressed(progress);
            }

            m_pActiveTransition->Render(m_eView);
        }
    }
}

/**
 * Offset/Address/Size: 0x78C | 0x8020587C | size: 0x68
 */
void ScreenTransitionManager::CancelAllTransitions()
{
    if (m_pActiveTransition != nullptr)
    {
        m_pActiveTransition->Cancel();

        if (m_pCallback != nullptr)
        {
            m_pCallback->TransitionFinished();
        }
    }
    m_pActiveTransition = nullptr;
}

/**
 * Offset/Address/Size: 0x71C | 0x8020580C | size: 0x70
 */
void ScreenTransitionManager::DeleteAllTransitions()
{
    if (m_pActiveTransition != nullptr)
    {
        m_pActiveTransition->Cancel();

        if (m_pCallback != nullptr)
        {
            m_pCallback->TransitionFinished();
        }
    }

    m_pActiveTransition = nullptr;
    m_TransitionMap.DeleteValues();
}

/**
 * Offset/Address/Size: 0x5A4 | 0x80205694 | size: 0x178
 */
/**
 * TODO: 99.07% match - this/name register allocation (r31/r29) and stack-slot
 * placement for outNode/nameString temp differ from target MWCC output.
 */
void ScreenTransitionManager::AddTransitionToMap(char* name, ScreenTransition* pTransition)
{
    FORCE_DONT_INLINE;
    u32 transitionHash = glHash(name);
    AVLTreeNode* outNode;

    m_TransitionMap.AddAVLNode((AVLTreeNode**)&m_TransitionMap.m_Root, &transitionHash, &pTransition, &outNode, m_TransitionMap.m_NumElements);

    if (outNode == nullptr)
    {
        m_TransitionMap.m_NumElements++;
    }

    BasicString<char, Detail::TempStringAllocator> nameString(name);
    m_Transitions.push_back(nameString);
}

/**
 * Offset/Address/Size: 0x504 | 0x802055F4 | size: 0xA0
 */
void ScreenTransitionManager::EnableRandomTransition(const char* filter)
{
    SelectRandomTransition(filter);

    if (m_pActiveTransition != nullptr)
    {
        m_pActiveTransition->Cancel();
    }

    if (m_SelectedTransition != nullptr)
    {
        m_SelectedTransition->Reset();
        m_pActiveTransition = m_SelectedTransition;
        m_SelectedTransition = nullptr;
        m_eView = static_cast<eGLView>(29);
        m_fCurrentTime = 0.0f;
        m_Cut = false;
        m_fCurrentLength = m_pActiveTransition->GetTransitionLength();
    }
}

/**
 * Offset/Address/Size: 0x33C | 0x8020542C | size: 0x1C8
 */
void ScreenTransitionManager::SelectRandomTransition(const char* filter)
{
    Vector<BasicString<char, Detail::TempStringAllocator>, DefaultAllocator> matchingTransitions;
    matchingTransitions.mData = nullptr;
    matchingTransitions.mSize = 0;
    matchingTransitions.mCapacity = 0;
    matchingTransitions.reserve(8);

    for (int i = 0; i < m_Transitions.mSize; i++)
    {
        const char* transitionName = m_Transitions.mData[i].c_str();
        if (strstr(transitionName, filter) != nullptr)
        {
            matchingTransitions.push_back(m_Transitions.mData[i]);
        }
    }

    m_SelectedTransition = nullptr;

    if (matchingTransitions.mSize > 0)
    {
        int randomIndex = nlRandom(matchingTransitions.mSize, &nlDefaultSeed);
        unsigned long transitionHash = glHash(matchingTransitions.mData[randomIndex].c_str());
        ScreenTransition** foundTransition = nullptr;
        AVLTreeEntry<unsigned long, ScreenTransition*>* current = m_TransitionMap.m_Root;
        unsigned char found;

        for (; current != nullptr || (found = 0, 0);)
        {
            int cmpResult;
            if (transitionHash == current->key)
            {
                cmpResult = 0;
            }
            else if (transitionHash < current->key)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                if (&foundTransition != nullptr)
                {
                    foundTransition = &current->value;
                }
                found = 1;
                break;
            }

            if (cmpResult < 0)
            {
                current = (AVLTreeEntry<unsigned long, ScreenTransition*>*)current->node.left;
            }
            else
            {
                current = (AVLTreeEntry<unsigned long, ScreenTransition*>*)current->node.right;
            }
        }

        if (found)
        {
            m_SelectedTransition = *foundTransition;
        }
    }
}

/**
 * Offset/Address/Size: 0x2FC | 0x802053EC | size: 0x40
 */
float ScreenTransitionManager::GetSelectedTransitionCutTime() const
{
    if (m_SelectedTransition != nullptr)
    {
        return m_SelectedTransition->CutTime();
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x260 | 0x80205350 | size: 0x9C
 */
void ScreenTransitionManager::EnableSelectedTransition()
{
    if (m_pActiveTransition != nullptr)
    {
        m_pActiveTransition->Cancel();
    }

    if (m_SelectedTransition != nullptr)
    {
        m_SelectedTransition->Reset();
        m_pActiveTransition = m_SelectedTransition;
        m_SelectedTransition = nullptr;
        m_eView = GLV_Transitions;
        m_fCurrentTime = 0.0f;
        m_Cut = false;

        m_fCurrentLength = m_pActiveTransition->GetTransitionLength();
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x802050F0 | size: 0x260
 */
void ScreenTransitionManager::AddTransitions(char* loadedData, unsigned long fileSize)
{
    SimpleParser parser;
    char szNameBuffer[64];

    parser.StartParsing(loadedData, fileSize, true);
    char* pToken = parser.NextToken(true);

    while (pToken != nullptr)
    {
        if (nlStrCmp<char>(pToken, "colourblend") == 0)
        {
            pToken = parser.NextTokenOnLine(true);
            nlStrNCpy<char>(szNameBuffer, pToken, 0x40);
            AddTransitionToMap(szNameBuffer, ColourBlendScreenTransition::GetFromParser(&parser));
        }
        else if (nlStrCmp<char>(pToken, "sequence") == 0)
        {
            nlStrNCpy<char>(szNameBuffer, parser.NextTokenOnLine(true), 0x40);
            TransitionSequence* transitionSequence = new (nlMalloc(0x20, 8, 0)) TransitionSequence();
            transitionSequence->Initialize(&parser);
            AddTransitionToMap(szNameBuffer, transitionSequence);
        }
        else if (nlStrCmp<char>(pToken, "transition") == 0)
        {
            nlStrNCpy<char>(szNameBuffer, parser.NextTokenOnLine(true), 0x40);
            ScriptedScreenTransition* scriptedTransition = new (nlMalloc(0x1C, 8, 0)) ScriptedScreenTransition();
            scriptedTransition->InitializeFromParser(&parser);
            AddTransitionToMap(szNameBuffer, scriptedTransition);
        }
        else if (nlStrCmp<char>(pToken, "model") == 0)
        {
            nlStrNCpy<char>(szNameBuffer, parser.NextTokenOnLine(true), 0x40);
            ModeledScreenTransition* modeledTransition = new (nlMalloc(0xC4, 8, 0)) ModeledScreenTransition();
            modeledTransition->LoadFromParser(&parser);
            AddTransitionToMap(szNameBuffer, modeledTransition);
        }
        else if (*pToken == 0x23)
        {
            while (pToken != nullptr)
            {
                pToken = parser.NextTokenOnLine(true);
            }
        }
        pToken = parser.NextToken(true);
    }
}

// At the bottom of ScreenTransitionManager.cpp -- REMOVE once real callers exist.
void ScreenTransitionManager_stub()
{
    NewAdapter<AVLTreeEntry<unsigned long, ScreenTransition*> > adapter;
    adapter.Delete(0);
}
