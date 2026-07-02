#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EmissionController.h"
#include "Game/Effects/EmissionManager.h"
#include "NL/nlAVLTree.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlDLRing.h"
#include "NL/nlMain.h"
#include "NL/nlString.h"

extern "C"
{
    void __vt__18AVLTreeUntemplated(void);
    void vtAVLTreeBaseEffectsGroup(void);
    void vtNlAVLTreeEffectsGroup(void);
    void vtAVLTreeBaseTerrainSpec(void);
    void vtNlAVLTreeTerrainSpec(void);
}

#pragma alias vtAVLTreeBaseEffectsGroup "__vt__104AVLTreeBase<Ul,P12EffectsGroup,46NewAdapter<32AVLTreeEntry<Ul,P12EffectsGroup>>,21DefaultKeyCompare<Ul>>"
#pragma alias vtNlAVLTreeEffectsGroup "__vt__53nlAVLTree<Ul,P12EffectsGroup,21DefaultKeyCompare<Ul>>"
#pragma alias vtAVLTreeBaseTerrainSpec "__vt__116AVLTreeBase<Ul,P18EffectsTerrainSpec,52NewAdapter<38AVLTreeEntry<Ul,P18EffectsTerrainSpec>>,21DefaultKeyCompare<Ul>>"
#pragma alias vtNlAVLTreeTerrainSpec "__vt__59nlAVLTree<Ul,P18EffectsTerrainSpec,21DefaultKeyCompare<Ul>>"

static nlAVLTree<unsigned long, EffectsGroup*, DefaultKeyCompare<unsigned long> >* pGroupMap = nullptr;
static nlAVLTree<unsigned long, EffectsTerrainSpec*, DefaultKeyCompare<unsigned long> >* pTerrainSpecMap = nullptr;
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
    for (u32 i = 0; i < m_uNumTerrains; i++)
    {
        if (m_pTerrainIDs[i] == terrainID)
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
    char* nextToken = nullptr;
    EffectsSpecShadow init;
    EffectsSpecShadow* initAlias = &init;
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
    spec.m_vLocalOffset = init.m_vLocalOffset;
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

        if (nlStrCmp<char>(token, "linger_start") == 0)
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
                char c;
                while ((c = *walk) != '\0')
                {
                    if (c == '_')
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
            for (s32 i = 0; i < 3; i++)
            {
                (&spec.m_vLocalOffset.f.x)[i] = atof(parser->NextTokenOnLine(true));
            }
            continue;
        }

        EmissionManager::AddError("parse_spec has an unrecognized token '%s'\n", token);
    }

    return true;
}

/**
 * Offset/Address/Size: 0x80C | 0x801F3254 | size: 0x224
 * TODO: 99.71% match - AVL lookup keeps hash key and compare result in swapped registers.
 */
EffectsTerrainSpec* parse_terrain_spec(SimpleParser* parser)
{
    unsigned long terrainIDs[256];
    unsigned long numTerrains = 0;
    char* token;

    while (true)
    {
        token = parser->NextTokenOnLine(true);
        if (token == nullptr)
        {
            break;
        }

        terrainIDs[numTerrains++] = nlStringLowerHash(token);
    }

    void* specMem = nlMalloc(sizeof(EffectsTerrainSpec), 8, false);
    EffectsTerrainSpec* pSpec = (EffectsTerrainSpec*)specMem;
    if (specMem != nullptr)
    {
        ((EffectsTerrainSpec*)specMem)->m_pTerrainIDs = nullptr;
        ((EffectsTerrainSpec*)specMem)->m_uNumTerrains = 0;
    }

    pSpec->m_uNumTerrains = numTerrains;
    pSpec->m_pTerrainIDs = (unsigned long*)nlMalloc(numTerrains * sizeof(unsigned long), 8, false);
    memcpy(pSpec->m_pTerrainIDs, terrainIDs, numTerrains << 2);

    RunningChecksum checksum;
    checksum.ChecksumInt(pSpec->m_uNumTerrains);
    checksum.ChecksumData(pSpec->m_pTerrainIDs, pSpec->m_uNumTerrains * 4);

    unsigned long hashID = ~checksum.m_nChecksum;
    EffectsTerrainSpec** foundValue;
    EffectsTerrainSpec* existingSpec = pTerrainSpecMap->FindGet(hashID, &foundValue) ? *foundValue : nullptr;

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

        ::operator delete(pSpec);
    }

    return existingSpec;
}

/**
 * Offset/Address/Size: 0x338 | 0x801F2D80 | size: 0x4D4
 * TODO: 99.43% match - pGroup register r29 vs target r30 in final assignments.
 */
static EffectsGroup* parse_group(SimpleParser* parser)
{
    unsigned long hashID;
    EffectsGroup* pGroup;
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

        if (nlStrCmp<char>(token, "terrain") == 0)
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
                    ((DLListEntry<UserEffectSpec*>*)mem)->m_next = nullptr;
                    ((DLListEntry<UserEffectSpec*>*)mem)->m_prev = nullptr;
                    ((DLListEntry<UserEffectSpec*>*)mem)->entry = pUserSpec;
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
    pGroup = (EffectsGroup*)groupMem;
    if (groupMem != nullptr)
    {
        ((EffectsGroup*)groupMem)->m_hashID = 0;
        ((EffectsGroup*)groupMem)->m_specs = nullptr;
        ((EffectsGroup*)groupMem)->m_numSpecs = 0;
        ((EffectsGroup*)groupMem)->m_userSpecsPtr = nullptr;
        ((EffectsGroup*)groupMem)->m_userSpecs = 0;
        ((EffectsGroup*)groupMem)->m_isLingering = false;
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
        DLListEntry<UserEffectSpec*>* pHead;
        DLListEntry<UserEffectSpec*>* pNode = nlDLRingGetStart(userSpecs.m_Head);
        pHead = userSpecs.m_Head;
        UserEffectSpec** pWalk = pUserSpecs;

        while (pNode != nullptr)
        {
            *pWalk = pNode->entry;
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
    void* raw;

    if (data == nullptr)
    {
        return false;
    }

    raw = nlMalloc(0x14, 8, false);
    if (raw != nullptr)
    {
        u32* map = (u32*)raw;
        map[0] = (u32)__vt__18AVLTreeUntemplated;
        map[0] = (u32)vtAVLTreeBaseEffectsGroup;
        map[4] = 0;
        map[2] = 0;
        map[3] = 0;
        map[0] = (u32)vtNlAVLTreeEffectsGroup;
    }
    pGroupMap = (nlAVLTree<unsigned long, EffectsGroup*, DefaultKeyCompare<unsigned long> >*)raw;

    raw = nlMalloc(0x14, 8, false);
    if (raw != nullptr)
    {
        u32* map = (u32*)raw;
        map[0] = (u32)__vt__18AVLTreeUntemplated;
        map[0] = (u32)vtAVLTreeBaseTerrainSpec;
        map[4] = 0;
        map[2] = 0;
        map[3] = 0;
        map[0] = (u32)vtNlAVLTreeTerrainSpec;
    }
    pTerrainSpecMap = (nlAVLTree<unsigned long, EffectsTerrainSpec*, DefaultKeyCompare<unsigned long> >*)raw;

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

EffectsGroup* fxGetGroup(const char* groupName)
{
    unsigned long hashID = nlStringHash(groupName);
    EffectsGroup** group;
    bool found = ((GroupMapFindHelper*)pGroupMap)->FindGet(hashID, &group);
    return found ? *group : nullptr;
}

// At the bottom of EffectsGroup.cpp -- REMOVE once real callers exist.
void EffectsGroup_stub()
{
    NewAdapter<AVLTreeEntry<unsigned long, EffectsGroup*> > adapter;
    adapter.Delete(0);
}

#pragma dont_inline on
void AVLTreeBase<unsigned long, EffectsGroup*, NewAdapter<AVLTreeEntry<unsigned long, EffectsGroup*> >, DefaultKeyCompare<unsigned long> >::DeleteValues()
{
    DestroyTree(&AVLTreeBase::DeleteValue);
    m_NumElements = 0;
}
void AVLTreeBase<unsigned long, EffectsTerrainSpec*, NewAdapter<AVLTreeEntry<unsigned long, EffectsTerrainSpec*> >, DefaultKeyCompare<unsigned long> >::DeleteValues()
{
    DestroyTree(&AVLTreeBase::DeleteValue);
    m_NumElements = 0;
}
#pragma dont_inline reset
