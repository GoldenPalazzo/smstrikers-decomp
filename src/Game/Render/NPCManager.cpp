#include "Game/Render/NPCManager.h"

#include "Game/Game.h"
#include "Game/SHierarchy.h"
#include "Game/GameInfo.h"
#include "Game/Render/AnimatedModelExplodable.h"
#include "Game/Render/CameraGuy.h"
#include "Game/World/worldanim.h"
#include "Game/WorldManager.h"
#include "NL/nlFile.h"
#include "NL/nlFileGC.h"
#include "NL/nlMemory.h"
#include "NL/nlString.h"
#include "NL/gl/gl.h"

struct NPCTemplateInfo
{
    /* 0x00 */ s32 id;
    /* 0x04 */ const char* modelFilename;
    /* 0x08 */ const char* textureFilename;
    /* 0x0C */ const char* hierarchyFilename;
    /* 0x10 */ const char* hierarchyName;
    /* 0x14 */ const char* animFilename;
    /* 0x18 */ u8 loadAnimsVirtual;
};

extern NPCTemplateInfo gNPCTemplateInfo[];

struct glModelData
{
    /* 0x00 */ u32 pad;
    /* 0x04 */ s32 numModels;
};

static inline cSHierarchy* FindHierarchy(ListEntry<cSHierarchy*>* hEntry, u32 hash)
{
    while (hEntry != NULL)
    {
        if (hash == hEntry->entry->m_uHashID)
        {
            return hEntry->entry;
        }
        hEntry = hEntry->next;
    }
    return NULL;
}

// /**
//  * Offset/Address/Size: 0x68 | 0x80167338 | size: 0x28
//  */
// void nlListAddStart<ListEntry<SkinAnimatedNPC*>>(ListEntry<SkinAnimatedNPC*>**, ListEntry<SkinAnimatedNPC*>*, ListEntry<SkinAnimatedNPC*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801672D0 | size: 0x68
//  */
// void nlWalkList<ListEntry<SkinAnimatedNPC*>, ListContainerBase<SkinAnimatedNPC*, NewAdapter<ListEntry<SkinAnimatedNPC*>>>>(ListEntry<SkinAnimatedNPC*>*, ListContainerBase<SkinAnimatedNPC*, NewAdapter<ListEntry<SkinAnimatedNPC*>>>*, void (ListContainerBase<SkinAnimatedNPC*, NewAdapter<ListEntry<SkinAnimatedNPC*>>>::*)(ListEntry<SkinAnimatedNPC*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801672AC | size: 0x24
//  */
// void ListContainerBase<SkinAnimatedNPC*, NewAdapter<ListEntry<SkinAnimatedNPC*>>>::DeleteEntry(ListEntry<SkinAnimatedNPC*>*)
// {
// }

/**
 * Offset/Address/Size: 0x8AC | 0x80166770 | size: 0xB3C
 * TODO: 99.93% match - the five animation inventory receiver loads use r26
 * instead of r27.
 */
NPCManager::NPCManager()
    : mpInventorySAnim(NULL)
    , mpInventorySHierarchy(NULL)
{
    mpInventorySHierarchy = new (nlMalloc(sizeof(cInventory<cSHierarchy>), 8, false)) cInventory<cSHierarchy>();
    mpInventorySAnim = new (nlMalloc(sizeof(cInventory<cSAnim>), 8, false)) cInventory<cSAnim>();

    mNPCTemplate[0].loaded = false;
    mNPCTemplate[1].loaded = false;
    mNPCTemplate[2].loaded = false;
    mNPCTemplate[3].loaded = false;
    mNPCTemplate[4].loaded = false;
    mNPCTemplate[5].loaded = false;
    mNPCTemplate[6].loaded = false;

    World* world = WorldManager::s_World;

    typedef nlAVLTreeIterator<unsigned long, HelperObject*, DefaultKeyCompare<unsigned long> > HelperIterator;
    HelperIterator* stack = new (nlMalloc(sizeof(HelperIterator), 8, false)) HelperIterator(world->m_helperMap);

    while (stack->m_NumStackEntries > 0)
    {
        if (world->CompareNameToGenericName(stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_szName, "cameraguy") == 0)
        {
            CreateNPCTemplate(0, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("camera_idle");
            cSAnim* foundAnim = ((AnimationSet*)animInv)->FindAnimationByHash(hash);
            CameraGuy* guy = new (nlMalloc(sizeof(CameraGuy), 8, false)) CameraGuy(*mNPCTemplate[0].hierarchy, mNPCTemplate[0].modelID);
            guy->Init();
            guy->SetIdleAnim(*foundAnim);
            guy->mWorldMatrix = stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->entry = guy;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
            new (nlMalloc(sizeof(AnimatedModelExplodable), 8, false)) AnimatedModelExplodable(EXPLODABLE_CAMERAGUY, guy);
        }
        else if (world->CompareNameToGenericName(stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_szName, "standupcamera") == 0)
        {
            CreateNPCTemplate(1, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("standupcamera_idle");
            cSAnim* foundAnim = ((AnimationSet*)animInv)->FindAnimationByHash(hash);
            CameraGuy* guy = new (nlMalloc(sizeof(CameraGuy), 8, false)) CameraGuy(*mNPCTemplate[1].hierarchy, mNPCTemplate[1].modelID);
            guy->Init();
            guy->SetIdleAnim(*foundAnim);
            guy->mWorldMatrix = stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->entry = guy;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
            new (nlMalloc(sizeof(AnimatedModelExplodable), 8, false)) AnimatedModelExplodable(EXPLODABLE_STANDUPCAMERA, guy);
        }
        else if (world->CompareNameToGenericName(stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_szName, "medic") == 0)
        {
            CreateNPCTemplate(2, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("medic_idle");
            cSAnim* foundAnim = ((AnimationSet*)animInv)->FindAnimationByHash(hash);
            SkinAnimatedNPC* npc = new (nlMalloc(sizeof(SkinAnimatedNPC), 8, false)) SkinAnimatedNPC(*mNPCTemplate[2].hierarchy, mNPCTemplate[2].modelID);
            npc->SetAnimState(*foundAnim, 0.2f, (ePlayMode)0);
            npc->mWorldMatrix = stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->entry = npc;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
        }
        else if (world->CompareNameToGenericName(stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_szName, "securityguard") == 0)
        {
            CreateNPCTemplate(3, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("securityguard_idle");
            cSAnim* foundAnim = ((AnimationSet*)animInv)->FindAnimationByHash(hash);
            SkinAnimatedNPC* npc = new (nlMalloc(sizeof(SkinAnimatedNPC), 8, false)) SkinAnimatedNPC(*mNPCTemplate[3].hierarchy, mNPCTemplate[3].modelID);
            npc->SetAnimState(*foundAnim, 0.2f, (ePlayMode)0);
            npc->mWorldMatrix = stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->entry = npc;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
        }
        else if (world->CompareNameToGenericName(stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_szName, "blimp") == 0)
        {
            CreateNPCTemplate(4, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("blimp_idle");
            cSAnim* foundAnim = ((AnimationSet*)animInv)->FindAnimationByHash(hash);
            SkinAnimatedNPC* npc = new (nlMalloc(sizeof(SkinAnimatedNPC), 8, false)) SkinAnimatedNPC(*mNPCTemplate[4].hierarchy, mNPCTemplate[4].modelID);
            npc->SetAnimState(*foundAnim, 0.2f, (ePlayMode)0);
            npc->mWorldMatrix = stack->m_Stack[stack->m_NumStackEntries - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->entry = npc;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
        }

        stack->Next();
    }

    delete stack;

    CreateNPCTemplate(5, true);
    PhysicsNPC* chainPhysics = new (nlMalloc(sizeof(PhysicsNPC), 8, false)) PhysicsNPC(g_pGame->m_pGameTweaks->fChainChompRadius);
    ChainChomp* chainChomp = new (nlMalloc(sizeof(ChainChomp), 8, false)) ChainChomp(*mNPCTemplate[5].hierarchy, mNPCTemplate[5].modelID, *chainPhysics, mpInventorySAnim);
    mpChainChomp = chainChomp;
    chainPhysics->SetCallbackFunction(&ChainChomp::CollisionCallback);

    if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
        CreateNPCTemplate(6, false);
    else
        CreateNPCTemplate(6, true);

    PhysicsNPC* bowserPhysics = new (nlMalloc(sizeof(PhysicsNPC), 8, false)) PhysicsNPC(g_pGame->m_pGameTweaks->unk304);
    Bowser* bowser = new (nlMalloc(sizeof(Bowser), 8, false)) Bowser(*mNPCTemplate[6].hierarchy, mNPCTemplate[6].modelID, *bowserPhysics, mpInventorySAnim);
    mpBowser = bowser;
    bowserPhysics->SetCallbackFunction(&Bowser::CollisionCallback);
}

typedef ListContainerBase<SkinAnimatedNPC*, NewAdapter<ListEntry<SkinAnimatedNPC*> > > NPCListBaseHelper;
typedef ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*> > > HierListBaseHelper;
typedef ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*> > > SAnimListBaseHelper;

static inline void DestroyNPCList(nlListContainer<SkinAnimatedNPC*>* npcList)
{
    ListEntry<SkinAnimatedNPC*>* npcEntry = npcList->m_Head;
    while (npcEntry != NULL)
    {
        delete npcEntry->entry;
        npcEntry = npcEntry->next;
    }

    nlWalkList(npcList->m_Head, (NPCListBaseHelper*)npcList, &NPCListBaseHelper::DeleteEntry);
    npcList->m_Head = NULL;
    npcList->m_Tail = NULL;
}

static inline void DestroyHierarchyInventory(cInventory<cSHierarchy>* pHierInv)
{
    ListEntry<cSHierarchy*>* hierEntry = pHierInv->m_lItemList.m_Head;
    while (hierEntry != NULL)
    {
        hierEntry = hierEntry->next;
    }

    void (HierListBaseHelper::*cbHier)(ListEntry<cSHierarchy*>*) = &HierListBaseHelper::DeleteEntry;
    nlWalkList(pHierInv->m_lItemList.m_Head, (HierListBaseHelper*)pHierInv, cbHier);

    ListEntry<char*>** pHead;
    ListEntry<char*>** pTail = &pHierInv->m_lMemList.m_Tail;
    pHierInv->m_lItemList.m_Head = NULL;
    pHead = &pHierInv->m_lMemList.m_Head;
    pHierInv->m_lItemList.m_Tail = NULL;
    while (pHierInv->m_lMemList.m_Head != NULL)
    {
        ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead, pTail);
        void* mesh;
        if (&mesh != NULL)
        {
            mesh = first->entry;
        }
        ::operator delete(first);
        ::operator delete(mesh);
    }

    pHierInv->m_nItemCount = 0;
    pHierInv->m_lMemList.~nlListContainer();
    pHierInv->m_lItemList.~nlListContainer();
    ::operator delete(pHierInv);
}

static inline void DestroySAnimInventory(cInventory<cSAnim>* pSAnimInv)
{
    ListEntry<cSAnim*>* animEntry = pSAnimInv->m_lItemList.m_Head;
    while (animEntry != NULL)
    {
        animEntry->entry->Destroy();
        animEntry = animEntry->next;
    }

    void (SAnimListBaseHelper::*cbAnim)(ListEntry<cSAnim*>*) = &SAnimListBaseHelper::DeleteEntry;
    nlWalkList(pSAnimInv->m_lItemList.m_Head, (SAnimListBaseHelper*)pSAnimInv, cbAnim);

    ListEntry<char*>** pHead2;
    ListEntry<char*>** pTail2 = &pSAnimInv->m_lMemList.m_Tail;
    pSAnimInv->m_lItemList.m_Head = NULL;
    pHead2 = &pSAnimInv->m_lMemList.m_Head;
    pSAnimInv->m_lItemList.m_Tail = NULL;
    while (pSAnimInv->m_lMemList.m_Head != NULL)
    {
        ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead2, pTail2);
        void* mesh;
        if (&mesh != NULL)
        {
            mesh = first->entry;
        }
        ::operator delete(first);
        ::operator delete(mesh);
    }

    pSAnimInv->m_nItemCount = 0;
    pSAnimInv->m_lMemList.~nlListContainer();
    pSAnimInv->m_lItemList.~nlListContainer();
    ::operator delete(pSAnimInv);
}

/**
 * Offset/Address/Size: 0x4D8 | 0x8016639C | size: 0x3D4
 * TODO: 99.90% match - remaining diff is the cSAnim destroy loop register.
 */
NPCManager::~NPCManager()
{
    DestroyNPCList(&mNPCList);

    delete mpBowser;
    delete mpChainChomp;

    cInventory<cSHierarchy>* pHierInv = mpInventorySHierarchy;
    if (pHierInv != NULL)
    {
        DestroyHierarchyInventory(pHierInv);
    }

    cInventory<cSAnim>* pSAnimInv = mpInventorySAnim;
    if (pSAnimInv != NULL)
    {
        DestroySAnimInventory(pSAnimInv);
    }
}

/**
 * Offset/Address/Size: 0x47C | 0x80166340 | size: 0x5C
 */
void NPCManager::UpdateNPCs(float dt)
{
    ListEntry<SkinAnimatedNPC*>* current = mNPCList.m_Head;
    while (current != nullptr)
    {
        current->entry->Update(dt);
        current = current->next;
    }
}

/**
 * Offset/Address/Size: 0x430 | 0x801662F4 | size: 0x4C
 */
void NPCManager::RenderNPCs()
{
    ListEntry<SkinAnimatedNPC*>* current = mNPCList.m_Head;
    while (current != nullptr)
    {
        current->entry->Render();
        current = current->next;
    }
}

/**
 * Offset/Address/Size: 0x3D0 | 0x80166294 | size: 0x60
 */
void NPCManager::UpdateAINPCs(float dt)
{
    mpChainChomp->Update(dt);
    mpBowser->Update(dt);
}

/**
 * Offset/Address/Size: 0x0 | 0x80165EC4 | size: 0x3D0
 * TODO: 99.14% match - extra return-value moves remain after hierarchy and
 * non-virtual animation file loads.
 */
void NPCManager::CreateNPCTemplate(int templateIndex, bool loadTextures)
{
    int animSizeVirt;
    u32 hierFileSize;
    u32 animFileSize;

    if (mNPCTemplate[templateIndex].loaded)
    {
        return;
    }

    glModel* model;
    if (loadTextures)
    {
        glLoadTextureBundle(gNPCTemplateInfo[templateIndex].textureFilename);
        model = glLoadModel(gNPCTemplateInfo[templateIndex].modelFilename, NULL);
    }
    else
    {
        model = NULL;
    }

    cInventory<cSHierarchy>* hierInv = mpInventorySHierarchy;
    nlChunk* hierData = (nlChunk*)nlLoadEntireFile(
        gNPCTemplateInfo[templateIndex].hierarchyFilename,
        &hierFileSize,
        0x20,
        AllocateStart);

    ListEntry<char*>* memEntry = (ListEntry<char*>*)nlMalloc(8, 8, false);
    if (memEntry != NULL)
    {
        memEntry->next = NULL;
        memEntry->entry = (char*)hierData;
    }
    nlListAddStart<ListEntry<char*> >(
        (ListEntry<char*>**)&hierInv->m_lMemList.m_Head,
        memEntry,
        (ListEntry<char*>**)&hierInv->m_lMemList.m_Tail);

    nlChunk* hierEnd = (nlChunk*)((char*)hierData + hierFileSize);
    while (hierData != hierEnd)
    {
        if ((hierData->m_ID & 0x80FFFFFF) == 0x80018000)
        {
            cSHierarchy* hier = cSHierarchy::Initialize(hierData);

            ListEntry<cSHierarchy*>* itemEntry = (ListEntry<cSHierarchy*>*)nlMalloc(8, 8, false);
            if (itemEntry != NULL)
            {
                itemEntry->next = NULL;
                itemEntry->entry = hier;
            }
            nlListAddStart<ListEntry<cSHierarchy*> >(
                (ListEntry<cSHierarchy*>**)&hierInv->m_lItemList.m_Head,
                itemEntry,
                (ListEntry<cSHierarchy*>**)&hierInv->m_lItemList.m_Tail);
            hierInv->m_nItemCount++;
        }
        else
        {
            nlPrintf("Unknown chunk\n");
        }
        hierData = (nlChunk*)((char*)hierData + hierData->m_Size + 8);
    }

    if (gNPCTemplateInfo[templateIndex].loadAnimsVirtual)
    {
        nlChunk* animData = (nlChunk*)nlLoadEntireFileToVirtualMemory(
            gNPCTemplateInfo[templateIndex].animFilename,
            &animSizeVirt,
            0x10000,
            NULL,
            AllocateStart);
        int animSize = animSizeVirt;
        nlChunk* animEnd;
        cInventory<cSAnim>* animInv = mpInventorySAnim;

        ListEntry<char*>* animMem = (ListEntry<char*>*)nlMalloc(8, 8, false);
        if (animMem != NULL)
        {
            animMem->next = NULL;
            animMem->entry = (char*)animData;
        }
        nlListAddStart<ListEntry<char*> >(
            (ListEntry<char*>**)&animInv->m_lMemList.m_Head,
            animMem,
            (ListEntry<char*>**)&animInv->m_lMemList.m_Tail);

        animEnd = (nlChunk*)((char*)animData + animSize);
        while (animData != animEnd)
        {
            if ((animData->m_ID & 0x80FFFFFF) == 0x80017000)
            {
                cSAnim* anim = cSAnim::Initialize(animData);
                ListEntry<cSAnim*>* animItem = (ListEntry<cSAnim*>*)nlMalloc(8, 8, false);
                if (animItem != NULL)
                {
                    animItem->next = NULL;
                    animItem->entry = anim;
                }
                nlListAddStart<ListEntry<cSAnim*> >(
                    (ListEntry<cSAnim*>**)&animInv->m_lItemList.m_Head,
                    animItem,
                    (ListEntry<cSAnim*>**)&animInv->m_lItemList.m_Tail);
                animInv->m_nItemCount++;
            }
            else
            {
                nlPrintf("Unknown chunk\n");
            }
            animData = (nlChunk*)((char*)animData + animData->m_Size + 8);
        }
    }
    else
    {
        nlChunk* animData;
        nlChunk* animEnd;
        cInventory<cSAnim>* animInv = mpInventorySAnim;
        animData = (nlChunk*)nlLoadEntireFile(
            gNPCTemplateInfo[templateIndex].animFilename,
            &animFileSize,
            0x20,
            AllocateStart);

        ListEntry<char*>* animMem = (ListEntry<char*>*)nlMalloc(8, 8, false);
        if (animMem != NULL)
        {
            animMem->next = NULL;
            animMem->entry = (char*)animData;
        }
        nlListAddStart<ListEntry<char*> >(
            (ListEntry<char*>**)&animInv->m_lMemList.m_Head,
            animMem,
            (ListEntry<char*>**)&animInv->m_lMemList.m_Tail);

        animEnd = (nlChunk*)((char*)animData + animFileSize);
        while (animData != animEnd)
        {
            if ((animData->m_ID & 0x80FFFFFF) == 0x80017000)
            {
                cSAnim* anim = cSAnim::Initialize(animData);
                ListEntry<cSAnim*>* animItem = (ListEntry<cSAnim*>*)nlMalloc(8, 8, false);
                if (animItem != NULL)
                {
                    animItem->next = NULL;
                    animItem->entry = anim;
                }
                nlListAddStart<ListEntry<cSAnim*> >(
                    (ListEntry<cSAnim*>**)&animInv->m_lItemList.m_Head,
                    animItem,
                    (ListEntry<cSAnim*>**)&animInv->m_lItemList.m_Tail);
                animInv->m_nItemCount++;
            }
            else
            {
                nlPrintf("Unknown chunk\n");
            }
            animData = (nlChunk*)((char*)animData + animData->m_Size + 8);
        }
    }

    mNPCTemplate[templateIndex].modelID = model == NULL ? -1 : ((glModelData*)model)->numModels;

    cInventory<cSHierarchy>* searchInv = mpInventorySHierarchy;
    u32 hash = nlStringHash(gNPCTemplateInfo[templateIndex].hierarchyName);
    mNPCTemplate[templateIndex].hierarchy = FindHierarchy(searchInv->m_lItemList.m_Head, hash);
    mNPCTemplate[templateIndex].loaded = true;
}
