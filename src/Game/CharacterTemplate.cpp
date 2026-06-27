#define NL_LEXICALCAST_DEFINE_BOOL
#include "Game/CharacterTemplate.h"
#include "Game/SHierarchy.h"
#include "Game/SAnim/AnimRetargeter.h"
#include "Game/Player.h"
#include "Game/AI/Fielder.h"
#include "Game/CharacterTweaks.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/Goalie.h"
#include "Game/AI/ScriptAction.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/AnimInventory.h"
#include "Game/Physics/CharacterPhysicsElement.h"
#include "Game/Triggers/AnimTrigger.h"
#include "Game/Triggers/SebringAnimScript.h"
#include "NL/nlFile.h"
#include "NL/nlFileGC.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/nlMemory.h"
#include "NL/gl/gl.h"
#include "NL/gl/glRenderList.h"
#include "NL/gl/glTexture.h"
#include "NL/glx/glxTexture.h"

extern SoundPropAccessor* gpBIRDOSoundPropAccessor;
extern SoundPropAccessor* gpDAISYSoundPropAccessor;
extern SoundPropAccessor* gpDKSoundPropAccessor;
extern SoundPropAccessor* gpHAMBROSSoundPropAccessor;
extern SoundPropAccessor* gpKOOPASoundPropAccessor;
extern SoundPropAccessor* gpLUIGISoundPropAccessor;
extern SoundPropAccessor* gpMARIOSoundPropAccessor;
extern SoundPropAccessor* gpPEACHSoundPropAccessor;
extern SoundPropAccessor* gpTOADSoundPropAccessor;
extern SoundPropAccessor* gpWALUIGISoundPropAccessor;
extern SoundPropAccessor* gpWARIOSoundPropAccessor;
extern SoundPropAccessor* gpYOSHISoundPropAccessor;
extern SoundPropAccessor* gpSUPERSoundPropAccessor;
extern SoundPropAccessor* gpCRITTERSoundPropAccessor;

extern SebringAnimTagScriptInterpreter* g_pAnimScriptInterp;

extern AnimProperties GLOBALAnimProperties[];
extern AnimProperties GOALIEAnimProperties[];
void AnimTriggerCallback_MARIO(unsigned int);

cCharacter* g_pCharacters[10];
static tCharacterTemplateInfo g_aCharacterTemplateInfo[13] = {
    { "birdo", FIELDER, "characters/birdo/birdo.glg", "characters/birdo/birdo_blend.glg", "characters/birdo/birdo.glt", "art/animation/birdo.trg", AnimTriggerCallback_MARIO, "art/animation/birdo.shier", "birdo", GLOBALAnimProperties, 119, "art/animation/birdo.sanim", "mario", gpBIRDOSoundPropAccessor, "art/animation/birdo.cph", "birdo.ini", "art/characters/birdo/animretarget/birdo.bin" },
    { "daisy", FIELDER, "characters/daisy/daisy.glg", "characters/daisy/daisy_blend.glg", "characters/daisy/daisy.glt", "art/animation/daisy.trg", AnimTriggerCallback_MARIO, "art/animation/daisy.shier", "daisy", GLOBALAnimProperties, 119, "art/animation/daisy.sanim", "daisy", gpDAISYSoundPropAccessor, "art/animation/daisy.cph", "daisy.ini", "art/characters/daisy/animretarget/daisy.bin" },
    { "donkeykong", FIELDER, "characters/donkeykong/donkeykong.glg", "characters/donkeykong/donkeykong_blend.glg", "characters/donkeykong/donkeykong.glt", "art/animation/donkeykong.trg", AnimTriggerCallback_MARIO, "art/animation/donkeykong.shier", "donkeykong", GLOBALAnimProperties, 119, "art/animation/donkeykong.sanim", "donkeykong", gpDKSoundPropAccessor, "art/animation/DonkeyKong.cph", "dk.ini", "art/characters/donkeykong/animretarget/donkeykong.bin" },
    { "hammerbro", FIELDER, "characters/hammerbro/hammerbro.glg", "characters/hammerbro/hammerbro_blend.glg", "characters/hammerbro/hammerbro.glt", "art/animation/hammerbro.trg", AnimTriggerCallback_MARIO, "art/animation/hammerbro.shier", "hammerbro", GLOBALAnimProperties, 119, "art/animation/hammerbro.sanim", "hammerbro", gpHAMBROSSoundPropAccessor, "art/animation/hammerbro.cph", "hammerbros.ini", "art/characters/hammerbro/animretarget/hammerbro.bin" },
    { "koopa", FIELDER, "characters/koopa/koopa.glg", "characters/koopa/koopa_blend.glg", "characters/koopa/koopa.glt", "art/animation/koopa.trg", AnimTriggerCallback_MARIO, "art/animation/koopa.shier", "koopa", GLOBALAnimProperties, 119, "art/animation/koopa.sanim", "mario", gpKOOPASoundPropAccessor, "art/animation/koopa.cph", "koopa.ini", "art/characters/koopa/animretarget/koopa.bin" },
    { "luigi", FIELDER, "characters/luigi/luigi.glg", "characters/luigi/luigi_blend.glg", "characters/luigi/luigi.glt", "art/animation/luigi.trg", NULL, "art/animation/luigi.shier", "luigi", GLOBALAnimProperties, 119, "art/animation/luigi.sanim", "luigi", gpLUIGISoundPropAccessor, "art/animation/mario.cph", "luigi.ini", "art/characters/luigi/animretarget/luigi.bin" },
    { "mario", FIELDER, "characters/mario/mario.glg", "characters/mario/mario_blend.glg", "characters/mario/mario.glt", "art/animation/mario.trg", AnimTriggerCallback_MARIO, "art/animation/mario.shier", "mario", GLOBALAnimProperties, 119, "art/animation/mario.sanim", "mario", gpMARIOSoundPropAccessor, "art/animation/mario.cph", "mario.ini", "art/characters/mario/animretarget/mario.bin" },
    { "peach", FIELDER, "characters/peach/peach.glg", "characters/peach/peach_blend.glg", "characters/peach/peach.glt", "art/animation/peach.trg", AnimTriggerCallback_MARIO, "art/animation/peach.shier", "peach", GLOBALAnimProperties, 119, "art/animation/peach.sanim", "peach", gpPEACHSoundPropAccessor, "art/animation/peach.cph", "peach.ini", "art/characters/peach/animretarget/peach.bin" },
    { "toad", FIELDER, "characters/toad/toad.glg", "characters/toad/toad_blend.glg", "characters/toad/toad.glt", "art/animation/toad.trg", AnimTriggerCallback_MARIO, "art/animation/toad.shier", "toad", GLOBALAnimProperties, 119, "art/animation/toad.sanim", "toad", gpTOADSoundPropAccessor, "art/animation/toad.cph", "toad.ini", "art/characters/toad/animretarget/toad.bin" },
    { "waluigi", FIELDER, "characters/waluigi/waluigi.glg", "characters/waluigi/waluigi_blend.glg", "characters/waluigi/waluigi.glt", "art/animation/waluigi.trg", NULL, "art/animation/waluigi.shier", "waluigi", GLOBALAnimProperties, 119, "art/animation/waluigi.sanim", "waluigi", gpWALUIGISoundPropAccessor, "art/animation/waluigi.cph", "waluigi.ini", "art/characters/waluigi/animretarget/waluigi.bin" },
    { "wario", FIELDER, "characters/wario/wario.glg", "characters/wario/wario_blend.glg", "characters/wario/wario.glt", "art/animation/wario.trg", NULL, "art/animation/wario.shier", "wario", GLOBALAnimProperties, 119, "art/animation/wario.sanim", "wario", gpWARIOSoundPropAccessor, "art/animation/wario.cph", "wario.ini", "art/characters/wario/animretarget/wario.bin" },
    { "yoshi", FIELDER, "characters/yoshi/yoshi.glg", "characters/yoshi/yoshi_blend.glg", "characters/yoshi/yoshi.glt", "art/animation/yoshi.trg", AnimTriggerCallback_MARIO, "art/animation/yoshi.shier", "yoshi", GLOBALAnimProperties, 119, "art/animation/yoshi.sanim", "yoshi", gpYOSHISoundPropAccessor, "art/animation/yoshi.cph", "yoshi.ini", "art/characters/yoshi/animretarget/yoshi.bin" },
    { "superteam", FIELDER, "characters/superteam/superteam.glg", "characters/superteam/superteam_blend.glg", "characters/superteam/superteam.glt", "art/animation/superteam.trg", NULL, "art/animation/superteam.shier", "superteam", GLOBALAnimProperties, 119, "art/animation/superteam.sanim", "superteam", gpSUPERSoundPropAccessor, "art/animation/superteam.cph", "superteam.ini", "art/characters/superteam/animretarget/superteam.bin" },
};
static tCharacterTemplate* g_aCharacterTemplates[13];
static tCharacterTemplateInfo g_GoalieTemplateInfo = {
    "mariogoalie", GOALIE, "characters/mariogoalie/mariogoalie.glg", "characters/mariogoalie/mariogoalie_blend.glg", "characters/mariogoalie/mariogoalie.glt", "art/animation/mariogoalie.trg", NULL, "art/animation/mariogoalie.shier", "mariogoalie", GOALIEAnimProperties, 149, "art/animation/mariogoalie.sanim", "mario", gpCRITTERSoundPropAccessor, "art/animation/mariogoalie.cph", "goalie.ini", NULL
};
static tCharacterTemplate* g_GoalieTemplate;

static s32 skiptexture = 0xFFFFFFFF;

tGoalieTemplateInfo g_GoalieTextureInfo[9] = {
    { "daisygoalie", "characters/daisygoalie/daisygoalie.glt", 0 },
    { "donkeykonggoalie", "characters/donkeykonggoalie/donkeykonggoalie.glt", 0 },
    { "luigigoalie", "characters/luigigoalie/luigigoalie.glt", 0 },
    { "mariogoalie", "characters/mariogoalie/mariogoalie.glt", 0 },
    { "peachgoalie", "characters/peachgoalie/peachgoalie.glt", 0 },
    { "waluigigoalie", "characters/waluigigoalie/waluigigoalie.glt", 0 },
    { "wariogoalie", "characters/wariogoalie/wariogoalie.glt", 0 },
    { "yoshigoalie", "characters/yoshigoalie/yoshigoalie.glt", 0 },
    { "superteamgoalie", "characters/superteamgoalie/superteamgoalie.glt", 0 },
};

static inline u32 GetHashFromTextureFile(const char* szTextureFileName)
{
    char name[200];
    char* pDest = name;
    const char* pSrc = NULL;
    int count = 0;
    const char* szPath = szTextureFileName;

    for (int j = 0; j < 10; j++)
    {
        char c;
        c = szPath[0];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[1];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[2];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[3];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[4];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[5];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[6];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[7];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[8];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        c = szPath[9];
        if (c == '\\' || c == '/')
        {
            count++;
            pSrc = szTextureFileName + count;
            goto foundSlash;
        }
        count++;
        szPath += 10;
    }
foundSlash:
    for (int k = 0; k < 100; k++)
    {
        if (*pSrc == '\0')
            goto copyDone;
        if (*pSrc == '.')
            goto copyDone;
        *pDest = *pSrc;
        pSrc++;
        pDest++;
        continue;
    copyDone:
        *pDest = '\0';
        return nlStringLowerHash(name);
    }
    return 0;
}

static inline cSHierarchy* FindHierarchy(ListEntry<cSHierarchy*>* hEntry, u32 hash)
{
    cSHierarchy* pHierarchy;
    while (hEntry != NULL)
    {
        pHierarchy = hEntry->data;
        if (hash == hEntry->data->m_uHashID)
        {
            return pHierarchy;
        }
        hEntry = hEntry->next;
    }
    return NULL;
}

template <typename T>
static inline nlChunk* AddLoadedInventoryMemory(cInventory<T>* inv, nlChunk* data)
{
    ListEntry<char*>* entry = (ListEntry<char*>*)nlMalloc(8, 8, false);
    if (entry != NULL)
    {
        entry->next = NULL;
        entry->data = (char*)data;
    }
    nlListAddStart<ListEntry<char*> >(
        (ListEntry<char*>**)&inv->m_lMemList.m_Head,
        entry,
        (ListEntry<char*>**)&inv->m_lMemList.m_Tail);
    return data;
}

extern SebringAnimTagScriptInterpreter* g_pAnimScriptInterp;

static cAnimInventory* FindDuplicateAnimInventory(int nCurIndex, unsigned long uHashID);
static char* GetCharacterTriggerFileName(eCharacterClass cc);

/**
 * Offset/Address/Size: 0x2128 | 0x80014410 | size: 0x34
 */
char* GetCharacterName(eCharacterClass cc)
{
    if (cc < 13)
    {
        return (char*)g_aCharacterTemplateInfo[cc].szCharName;
    }
    return (char*)g_GoalieTextureInfo[cc - 13].szCharName;
}

/**
 * Offset/Address/Size: 0x20EC | 0x800143D4 | size: 0x3C
 */
bool IsCaptain(eCharacterClass cc)
{
    if (((cc - 1) <= 1U) || ((cc - 5) <= 2U) || ((cc - 9) <= 2U) || (cc == 0xC))
    {
        return true;
    }
    return false;
}

/**
 * Offset/Address/Size: 0x1CFC | 0x80013FE4 | size: 0x3F0
 * TODO: 99.23% match - hierarchy file load keeps an extra return-value move; retarget load loop keeps inventory/data register differences
 */
void CharacterLoadingGuts(tCharacterTemplate* pCharacterTemplate, const tCharacterTemplateInfo& charTemplateInfo, eCharacterClass cc, bool bForViewer)
{
    glModel* pRigidCharacterModel = glLoadModel(charTemplateInfo.szModelFilename, NULL);
    glModel* pBlendCharacterModel = glLoadModel(charTemplateInfo.szBlendedModelFilename, NULL);

    pCharacterTemplate->nCharacterModelID[0] = pRigidCharacterModel->id;
    pCharacterTemplate->nCharacterModelID[1] = pBlendCharacterModel->id;

    cInventory<cSHierarchy>* pHierInv = new (nlMalloc(sizeof(cInventory<cSHierarchy>), 8, false)) cInventory<cSHierarchy>();
    cInventory<AnimRetargetList>* pRetInv;
    pCharacterTemplate->pHierarchyInventory = pHierInv;

    u32 hierFileSize;
    cInventory<cSHierarchy>* pHierInv2 = pCharacterTemplate->pHierarchyInventory;
    nlChunk* hierData = (nlChunk*)nlLoadEntireFile(charTemplateInfo.szHierarchyFilename, &hierFileSize, 0x20, AllocateStart);

    ListEntry<char*>* memEntry = (ListEntry<char*>*)nlMalloc(8, 8, false);
    if (memEntry != NULL)
    {
        memEntry->next = NULL;
        memEntry->data = (char*)hierData;
    }
    nlListAddStart<ListEntry<char*> >(
        (ListEntry<char*>**)&pHierInv2->m_lMemList.m_Head,
        memEntry,
        (ListEntry<char*>**)&pHierInv2->m_lMemList.m_Tail);

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
                itemEntry->data = hier;
            }
            nlListAddStart<ListEntry<cSHierarchy*> >(
                &pHierInv2->m_lItemList.m_Head,
                itemEntry,
                &pHierInv2->m_lItemList.m_Tail);
            pHierInv2->m_nItemCount++;
        }
        else
        {
            nlPrintf("Warning: inventory encountered an unknown chunk type\n");
        }
        hierData = (nlChunk*)((char*)hierData + hierData->m_Size + 8);
    }

    if (!bForViewer)
    {
        CharacterPhysicsData* pPhys = new (nlMalloc(sizeof(CharacterPhysicsData), 8, false)) CharacterPhysicsData();
        pCharacterTemplate->pPhysicsData = pPhys;
        LoadCharacterPhysicsElements(charTemplateInfo.szPhysicsFilename, (CharacterPhysicsData*)pCharacterTemplate->pPhysicsData);
    }
    else
    {
        pCharacterTemplate->pPhysicsData = NULL;
    }

    pCharacterTemplate->uAnimInventoryHashID = nlStringLowerHash(charTemplateInfo.szAnimFilename);

    cAnimInventory* found = FindDuplicateAnimInventory(cc, pCharacterTemplate->uAnimInventoryHashID);

    if (found != NULL)
    {
        pCharacterTemplate->pAnimInventory = found;
        pCharacterTemplate->bAnimInventoryCopy = true;
    }
    else
    {
        cAnimInventory* pAnim = new (nlMalloc(sizeof(cAnimInventory), 8, false))
            cAnimInventory(charTemplateInfo.pAnimProperties, charTemplateInfo.nNumAnimProperties);
        pCharacterTemplate->pAnimInventory = pAnim;
        pCharacterTemplate->pAnimInventory->AddAnimBundle(charTemplateInfo.szAnimFilename);
        pCharacterTemplate->bAnimInventoryCopy = false;

        cAnimInventory* pAI = pCharacterTemplate->pAnimInventory;
        cInventory<cSAnim>* pAnimCont = (cInventory<cSAnim>*)pAI->m_cont;
        SebringAnimTagScriptInterpreter* pInterp = g_pAnimScriptInterp;
        const char* triggerFilename;
        if (cc < NUM_FIELDER_CLASSES)
        {
            triggerFilename = g_aCharacterTemplateInfo[cc].szTriggerFilename;
        }
        else
        {
            triggerFilename = g_GoalieTemplateInfo.szTriggerFilename;
        }
        pInterp->SetupAnimationTriggers(triggerFilename, pAnimCont);
    }

    if (charTemplateInfo.szAnimRetargetFilename != NULL)
    {
        cInventory<AnimRetargetList>* pRetargetInv = new (nlMalloc(sizeof(cInventory<AnimRetargetList>), 8, false)) cInventory<AnimRetargetList>();
        pCharacterTemplate->pAnimRetargetListInventory = pRetargetInv;

        u32 retargetFileSize;
        pRetInv = pCharacterTemplate->pAnimRetargetListInventory;
        nlChunk* retargetData = AddLoadedInventoryMemory(
            pRetInv,
            (nlChunk*)nlLoadEntireFile(charTemplateInfo.szAnimRetargetFilename, &retargetFileSize, 0x20, AllocateStart));

        nlChunk* retargetEnd = (nlChunk*)((char*)retargetData + retargetFileSize);
        while (retargetData != retargetEnd)
        {
            if ((retargetData->m_ID & 0x80FFFFFF) == 0x80017104)
            {
                AnimRetargetList* retarget = AnimRetargetList::Initialize(retargetData);

                ListEntry<AnimRetargetList*>* retItemEntry = (ListEntry<AnimRetargetList*>*)nlMalloc(8, 8, false);
                if (retItemEntry != NULL)
                {
                    retItemEntry->next = NULL;
                    retItemEntry->data = retarget;
                }
                nlListAddStart<ListEntry<AnimRetargetList*> >(
                    &pRetInv->m_lItemList.m_Head,
                    retItemEntry,
                    &pRetInv->m_lItemList.m_Tail);
                pRetInv->m_nItemCount++;
            }
            else
            {
                nlPrintf("Warning: inventory encountered an unknown chunk type\n");
            }
            retargetData = (nlChunk*)((char*)retargetData + retargetData->m_Size + 8);
        }
    }
    else
    {
        pCharacterTemplate->pAnimRetargetListInventory = NULL;
    }
}

static cAnimInventory* FindDuplicateAnimInventory(int nCurIndex, unsigned long uHashID)
{
    for (int index = 0; index < NUM_FIELDER_CLASSES; index++)
    {
        if (index == nCurIndex)
            continue;
        if (g_aCharacterTemplates[index] == NULL)
            continue;
        if (uHashID != g_aCharacterTemplates[index]->uAnimInventoryHashID)
            continue;
        return g_aCharacterTemplates[index]->pAnimInventory;
    }
    return NULL;
}

static char* GetCharacterTriggerFileName(eCharacterClass cc)
{
    if (cc < NUM_FIELDER_CLASSES)
    {
        return (char*)g_aCharacterTemplateInfo[cc].szTriggerFilename;
    }
    return (char*)g_GoalieTemplateInfo.szTriggerFilename;
}

/**
 * Offset/Address/Size: 0x1ABC | 0x80013DA4 | size: 0x240
 */
cPlayer* CreateCharacter(int nPlayerID, int nTeamID, eCharacterClass cc, bool bForViewer)
{
    if (cc >= NUM_FIELDER_CLASSES)
    {
        return CreateGoalie(cc, bForViewer);
    }

    if (g_aCharacterTemplates[cc] == NULL)
    {
        glLoadTextureBundle(g_aCharacterTemplateInfo[cc].szTextureFilename);
        g_aCharacterTemplates[cc] = (tCharacterTemplate*)nlMalloc(sizeof(tCharacterTemplate), 8, false);
        CharacterLoadingGuts(g_aCharacterTemplates[cc], g_aCharacterTemplateInfo[cc], cc, bForViewer);
    }

    cInventory<cSHierarchy>* pHierInv = g_aCharacterTemplates[cc]->pHierarchyInventory;
    u32 hash = nlStringHash(g_aCharacterTemplateInfo[cc].szHierarchy);

    AnimRetargetList* pAnimRetarget;
    FielderTweaks* pTweaks;
    cSHierarchy* pHierarchy = FindHierarchy(pHierInv->m_lItemList.m_Head, hash);

    pAnimRetarget = NULL;
    if (g_aCharacterTemplates[cc]->pAnimRetargetListInventory != NULL)
    {
        int idx = 0;
        ListEntry<AnimRetargetList*>* retEntry = g_aCharacterTemplates[cc]->pAnimRetargetListInventory->m_lItemList.m_Head;
        u32 retData;
        while (retEntry != NULL)
        {
            if (idx == 0)
            {
                retData = (u32)retEntry->data;
                goto retFound;
            }
            retEntry = retEntry->next;
            idx++;
        }
        retData = 0;

    retFound:
        pAnimRetarget = (AnimRetargetList*)retData;
    }

    pTweaks = new (nlMalloc(0x124, 8, false)) FielderTweaks(g_aCharacterTemplateInfo[cc].szTweaksFilename);

    cPlayer* pPlayer;
    if (!bForViewer)
    {
        cFielder* pFielder = new (nlMalloc(0x3EC, 8, false)) cFielder(
            nPlayerID, nTeamID, cc, (const int*)g_aCharacterTemplates[cc], pHierarchy, g_aCharacterTemplates[cc]->pAnimInventory, g_aCharacterTemplates[cc]->pPhysicsData, pTweaks, pAnimRetarget);
        pPlayer = pFielder;
    }
    else
    {
        cPlayer* p = new (nlMalloc(0x1D4, 8, false)) cPlayer(
            nPlayerID, cc, (const int*)g_aCharacterTemplates[cc], pHierarchy, g_aCharacterTemplates[cc]->pAnimInventory, g_aCharacterTemplates[cc]->pPhysicsData, (PlayerTweaks*)pTweaks, pAnimRetarget, (eClassTypes)1);
        pPlayer = p;
    }

    pPlayer->m_szEffectsName = g_aCharacterTemplateInfo[cc].szEffectsName;
    if (!AudioLoader::gbDisableAudio)
    {
        pPlayer->SetSFX(g_aCharacterTemplateInfo[cc].pSFXPropAccessor);
    }

    return pPlayer;
}

/**
 * Offset/Address/Size: 0x1AA0 | 0x80013D88 | size: 0x1C
 */
static s32 SidekickTexture_cb(unsigned long arg0)
{
    s32 var_r4 = -1;
    if (arg0 != skiptexture)
    {
        var_r4 = arg0;
    }
    return var_r4;
}

/**
 * Offset/Address/Size: 0x14A4 | 0x8001378C | size: 0x5FC
 */
cPlayer* CreateSidekick(int nPlayerID, int nTeamID, eCharacterClass cc, eCharacterClass captainCC, bool bForViewer)
{
    char szTexPath[64];
    char szArtPath[64];
    char szBundlePath[64];
    char szPlayerPath[64];

    glxTextureLoadCallback_t oldCallback = glx_SetLoadCallback((glxTextureLoadCallback_t)SidekickTexture_cb);

    if (cc == HAMMERBROS)
    {
        nlSNPrintf(szTexPath, 64, "hammerbro/hammer_mario");
    }
    else
    {
        nlSNPrintf(szTexPath, 64, "%s/%s_mario", GetCharacterName(cc), GetCharacterName(cc));
    }

    skiptexture = glGetTexture(szTexPath);

    cPlayer* pPlayer = CreateCharacter(nPlayerID, nTeamID, cc, bForViewer);

    glx_SetLoadCallback(oldCallback);

    bool bLoaded = false;

    if (cc == HAMMERBROS)
    {
        nlSNPrintf(szBundlePath, 64, "characters/%s/hammer_%s.glt", GetCharacterName(cc), GetCharacterName(captainCC));
        nlSNPrintf(szArtPath, 64, "art/characters/%s/hammer_%s.glt", GetCharacterName(cc), GetCharacterName(captainCC));
    }
    else
    {
        nlSNPrintf(szBundlePath, 64, "characters/%s/%s_%s.glt", GetCharacterName(cc), GetCharacterName(cc), GetCharacterName(captainCC));
        nlSNPrintf(szArtPath, 64, "art/characters/%s/%s_%s.glt", GetCharacterName(cc), GetCharacterName(cc), GetCharacterName(captainCC));
    }

    if (cc == HAMMERBROS)
    {
        nlSNPrintf(szPlayerPath, 64, "hammer_%s/hammer_%s", GetCharacterName(captainCC), GetCharacterName(captainCC));
    }
    else
    {
        nlSNPrintf(szPlayerPath, 64, "%s_%s/%s_%s", GetCharacterName(cc), GetCharacterName(captainCC), GetCharacterName(cc), GetCharacterName(captainCC));
    }

    if (glTextureLoad(glGetTexture(szPlayerPath)))
    {
        bLoaded = true;
    }
    else
    {
        nlFile* fp = nlOpen(szArtPath);
        if (fp != NULL)
        {
            nlClose(fp);
            bLoaded = glLoadTextureBundle(szBundlePath);
        }
    }

    if (bLoaded)
    {
        if (cc == HAMMERBROS)
        {
            nlSNPrintf(szBundlePath, 64, "%s/hammer_mario", GetCharacterName(cc));
        }
        else
        {
            nlSNPrintf(szBundlePath, 64, "%s/%s_mario", GetCharacterName(cc), GetCharacterName(cc));
        }
        pPlayer->m_uNormalTextureID = glGetTexture(szBundlePath);
        pPlayer->m_uSwapTextureID = glGetTexture(szPlayerPath);
    }
    else
    {
        pPlayer->m_uNormalTextureID = (u32)-1;
        pPlayer->m_uSwapTextureID = (u32)-1;
    }

    return pPlayer;
}

/**
 * Offset/Address/Size: 0xE70 | 0x80013158 | size: 0x634
 * TODO: 93.80% match - hierarchy inventory load uses r26 vs r29; the two
 * inlined GetHashFromTextureFile scan loops fold the running count into the
 * found-offset immediate instead of keeping it in a register (extra addi per
 * char in target), and the copy loop reloads *pSrc instead of reusing it.
 */
cPlayer* CreateGoalie(eCharacterClass gcc, bool bForViewer)
{
    s32 goalieIdx = gcc - NUM_FIELDER_CLASSES;
    if (!g_GoalieTextureInfo[goalieIdx].bLoaded)
    {
        glLoadTextureBundle(g_GoalieTextureInfo[goalieIdx].szTextureFilename);
        g_GoalieTextureInfo[goalieIdx].bLoaded = 1;
    }

    if (g_GoalieTemplate == NULL)
    {
        g_GoalieTemplate = (tCharacterTemplate*)nlMalloc(sizeof(tCharacterTemplate), 8, false);
        CharacterLoadingGuts(g_GoalieTemplate, g_GoalieTemplateInfo, gcc, bForViewer);
    }

    cInventory<cSHierarchy>* pHierInv = g_GoalieTemplate->pHierarchyInventory;
    u32 hash = nlStringHash(g_GoalieTemplateInfo.szHierarchy);

    cSHierarchy* pHierarchy = FindHierarchy(pHierInv->m_lItemList.m_Head, hash);

    AnimRetargetList* pAnimRetarget = NULL;
    if (g_GoalieTemplate->pAnimRetargetListInventory != NULL)
    {
        int idx = 0;
        ListEntry<AnimRetargetList*>* retEntry = g_GoalieTemplate->pAnimRetargetListInventory->m_lItemList.m_Head;
        AnimRetargetList* retResult;
        while (retEntry != NULL)
        {
            if (idx == 0)
            {
                retResult = retEntry->data;
                goto retDone;
            }
            retEntry = retEntry->next;
            idx++;
        }
        retResult = NULL;
    retDone:
        pAnimRetarget = retResult;
    }

    GoalieTweaks* pTweaks = new (nlMalloc(0xF4, 8, false)) GoalieTweaks(g_GoalieTemplateInfo.szTweaksFilename);

    cPlayer* pPlayer;
    if (!bForViewer)
    {
        Goalie* pGoalie = new (nlMalloc(0x310, 8, false)) Goalie(
            gcc, (const int*)g_GoalieTemplate, pHierarchy, g_GoalieTemplate->pAnimInventory, g_GoalieTemplate->pPhysicsData, pTweaks, pAnimRetarget);
        pPlayer = pGoalie;
    }
    else
    {
        cPlayer* p = new (nlMalloc(0x1D4, 8, false)) cPlayer(
            4, gcc, (const int*)g_GoalieTemplate, pHierarchy, g_GoalieTemplate->pAnimInventory, g_GoalieTemplate->pPhysicsData, (PlayerTweaks*)pTweaks, pAnimRetarget, (eClassTypes)3);
        pPlayer = p;
    }

    pPlayer->m_szEffectsName = g_GoalieTemplateInfo.szEffectsName;
    pPlayer->m_uNormalTextureID = GetHashFromTextureFile(g_GoalieTemplateInfo.szTextureFilename);
    pPlayer->m_uSwapTextureID = GetHashFromTextureFile(g_GoalieTextureInfo[goalieIdx].szTextureFilename);

    if (!AudioLoader::gbDisableAudio)
    {
        pPlayer->SetSFX(g_GoalieTemplateInfo.pSFXPropAccessor);
    }

    return pPlayer;
}

/**
 * Offset/Address/Size: 0x954 | 0x80012C3C | size: 0x51C
 * TODO: 97.23% match - register allocation diffs throughout both loops (r21<>r28 for g_pCharacters,
 * r23<>r21 for teami, r27<>r30 for captain values). Inner loop compare loads sidekick/captain into
 * r5/r6 (reused as args) vs target r3/r0 (reloads into r5/r6), causing 2 instruction size difference.
 */
static inline eCharacterClass GetGoalieFromCaptain(eCharacterClass captain)
{
    switch (captain)
    {
    case DAISY:
        return DAISY_GOALIE;
    case DONKEYKONG:
        return DONKEYKONG_GOALIE;
    case LUIGI:
        return LUIGI_GOALIE;
    case MARIO:
        return MARIO_GOALIE;
    case PEACH:
        return PEACH_GOALIE;
    case WALUIGI:
        return WALUIGI_GOALIE;
    case WARIO:
        return WARIO_GOALIE;
    case YOSHI:
        return YOSHI_GOALIE;
    case MYSTERY:
        return SUPERTEAM_GOALIE;
    default:
        return MARIO_GOALIE;
    }
}

/**
 * Offset/Address/Size: 0x3DC | 0x80012C3C | size: 0x51C
 * TODO: 98.65% match - register allocation diffs remain in both character creation loops
 */
void CreateCharacters()
{
    eCharacterClass captain[2];
    eCharacterClass sidekick[2];
    eCharacterClass goalie[2];
    captain[0] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0));
    captain[1] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1));
    sidekick[0] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(0));
    sidekick[1] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(1));

    goalie[0] = GetGoalieFromCaptain(captain[0]);
    goalie[1] = GetGoalieFromCaptain(captain[1]);

    Config& cfg = Config::Global();
    TagValuePair& tvp = cfg.FindTvp("allcaptains");
    bool allcaptains;
    if (tvp.tag == NULL)
    {
        cfg.Set("allcaptains", false);
        allcaptains = false;
    }
    else if (tvp.type == _BOOL)
    {
        allcaptains = LexicalCast<bool, bool>(tvp.value.b);
    }
    else if (tvp.type == _INT)
    {
        allcaptains = LexicalCast<bool, int>(tvp.value.i);
    }
    else if (tvp.type == _FLOAT)
    {
        bool (*castFloat)(const float&) = LexicalCast<bool, float>;
        allcaptains = castFloat(tvp.value.f);
    }
    else if (tvp.type == _STRING)
    {
        allcaptains = LexicalCast<bool, const char*>(tvp.value.s);
    }
    else
    {
        allcaptains = 0;
    }
    if (allcaptains)
    {
        sidekick[0] = captain[0];
        sidekick[1] = captain[1];
    }

    if (captain[0] == MYSTERY)
    {
        sidekick[0] = MYSTERY;
    }
    else if (captain[1] == MYSTERY)
    {
        sidekick[1] = MYSTERY;
    }

    nlVector3 pos[8] = {
        { 1.5f, 1.5f, 0.0f },
        { 1.5f, -1.5f, 0.0f },
        { 1.5f, 0.0f, 0.0f },
        { 1.5f, 2.5f, 0.0f },
        { -1.5f, 1.5f, 0.0f },
        { -1.5f, -1.5f, 0.0f },
        { -1.5f, 0.0f, 0.0f },
        { -1.5f, 2.5f, 0.0f },
    };

    nlVector3 goaliepos[2] = {
        { 18.0f, 0.0f, 0.0f },
        { -18.0f, 0.0f, 0.0f },
    };

    SebringAnimTagScriptInterpreter* pInterp = new (nlMalloc(sizeof(SebringAnimTagScriptInterpreter), 8, false)) SebringAnimTagScriptInterpreter();

    g_pAnimScriptInterp = pInterp;

    eCharacterClass captain1 = captain[1];
    eCharacterClass captain0 = captain[0];

    for (int teami = 0; teami < 2; teami++)
    {
        int plrindex;
        if (captain0 > captain1)
        {
            plrindex = !teami;
        }
        else
        {
            plrindex = teami;
        }

        int idx = plrindex * 4;
        g_pCharacters[idx] = CreateCharacter(0, plrindex, captain[plrindex], false);
        g_pCharacters[idx]->SetPosition(pos[idx]);
        ((Audio::cCharacterSFX*)g_pCharacters[idx]->m_pCharacterSFX)->mGroup = idx;

        g_pTeams[plrindex]->SetPlayer((cPlayer*)g_pCharacters[idx], 0);
        ((cPlayer*)g_pCharacters[idx])->m_pTeam = g_pTeams[plrindex];

        g_pCharacters[plrindex + 8] = CreateGoalie(goalie[plrindex], false);
        g_pCharacters[plrindex + 8]->SetPosition(goaliepos[plrindex]);

        g_pTeams[plrindex]->SetGoalie((Goalie*)g_pCharacters[plrindex + 8]);
        ((cPlayer*)g_pCharacters[plrindex + 8])->m_pTeam = g_pTeams[plrindex];
    }

    for (int teami = 0; teami < 2; teami++)
    {
        int plrindex;
        if (sidekick[0] > sidekick[1])
        {
            plrindex = !teami;
        }
        else
        {
            plrindex = teami;
        }

        int charIdx = plrindex * 4 + 1;
        volatile eCharacterClass* vpSidekick = sidekick;
        volatile eCharacterClass* vpCaptain = captain;
        cCharacter** pChar = &g_pCharacters[charIdx];
        nlVector3* pPos = &pos[charIdx];

        for (int index = 1; index < 4; index++)
        {
            if (vpSidekick[plrindex] == vpCaptain[plrindex])
            {
                *pChar = CreateCharacter(index, plrindex, captain[plrindex], false);
            }
            else
            {
                *pChar = (cCharacter*)CreateSidekick(index, plrindex, sidekick[plrindex], captain[plrindex], false);
            }

            (*pChar)->SetPosition(*pPos);
            ((Audio::cCharacterSFX*)(*pChar)->m_pCharacterSFX)->mGroup = charIdx;

            g_pTeams[plrindex]->SetPlayer((cPlayer*)*pChar, index);
            ((cPlayer*)*pChar)->m_pTeam = g_pTeams[plrindex];

            pChar++;
            pPos++;
            charIdx++;
        }

        g_pTeams[plrindex]->UpdateControllers();
    }
}

/**
 * Offset/Address/Size: 0x294 | 0x8001257C | size: 0x6C0
 * TODO: 99.44% match - character cleanup index register and callback literal-pool/address diffs
 * across inventory cleanup paths.
 */
void DestroyCharacters()
{
    typedef ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*> > > HierListBase;
    typedef ListContainerBase<char*, NewAdapter<ListEntry<char*> > > FileListBase;
    typedef ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*> > > RetargetListBase;
    cInventory<cSHierarchy>* pHierInv;
    cInventory<AnimRetargetList>* pRetInv;

    delete g_pAnimScriptInterp;
    g_pAnimScriptInterp = NULL;

    for (s32 charIndex = 0; charIndex < 10; charIndex++)
    {
        delete g_pCharacters[charIndex];
        g_pCharacters[charIndex] = NULL;
    }

    tCharacterTemplate** ppCharacterTemplate = g_aCharacterTemplates;
    for (int i = 0; i < 13; i++, ppCharacterTemplate++)
    {
        if (*ppCharacterTemplate != NULL)
        {
            pHierInv = (*ppCharacterTemplate)->pHierarchyInventory;
            if (pHierInv != NULL)
            {
                ListEntry<cSHierarchy*>* hierEntry = pHierInv->m_lItemList.m_Head;
                while (hierEntry != NULL)
                {
                    hierEntry = hierEntry->next;
                }

                void (HierListBase::*cbHier)(ListEntry<cSHierarchy*>*) = &HierListBase::DeleteEntry;
                nlWalkList(pHierInv->m_lItemList.m_Head, (HierListBase*)pHierInv, cbHier);

                ListEntry<char*>** pTail = &pHierInv->m_lMemList.m_Tail;
                pHierInv->m_lItemList.m_Head = NULL;
                ListEntry<char*>** pHead = &pHierInv->m_lMemList.m_Head;
                pHierInv->m_lItemList.m_Tail = NULL;

                while (pHierInv->m_lMemList.m_Head != NULL)
                {
                    ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead, pTail);
                    void* mesh;
                    if (&mesh != NULL)
                    {
                        mesh = first->data;
                    }
                    ::operator delete(first);
                    ::operator delete(mesh);
                }

                pHierInv->m_nItemCount = 0;
                pHierInv->m_lMemList.~nlListContainer();
                pHierInv->m_lItemList.~nlListContainer();
                ::operator delete(pHierInv);
            }

            if (!(*ppCharacterTemplate)->bAnimInventoryCopy)
            {
                delete (*ppCharacterTemplate)->pAnimInventory;
            }

            delete (*ppCharacterTemplate)->pPhysicsData;

            pRetInv = (*ppCharacterTemplate)->pAnimRetargetListInventory;
            if (pRetInv != NULL && pRetInv != NULL)
            {
                ListEntry<AnimRetargetList*>* retEntry = pRetInv->m_lItemList.m_Head;
                while (retEntry != NULL)
                {
                    retEntry = retEntry->next;
                }

                void (RetargetListBase::*cbRet)(ListEntry<AnimRetargetList*>*) = &RetargetListBase::DeleteEntry;
                nlWalkList(pRetInv->m_lItemList.m_Head, (RetargetListBase*)pRetInv, cbRet);

                ListEntry<char*>** pTail2 = &pRetInv->m_lMemList.m_Tail;
                pRetInv->m_lItemList.m_Head = NULL;
                ListEntry<char*>** pHead2 = &pRetInv->m_lMemList.m_Head;
                pRetInv->m_lItemList.m_Tail = NULL;

                while (pRetInv->m_lMemList.m_Head != NULL)
                {
                    ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead2, pTail2);
                    void* mesh;
                    if (&mesh != NULL)
                    {
                        mesh = first->data;
                    }
                    ::operator delete(first);
                    ::operator delete(mesh);
                }

                pRetInv->m_nItemCount = 0;
                pRetInv->m_lMemList.~nlListContainer();
                pRetInv->m_lItemList.~nlListContainer();
                ::operator delete(pRetInv);
            }

            ::operator delete(*ppCharacterTemplate);
            *ppCharacterTemplate = NULL;
        }
    }

    if (g_GoalieTemplate != NULL)
    {
        pHierInv = g_GoalieTemplate->pHierarchyInventory;
        if (pHierInv != NULL)
        {
            ListEntry<cSHierarchy*>* hierEntry = pHierInv->m_lItemList.m_Head;
            while (hierEntry != NULL)
            {
                hierEntry = hierEntry->next;
            }

            void (HierListBase::*cbHier)(ListEntry<cSHierarchy*>*) = &HierListBase::DeleteEntry;
            nlWalkList(pHierInv->m_lItemList.m_Head, (HierListBase*)pHierInv, cbHier);

            ListEntry<char*>** pTail = &pHierInv->m_lMemList.m_Tail;
            pHierInv->m_lItemList.m_Head = NULL;
            ListEntry<char*>** pHead = &pHierInv->m_lMemList.m_Head;
            pHierInv->m_lItemList.m_Tail = NULL;

            while (pHierInv->m_lMemList.m_Head != NULL)
            {
                ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead, pTail);
                void* mesh;
                if (&mesh != NULL)
                {
                    mesh = first->data;
                }
                ::operator delete(first);
                ::operator delete(mesh);
            }

            pHierInv->m_nItemCount = 0;
            pHierInv->m_lMemList.~nlListContainer();
            pHierInv->m_lItemList.~nlListContainer();
            ::operator delete(pHierInv);
        }

        if (!g_GoalieTemplate->bAnimInventoryCopy)
        {
            delete g_GoalieTemplate->pAnimInventory;
        }

        delete g_GoalieTemplate->pPhysicsData;

        pRetInv = g_GoalieTemplate->pAnimRetargetListInventory;
        if (pRetInv != NULL && pRetInv != NULL)
        {
            ListEntry<AnimRetargetList*>* retEntry = pRetInv->m_lItemList.m_Head;
            while (retEntry != NULL)
            {
                retEntry = retEntry->next;
            }

            void (RetargetListBase::*cbRet)(ListEntry<AnimRetargetList*>*) = &RetargetListBase::DeleteEntry;
            nlWalkList(pRetInv->m_lItemList.m_Head, (RetargetListBase*)pRetInv, cbRet);

            ListEntry<char*>** pTail2 = &pRetInv->m_lMemList.m_Tail;
            pRetInv->m_lItemList.m_Head = NULL;
            ListEntry<char*>** pHead2 = &pRetInv->m_lMemList.m_Head;
            pRetInv->m_lItemList.m_Tail = NULL;

            while (pRetInv->m_lMemList.m_Head != NULL)
            {
                ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead2, pTail2);
                void* mesh;
                if (&mesh != NULL)
                {
                    mesh = first->data;
                }
                ::operator delete(first);
                ::operator delete(mesh);
            }

            pRetInv->m_nItemCount = 0;
            pRetInv->m_lMemList.~nlListContainer();
            pRetInv->m_lItemList.~nlListContainer();
            ::operator delete(pRetInv);
        }

        ::operator delete(g_GoalieTemplate);
        g_GoalieTemplate = NULL;
    }

    g_GoalieTextureInfo[0].bLoaded = 0;
    g_GoalieTextureInfo[1].bLoaded = 0;
    g_GoalieTextureInfo[2].bLoaded = 0;
    g_GoalieTextureInfo[3].bLoaded = 0;
    g_GoalieTextureInfo[4].bLoaded = 0;
    g_GoalieTextureInfo[5].bLoaded = 0;
    g_GoalieTextureInfo[6].bLoaded = 0;
    g_GoalieTextureInfo[7].bLoaded = 0;
    g_GoalieTextureInfo[8].bLoaded = 0;

    SlotPoolBase::BaseFreeBlocks(&AnimTriggerCallbackInfo::m_AnimTriggerCallbackInfoSlotPool, 8);
    SlotPoolBase::BaseFreeBlocks(&ScriptAction::m_ScriptActionSlotPool, 0x80);
}

/**
 * Offset/Address/Size: 0x1C0 | 0x800124A8 | size: 0xD4
 */
s32 GetCharacterIndex(const cCharacter* character)
{
    cCharacter** ptr = g_pCharacters;
    for (s32 i = 0; i < 10; ++i, ptr++)
    {
        if (*ptr == character)
            return i;
    }
    return -1;
}

/**
 * Offset/Address/Size: 0x0 | 0x800122E8 | size: 0x1C0
 */
s32 GetGoalieIndex(int arg0)
{
    if (arg0 == 0)
    {
        cCharacter** ptr = g_pCharacters;
        for (s32 i = 0; i < 10; ++i, ptr++)
        {
            if (*ptr == g_pCharacters[8])
                return i;
        }
        return -1;
    }

    cCharacter** ptr = g_pCharacters;
    for (s32 i = 0; i < 10; ++i, ptr++)
    {
        if (*ptr == g_pCharacters[9])
            return i;
    }
    return -1;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80014444 | size: 0x10
//  */
// void DLListContainerBase<GCAudioStreaming::StereoAudioStream*, BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>::DeleteEntry(DLListEntry<GCAudioStreaming::StereoAudioStream*>*)
// {
// }

// Force weak symbol emission for DLListContainerBase::DeleteEntry<StereoAudioStream*>
typedef DLListContainerBase<GCAudioStreaming::StereoAudioStream*, BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*> > > _StereoStreamDLList;

// Force weak symbol emission for DLListContainerBase::DeleteEntry<QUEUED_STREAM>
typedef DLListContainerBase<AudioStreamTrack::StreamTrack::QUEUED_STREAM, nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>, 4> > _QueuedStreamDLList;

// Force weak symbol emission for DLListContainerBase::DeleteEntry<STREAM_FADE_CTRL>
typedef DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL, BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL> > > _FadeDLList;

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler
 * lays out the related fields to match the original binary.
 */
typedef WalkHelper<
    AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
    DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
    AudioStreamTrack::TrackManagerBase::FadeManager>
    _FadeWalkHelper;

void CharacterTemplate_stub()
{
    void (_StereoStreamDLList::* volatile forceStereoDelete)(DLListEntry<GCAudioStreaming::StereoAudioStream*>*) = &_StereoStreamDLList::DeleteEntry;
    void (_QueuedStreamDLList::* volatile forceQueuedDelete)(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*) = &_QueuedStreamDLList::DeleteEntry;
    void (_FadeDLList::* volatile forceFadeDelete)(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*) = &_FadeDLList::DeleteEntry;
    (void)forceStereoDelete;
    (void)forceQueuedDelete;
    (void)forceFadeDelete;

    _FadeWalkHelper fadeHelper;
    DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>* fadeEntry = 0;
    fadeHelper.Callback(fadeEntry);
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800144F4 | size: 0x34
//  */
// void WalkHelper<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>, AudioStreamTrack::TrackManagerBase::FadeManager>::Callback(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80014528 | size: 0x24
//  */
// void ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>::DeleteEntry(ListEntry<cSHierarchy*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x24 | 0x8001454C | size: 0x24
//  */
// void ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>::DeleteEntry(ListEntry<AnimRetargetList*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80014570 | size: 0x48
//  */
// void Function0<void>::FunctorBase::~FunctorBase()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800145B8 | size: 0x60
 */
CharacterPhysicsData::~CharacterPhysicsData()
{
    delete[] pPhysicsElements;
}
