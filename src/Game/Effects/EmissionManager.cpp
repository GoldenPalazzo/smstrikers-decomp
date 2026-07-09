#include "Game/Effects/EmissionManager.h"
#include "dolphin/types.h"
#include "NL/nlAVLTree.h"
#include "NL/gl/glFont.h"
#include "Game/NisPlayer.h"
#include "Game/Sys/debug.h"
#include "Game/Replay.h"

// ---- Replayable specs OWNED by EmissionManager ----
// Eight `template <>` definitions emit in this TU's .o. Note: bodies for
// `<0, *, char>` and `<0, *, EmissionController>` use plain `template <>`
// (not inline) because their bodies are too small -- MWCC inlines them away
// even with FORCE_DONT_INLINE when marked inline. They emit as global
// instead of weak; the rest use inline-weak. See Replay.h for rationale.
//   <0, Load, EmissionController> (top)   <0, Save, char>
//   <0, Load, char>   <0, Save, unsigned long>   <0, Save, unsigned short>
//   <0, Load, unsigned long>   <0, Load, unsigned short>
//
template <>
void Replayable<0, LoadFrame, EmissionController>(LoadFrame& frame, EmissionController& controller)
{
    frame.Replayable<0>(controller);
}

extern "C"
{
    void __vt__18AVLTreeUntemplated(void);
    void vtAVLTreeBaseLingerers(void);
    void vtNlAVLTreeLingerers(void);
}

#pragma alias vtAVLTreeBaseLingerers "__vt__106AVLTreeBase<Ul,P13LingerMessage,47NewAdapter<33AVLTreeEntry<Ul,P13LingerMessage>>,21DefaultKeyCompare<Ul>>"
#pragma alias vtNlAVLTreeLingerers "__vt__54nlAVLTree<Ul,P13LingerMessage,21DefaultKeyCompare<Ul>>"

template <>
nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >::~nlAVLTree();

static class efList* controllers = nullptr;
static class efList* errors = nullptr;
static nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >* lingerers = nullptr;
static int g_nNumLights = 0;
static EffectsLight g_EffectsLights[3];

extern eGLView defaultView;

static unsigned long fx_sTerrain;

typedef AVLTreeEntry<unsigned long, LingerMessage*> LMEntry;

struct nlAVLTreeIter
{
    LMEntry** m_Stack;
    unsigned int m_NumStackEntries;
};

static const nlColour kLingerColour = { 0xFF, 0xFF, 0x40, 0xFF };

/**
 * Offset/Address/Size: 0xE38 | 0x801F9758 | size: 0x20
 */
EmissionManager& EmissionManager::InstanceForReplayOnly()
{
    static EmissionManager instance(true);
    return instance;
}

/**
 * Offset/Address/Size: 0xD6C | 0x801F968C | size: 0xCC
 */
bool EmissionManager::Startup(eGLView view)
{
    defaultView = view;

    efList* tmp_controllers = (efList*)nlMalloc(sizeof(efList), 8, false);
    if (tmp_controllers != nullptr)
    {
        tmp_controllers->m_headNode = nullptr;
        tmp_controllers->m_tailNode = nullptr;
        tmp_controllers->m_numNodes = 0;
    }
    controllers = tmp_controllers;

    efList* tmp_errors = (efList*)nlMalloc(sizeof(efList), 8, false);
    if (tmp_errors != nullptr)
    {
        tmp_errors->m_headNode = nullptr;
        tmp_errors->m_tailNode = nullptr;
        tmp_errors->m_numNodes = 0;
    }
    errors = tmp_errors;

    nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >* tmp_lingerers = (nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >*)nlMalloc(
        sizeof(nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >), 8, false);
    if (tmp_lingerers != nullptr)
    {
        u32* map = (u32*)tmp_lingerers;
        map[0] = (u32)__vt__18AVLTreeUntemplated;
        map[0] = (u32)vtAVLTreeBaseLingerers;
        map[4] = 0;
        map[2] = 0;
        map[3] = 0;
        map[0] = (u32)vtNlAVLTreeLingerers;
    }
    lingerers = tmp_lingerers;

    return true;
}

/**
 * Offset/Address/Size: 0xC1C | 0x801F953C | size: 0xF0
 */
bool EmissionManager::Shutdown()
{
    if (controllers->m_headNode != nullptr)
    {
        tDebugPrintManager::Print(DC_RENDER, "EmissionManager being deleted non-empty\n");
    }

    EmissionController* next;
    EmissionController* current;
    current = (EmissionController*)(controllers->m_headNode);

    while (current != nullptr)
    {
        next = (EmissionController*)(current->m_nextNode);
        controllers->Remove(current);
        delete current;
        current = next;
    }

    delete controllers;
    controllers = nullptr;

    LingerMessage* errorCurrent;
    errorCurrent = (LingerMessage*)(errors->m_headNode);

    while (errorCurrent != nullptr)
    {
        LingerMessage* errorNext = (LingerMessage*)(errorCurrent->m_nextNode);
        errors->Remove(errorCurrent);
        delete errorCurrent;
        errorCurrent = errorNext;
    }

    delete errors;
    errors = nullptr;

    void (*resetLingerers)() = &EmissionManager::ResetLingerers;
    resetLingerers();

    delete lingerers;
    lingerers = nullptr;

    return true;
}

/**
 * Offset/Address/Size: 0x930 | 0x801F9250 | size: 0x2EC
 */
void EmissionManager::Update(float dt)
{
    if (NisPlayer::Instance()->WorldIsFrozen())
    {
        dt = 0.0f;
    }

    EmissionController* p = (EmissionController*)controllers->m_headNode;
    while (p != NULL)
    {
        bool stillAlive = p->Update(dt);
        if (stillAlive)
        {
            p = (EmissionController*)p->m_nextNode;
        }
        else
        {
            EmissionController* toBeDeleted = p;
            p = (EmissionController*)p->m_nextNode;
            controllers->Remove(toBeDeleted);
            delete toBeDeleted;
        }
    }

    // KillOldest inlined
    {
        int num = controllers->m_numNodes - 128;
        float currentBestAge = 0.0f;
        float prevBestAge = currentBestAge;

        while (num > 0)
        {
            EmissionController* toKill = NULL;
            float bestAge = 0.0f;
            EmissionController* current = (EmissionController*)controllers->m_headNode;

            while (current != NULL)
            {
                if (current->m_uUserData + 0x21530000 != 0x0000BEEF)
                {
                    float age = current->m_Age;
                    if (bestAge < age && (prevBestAge == currentBestAge || age < currentBestAge))
                    {
                        bestAge = age;
                        toKill = current;
                        currentBestAge = age;
                    }
                }
                current = (EmissionController*)current->m_nextNode;
            }

            if (toKill == NULL)
            {
                break;
            }

            toKill->Die();
            num--;
        }
    }

    // Lingerers display
    if (lingerers->m_Root != NULL)
    {
        nlColour colour = { 0xFF, 0xFF, 0x40, 0xFF };
        nlAVLTreeIter* iter;
        int y = 3;
        glFontBegin(false);

        nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >* tree = lingerers;
        iter = (nlAVLTreeIter*)nlMalloc(sizeof(nlAVLTreeIter), 8, false);
        if (iter != NULL)
        {
            unsigned int numEntries = tree->m_NumElements;
            LMEntry* node = tree->m_Root;
            iter->m_Stack = (LMEntry**)nlMalloc((numEntries + 1) * 4, 8, false);
            iter->m_NumStackEntries = 0;

            if (node != NULL)
            {
                while (node->node.left != NULL)
                {
                    iter->m_Stack[iter->m_NumStackEntries] = node;
                    iter->m_NumStackEntries++;
                    node = (LMEntry*)node->node.left;
                }
                iter->m_Stack[iter->m_NumStackEntries] = node;
                iter->m_NumStackEntries++;
            }
        }

        while (iter->m_NumStackEntries != 0)
        {
            LMEntry* entry = iter->m_Stack[iter->m_NumStackEntries - 1];
            LingerMessage* l = entry->value;

            glFontPrintf((eGLView)0x21, 0, y, colour, "%s: %d (%d)", l->szMessage, l->nLingers, l->nParticles);

            iter->m_NumStackEntries--;

            entry = iter->m_Stack[iter->m_NumStackEntries];
            LMEntry* right = (LMEntry*)entry->node.right;
            if (right != NULL)
            {
                while (right->node.left != NULL)
                {
                    iter->m_Stack[iter->m_NumStackEntries] = right;
                    iter->m_NumStackEntries++;
                    right = (LMEntry*)right->node.left;
                }
                iter->m_Stack[iter->m_NumStackEntries] = right;
                iter->m_NumStackEntries++;
            }

            y++;
        }

        if (iter != NULL)
        {
            delete[] iter->m_Stack;
            delete iter;
        }

        glFontEnd();
        ResetLingerers();
    }
}

/**
 * Offset/Address/Size: 0x928 | 0x801F9248 | size: 0x8
 */
s32 EmissionManager::GetNumLights()
{
    return g_nNumLights;
}

/**
 * Offset/Address/Size: 0x8F8 | 0x801F9218 | size: 0x30
 */
EffectsLight* EmissionManager::GetLight(int index)
{
    if (index < 0 || index >= g_nNumLights)
    {
        return nullptr;
    }
    return &g_EffectsLights[index];
}

/**
 * Offset/Address/Size: 0x8A8 | 0x801F91C8 | size: 0x50
 */
void EmissionManager::AddEffectsLight(const EffectsLight& light)
{
    if (g_nNumLights >= 3)
    {
        return;
    }
    g_EffectsLights[g_nNumLights++] = light;
}

/**
 * Offset/Address/Size: 0x85C | 0x801F917C | size: 0x4C
 */
void EmissionManager::Render()
{
    g_nNumLights = 0;
    EmissionController* current = (EmissionController*)(controllers->m_headNode);

    while (current != nullptr)
    {
        current->Render();
        current = (EmissionController*)(current->m_nextNode);
    }
}

/**
 * Offset/Address/Size: 0x854 | 0x801F9174 | size: 0x8
 */
efList* EmissionManager::GetContainer()
{
    return controllers;
}

/**
 * Offset/Address/Size: 0x798 | 0x801F90B8 | size: 0xBC
 */
EmissionController* EmissionManager::Create(EffectsGroup* pEffectsGroup, unsigned short id)
{
    EmissionController* pController;

    static u16 globalIdCounter = 1;

    if (id == 0)
    {
        id = globalIdCounter++;
    }

    if (globalIdCounter > 0x7E16) // 32278
    {
        globalIdCounter = 0;
    }

    pController = new (nlMalloc(sizeof(EmissionController), 8, false)) EmissionController(pEffectsGroup, id, defaultView);
    controllers->Append(pController);
    return pController;
}

/**
 * Offset/Address/Size: 0x768 | 0x801F9088 | size: 0x30
 */
bool EmissionManager::IsStillAlive(EmissionController* controller)
{
    EmissionController* current;
    current = (EmissionController*)(controllers->m_headNode);

    while (current != nullptr)
    {
        if (current == controller)
        {
            return true;
        }
        current = (EmissionController*)(current->m_nextNode);
    }

    return false;
}

/**
 * Offset/Address/Size: 0x6EC | 0x801F900C | size: 0x7C
 */
void EmissionManager::Kill(unsigned long userData, const EffectsGroup* pEffectsGroup)
{
    EmissionController* current;

    current = (EmissionController*)(controllers->m_headNode);

    while (current != nullptr)
    {
        if (pEffectsGroup == nullptr || current->m_pGroup == pEffectsGroup)
        {
            if (userData == current->m_uUserData)
            {
                current->Die();
            }
        }

        current = (EmissionController*)(current->m_nextNode);
    }
}

/**
 * Offset/Address/Size: 0x68C | 0x801F8FAC | size: 0x60
 */
bool EmissionManager::IsPlaying(unsigned long userData, const EffectsGroup* pEffectsGroup)
{
    EmissionController* current;

    if (controllers == nullptr)
    {
        return false;
    }

    current = (EmissionController*)(controllers->m_headNode);

    while (current != nullptr)
    {
        if (pEffectsGroup == nullptr || current->m_pGroup == pEffectsGroup)
        {
            if (userData == 0 || userData == current->m_uUserData)
            {
                return true;
            }
        }

        current = (EmissionController*)(current->m_nextNode);
    }

    return false;
}

/**
 * Offset/Address/Size: 0x5EC | 0x801F8F0C | size: 0xA0
 */
void EmissionManager::DestroyAll(bool exceptPersistent)
{
    EmissionController* next;
    EmissionController* current;

    if (controllers == nullptr)
    {
        return;
    }

    current = (EmissionController*)(controllers->m_headNode);

    while (current != nullptr)
    {
        next = (EmissionController*)(current->m_nextNode);

        if ((defaultView == current->m_GlView) && ((!exceptPersistent) || (current->m_uUserData + 0x21530000 != 0x0000BEEF)))
        {
            controllers->Remove(current);
            delete current;
        }

        current = next;
    }
}

/**
 * Offset/Address/Size: 0x54C | 0x801F8E6C | size: 0xA0
 */
void EmissionManager::Destroy(unsigned long userData, const EffectsGroup* pEffectsGroup)
{
    EmissionController* next;
    EmissionController* current;

    if (controllers == nullptr)
    {
        return;
    }

    current = (EmissionController*)(controllers->m_headNode);

    while (current != nullptr)
    {
        next = (EmissionController*)(current->m_nextNode);

        if (((pEffectsGroup == nullptr || current->m_pGroup == pEffectsGroup)) && (userData == current->m_uUserData))
        {
            controllers->Remove(current);
            delete current;
        }

        current = next;
    }
}

/**
 * Offset/Address/Size: 0x520 | 0x801F8E40 | size: 0x2C
 */
#pragma dont_inline on
void EmissionManager::ResetLingerers()
{
    if (lingerers != nullptr)
    {
        lingerers->DeleteValues();
    }
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x4D0 | 0x801F8DF0 | size: 0x50
 */
void EmissionManager::AddError(const char*, ...)
{
}

/**
 * Offset/Address/Size: 0x24C | 0x801F8B6C | size: 0x284
 * TODO: 99.38% match - remaining r28/r29 allocation diffs in the new-controller path
 */
void EmissionManager::Replay(LoadFrame& frame)
{
    int i;
    EmissionController* next;
    EmissionController* current;

    if (m_bRecording)
    {
        if (controllers != nullptr)
        {
            current = (EmissionController*)controllers->m_headNode;
            while (current != nullptr)
            {
                next = (EmissionController*)current->m_nextNode;
                eGLView glView = (eGLView)current->m_GlView;
                if ((defaultView == glView) && (current->m_uUserData + 0x21530000 != 0x0000BEEF))
                {
                    controllers->Remove(current);
                    delete current;
                }
                current = next;
            }
        }
        m_bRecording = false;
    }

    int numEffects = 0;
    Replayable<0>(frame, numEffects);

    efList oldControllers;
    oldControllers.m_headNode = nullptr;
    oldControllers.m_tailNode = nullptr;
    oldControllers.m_numNodes = 0;

    efBaseNode* head;
    efBaseNode* tail;
    int num;
    efList* const ctrl = controllers;

    head = ctrl->m_headNode;
    ctrl->m_headNode = oldControllers.m_headNode;
    oldControllers.m_headNode = head;

    tail = ctrl->m_tailNode;
    ctrl->m_tailNode = oldControllers.m_tailNode;
    oldControllers.m_tailNode = tail;

    num = ctrl->m_numNodes;
    ctrl->m_numNodes = oldControllers.m_numNodes;
    oldControllers.m_numNodes = num;

    i = 0;
    while (i < numEffects)
    {
        unsigned short id;
        Replayable<0>(frame, id);

        unsigned long group = 0;
        Replayable<0>(frame, group);

        unsigned short idToUse;
        EmissionController* iter = (EmissionController*)oldControllers.m_headNode;
        unsigned short idCheck = id;
        while (iter != nullptr)
        {
            next = (EmissionController*)iter->m_nextNode;
            if (idCheck == iter->m_Id)
            {
                Replayable<0>(frame, *iter);
                oldControllers.Remove(iter);
                controllers->Insert(iter);
                break;
            }
            iter = next;
        }

        if (iter == nullptr)
        {
            idToUse = id;
            static u16 globalIdCounter = 1;

            if (idToUse == 0)
            {
                idToUse = globalIdCounter;
                globalIdCounter++;
            }

            if (globalIdCounter > 0x7E16)
            {
                globalIdCounter = 0;
            }

            iter = new (nlMalloc(sizeof(EmissionController), 8, false)) EmissionController((EffectsGroup*)group, idToUse, defaultView);
            controllers->Append(iter);
            Replayable<0>(frame, *iter);
        }

        i++;
    }

    current = (EmissionController*)oldControllers.m_headNode;
    while (current != nullptr)
    {
        EmissionController* loopNext = (EmissionController*)current->m_nextNode;
        if (defaultView != current->m_GlView)
        {
            oldControllers.Remove(current);
            controllers->Insert(current);
        }
        else
        {
            oldControllers.Remove(current);
            delete current;
        }
        current = loopNext;
    }
}

/**
 * Offset/Address/Size: 0x11C | 0x801F8A3C | size: 0x130
 */

void EmissionManager::Replay(SaveFrame& frame)
{
    if (!m_bRecording)
    {
        if (controllers != nullptr)
        {
            EmissionController* current = (EmissionController*)(controllers->m_headNode);
            while (current != nullptr)
            {
                EmissionController* next = (EmissionController*)(current->m_nextNode);
                eGLView glView = (eGLView)current->m_GlView;
                if ((defaultView == glView) && (current->m_uUserData + 0x21530000 != 0x0000BEEF))
                {
                    controllers->Remove(current);
                    delete current;
                }
                current = next;
            }
        }
        m_bRecording = true;
    }

    int numEffects = controllers->m_numNodes;
    Replayable<0>(frame, numEffects);

    EmissionController* current = (EmissionController*)(controllers->m_headNode);
    while (current != nullptr)
    {
        if (defaultView == current->m_GlView)
        {
            unsigned short id = current->m_Id;
            unsigned long group = (unsigned long)current->m_pGroup;
            Replayable<0>(frame, id);
            Replayable<0>(frame, group);
            Replayable<0>(frame, *current);
        }
        current = (EmissionController*)(current->m_nextNode);
    }
}

/**
 * Offset/Address/Size: 0x114 | 0x801F8A34 | size: 0x8
 */
u32 fxGetTerrain()
{
    return fx_sTerrain;
}

/**
 * Offset/Address/Size: 0x10C | 0x801F8A2C | size: 0x8
 */
void fxSetTerrain(unsigned long terrainID)
{
    fx_sTerrain = terrainID;
}

/**
 * Offset/Address/Size: 0x0 | 0x801F8920 | size: 0x10C
 */
void EmissionManager::KillOldest(int num, bool lingeringOnly)
{
    float prevBestAge = 0.0f;
    float currentBestAge = 0.0f;

    while (num > 0)
    {
        EmissionController* bestController = nullptr;
        float bestAge = 0.0f;
        EmissionController* current = (EmissionController*)(controllers->m_headNode);

        while (current != nullptr)
        {
            if ((!lingeringOnly || current->IsLingering()) && (current->m_uUserData + 0x21530000 != 0x0000BEEF))
            {
                float age = current->m_Age;
                if ((bestAge < age) && (prevBestAge == currentBestAge || age < currentBestAge))
                {
                    bestAge = age;
                    bestController = current;
                    currentBestAge = age;
                }
            }

            current = (EmissionController*)(current->m_nextNode);
        }

        if (bestController == nullptr)
        {
            break;
        }

        bestController->Die();
        num--;
    }
}

// REMOVE once real callers exist.
void EmissionManager_stub()
{
    NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*> > adapter;
    adapter.Delete((AVLTreeEntry<unsigned long, LingerMessage*>*)0);

    float f = 0.0f;
    FloatCompressor<-255, 255, 6> fc(f);
    Replayable<0>(*(SaveFrame*)0, (const FloatCompressor<-255, 255, 6>&)fc);
    Replayable<0>(*(LoadFrame*)0, (const FloatCompressor<-255, 255, 6>&)fc);
}

/**
 * Offset/Address/Size: 0x2C | 0x801FA600 | size: 0x254
 */
template <>
void LoadFrame::Replayable<0, EmissionController>(EmissionController& current, NotReplayablePod)
{
    FORCE_DONT_INLINE;
    ::Replayable<0>(*this, (unsigned int&)current.m_pPose);
    ::Replayable<0>(*this, (unsigned int&)current.m_pAnimController);
    memcpy(&current.m_uUserData, mStream.mStorage, sizeof(unsigned long));
    mStream.mStorage += sizeof(unsigned long);
    ::Replayable<0>(*this, current.m_fGround);
    memcpy(&current.m_aFacing, mStream.mStorage, sizeof(unsigned short));
    mStream.mStorage += sizeof(unsigned short);
    ::Replayable<0>(*this, (char&)current.m_GlView);
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vPosition.f.x));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vPosition.f.y));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vPosition.f.z));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vDirection.f.x));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vDirection.f.y));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vDirection.f.z));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vVelocity.f.x));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vVelocity.f.y));
    ::Replayable<0>(*this, FloatCompressor<-255, 255, 6>(current.m_vVelocity.f.z));
    current.m_Replaying = true;
    float age = 0.0f;
    ::Replayable<0>(*this, age);
    age += mNonBlendableAheadOfFrame;
    current.m_ReplayDeltaTime = age - current.m_Age;
    current.m_Age = age;
    unsigned int updateCb;
    ::Replayable<0>(*this, updateCb);
    if (updateCb != 0)
    {
        if (current.mUpdateCallback.mTag == FUNCTOR && current.mUpdateCallback.mFunctor != 0)
        {
            delete current.mUpdateCallback.mFunctor;
        }
        current.mUpdateCallback.mTag = FREE_FUNCTION;
        current.mUpdateCallback.mFreeFunction = (void (*)(EmissionController&))updateCb;
    }
    unsigned int finishedCb;
    ::Replayable<0>(*this, finishedCb);
    if (finishedCb != 0)
    {
        if (current.mFinishedCallback.mTag == FUNCTOR && current.mFinishedCallback.mFunctor != 0)
        {
            delete current.mFinishedCallback.mFunctor;
        }
        current.mFinishedCallback.mTag = FREE_FUNCTION;
        current.mFinishedCallback.mFreeFunction = (void (*)(EmissionController&))finishedCb;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801FA5D4 | size: 0x2C
 * TODO: 99.73% match - stack offset swap (0x8/0xc) for NotReplayablePod local vs argument copy
 * Implemented in include/Game/Replay.h as LoadFrame::Replayable<N, T> template body.
 */

//  /**
//   * Offset/Address/Size: 0x3C4 | 0x801FA558 | size: 0x7C
//   */
//  void Replayable<0, SaveFrame, FloatCompressor<-255, 255, 6>>(SaveFrame&, const FloatCompressor<-255, 255, 6>&)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x364 | 0x801FA4F8 | size: 0x60
//   */
//  void Replayable<0, LoadFrame, FloatCompressor<-255, 255, 6>>(LoadFrame&, const FloatCompressor<-255, 255, 6>&)
//  {
//  }

/**
 * Offset/Address/Size: 0x324 | 0x801FA4B8 | size: 0x40
 */
#pragma dont_inline on
template <>
void Replayable<0, SaveFrame, char>(SaveFrame& frame, char& value)
{
    memcpy(frame.mStream.mStorage, &value, sizeof(char));
    frame.mStream.mStorage += sizeof(char);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x2E0 | 0x801FA474 | size: 0x44
 */
#pragma dont_inline on
template <>
void Replayable<0, LoadFrame, char>(LoadFrame& frame, char& value)
{
    memcpy(&value, frame.mStream.mStorage, sizeof(char));
    frame.mStream.mStorage += sizeof(char);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x128 | 0x801FA2BC | size: 0x1B8
 */
template <>
void Replayable<0, SaveFrame, EmissionController>(SaveFrame& frame, EmissionController& controller)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (unsigned int&)controller.m_pPose);
    Replayable<0>(frame, (unsigned int&)controller.m_pAnimController);
    memcpy(frame.mStream.mStorage, &controller.m_uUserData, sizeof(unsigned long));
    frame.mStream.mStorage += sizeof(unsigned long);
    Replayable<0>(frame, controller.m_fGround);
    memcpy(frame.mStream.mStorage, &controller.m_aFacing, sizeof(unsigned short));
    frame.mStream.mStorage += sizeof(unsigned short);
    Replayable<0>(frame, (char&)controller.m_GlView);
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vPosition.f.x));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vPosition.f.y));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vPosition.f.z));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vDirection.f.x));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vDirection.f.y));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vDirection.f.z));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vVelocity.f.x));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vVelocity.f.y));
    Replayable<0>(frame, FloatCompressor<-255, 255, 6>(controller.m_vVelocity.f.z));
    controller.m_Replaying = false;
    controller.m_ReplayDeltaTime = 0.0f;
    Replayable<0>(frame, controller.m_Age);
    unsigned int updateCb = (controller.mUpdateCallback.mTag == FREE_FUNCTION)
                              ? (unsigned int)controller.mUpdateCallback.mFreeFunction
                              : 0;
    Replayable<0>(frame, updateCb);
    unsigned int finishedCb = (controller.mFinishedCallback.mTag == FREE_FUNCTION)
                                ? (unsigned int)controller.mFinishedCallback.mFreeFunction
                                : 0;
    Replayable<0>(frame, finishedCb);
}

/**
 * Offset/Address/Size: 0xE8 | 0x801FA27C | size: 0x40
 */
template <>
inline void Replayable<0, SaveFrame, unsigned long>(SaveFrame& frame, unsigned long& value)
{
    FORCE_DONT_INLINE;
    memcpy(frame.mStream.mStorage, &value, sizeof(unsigned long));
    frame.mStream.mStorage += sizeof(unsigned long);
}

/**
 * Offset/Address/Size: 0xA8 | 0x801FA23C | size: 0x40
 */
template <>
inline void Replayable<0, SaveFrame, unsigned short>(SaveFrame& frame, unsigned short& value)
{
    FORCE_DONT_INLINE;
    memcpy(frame.mStream.mStorage, &value, sizeof(unsigned short));
    frame.mStream.mStorage += sizeof(unsigned short);
}

/**
 * Offset/Address/Size: 0x44 | 0x801FA1D8 | size: 0x44
 */
template <>
inline void Replayable<0, LoadFrame, unsigned long>(LoadFrame& frame, unsigned long& value)
{
    FORCE_DONT_INLINE;
    memcpy(&value, frame.mStream.mStorage, sizeof(unsigned long));
    frame.mStream.mStorage += sizeof(unsigned long);
}

/**
 * Offset/Address/Size: 0x0 | 0x801FA194 | size: 0x44
 */
template <>
inline void Replayable<0, LoadFrame, unsigned short>(LoadFrame& frame, unsigned short& value)
{
    FORCE_DONT_INLINE;
    memcpy(&value, frame.mStream.mStorage, sizeof(unsigned short));
    frame.mStream.mStorage += sizeof(unsigned short);
}

//  /**
//   * Offset/Address/Size: 0x0 | 0x801FA170 | size: 0x24
//   */
//  void NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>::Delete(AVLTreeEntry<unsigned long, LingerMessage*>*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x9CC | 0x801FA144 | size: 0x2C
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x9A0 | 0x801FA118 | size: 0x2C
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::CompareKey(void*, AVLTreeNode*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x93C | 0x801FA0B4 | size: 0x64
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::AllocateEntry(void*, void*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x8E4 | 0x801FA05C | size: 0x58
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::Clear()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x888 | 0x801FA000 | size: 0x5C
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::~AVLTreeBase()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x880 | 0x801F9FF8 | size: 0x8
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::CastUp(AVLTreeNode*) const
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x128 | 0x801F98A0 | size: 0x758
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::PostorderTraversal(AVLTreeEntry<unsigned long, LingerMessage*>*, void (AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, LingerMessage*>*))
//  {
//  }

//  /**
//   * Offset/Address/Size: 0xC4 | 0x801F983C | size: 0x64
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::DestroyTree(void (AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, LingerMessage*>*))
//  {
//  }

/**
 * Offset/Address/Size: 0x6C | 0x801F97E4 | size: 0x58
 */
#pragma dont_inline on
void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*> >, DefaultKeyCompare<unsigned long> >::DeleteValues()
{
    DestroyTree(&AVLTreeBase::DeleteValue);
    m_NumElements = 0;
}
#pragma dont_inline reset

//  /**
//   * Offset/Address/Size: 0x48 | 0x801F97C0 | size: 0x24
//   */
//  void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*>>, DefaultKeyCompare<unsigned long>>::DeleteEntry(AVLTreeEntry<unsigned long, LingerMessage*>*)
//  {
//  }

/**
 * Offset/Address/Size: 0x0 | 0x801F9778 | size: 0x48
 */
void AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<AVLTreeEntry<unsigned long, LingerMessage*> >, DefaultKeyCompare<unsigned long> >::DeleteValue(AVLTreeEntry<unsigned long, LingerMessage*>* entry)
{
    delete entry->value;
    m_Allocator.Delete(entry);
}

//  /**
//   * Offset/Address/Size: 0xD0C | 0x801F962C | size: 0x60
//   */
//  void nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long>>::~nlAVLTree()
//  {
//  }
