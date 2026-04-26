#define NL_LEXICALCAST_DEFINE_BOOL
#include "Game/CharacterTemplate.h"
#include "Game/Player.h"
#include "Game/AI/Fielder.h"
#include "Game/CharacterTweaks.h"
#include "Game/Goalie.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/AnimInventory.h"
#include "Game/Physics/CharacterPhysicsElement.h"
#include "Game/Triggers/SebringAnimScript.h"
#include "NL/nlFile.h"
#include "NL/nlFileGC.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/nlMemory.h"
#include "NL/gl/gl.h"
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

cCharacter* g_pCharacters[10];
static tCharacterTemplateInfo g_aCharacterTemplateInfo[13];
static tCharacterTemplate* g_aCharacterTemplates[13];
static tCharacterTemplateInfo g_GoalieTemplateInfo;
static tCharacterTemplate* g_GoalieTemplate;

s32 skiptexture = 0xFFFFFFFF;

static const char* const s_GoalieCharacterNames[9] = {
    "daisygoalie",      // @1109
    "donkeykonggoalie", // @1111
    "luigigoalie",      // @1113
    "mariogoalie",      // @1100
    "peachgoalie",      // @1115
    "waluigigoalie",    // @1117
    "wariogoalie",      // @1119
    "yoshigoalie",      // @1121
    "superteamgoalie"   // @1123
};

static const char* const s_GoalieTexturePaths[9] = {
    "characters/daisygoalie/daisygoalie.glt",           // @1110
    "characters/donkeykonggoalie/donkeykonggoalie.glt", // @1112
    "characters/luigigoalie/luigigoalie.glt",           // @1114
    "characters/mariogoalie/mariogoalie.glt",           // @1103
    "characters/peachgoalie/peachgoalie.glt",           // @1116
    "characters/waluigigoalie/waluigigoalie.glt",       // @1118
    "characters/wariogoalie/wariogoalie.glt",           // @1120
    "characters/yoshigoalie/yoshigoalie.glt",           // @1122
    "characters/superteamgoalie/superteamgoalie.glt"    // @1124
};

// tGoalieTemplateInfo g_GoalieTextureInfo[9];

// Global goalie texture info array
tGoalieTemplateInfo g_GoalieTextureInfo[9] = {
    { s_GoalieCharacterNames[0], s_GoalieTexturePaths[0], nullptr },
    { s_GoalieCharacterNames[1], s_GoalieTexturePaths[1], nullptr },
    { s_GoalieCharacterNames[2], s_GoalieTexturePaths[2], nullptr },
    { s_GoalieCharacterNames[3], s_GoalieTexturePaths[3], nullptr },
    { s_GoalieCharacterNames[4], s_GoalieTexturePaths[4], nullptr },
    { s_GoalieCharacterNames[5], s_GoalieTexturePaths[5], nullptr },
    { s_GoalieCharacterNames[6], s_GoalieTexturePaths[6], nullptr },
    { s_GoalieCharacterNames[7], s_GoalieTexturePaths[7], nullptr },
    { s_GoalieCharacterNames[8], s_GoalieTexturePaths[8], nullptr }
};

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
 * Offset/Address/Size: 0x294 | 0x8001257C | size: 0x6C0
 */
void DestroyCharacters()
{
}

/**
 * Offset/Address/Size: 0x954 | 0x80012C3C | size: 0x51C
 * TODO: 97.23% match - register allocation diffs throughout both loops (r21<>r28 for g_pCharacters,
 * r23<>r21 for teami, r27<>r30 for captain values). Inner loop compare loads sidekick/captain into
 * r5/r6 (reused as args) vs target r3/r0 (reloads into r5/r6), causing 2 instruction size difference.
 */
extern eCharacterClass ConvertToCharacterClass(eTeamID);
extern eCharacterClass ConvertToCharacterClass(eSidekickID);

static eCharacterClass GetGoalieFromCaptain(eCharacterClass captain)
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

struct PosBlock
{
    nlVector3 v[8];
};
struct GoaliePosBlock
{
    nlVector3 v[2];
};

/**
 * Offset/Address/Size: 0x3DC | 0x80012C3C | size: 0x51C
 * TODO: 97.26% match - register allocation diffs in both loops, 2 instruction
 * size difference from CSE of comparison values in inner loop
 */
void CreateCharacters()
{
    eCharacterClass captain[2];
    eCharacterClass sidekick[2];
    eCharacterClass goalie[2];
    PosBlock posBlock;
    GoaliePosBlock goaliePosBlock;
    int plrindex;
    int teami;

    captain[0] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0));
    captain[1] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1));
    sidekick[0] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(0));
    sidekick[1] = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(1));

    goalie[0] = GetGoalieFromCaptain(captain[0]);
    goalie[1] = GetGoalieFromCaptain(captain[1]);

    bool allcaptains = GetConfigBool(Config::Global(), "allcaptains", false);
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

    static const PosBlock s_Positions = { {
        { 1.5f, 1.5f, 0.0f },
        { 1.5f, -1.5f, 0.0f },
        { 1.5f, 0.0f, 0.0f },
        { 1.5f, 2.5f, 0.0f },
        { -1.5f, 1.5f, 0.0f },
        { -1.5f, -1.5f, 0.0f },
        { -1.5f, 0.0f, 0.0f },
        { -1.5f, 2.5f, 0.0f },
    } };

    static const GoaliePosBlock s_GoaliePositions = { {
        { 18.0f, 0.0f, 0.0f },
        { -18.0f, 0.0f, 0.0f },
    } };

    posBlock = s_Positions;
    goaliePosBlock = s_GoaliePositions;

    SebringAnimTagScriptInterpreter* pInterp = new (nlMalloc(sizeof(SebringAnimTagScriptInterpreter), 8, false)) SebringAnimTagScriptInterpreter();

    g_pAnimScriptInterp = pInterp;

    for (teami = 0; teami < 2; teami++)
    {
        if (captain[0] > captain[1])
        {
            plrindex = !teami;
        }
        else
        {
            plrindex = teami;
        }

        int idx = plrindex * 4;
        g_pCharacters[idx] = CreateCharacter(0, plrindex, captain[plrindex], false);
        g_pCharacters[idx]->SetPosition(posBlock.v[idx]);
        ((Audio::cCharacterSFX*)g_pCharacters[idx]->m_pCharacterSFX)->mGroup = idx;

        g_pTeams[plrindex]->SetPlayer((cPlayer*)g_pCharacters[idx], 0);
        ((cPlayer*)g_pCharacters[idx])->m_pTeam = g_pTeams[plrindex];

        g_pCharacters[plrindex + 8] = CreateGoalie(goalie[plrindex], false);
        g_pCharacters[plrindex + 8]->SetPosition(goaliePosBlock.v[plrindex]);

        g_pTeams[plrindex]->SetGoalie((Goalie*)g_pCharacters[plrindex + 8]);
        ((cPlayer*)g_pCharacters[plrindex + 8])->m_pTeam = g_pTeams[plrindex];
    }

    for (teami = 0; teami < 2; teami++)
    {
        if (sidekick[0] > sidekick[1])
        {
            plrindex = !teami;
        }
        else
        {
            plrindex = teami;
        }

        int charIdx = plrindex * 4 + 1;
        cCharacter** pChar = &g_pCharacters[charIdx];
        nlVector3* pPos = &posBlock.v[charIdx];

        for (int index = 1; index < 4; index++)
        {
            if (sidekick[plrindex] == captain[plrindex])
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
 * Offset/Address/Size: 0xE70 | 0x80013158 | size: 0x634
 * TODO: 66.69% match - scan loops unroll by 5 instead of target's 10 (MWCC build difference),
 * lbzu vs lbz pattern, bne+b vs beq branch in hierarchy search, register allocation diffs
 */
cPlayer* CreateGoalie(eCharacterClass gcc, bool bForViewer)
{
    s32 goalieIdx = gcc - NUM_FIELDER_CLASSES;
    tGoalieTemplateInfo* pTexInfo = &g_GoalieTextureInfo[goalieIdx];
    if (!pTexInfo->bLoaded)
    {
        glLoadTextureBundle(pTexInfo->szTextureFilename);
        pTexInfo->bLoaded = 1;
    }

    if (g_GoalieTemplate == NULL)
    {
        g_GoalieTemplate = (tCharacterTemplate*)nlMalloc(sizeof(tCharacterTemplate), 8, false);
        CharacterLoadingGuts(g_GoalieTemplate, g_GoalieTemplateInfo, gcc, bForViewer);
    }

    cSHierarchy* pHierarchy;
    AnimRetargetList* pAnimRetarget;

    cInventory<cSHierarchy>* pHierInv = g_GoalieTemplate->pHierarchyInventory;
    u32 hash = nlStringHash(g_GoalieTemplateInfo.szHierarchy);

    ListEntry<cSHierarchy*>* hEntry = pHierInv->m_lItemList.m_Head;
    while (hEntry != NULL)
    {
        pHierarchy = hEntry->data;
        if (hash != pHierarchy->m_uHashID)
        {
            hEntry = hEntry->next;
        }
        else
        {
            goto hierFound;
        }
    }
    pHierarchy = NULL;
hierFound:

    pAnimRetarget = NULL;
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

    char buf1[200];
    char buf2[200];
    const char* szPath;
    const char* pStart;
    char* pDst;
    int i;
    u32 texHash;

    szPath = g_GoalieTemplateInfo.szTextureFilename;
    pStart = NULL;
    for (i = 0; i < 100; i++)
    {
        char c = szPath[i];
        if (c == '\\' || c == '/')
        {
            pStart = &szPath[i + 1];
            break;
        }
    }

    pDst = buf1;
    for (i = 0; i < 100; i++)
    {
        if (*pStart == '\0' || *pStart == '.')
        {
            *pDst = '\0';
            texHash = nlStringLowerHash(buf1);
            goto copyDone1;
        }
        *pDst++ = *pStart++;
    }
    texHash = 0;
copyDone1:
    pPlayer->m_uNormalTextureID = texHash;

    szPath = g_GoalieTextureInfo[goalieIdx].szTextureFilename;
    pStart = NULL;
    for (i = 0; i < 100; i++)
    {
        char c = szPath[i];
        if (c == '\\' || c == '/')
        {
            pStart = &szPath[i + 1];
            break;
        }
    }

    pDst = buf2;
    for (i = 0; i < 100; i++)
    {
        if (*pStart == '\0' || *pStart == '.')
        {
            *pDst = '\0';
            texHash = nlStringLowerHash(buf2);
            goto copyDone2;
        }
        *pDst++ = *pStart++;
    }
    texHash = 0;
copyDone2:
    pPlayer->m_uSwapTextureID = texHash;

    if (!AudioLoader::gbDisableAudio)
    {
        pPlayer->SetSFX(g_GoalieTemplateInfo.pSFXPropAccessor);
    }

    return pPlayer;
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
 * Offset/Address/Size: 0x1AA0 | 0x80013D88 | size: 0x1C
 */
s32 SidekickTexture_cb(unsigned long arg0)
{
    s32 var_r4 = -1;
    if (arg0 != skiptexture)
    {
        var_r4 = arg0;
    }
    return var_r4;
}

/**
 * Offset/Address/Size: 0x1ABC | 0x80013DA4 | size: 0x240
 * TODO: 96.11% match - register allocation diffs (r29<>r31, r28<>r30, r26<>r28, r27<>r29,
 * r31<>r26, r30<>r25, r25<>r27), beq vs bne/b branch in hierarchy search loop, extra li r0,0
 * in retarget search setup
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
    cSHierarchy* pHierarchy;
    ListEntry<cSHierarchy*>* hEntry = pHierInv->m_lItemList.m_Head;
    while (hEntry != NULL)
    {
        pHierarchy = hEntry->data;
        if (hash != pHierarchy->m_uHashID)
        {
            hEntry = hEntry->next;
        }
        else
        {
            goto hierFound;
        }
    }
    pHierarchy = NULL;
hierFound:

    pAnimRetarget = NULL;
    if (g_aCharacterTemplates[cc]->pAnimRetargetListInventory != NULL)
    {
        int idx = 0;
        ListEntry<AnimRetargetList*>* retEntry = g_aCharacterTemplates[cc]->pAnimRetargetListInventory->m_lItemList.m_Head;
        AnimRetargetList* retResult = NULL;
        while (retEntry != NULL)
        {
            if (idx == 0)
            {
                retResult = retEntry->data;
                break;
            }
            retEntry = retEntry->next;
            idx++;
        }
        pAnimRetarget = retResult;
    }

    FielderTweaks* pTweaks = new (nlMalloc(0x124, 8, false)) FielderTweaks(g_aCharacterTemplateInfo[cc].szTweaksFilename);

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

extern "C" cSHierarchy* Initialize__11cSHierarchyFP7nlChunk(nlChunk*);
extern "C" AnimRetargetList* Initialize__16AnimRetargetListFP7nlChunk(nlChunk*);

extern SebringAnimTagScriptInterpreter* g_pAnimScriptInterp;

static cAnimInventory* FindDuplicateAnimInventory(int nCurIndex, unsigned long uHashID);
static char* GetCharacterTriggerFileName(eCharacterClass cc);

/**
 * Offset/Address/Size: 0x1CFC | 0x80013FE4 | size: 0x3F0
 * TODO: 94.27% match - 2 scheduling diffs (lwz pHierInv2 hoisted before nlLoadEntireFile in target)
 */
void CharacterLoadingGuts(tCharacterTemplate* pCharacterTemplate, const tCharacterTemplateInfo& charTemplateInfo, eCharacterClass cc, bool bForViewer)
{
    glModel* pRigidCharacterModel = glLoadModel(charTemplateInfo.szModelFilename, NULL);
    glModel* pBlendCharacterModel = glLoadModel(charTemplateInfo.szBlendedModelFilename, NULL);

    pCharacterTemplate->nCharacterModelID[0] = pRigidCharacterModel->id;
    pCharacterTemplate->nCharacterModelID[1] = pBlendCharacterModel->id;

    cInventory<cSHierarchy>* pHierInv = new (nlMalloc(sizeof(cInventory<cSHierarchy>), 8, false)) cInventory<cSHierarchy>();
    pCharacterTemplate->pHierarchyInventory = pHierInv;

    u32 hierFileSize;
    cInventory<cSHierarchy>* pHierInv2;
    nlChunk* hierData = (nlChunk*)nlLoadEntireFile(charTemplateInfo.szHierarchyFilename, &hierFileSize, 0x20, AllocateStart);
    pHierInv2 = pCharacterTemplate->pHierarchyInventory;

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
            cSHierarchy* hier = Initialize__11cSHierarchyFP7nlChunk(hierData);

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

    cAnimInventory* found = NULL;
    s32 i = 0;
    while (i < NUM_FIELDER_CLASSES)
    {
        if (i != (s32)cc)
        {
            if (g_aCharacterTemplates[i] != NULL)
            {
                if (pCharacterTemplate->uAnimInventoryHashID == g_aCharacterTemplates[i]->uAnimInventoryHashID)
                {
                    found = g_aCharacterTemplates[i]->pAnimInventory;
                    break;
                }
            }
        }
        i++;
    }

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
        nlChunk* retargetData = (nlChunk*)nlLoadEntireFile(charTemplateInfo.szAnimRetargetFilename, &retargetFileSize, 0x20, AllocateStart);
        cInventory<AnimRetargetList>* pRetInv = pCharacterTemplate->pAnimRetargetListInventory;

        ListEntry<char*>* retMemEntry = (ListEntry<char*>*)nlMalloc(8, 8, false);
        if (retMemEntry != NULL)
        {
            retMemEntry->next = NULL;
            retMemEntry->data = (char*)retargetData;
        }
        nlListAddStart<ListEntry<char*> >(
            (ListEntry<char*>**)&pRetInv->m_lMemList.m_Head,
            retMemEntry,
            (ListEntry<char*>**)&pRetInv->m_lMemList.m_Tail);

        nlChunk* retargetEnd = (nlChunk*)((char*)retargetData + retargetFileSize);
        while (retargetData != retargetEnd)
        {
            if ((retargetData->m_ID & 0x80FFFFFF) == 0x80017104)
            {
                AnimRetargetList* retarget = Initialize__16AnimRetargetListFP7nlChunk(retargetData);

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

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler
 * lays out the related fields to match the original binary.
 */
void CharacterTemplate_stub()
{
    void (_StereoStreamDLList::* volatile forceStereoDelete)(DLListEntry<GCAudioStreaming::StereoAudioStream*>*) = &_StereoStreamDLList::DeleteEntry;
    void (_QueuedStreamDLList::* volatile forceQueuedDelete)(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*) = &_QueuedStreamDLList::DeleteEntry;
    (void)forceStereoDelete;
    (void)forceQueuedDelete;
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

/**
 * Offset/Address/Size: 0x60 | 0x80014618 | size: 0xE0
 */
SebringAnimTagScriptInterpreter::~SebringAnimTagScriptInterpreter()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800146F8 | size: 0xD0
 */
// AnimTagScriptInterpreter::~AnimTagScriptInterpreter() — defined inline in AnimTagScript.h

/**
 * Offset/Address/Size: 0xD0 | 0x800147C8 | size: 0x4
 */
void AnimTagScriptInterpreter::TriggerFired(unsigned long)
{
}

// LexicalCast<bool, *> specializations are emitted via NL_LEXICALCAST_DEFINE_BOOL

/**
 * Offset/Address/Size: 0x70 | 0x8001483C | size: 0x94
 */
extern "C" void __sinit_CharacterTemplate_cpp()
{
    SoundPropAccessor* pBirdo = gpBIRDOSoundPropAccessor;
    SoundPropAccessor* pDaisy = gpDAISYSoundPropAccessor;
    SoundPropAccessor* pDK = gpDKSoundPropAccessor;
    SoundPropAccessor* pHambros = gpHAMBROSSoundPropAccessor;
    SoundPropAccessor* pKoopa = gpKOOPASoundPropAccessor;
    SoundPropAccessor* pLuigi = gpLUIGISoundPropAccessor;
    SoundPropAccessor* pMario = gpMARIOSoundPropAccessor;
    SoundPropAccessor* pPeach = gpPEACHSoundPropAccessor;
    SoundPropAccessor* pToad = gpTOADSoundPropAccessor;
    SoundPropAccessor* pWaluigi = gpWALUIGISoundPropAccessor;
    SoundPropAccessor* pWario = gpWARIOSoundPropAccessor;
    SoundPropAccessor* pYoshi = gpYOSHISoundPropAccessor;
    SoundPropAccessor* pSuper = gpSUPERSoundPropAccessor;
    SoundPropAccessor* pCritter = gpCRITTERSoundPropAccessor;

    g_aCharacterTemplateInfo[0].pSFXPropAccessor = pBirdo;
    g_aCharacterTemplateInfo[1].pSFXPropAccessor = pDaisy;
    g_aCharacterTemplateInfo[2].pSFXPropAccessor = pDK;
    g_aCharacterTemplateInfo[3].pSFXPropAccessor = pHambros;
    g_aCharacterTemplateInfo[4].pSFXPropAccessor = pKoopa;
    g_aCharacterTemplateInfo[5].pSFXPropAccessor = pLuigi;
    g_aCharacterTemplateInfo[6].pSFXPropAccessor = pMario;
    g_aCharacterTemplateInfo[7].pSFXPropAccessor = pPeach;
    g_aCharacterTemplateInfo[8].pSFXPropAccessor = pToad;
    g_aCharacterTemplateInfo[9].pSFXPropAccessor = pWaluigi;
    g_aCharacterTemplateInfo[10].pSFXPropAccessor = pWario;
    g_aCharacterTemplateInfo[11].pSFXPropAccessor = pYoshi;
    g_aCharacterTemplateInfo[12].pSFXPropAccessor = pSuper;
    g_GoalieTemplateInfo.pSFXPropAccessor = pCritter;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800148D0 | size: 0x68
//  */
// void nlWalkList<ListEntry<AnimRetargetList*>, ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>>(ListEntry<AnimRetargetList*>*, ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>*, void (ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>::*)(ListEntry<AnimRetargetList*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x80014938 | size: 0x68
//  */
// void nlWalkList<ListEntry<cSHierarchy*>, ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>>(ListEntry<cSHierarchy*>*, ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>*, void (ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>::*)(ListEntry<cSHierarchy*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xD0 | 0x800149A0 | size: 0x28
//  */
// void nlListAddStart<ListEntry<AnimRetargetList*>>(ListEntry<AnimRetargetList*>**, ListEntry<AnimRetargetList*>*, ListEntry<AnimRetargetList*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0xF8 | 0x800149C8 | size: 0x28
//  */
// void nlListAddStart<ListEntry<cSHierarchy*>>(ListEntry<cSHierarchy*>**, ListEntry<cSHierarchy*>*, ListEntry<cSHierarchy*>**)
// {
// }
