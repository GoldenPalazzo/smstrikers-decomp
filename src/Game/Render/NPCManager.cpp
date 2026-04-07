#include "Game/Render/NPCManager.h"

#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/Render/AnimatedModelExplodable.h"
#include "Game/Render/CameraGuy.h"
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

extern "C" cSHierarchy* Initialize__11cSHierarchyFP7nlChunk(nlChunk*);

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
 * TODO: 96.50% match - decomp.me generates beq for while+break instead of target's
 * bne+b pattern, causing li r26,0 placement diff (before loop vs fall-through).
 * Also r26/r27 swap for animInv in else-if blocks 1-4. Likely inherent decomp.me
 * compiler optimization difference.
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

    u32* stack = (u32*)nlMalloc(8, 8, false);
    if (stack != NULL)
    {
        AVLTreeEntry<unsigned long, HelperObject*>* node = world->m_helperMap.m_Root;
        stack[0] = (u32)nlMalloc((world->m_helperMap.m_NumElements + 1) * 4, 8, false);
        stack[1] = 0;
        if (node != NULL)
        {
            while (node->node.left != NULL)
            {
                ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1]] = node;
                stack[1]++;
                node = (AVLTreeEntry<unsigned long, HelperObject*>*)node->node.left;
            }
            ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1]] = node;
            stack[1]++;
        }
    }

    while (stack[1] > 0)
    {
        if (world->CompareNameToGenericName(((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_szName, "cameraguy") == 0)
        {
            CreateNPCTemplate(0, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("camera_idle");
            ListEntry<cSAnim*>* animEntry = animInv->m_lItemList.m_Head;
            cSAnim* foundAnim = NULL;
            while (animEntry != NULL)
            {
                foundAnim = animEntry->data;
                if (hash == foundAnim->m_uHashID)
                    break;
                animEntry = animEntry->next;
            }
            if (animEntry == NULL)
                foundAnim = NULL;
            CameraGuy* guy = new (nlMalloc(sizeof(CameraGuy), 8, false)) CameraGuy(*mNPCTemplate[0].hierarchy, mNPCTemplate[0].modelID);
            guy->Init();
            guy->SetIdleAnim(*foundAnim);
            guy->mWorldMatrix = ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->data = guy;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
            new (nlMalloc(sizeof(AnimatedModelExplodable), 8, false)) AnimatedModelExplodable(EXPLODABLE_CAMERAGUY, guy);
        }
        else if (world->CompareNameToGenericName(((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_szName, "standupcamera") == 0)
        {
            CreateNPCTemplate(1, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("standupcamera_idle");
            ListEntry<cSAnim*>* animEntry = animInv->m_lItemList.m_Head;
            cSAnim* foundAnim = NULL;
            while (animEntry != NULL)
            {
                foundAnim = animEntry->data;
                if (hash == foundAnim->m_uHashID)
                    break;
                animEntry = animEntry->next;
            }
            if (animEntry == NULL)
                foundAnim = NULL;
            CameraGuy* guy = new (nlMalloc(sizeof(CameraGuy), 8, false)) CameraGuy(*mNPCTemplate[1].hierarchy, mNPCTemplate[1].modelID);
            guy->Init();
            guy->SetIdleAnim(*foundAnim);
            guy->mWorldMatrix = ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->data = guy;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
            new (nlMalloc(sizeof(AnimatedModelExplodable), 8, false)) AnimatedModelExplodable(EXPLODABLE_STANDUPCAMERA, guy);
        }
        else if (world->CompareNameToGenericName(((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_szName, "medic") == 0)
        {
            CreateNPCTemplate(2, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("medic_idle");
            ListEntry<cSAnim*>* animEntry = animInv->m_lItemList.m_Head;
            cSAnim* foundAnim = NULL;
            while (animEntry != NULL)
            {
                foundAnim = animEntry->data;
                if (hash == foundAnim->m_uHashID)
                    break;
                animEntry = animEntry->next;
            }
            if (animEntry == NULL)
                foundAnim = NULL;
            SkinAnimatedNPC* npc = new (nlMalloc(sizeof(SkinAnimatedNPC), 8, false)) SkinAnimatedNPC(*mNPCTemplate[2].hierarchy, mNPCTemplate[2].modelID);
            npc->SetAnimState(*foundAnim, 0.2f, (ePlayMode)0);
            npc->mWorldMatrix = ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->data = npc;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
        }
        else if (world->CompareNameToGenericName(((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_szName, "securityguard") == 0)
        {
            CreateNPCTemplate(3, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("securityguard_idle");
            ListEntry<cSAnim*>* animEntry = animInv->m_lItemList.m_Head;
            cSAnim* foundAnim = NULL;
            while (animEntry != NULL)
            {
                foundAnim = animEntry->data;
                if (hash == foundAnim->m_uHashID)
                    break;
                animEntry = animEntry->next;
            }
            if (animEntry == NULL)
                foundAnim = NULL;
            SkinAnimatedNPC* npc = new (nlMalloc(sizeof(SkinAnimatedNPC), 8, false)) SkinAnimatedNPC(*mNPCTemplate[3].hierarchy, mNPCTemplate[3].modelID);
            npc->SetAnimState(*foundAnim, 0.2f, (ePlayMode)0);
            npc->mWorldMatrix = ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->data = npc;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
        }
        else if (world->CompareNameToGenericName(((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_szName, "blimp") == 0)
        {
            CreateNPCTemplate(4, true);
            cInventory<cSAnim>* animInv = mpInventorySAnim;
            u32 hash = nlStringHash("blimp_idle");
            ListEntry<cSAnim*>* animEntry = animInv->m_lItemList.m_Head;
            cSAnim* foundAnim = NULL;
            while (animEntry != NULL)
            {
                foundAnim = animEntry->data;
                if (hash == foundAnim->m_uHashID)
                    break;
                animEntry = animEntry->next;
            }
            if (animEntry == NULL)
                foundAnim = NULL;
            SkinAnimatedNPC* npc = new (nlMalloc(sizeof(SkinAnimatedNPC), 8, false)) SkinAnimatedNPC(*mNPCTemplate[4].hierarchy, mNPCTemplate[4].modelID);
            npc->SetAnimState(*foundAnim, 0.2f, (ePlayMode)0);
            npc->mWorldMatrix = ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1] - 1]->value->m_worldMatrix;
            ListEntry<SkinAnimatedNPC*>* listEntry = (ListEntry<SkinAnimatedNPC*>*)nlMalloc(8, 8, false);
            if (listEntry != NULL)
            {
                listEntry->next = NULL;
                listEntry->data = npc;
            }
            nlListAddStart<ListEntry<SkinAnimatedNPC*> >(&mNPCList.m_Head, listEntry, &mNPCList.m_Tail);
        }

        stack[1]--;
        AVLTreeEntry<unsigned long, HelperObject*>* popped = ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1]];
        AVLTreeEntry<unsigned long, HelperObject*>* rightChild = (AVLTreeEntry<unsigned long, HelperObject*>*)popped->node.right;
        if (rightChild != NULL)
        {
            while (rightChild->node.left != NULL)
            {
                ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1]] = rightChild;
                stack[1]++;
                rightChild = (AVLTreeEntry<unsigned long, HelperObject*>*)rightChild->node.left;
            }
            ((AVLTreeEntry<unsigned long, HelperObject*>**)stack[0])[stack[1]] = rightChild;
            stack[1]++;
        }
    }

    if (stack != NULL)
    {
        ::operator delete[]((void*)stack[0]);
        ::operator delete(stack);
    }

    CreateNPCTemplate(5, true);
    PhysicsNPC* chainPhysics = new (nlMalloc(sizeof(PhysicsNPC), 8, false)) PhysicsNPC(g_pGame->m_pGameTweaks->fChainChompRadius);
    ChainChomp* chainChomp = new (nlMalloc(sizeof(ChainChomp), 8, false)) ChainChomp(*mNPCTemplate[5].hierarchy, mNPCTemplate[5].modelID, *chainPhysics, mpInventorySAnim);
    mpChainChomp = chainChomp;
    {
        union
        {
            void (ChainChomp::*mfp)(PhysicsObject*, PhysicsObject*, const nlVector3&);
            PhysicsNPC::CallbackFn fp;
        } u;
        u.mfp = &ChainChomp::CollisionCallback;
        chainPhysics->SetCallbackFunction(u.fp);
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
        CreateNPCTemplate(6, false);
    else
        CreateNPCTemplate(6, true);

    PhysicsNPC* bowserPhysics = new (nlMalloc(sizeof(PhysicsNPC), 8, false)) PhysicsNPC(g_pGame->m_pGameTweaks->unk304);
    Bowser* bowser = new (nlMalloc(sizeof(Bowser), 8, false)) Bowser(*mNPCTemplate[6].hierarchy, mNPCTemplate[6].modelID, *bowserPhysics, mpInventorySAnim);
    mpBowser = bowser;
    {
        union
        {
            void (Bowser::*mfp)(PhysicsObject*, PhysicsObject*, const nlVector3&);
            PhysicsNPC::CallbackFn fp;
        } u;
        u.mfp = &Bowser::CollisionCallback;
        bowserPhysics->SetCallbackFunction(u.fp);
    }
}

/**
 * Offset/Address/Size: 0x4D8 | 0x8016639C | size: 0x3D4
 */
NPCManager::~NPCManager()
{
}

/**
 * Offset/Address/Size: 0x47C | 0x80166340 | size: 0x5C
 */
void NPCManager::UpdateNPCs(float dt)
{
    ListEntry<SkinAnimatedNPC*>* current = mNPCList.m_Head;
    while (current != nullptr)
    {
        current->data->Update(dt);
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
        current->data->Render();
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
 * TODO: 95.90% match - MWCC scheduling diff: extra mr r0,r3 + mr r25,r0 instead
 * of direct mr r25,r3 at nlLoadEntireFile return sites (hierarchy + else branch).
 * Also r25/r26 swap for animInv/animEnd in virtual/else branches and search loop
 * register differences. Likely inherent scheduler difference on decomp.me.
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
        memEntry->data = (char*)hierData;
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
            cSHierarchy* hier = Initialize__11cSHierarchyFP7nlChunk(hierData);

            ListEntry<cSHierarchy*>* itemEntry = (ListEntry<cSHierarchy*>*)nlMalloc(8, 8, false);
            if (itemEntry != NULL)
            {
                itemEntry->next = NULL;
                itemEntry->data = hier;
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
        cInventory<cSAnim>* animInv = mpInventorySAnim;

        ListEntry<char*>* animMem = (ListEntry<char*>*)nlMalloc(8, 8, false);
        if (animMem != NULL)
        {
            animMem->next = NULL;
            animMem->data = (char*)animData;
        }
        nlListAddStart<ListEntry<char*> >(
            (ListEntry<char*>**)&animInv->m_lMemList.m_Head,
            animMem,
            (ListEntry<char*>**)&animInv->m_lMemList.m_Tail);

        nlChunk* animEnd = (nlChunk*)((char*)animData + animSize);
        while (animData != animEnd)
        {
            if ((animData->m_ID & 0x80FFFFFF) == 0x80017000)
            {
                cSAnim* anim = cSAnim::Initialize(animData);
                ListEntry<cSAnim*>* animItem = (ListEntry<cSAnim*>*)nlMalloc(8, 8, false);
                if (animItem != NULL)
                {
                    animItem->next = NULL;
                    animItem->data = anim;
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
        cInventory<cSAnim>* animInv = mpInventorySAnim;
        nlChunk* animData = (nlChunk*)nlLoadEntireFile(
            gNPCTemplateInfo[templateIndex].animFilename,
            &animFileSize,
            0x20,
            AllocateStart);

        ListEntry<char*>* animMem = (ListEntry<char*>*)nlMalloc(8, 8, false);
        if (animMem != NULL)
        {
            animMem->next = NULL;
            animMem->data = (char*)animData;
        }
        nlListAddStart<ListEntry<char*> >(
            (ListEntry<char*>**)&animInv->m_lMemList.m_Head,
            animMem,
            (ListEntry<char*>**)&animInv->m_lMemList.m_Tail);

        nlChunk* animEnd = (nlChunk*)((char*)animData + animFileSize);
        while (animData != animEnd)
        {
            if ((animData->m_ID & 0x80FFFFFF) == 0x80017000)
            {
                cSAnim* anim = cSAnim::Initialize(animData);
                ListEntry<cSAnim*>* animItem = (ListEntry<cSAnim*>*)nlMalloc(8, 8, false);
                if (animItem != NULL)
                {
                    animItem->next = NULL;
                    animItem->data = anim;
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

    mNPCTemplate[templateIndex].modelID = model != NULL ? ((glModelData*)model)->numModels : -1;

    cInventory<cSHierarchy>* searchInv = mpInventorySHierarchy;
    u32 hash = nlStringHash(gNPCTemplateInfo[templateIndex].hierarchyName);
    ListEntry<cSHierarchy*>* entry = (ListEntry<cSHierarchy*>*)searchInv->m_lItemList.m_Head;
    cSHierarchy* foundHier = NULL;
    while (entry != NULL)
    {
        if (hash == entry->data->m_uHashID)
        {
            foundHier = entry->data;
            break;
        }
        entry = entry->next;
    }

    mNPCTemplate[templateIndex].hierarchy = foundHier;
    mNPCTemplate[templateIndex].loaded = true;
}
