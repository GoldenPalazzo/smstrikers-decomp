#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EmissionController.h"
#include "Game/Effects/EmissionManager.h"
#include "NL/nlAVLTree.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlDLRing.h"
#include "NL/nlMain.h"
#include "NL/nlString.h"

nlAVLTree<unsigned long, EffectsGroup*, DefaultKeyCompare<unsigned long> >* pGroupMap = nullptr;
nlAVLTree<unsigned long, EffectsTerrainSpec*, DefaultKeyCompare<unsigned long> >* pTerrainSpecMap = nullptr;
extern "C" int atoi(const char*);
static char last_spec_name[0x100];
static s32 gnUserEffectTypes;
static class UserEffectFactory* gUserEffectTypes[3];

class UserEffectFactory
{
public:
    virtual ~UserEffectFactory();
    virtual UserEffectSpec* ParseSpec(SimpleParser* parser);
    virtual const char* GetName();
};

typedef DLListContainerBase<UserEffectSpec*, NewAdapter<DLListEntry<UserEffectSpec*> > > UserSpecContainer;

struct EffectsSpecRaw
{
    u32 m_uHashID;
    EffectsTemplate* m_pTemplate;
    eFXBinding m_eAttach;
    u32 m_uJointID;
    f32 m_fDelay;
    u32 m_uLayer;
    eJointBinding m_eJointBinding;
    f32 m_fJointVelocity;
    u8 m_bInFront;
    u8 m_bGround;
    u8 m_bLight;
    u8 _pad23;
    f32 m_fOffset;
    nlVector3 m_vLocalOffset;
    EffectsTerrainSpec* m_pTerrainSpec;
    f32 m_fLingerStart;
    f32 m_fLingerEnd;
};

/**
 * Offset/Address/Size: 0xFC8 | 0x801F3A10 | size: 0x38
 * TODO: 98.6% match - r3/r5 register swap for pTerrainIDs pointer (leaf function register allocation artifact)
 */
bool EffectsTerrainSpec::HasTerrain(unsigned long terrainID) const
{
    u32 count = m_uNumTerrains;
    u32* pTerrainIDs = m_pTerrainIDs;

    for (u32 i = 0; i < count; i++)
    {
        if (pTerrainIDs[i] == terrainID)
        {
            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0xF70 | 0x801F39B8 | size: 0x58
 */
EffectsSpec::EffectsSpec()
{
    m_uHashID = 0;
    m_pTemplate = nullptr;
    m_eAttach = FXBind_Emitter;
    m_uJointID = 0;
    m_fDelay = 0.0f;
    m_uLayer = 0;
    m_eJointBinding = JB_Normal;
    m_fJointVelocity = 0.0f;
    m_bInFront = false;
    m_bGround = false;
    m_bLight = false;
    m_fOffset = 0.0f;
    m_pTerrainSpec = nullptr;
    m_fLingerStart = 1.0f;
    m_fLingerEnd = 1.0f;
    m_vLocalOffset.f.x = 0.0f;
    m_vLocalOffset.f.y = 0.0f;
    m_vLocalOffset.f.z = 0.0f;
}

/**
 * Offset/Address/Size: 0xA30 | 0x801F3478 | size: 0x540
 * TODO: 94.45% match - the init default struct's per-field stores for m_eAttach
 * through m_fLingerEnd are optimized away (the values are never read); the target
 * retains those stores.
 */
struct EffectsSpecShadow
{
    u32 m_uHashID;
    EffectsTemplate* m_pTemplate;
    eFXBinding m_eAttach;
    u32 m_uJointID;
    f32 m_fDelay;
    u32 m_uLayer;
    eJointBinding m_eJointBinding;
    f32 m_fJointVelocity;
    u8 m_bInFront;
    u8 m_bGround;
    u8 m_bLight;
    u8 _pad23;
    f32 m_fOffset;
    nlVector3 m_vLocalOffset;
    EffectsTerrainSpec* m_pTerrainSpec;
    f32 m_fLingerStart;
    f32 m_fLingerEnd;
};

bool parse_spec(SimpleParser* parser, EffectsSpec& spec)
{
    char* token;
    char* lingerStartToken = nullptr;
    char* nextToken = nullptr;
    EffectsSpecShadow init;
    char jointName[128];

    init.m_vLocalOffset.f.x = 0.0f;
    init.m_vLocalOffset.f.y = 0.0f;
    init.m_vLocalOffset.f.z = 0.0f;

    spec.m_uHashID = 0;
    spec.m_pTemplate = nullptr;
    spec.m_eAttach = FXBind_Emitter;
    spec.m_uJointID = 0;
    spec.m_fDelay = 0.0f;
    spec.m_uLayer = 0;
    spec.m_eJointBinding = JB_Normal;
    spec.m_fJointVelocity = 0.0f;
    spec.m_bInFront = false;
    spec.m_bGround = false;
    spec.m_bLight = false;
    spec.m_fOffset = 0.0f;
    spec.m_vLocalOffset.as_u32[0] = init.m_vLocalOffset.as_u32[0];
    spec.m_vLocalOffset.as_u32[1] = init.m_vLocalOffset.as_u32[1];
    spec.m_vLocalOffset.as_u32[2] = init.m_vLocalOffset.as_u32[2];
    spec.m_pTerrainSpec = nullptr;
    spec.m_fLingerStart = -1.0f;

    init.m_eAttach = FXBind_Emitter;
    init.m_uJointID = 0;
    init.m_fDelay = 0.0f;
    init.m_uLayer = 0;
    init.m_eJointBinding = JB_Normal;
    init.m_fJointVelocity = 0.0f;
    init.m_bInFront = false;
    init.m_bGround = false;
    init.m_bLight = false;
    init.m_fOffset = 0.0f;
    init.m_pTerrainSpec = nullptr;
    init.m_fLingerStart = -1.0f;
    init.m_fLingerEnd = -1.0f;

    spec.m_fLingerEnd = -1.0f;

    token = parser->NextToken(true);
    if (token == nullptr)
    {
        return false;
    }

    nlStrNCpy<char>(last_spec_name, token, 0x100);

    spec.m_pTemplate = fxGetTemplate(nlStringHash(token));
    if (spec.m_pTemplate == nullptr)
    {
        EmissionManager::AddError("parse_group couldn't find template '%s'\n", last_spec_name);
        return false;
    }

    lingerStartToken = (char*)"linger_start";

    while (true)
    {
        if (nextToken == nullptr)
        {
            token = parser->NextTokenOnLine(true);
        }
        else
        {
            token = nextToken;
        }

        nextToken = nullptr;

        if (token == nullptr)
        {
            break;
        }

        if (nlStrCmp<char>(token, lingerStartToken) == 0)
        {
            f32 value = atof(parser->NextTokenOnLine(true));
            spec.m_fLingerEnd = value;
            spec.m_fLingerStart = value;
            continue;
        }

        if (nlStrCmp<char>(token, "linger_end") == 0)
        {
            spec.m_fLingerEnd = atof(parser->NextTokenOnLine(true));
            continue;
        }

        if (nlStrCmp<char>(token, "at") == 0 || nlStrCmp<char>(token, "on") == 0)
        {
            token = parser->NextTokenOnLine(true);

            if (nlStrCmp<char>(token, "emitter") == 0)
            {
                spec.m_eAttach = FXBind_Emitter;
                continue;
            }

            if (nlStrCmp<char>(token, "joint") == 0)
            {
                char* jointToken;

                jointToken = parser->NextTokenOnLine(true);
                spec.m_eAttach = FXBind_Joint;
                nlStrNCat<char>(jointName, "bip01 ", jointToken, 0x80);

                char* walk = jointName;
                while (*walk != '\0')
                {
                    if (*walk == '_')
                    {
                        *walk = ' ';
                    }
                    walk++;
                }

                spec.m_uJointID = nlStringLowerHash(jointName);

                token = parser->NextTokenOnLine(true);
                if (token == nullptr)
                {
                    continue;
                }

                if (nlStrCmp<char>(token, "ascend") == 0)
                {
                    spec.m_fJointVelocity = 1.0f;
                    spec.m_eJointBinding = JB_Ascend;

                    token = parser->NextTokenOnLine(true);
                    if (token != nullptr)
                    {
                        spec.m_fJointVelocity = atof(token);
                    }
                    continue;
                }

                nextToken = token;
                continue;
            }

            if (nlStrCmp<char>(token, "object") == 0 || nlStrCmp<char>(token, "ball") == 0 || nlStrCmp<char>(token, "puck") == 0)
            {
                spec.m_eAttach = FXBind_Object;
                continue;
            }

            EmissionManager::AddError("parse_spec: unknown fx binding '%s'\n", token);
            continue;
        }

        if (nlStrCmp<char>(token, "layer") == 0)
        {
            spec.m_uLayer = atoi(parser->NextTokenOnLine(true));
            continue;
        }

        if (nlStrCmp<char>(token, "infront") == 0)
        {
            spec.m_bInFront = true;
            continue;
        }

        if (nlStrCmp<char>(token, "delay") == 0)
        {
            spec.m_fDelay = atof(parser->NextTokenOnLine(true));
            continue;
        }

        if (nlStrCmp<char>(token, "ground") == 0)
        {
            spec.m_bGround = true;
            continue;
        }

        if (nlStrCmp<char>(token, "offset") == 0)
        {
            spec.m_fOffset = atof(parser->NextTokenOnLine(true));
            continue;
        }

        if (nlStrCmp<char>(token, "light") == 0)
        {
            spec.m_bLight = true;
            continue;
        }

        if (nlStrCmp<char>(token, "offsetx") == 0)
        {
            spec.m_vLocalOffset.f.x = atof(parser->NextTokenOnLine(true));
            continue;
        }

        if (nlStrCmp<char>(token, "offsety") == 0)
        {
            spec.m_vLocalOffset.f.y = atof(parser->NextTokenOnLine(true));
            continue;
        }

        if (nlStrCmp<char>(token, "offsetz") == 0)
        {
            spec.m_vLocalOffset.f.z = atof(parser->NextTokenOnLine(true));
            continue;
        }

        if (nlStrCmp<char>(token, "offsetxyz") == 0)
        {
            f32* pOffset = &spec.m_vLocalOffset.f.x;
            for (s32 i = 0; i < 3; i++)
            {
                *pOffset = atof(parser->NextTokenOnLine(true));
                pOffset++;
            }
            continue;
        }

        EmissionManager::AddError("parse_spec has an unrecognized token '%s'\n", token);
    }

    return true;
}

/**
 * Offset/Address/Size: 0x80C | 0x801F3254 | size: 0x224
 * TODO: 91.9% match - still blocked by MWCC register/induction choices:
 * parser stays in r31 with pointer walk (`stw` + pointer increment) instead of target's r28 parser + `stwx` with byte offset,
 * plus AVL search keeps hash/cmp/found in r4/r0/r28 with an extra post-loop `node != nullptr` check.
 */
EffectsTerrainSpec* parse_terrain_spec(SimpleParser* parser)
{
    unsigned long terrainIDs[256];
    unsigned long offset = 0;
    unsigned long numTerrains = 0;
    SimpleParser* p = parser;

    while (true)
    {
        char* token = p->NextTokenOnLine(true);
        if (token == nullptr)
        {
            break;
        }

        *(unsigned long*)((char*)terrainIDs + offset) = nlStringLowerHash(token);
        numTerrains++;
        offset += 4;
    }

    EffectsTerrainSpec* pSpec = (EffectsTerrainSpec*)nlMalloc(8, 8, false);
    if (pSpec != nullptr)
    {
        pSpec->m_pTerrainIDs = nullptr;
        pSpec->m_uNumTerrains = 0;
    }

    pSpec->m_uNumTerrains = numTerrains;
    pSpec->m_pTerrainIDs = (unsigned long*)nlMalloc(numTerrains * 4, 8, false);
    memcpy(pSpec->m_pTerrainIDs, terrainIDs, pSpec->m_uNumTerrains * 4);

    RunningChecksum checksum;
    checksum.ChecksumInt(pSpec->m_uNumTerrains);
    checksum.ChecksumData(pSpec->m_pTerrainIDs, pSpec->m_uNumTerrains * 4);

    AVLTreeEntry<unsigned long, EffectsTerrainSpec*>* node = pTerrainSpecMap->m_Root;
    unsigned long hashID = ~checksum.m_nChecksum;
    unsigned char found;
    EffectsTerrainSpec** foundValue;

    while (node != nullptr)
    {
        int cmpResult;

        if (hashID == node->key)
        {
            cmpResult = 0;
        }
        else if (hashID < node->key)
        {
            cmpResult = -1;
        }
        else
        {
            cmpResult = 1;
        }

        if (cmpResult == 0)
        {
            if (&foundValue != nullptr)
            {
                foundValue = &node->value;
            }

            found = true;
            break;
        }

        if (cmpResult < 0)
        {
            node = (AVLTreeEntry<unsigned long, EffectsTerrainSpec*>*)node->node.left;
        }
        else
        {
            node = (AVLTreeEntry<unsigned long, EffectsTerrainSpec*>*)node->node.right;
        }
    }

    if (node == nullptr)
    {
        found = false;
    }

    EffectsTerrainSpec* existingSpec = found ? *foundValue : nullptr;

    if (existingSpec == nullptr)
    {
        unsigned long key;
        EffectsTerrainSpec* pNewSpec = pSpec;
        RunningChecksum checksum2;
        AVLTreeNode* existingNode;

        checksum2.ChecksumInt(pSpec->m_uNumTerrains);
        checksum2.ChecksumData(pSpec->m_pTerrainIDs, pSpec->m_uNumTerrains * 4);
        key = ~checksum2.m_nChecksum;

        nlAVLTree<unsigned long, EffectsTerrainSpec*, DefaultKeyCompare<unsigned long> >* map = pTerrainSpecMap;
        map->AddAVLNode((AVLTreeNode**)&map->m_Root, &key, &pNewSpec, &existingNode, map->m_NumElements);

        if (existingNode == nullptr)
        {
            map->m_NumElements++;
        }

        return pSpec;
    }

    if (pSpec != nullptr)
    {
        if (pSpec->m_pTerrainIDs != nullptr)
        {
            delete[] pSpec->m_pTerrainIDs;
            pSpec->m_pTerrainIDs = nullptr;
        }

        delete pSpec;
    }

    return existingSpec;
}

/**
 * Offset/Address/Size: 0x338 | 0x801F2D80 | size: 0x4D4
 * TODO: 98.69% match - pGroup register r26 vs target r30 (cascading),
 * terrain_spec string SDA vs full address (1 extra inst),
 * pSpecEntry stores via r4 vs r3, user specs loop register diffs.
 */
EffectsGroup* parse_group(SimpleParser* parser)
{
    unsigned long hashID;
    EffectsSpec specs[64];
    unsigned long specCount;
    EffectsSpecRaw spec;
    nlDLListContainer<UserEffectSpec*> userSpecs;
    EffectsTerrainSpec* pTerrainSpec;
    int i;
    EffectsSpec* pSpecs;
    unsigned long specOffset;
    char* token;

    spec.m_uHashID = 0;
    spec.m_pTemplate = nullptr;
    spec.m_eAttach = FXBind_Emitter;
    spec.m_uJointID = 0;
    spec.m_fDelay = 0.0f;
    spec.m_uLayer = 0;
    spec.m_eJointBinding = JB_Normal;
    spec.m_fJointVelocity = 0.0f;
    spec.m_bInFront = false;
    spec.m_bGround = false;
    spec.m_bLight = false;
    spec.m_fOffset = 0.0f;
    spec.m_pTerrainSpec = nullptr;
    spec.m_fLingerStart = -1.0f;
    spec.m_fLingerEnd = -1.0f;
    spec.m_vLocalOffset.f.x = 0.0f;
    spec.m_vLocalOffset.f.y = 0.0f;
    spec.m_vLocalOffset.f.z = 0.0f;

    pTerrainSpec = nullptr;
    userSpecs.m_Head = nullptr;

    token = parser->NextToken(true);
    if (token == nullptr)
    {
        nlWalkDLRing(userSpecs.m_Head, (UserSpecContainer*)&userSpecs, &UserSpecContainer::DeleteEntry);
        userSpecs.m_Head = nullptr;
        return nullptr;
    }

    hashID = nlStringHash(token);
    specCount = 0;
    specOffset = 0;

    while (true)
    {
        token = parser->NextToken(true);
        if (token == nullptr)
        {
            nlWalkDLRing(userSpecs.m_Head, (UserSpecContainer*)&userSpecs, &UserSpecContainer::DeleteEntry);
            userSpecs.m_Head = nullptr;
            return nullptr;
        }

        if (nlStrCmp<char>(token, "end") == 0)
        {
            if (pTerrainSpec == nullptr)
            {
                break;
            }

            pTerrainSpec = nullptr;
            continue;
        }

        if (nlStrCmp<char>(token, "spec") == 0)
        {
            if (!parse_spec(parser, *(EffectsSpec*)&spec))
            {
                continue;
            }

            spec.m_pTerrainSpec = pTerrainSpec;
            specs[specOffset >> 6] = *(EffectsSpec*)&spec;
            specCount++;
            specOffset += sizeof(EffectsSpec);
            continue;
        }

        if (nlStrCmp<char>(token, "terrain_spec") == 0)
        {
            pTerrainSpec = parse_terrain_spec(parser);
            continue;
        }

        for (i = 0; i < gnUserEffectTypes; i++)
        {
            if (nlStrCmp<char>(gUserEffectTypes[i]->GetName(), token) == 0)
            {
                UserEffectSpec* pUserSpec = gUserEffectTypes[i]->ParseSpec(parser);
                void* mem = nlMalloc(0xC, 8, false);
                DLListEntry<UserEffectSpec*>* pSpecEntry = (DLListEntry<UserEffectSpec*>*)mem;
                if (mem != nullptr)
                {
                    pSpecEntry->m_next = nullptr;
                    pSpecEntry->m_prev = nullptr;
                    pSpecEntry->m_data = pUserSpec;
                }

                nlDLRingAddEnd(&userSpecs.m_Head, pSpecEntry);
                break;
            }
        }

        if (i == gnUserEffectTypes)
        {
            EmissionManager::AddError("parse_group has an unrecognized token '%s'\n", token);
        }
    }

    if (specCount == 0)
    {
        pSpecs = nullptr;
    }
    else
    {
        pSpecs = new (nlMalloc(specCount * sizeof(EffectsSpec) + 0x10, 8, false)) EffectsSpec[specCount];
        memcpy(pSpecs, specs, specCount << 6);
    }

    void* groupMem = nlMalloc(sizeof(EffectsGroup), 8, false);
    EffectsGroup* pGroup = (EffectsGroup*)groupMem;
    if (groupMem != nullptr)
    {
        pGroup->m_hashID = 0;
        pGroup->m_specs = nullptr;
        pGroup->m_numSpecs = 0;
        pGroup->m_userSpecsPtr = nullptr;
        pGroup->m_userSpecs = 0;
        pGroup->m_isLingering = false;
    }

    pGroup->m_hashID = hashID;
    pGroup->m_specs = pSpecs;
    pGroup->m_numSpecs = specCount;

    specOffset = 0;
    for (i = 0; i < pGroup->m_numSpecs; i++)
    {
        if (*(f32*)((u8*)pGroup->m_specs + specOffset + 0x38) >= 0.0f)
        {
            pGroup->m_isLingering = true;
            break;
        }

        specOffset += sizeof(EffectsSpec);
    }

    i = nlDLRingCountElements(userSpecs.m_Head);
    if (i > 0)
    {
        UserEffectSpec** pUserSpecs = (UserEffectSpec**)nlMalloc(i * 4, 8, false);
        DLListEntry<UserEffectSpec*>* pNode = nlDLRingGetStart(userSpecs.m_Head);
        DLListEntry<UserEffectSpec*>* pHead = userSpecs.m_Head;
        UserEffectSpec** pWalk = pUserSpecs;

        while (pNode != nullptr)
        {
            *pWalk = pNode->m_data;
            pWalk++;

            if (nlDLRingIsEnd(pHead, pNode) || pNode == nullptr)
            {
                pNode = nullptr;
            }
            else
            {
                pNode = pNode->m_next;
            }
        }

        if (i > 0)
        {
            pGroup->m_userSpecs = i;
            pGroup->m_userSpecsPtr = pUserSpecs;
        }
    }

    nlWalkDLRing(userSpecs.m_Head, (UserSpecContainer*)&userSpecs, &UserSpecContainer::DeleteEntry);
    userSpecs.m_Head = nullptr;
    return pGroup;
}

/**
 * Offset/Address/Size: 0x30C | 0x801F2D54 | size: 0x2C
 */
bool fxLoadGroupBundle(const char* filename)
{
    unsigned long fileSize;
    void* data = fxLoadEntireFileHigh(filename, &fileSize);
    return fxLoadGroupBundle(data, fileSize);
}

/**
 * Offset/Address/Size: 0x178 | 0x801F2BC0 | size: 0x194
 */
bool fxLoadGroupBundle(void* data, unsigned long size)
{

    if (data == nullptr)
    {
        return false;
    }

    pGroupMap = new (nlMalloc(0x14, 8, false)) nlAVLTree<unsigned long, EffectsGroup*, DefaultKeyCompare<unsigned long> >();
    pTerrainSpecMap = new (nlMalloc(0x14, 8, false)) nlAVLTree<unsigned long, EffectsTerrainSpec*, DefaultKeyCompare<unsigned long> >();

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
            unsigned long hashID;
            EffectsGroup* group;
            nlAVLTree<unsigned long, EffectsGroup*, DefaultKeyCompare<unsigned long> >* map;
            AVLTreeNode* existingNode;

            group = parse_group(&parser);
            hashID = group->m_hashID;

            map = pGroupMap;

            map->AddAVLNode((AVLTreeNode**)&map->m_Root, &hashID, &group, &existingNode, map->m_NumElements);

            if (existingNode == nullptr)
            {
                map->m_NumElements++;
            }
        }
        else
        {
            EmissionManager::AddError("EffectsGroup: unrecognized token '%s'\n", token);
        }
    }

    nlFree(data);
    return true;
}

/**
 * Offset/Address/Size: 0xA4 | 0x801F2AEC | size: 0x74
 */
bool fxUnloadGroups()
{
    if ((pGroupMap == NULL) && (pTerrainSpecMap == NULL))
    {
        return true;
    }

    pGroupMap->DeleteValues();
    delete pGroupMap;
    pGroupMap = nullptr;

    pTerrainSpecMap->DeleteValues();
    delete pTerrainSpecMap;
    pTerrainSpecMap = nullptr;

    return true;
}

/**
 * Helper struct for inlining FindGet with bool return to match target assembly.
 * The target uses a bool found flag pattern (li r0,1 / li r0,0 / clrlwi.)
 * which the native AVLTreeBase::FindGet (returning ValueType*) does not produce.
 */
struct GroupMapFindHelper
{
    char pad[0x8];
    AVLTreeEntry<unsigned long, EffectsGroup*>* m_Root;

    inline bool FindGet(unsigned long key, EffectsGroup*** foundValue) const
    {
        AVLTreeEntry<unsigned long, EffectsGroup*>* node = m_Root;
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
                if (foundValue != NULL)
                    *foundValue = &node->value;
                return true;
            }
            else
            {
                if (cmpResult < 0)
                    node = (AVLTreeEntry<unsigned long, EffectsGroup*>*)node->node.left;
                else
                    node = (AVLTreeEntry<unsigned long, EffectsGroup*>*)node->node.right;
            }
        }
        return false;
    }
};

/**
 * Offset/Address/Size: 0x0 | 0x801F2A48 | size: 0xA4
 */
EffectsGroup* fxGetGroup(const char* name)
{
    EffectsGroup** foundValue;
    bool found = ((GroupMapFindHelper*)pGroupMap)->FindGet(nlStringHash(name), &foundValue);
    if (found)
        return *foundValue;
    return nullptr;
}

// At the bottom of EffectsGroup.cpp -- REMOVE once real callers exist.
void EffectsGroup_stub()
{
    NewAdapter<AVLTreeEntry<unsigned long, EffectsGroup*> > adapter;
    adapter.Delete(0);
}
