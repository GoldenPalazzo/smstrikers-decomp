#define NL_AVLTREEBASE_REVERSE_LINK_ORDER
#define NL_NEWADAPTER_EXPLICIT_LINK_ORDER
// The retained weak Function1 base destructor is owned by an earlier TU.
#define FUNCTION1_BASE_DTOR_DECLARE_ONLY
#include "Game/Effects/EmissionManager.h"
#include "dolphin/types.h"
#include "NL/nlAVLTree.h"
#include "NL/gl/glFont.h"
#include "Game/NisPlayer.h"
#include "Game/Sys/debug.h"
#include "Game/Replay.h"

template <typename FrameType>
static inline void ReplayControllerFloats(FrameType& frame, EmissionController& controller)
{
    const FloatCompressor<-255, 255, 6> positionX(controller.m_vPosition.f.x);
    ::Replayable<0>(frame, positionX);
    const FloatCompressor<-255, 255, 6> positionY(controller.m_vPosition.f.y);
    ::Replayable<0>(frame, positionY);
    const FloatCompressor<-255, 255, 6> positionZ(controller.m_vPosition.f.z);
    ::Replayable<0>(frame, positionZ);
    const FloatCompressor<-255, 255, 6> directionX(controller.m_vDirection.f.x);
    ::Replayable<0>(frame, directionX);
    const FloatCompressor<-255, 255, 6> directionY(controller.m_vDirection.f.y);
    ::Replayable<0>(frame, directionY);
    const FloatCompressor<-255, 255, 6> directionZ(controller.m_vDirection.f.z);
    ::Replayable<0>(frame, directionZ);
    const FloatCompressor<-255, 255, 6> velocityX(controller.m_vVelocity.f.x);
    ::Replayable<0>(frame, velocityX);
    const FloatCompressor<-255, 255, 6> velocityY(controller.m_vVelocity.f.y);
    ::Replayable<0>(frame, velocityY);
    const FloatCompressor<-255, 255, 6> velocityZ(controller.m_vVelocity.f.z);
    ::Replayable<0>(frame, velocityZ);
}

static inline void ReplayControllerState(LoadFrame& frame, EmissionController& controller)
{
    float age = 0.0f;
    ::Replayable<0>(frame, age);
    age += frame.mNonBlendableAheadOfFrame;
    controller.m_ReplayDeltaTime = age - controller.m_Age;
    controller.m_Age = age;

    unsigned int updateCb = 0;
    ::Replayable<0>(frame, updateCb);
    register unsigned int callback = updateCb;
    if (callback != 0)
    {
        if (controller.mUpdateCallback.mTag == FUNCTOR)
        {
            delete controller.mUpdateCallback.mFunctor;
        }
        controller.mUpdateCallback.mTag = EMPTY;
        controller.mUpdateCallback.mTag = FREE_FUNCTION;
        controller.mUpdateCallback.mFreeFunction = (void (*)(EmissionController&))callback;
    }

    unsigned int finishedCb = 0;
    ::Replayable<0>(frame, finishedCb);
    callback = finishedCb;
    if (callback != 0)
    {
        if (controller.mFinishedCallback.mTag == FUNCTOR)
        {
            delete controller.mFinishedCallback.mFunctor;
        }
        controller.mFinishedCallback.mTag = EMPTY;
        controller.mFinishedCallback.mTag = FREE_FUNCTION;
        controller.mFinishedCallback.mFreeFunction = (void (*)(EmissionController&))callback;
    }
}

static inline void ReplayControllerCallbacks(SaveFrame& frame, EmissionController& controller)
{
    unsigned int updateCb = (controller.mUpdateCallback.mTag == FREE_FUNCTION)
                              ? (unsigned int)controller.mUpdateCallback.mFreeFunction
                              : 0;
    Replayable<0>(frame, updateCb);

    unsigned int finishedCb = (controller.mFinishedCallback.mTag == FREE_FUNCTION)
                                ? (unsigned int)controller.mFinishedCallback.mFreeFunction
                                : 0;
    Replayable<0>(frame, finishedCb);
}

template <>
WEAKFUNC nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >::~nlAVLTree();

static class efList* controllers = nullptr;
static class efList* errors = nullptr;
static nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >* lingerers = nullptr;
static EffectsLight g_EffectsLights[3];

static eGLView defaultView = GLV_Particles;

typedef AVLTreeEntry<unsigned long, LingerMessage*> LMEntry;
typedef AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<LMEntry>, DefaultKeyCompare<unsigned long> > LingerTreeBase;

template <>
inline LingerTreeBase::ENTRY_DELETE_FUNC LingerTreeBase::DeleteEntryFunc()
{
    return &LingerTreeBase::DeleteEntry;
}

template class AVLTreeBase<unsigned long, LingerMessage*, NewAdapter<LMEntry>, DefaultKeyCompare<unsigned long> >;
#pragma defer_codegen off

static char lingerMessageFormat[] = "%s lingers (%d .. %d)";
static char nonEmptyEmissionManagerMessage[] = "EmissionManager being deleted non-empty\n";

template class NewAdapter<LMEntry>;
#define REPLAY_EMISSION_FREE_SPECIALIZATIONS
#include "Game/Replay.h"
#undef REPLAY_EMISSION_FREE_SPECIALIZATIONS

#define LOADFRAME_EMISSIONCONTROLLER_SPECIALIZATIONS
#include "Game/LoadFrame.h"
#undef LOADFRAME_EMISSIONCONTROLLER_SPECIALIZATIONS

#pragma defer_codegen reset

struct nlAVLTreeIter
{
    LMEntry** m_Stack;
    unsigned int m_NumStackEntries;
};

/**
 * Offset/Address/Size: 0xE38 | 0x801F9758 | size: 0x20
 */
EmissionManager& EmissionManager::InstanceForReplayOnly()
{
    static EmissionManager instance(true);
    return instance;
}

static int g_nNumLights = 0;
static unsigned short globalIdCounter;
static signed char globalIdCounterInit;

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
    new (tmp_lingerers) nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >();
    lingerers = tmp_lingerers;

    return true;
}

template <>
WEAKFUNC nlAVLTree<unsigned long, LingerMessage*, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0xC1C | 0x801F953C | size: 0xF0
 */
bool EmissionManager::Shutdown()
{
    if (controllers->m_headNode != nullptr)
    {
        tDebugPrintManager::Print(DC_RENDER, nonEmptyEmissionManagerMessage);
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

            glFontPrintf((eGLView)0x21, 0, y, colour, lingerMessageFormat, l->szMessage, l->nLingers, l->nParticles);

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

    if (!globalIdCounterInit)
    {
        globalIdCounter = 1;
        globalIdCounterInit = 1;
    }

    if (id == 0)
    {
        id = globalIdCounter++;
    }

    if (globalIdCounter > 0x7E16)
    {
        globalIdCounter = 0;
    }

    pController = new (nlMalloc(sizeof(EmissionController), 8, false)) EmissionController(pEffectsGroup, id, defaultView);
    controllers->Append(pController);
    return pController;
}

static unsigned long fx_sTerrain;

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

static inline EmissionController* ReplayCreateController(EffectsGroup*& group, unsigned short id)
{
    if (!globalIdCounterInit)
    {
        globalIdCounter = 1;
        globalIdCounterInit = 1;
    }

    if (id == 0)
    {
        id = globalIdCounter++;
    }

    if (globalIdCounter > 0x7E16)
    {
        globalIdCounter = 0;
    }

    EmissionController* controller = new (nlMalloc(sizeof(EmissionController), 8, false)) EmissionController(group, id, defaultView);
    controllers->Append(controller);
    return controller;
}

static inline void ReplayRemoveAllNonPersistent()
{
    if (controllers != nullptr)
    {
        EmissionController* next;
        EmissionController* current = (EmissionController*)controllers->m_headNode;
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
}

/**
 * Offset/Address/Size: 0x24C | 0x801F8B6C | size: 0x284
 */
void EmissionManager::Replay(LoadFrame& frame)
{
    int i;

    if (m_bRecording)
    {
        ReplayRemoveAllNonPersistent();
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

        EffectsGroup* group = 0;
        Replayable<0>(frame, (unsigned long&)group);

        EmissionController* iter = (EmissionController*)oldControllers.m_headNode;
        EmissionController* next;
        unsigned short idCheck = id;
        while (iter != nullptr)
        {
            next = (EmissionController*)iter->m_nextNode;
            if (idCheck == iter->m_Id)
            {
                ::Replayable<0>(frame, *iter);
                oldControllers.Remove(iter);
                controllers->Insert(iter);
                break;
            }
            iter = next;
        }

        if (iter == nullptr)
        {
            iter = ReplayCreateController(group, id);
            ::Replayable<0>(frame, *iter);
        }

        i++;
    }

    EmissionController* next;
    EmissionController* iter = (EmissionController*)oldControllers.m_headNode;
    while (iter != nullptr)
    {
        next = (EmissionController*)iter->m_nextNode;
        if (defaultView != iter->m_GlView)
        {
            oldControllers.Remove(iter);
            controllers->Insert(iter);
        }
        else
        {
            oldControllers.Remove(iter);
            delete iter;
        }
        iter = next;
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

// MWCC flushes these deferred inline specializations in reverse reference
// order. This discarded anchor reproduces the target linkonce order.
#pragma section ".dead"
DECL_SECT(".dead")
void EmissionManagerReplayOrder_stub()
{
    void (*saveFloat)(SaveFrame&, const FloatCompressor<-255, 255, 6>&) = Replayable<0, SaveFrame, FloatCompressor<-255, 255, 6> >;
    void (*loadFloat)(LoadFrame&, const FloatCompressor<-255, 255, 6>&) = Replayable<0, LoadFrame, FloatCompressor<-255, 255, 6> >;
    void (*saveChar)(SaveFrame&, char&) = Replayable<0, SaveFrame, char>;
    void (*loadChar)(LoadFrame&, char&) = Replayable<0, LoadFrame, char>;
    void (*saveController)(SaveFrame&, EmissionController&) = Replayable<0, SaveFrame, EmissionController>;
    void (*saveLong)(SaveFrame&, unsigned long&) = Replayable<0, SaveFrame, unsigned long>;
    void (*saveShort)(SaveFrame&, unsigned short&) = Replayable<0, SaveFrame, unsigned short>;
    void (*loadController)(LoadFrame&, EmissionController&) = Replayable<0, LoadFrame, EmissionController>;
    void (*loadLong)(LoadFrame&, unsigned long&) = Replayable<0, LoadFrame, unsigned long>;
    void (*loadShort)(LoadFrame&, unsigned short&) = Replayable<0, LoadFrame, unsigned short>;
    (void)saveFloat;
    (void)loadFloat;
    (void)saveChar;
    (void)loadChar;
    (void)saveController;
    (void)saveLong;
    (void)saveShort;
    (void)loadController;
    (void)loadLong;
    (void)loadShort;
}
