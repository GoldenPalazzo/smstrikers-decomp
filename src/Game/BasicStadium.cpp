#pragma pool_data off

#include "Game/BasicStadium.h"
#include "Game/AI/Powerups.h"
#include "Game/Render/NetMesh.h"
#include "Game/Render/CrowdManager.h"
#include "Game/Render/Presentation.h"
#include "Game/Render/StaticModelExplodable.h"
#include "Game/Render/AnimatedModelExplodable.h"
#include "Game/WorldTriggers.h"

#include "Game/Field.h"
#include "Game/Drawable/DrawableModel.h"

#include "NL/nlString.h"
#include "NL/nlPrint.h"
#include "NL/nlFile.h"
#include "NL/gl/glPlat.h"
#include "NL/gl/glState.h"
#include "NL/gl/glTexture.h"
#include "string.h"

#include "Game/World/WorldLoader.h"
#include "Game/Physics/PhysicsNet.h"
#include "Game/Effects/EmissionManager.h"

extern u32 eOC_OPTIMIZE_OUT_FROM_GAMEPLAY;
extern bool g_GoalLightEnabled;
float g_fSkyboxRotationTime = 1420.0f;
float g_fCloudRotationTime = 720.0f;
EventManager* g_pEventManager = nullptr;

char szBasicStadiumName[0x40];

const char szBallName[] = "gameplay/ball";
const char szPowerupsName[] = "gameplay/powerups";

u32 uSkyBoxHashID;
u32 uCloudsHashID;
u32 uNetHelperHashID;
u32 uFieldHelperHashID;
u32 uPenaltyHelperHashID;
BasicStadium* pBasicStadiumInstance;

static nlMatrix4 mSkyBoxTM;
static nlMatrix4 mCloudTM;

extern unsigned int nlDefaultSeed;

/**
 * Offset/Address/Size: 0x2D70 | 0x8019EAF0 | size: 0x44
 */
void BasicStadium::BasicStadiumEventHandler(Event* pEvent, void*)
{
    switch (pEvent->m_uEventID)
    {
    case 5:
        g_GoalLightEnabled = true;
        return;
    case 9:
        g_GoalLightEnabled = false;
        return;
    case 6:
    case 4:
        return;
    }
}

/**
 * Offset/Address/Size: 0x2B64 | 0x8019E8E4 | size: 0x20C
 */
BasicStadium::BasicStadium(const char* name)
    : World(name)
{
    m_CameraFlashPositions = NULL;
    m_NumCameraFlashPositions = 0;
    m_fSkyboxRotationAng = 0.0f;
    m_fCloudRotationAng = 0.0f;
    m_bCameraFlashesEnabled = false;
    mpNPCManager = NULL;
    pBasicStadiumInstance = this;

    m_pEventHandler = g_pEventManager->AddEventHandler(BasicStadium::BasicStadiumEventHandler, NULL, 4);

    nlStrNCpy<char>(m_szBaseName, name, 0x20);
    nlToLower<char>(m_szBaseName);
    if (strstr(m_szBaseName, "mario_") != 0)
    {
        glx_SetFog(0);
    }
    else if (strstr(m_szBaseName, "palace") != 0)
    {
        glx_SetFog(1);
    }
    else if (strstr(m_szBaseName, "dk_daisy") != 0)
    {
        glx_SetFog(2);
    }

    CrowdManager::instance.SetStadium(m_szBaseName);

    g_fSkyboxRotationTime = (strstr(m_szBaseName, "super") != nullptr) ? 800.0f : 1420.0f;

    nlSNPrintf(szBasicStadiumName, 0x40, "Environment/%s/%s", m_szBaseName, m_szBaseName);

    char szTemp[0x100];
    nlStrNCat<char>(szTemp, m_szBaseName, "/skybox back", 0x100);
    uSkyBoxHashID = nlStringLowerHash(szTemp);
    nlStrNCat<char>(szTemp, m_szBaseName, "/skybox clouds", 0x100);
    uCloudsHashID = nlStringLowerHash(szTemp);
    nlStrNCat<char>(szTemp, m_szBaseName, "/NetCorner", 0x100);
    uNetHelperHashID = nlStringLowerHash(szTemp);
    nlStrNCat<char>(szTemp, m_szBaseName, "/FieldCorner", 0x100);
    uFieldHelperHashID = nlStringLowerHash(szTemp);
    nlStrNCat<char>(szTemp, m_szBaseName, "/PenaltyCorner", 0x100);
    uPenaltyHelperHashID = nlStringLowerHash(szTemp);
}

/**
 * Offset/Address/Size: 0x2ABC | 0x8019E83C | size: 0xA8
 */
BasicStadium::~BasicStadium()
{
    g_pEventManager->RemoveEventHandler(m_pEventHandler);
    pBasicStadiumInstance = NULL;

    delete mpNPCManager;

    if (m_CameraFlashPositions != NULL)
    {
        delete[] m_CameraFlashPositions;
    }
}

/**
 * Offset/Address/Size: 0x28F0 | 0x8019E670 | size: 0x1CC
 */
bool BasicStadium::DoLoad()
{
    if (!LoadGeometry(szBasicStadiumName, false, false, 0, 0))
        return false;
    if (!LoadGeometry(szBallName, true, false, 0, 0))
        return false;
    if (!LoadGeometry(szPowerupsName, true, false, 0, 0))
        return false;

    if (!LoadObjectData(szBasicStadiumName))
        return false;

    if (!StaticModelExplodable::LoadGeometry())
        return false;

    StaticModelExplodable::CreateExplodablesFromHelperObjects();

    if (!AnimatedModelExplodable::LoadGeometry())
        return false;

    nlToLower<char>(m_szBaseName);

    char szTemp[0x40];

    nlSNPrintf(szTemp, 0x40, "%s/lightramp", m_szBaseName);
    u32 lightRamp = glGetTexture(szTemp);

    nlSNPrintf(szTemp, 0x40, "%s/playerlightramp", m_szBaseName);
    u32 playerLightRamp = glGetTexture(szTemp);

    if (glTextureLoad(lightRamp))
    {
        m_LightRampTexA = lightRamp; // at +0x124
        m_LightRampTexB = lightRamp; // at +0x128
    }

    if (glTextureLoad(playerLightRamp))
    {
        m_PlayerLightRampTex = playerLightRamp; // at +0x12C
    }

    m_GlobalLightRampSTSTex = glGetTexture("global/lightramp_sts"); // as +0x130

    Presentation::Instance().LoadTrophyModel();
    InitializePowerups();

    NetMesh::s_bAnimatedNetMeshEnabled = true;
    g_GoalLightEnabled = false;

    return true;
}

/**
 * Offset/Address/Size: 0x2E8 | 0x8019C068 | size: 0x2608
 */
bool BasicStadium::DoInitialize()
{
    DrawableObject* pObject;

    FindDrawableObject(nlStringLowerHash("gameplay/ball"));
    m_pSkyboxObject = FindDrawableObject(uSkyBoxHashID);
    m_pCloudsObject = FindDrawableObject(uCloudsHashID);

    if (m_pSkyboxObject != NULL)
    {
        mSkyBoxTM = m_pSkyboxObject->GetWorldMatrix();
    }

    if (m_pCloudsObject != NULL)
    {
        mCloudTM = m_pCloudsObject->GetWorldMatrix();
    }

    pObject = FindDrawableObject(nlStringLowerHash("gameplay/greenshell"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;
    pObject = FindDrawableObject(nlStringLowerHash("gameplay/redshell"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;
    pObject = FindDrawableObject(nlStringLowerHash("gameplay/mushroom"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;
    pObject = FindDrawableObject(nlStringLowerHash("gameplay/banana"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;
    pObject = FindDrawableObject(nlStringLowerHash("gameplay/blueshell"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;
    pObject = FindDrawableObject(nlStringLowerHash("gameplay/spikeshell"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;
    pObject = FindDrawableObject(nlStringLowerHash("gameplay/bobomb"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;
    pObject = FindDrawableObject(nlStringLowerHash("gameplay/star"));
    pObject->m_uObjectFlags &= 0xFFFFFFFE;

    /**
     * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0x2608
     * TODO: 98.14% match - register allocation shift: pPacket=r30 (target r26),
     * pGlModel=r27 (target r30), formatStr=r26 (target r27). 397 register diffs,
     * 6 extra instructions from hash save pattern.
     */
    glModel* pGlModel;
    int counter = 1;
    u8 keepLooking = 1;
    while (keepLooking)
    {
        char szTemp1[256];
        char szTemp2[256];
        nlSNPrintf(szTemp1, 0x100, "/InvisiblePoly0%d", counter);
        nlStrNCat<char>(szTemp2, m_szBaseName, szTemp1, 0x100);
        pObject = FindDrawableObject(nlStringLowerHash(szTemp2));
        if (pObject != NULL)
        {
            DrawableModel* model = pObject->AsDrawableModel();
            pGlModel = model->m_pModel;
            for (glModelPacket* pPacket = pGlModel->packets; pPacket < pGlModel->packets + pGlModel->numPackets; pPacket++)
            {
                glSetRasterState(pPacket->state.raster, GLS_DepthWrite, 1);
                glSetRasterState(pPacket->state.raster, GLS_AlphaTest, 0);
                glSetRasterState(pPacket->state.raster, GLS_AlphaBlend, 1);
            }
            counter += 1;
        }
        else
        {
            keepLooking = 0;
        }
    }

    u32 hash = nlStringLowerHash("gameplay/ball");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/WarioNet05");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/WarioNet01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/Goal_Lights_01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/Goal_Lights_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/WarioNet_Reflect_01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/WarioNet_Reflect_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/WarioNet_Reflect_03");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("wario_stadium/WarioNet_Reflect_04");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("mario_stadium/MarioNet04");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("mario_stadium/MarioNet05");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("yoshi_stadium/Net04");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("yoshi_stadium/Net05");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/Goal_Front_01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/Goal_Front_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/GoldGoal_01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/GoldGoal_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/line01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/line02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/line03");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("DK_Daisy/line04");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/goal01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/goal05");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/posts_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/posts_03");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/posts_shadow");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/posts_shadowed_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_03");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_04");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_05");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_06");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_07");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_08");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_09");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_10");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_11");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_12");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_13");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_14");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_15");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_16");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_17");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_18");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_19");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("The_Palace/Rebar_20");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Forbidden_Dome/Goalpost_04");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Forbidden_Dome/Goalpost_05");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Forbidden_Dome/Net_Front_02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Forbidden_Dome/Net_Front_05");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Forbidden_Dome/Net_Front_06");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Forbidden_Dome/Net_Front_07");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/goal01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/goal02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/Goalballs01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/Goalballs02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/goal_glass01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/goal_glass02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/object668");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/object669");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/object670");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/object671");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/object672");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/object673");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/Laser01");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    hash = nlStringLowerHash("Super_Stadium/Laser02");
    pObject = FindDrawableObject(hash);
    if (pObject != NULL)
    {
        AddToHyperSTSDrawables(hash, pObject->AsDrawableModel());
    }

    HelperObject* pFieldHelper = FindHelperObject(uFieldHelperHashID);
    if (pFieldHelper != NULL)
    {
        cField::SetFieldDimensions(pFieldHelper->m_worldMatrix.f.m41, pFieldHelper->m_worldMatrix.f.m42, 0.0f);
    }

    // Stadium.ini Config
    {
        Config stadConfig(Config::ALLOCATE_HIGH);
        eStadiumID stadiumid = nlSingleton<GameInfoManager>::s_pInstance->GetStadium();
        stadConfig.LoadFromFile("Stadium.ini");
        char szKey[256];
        nlStrNCat<char>(szKey, m_szBaseName, " pitch", 0x100);
        BasicString<char, Detail::TempStringAllocator> terrain = stadConfig.Get<BasicString<char, Detail::TempStringAllocator> >(szKey, BasicString<char, Detail::TempStringAllocator>("grass"));
        TheWorldLoader.SetStadiumTerrain(stadiumid, terrain.c_str());
        m_TerrainType = TheWorldLoader.GetStadiumTerrain(stadiumid);
        fxSetTerrain(g_TerrainTypeHashes[m_TerrainType]);
    }

    // GoalPostDimensions.ini Config
    {
        Config gpConfig(Config::ALLOCATE_HIGH);
        gpConfig.LoadFromFile("GoalPostDimensions.ini");
        char szKey[256];
        nlStrNCat<char>(szKey, m_szBaseName, " goalpost radius", 0x100);
        float fGoalpostRadius = GetConfigFloat(gpConfig, szKey, 0.0f);
        nlStrNCat<char>(szKey, m_szBaseName, " goalpost offset", 0x100);
        float fGoalpostOffset = GetConfigFloat(gpConfig, szKey, 0.0f);
        nlStrNCat<char>(szKey, m_szBaseName, " net width", 0x100);
        float fNetWidth = GetConfigFloat(gpConfig, szKey, 0.0f);
        nlStrNCat<char>(szKey, m_szBaseName, " net height", 0x100);
        float fNetHeight = GetConfigFloat(gpConfig, szKey, 0.0f);
        cField::mpNet[0]->SetNetDimensions(fNetWidth, fNetHeight, fGoalpostRadius, fGoalpostOffset);
        nlStrNCat<char>(szKey, m_szBaseName, " physics net width", 0x100);
        float fPhysNetWidth = GetConfigFloat(gpConfig, szKey, 0.0f);
        nlStrNCat<char>(szKey, m_szBaseName, " physics net height", 0x100);
        float fPhysNetHeight = GetConfigFloat(gpConfig, szKey, 0.0f);
        nlStrNCat<char>(szKey, m_szBaseName, " physics net depth", 0x100);
        float fPhysNetDepth = GetConfigFloat(gpConfig, szKey, 0.0f);
        PhysicsNet::sfPhysicsNetWidth = fPhysNetWidth;
        PhysicsNet::sfPhysicsNetHeight = fPhysNetHeight;
        PhysicsNet::sfPhysicsNetDepth = fPhysNetDepth;
        nlStrNCat<char>(szKey, m_szBaseName, " physics net softness", 0x100);
        float fSoftness = GetConfigFloat(gpConfig, szKey, -1.0f);
        if (fSoftness >= 0.0f)
        {
            PhysicsNet::sfWallSoftness = fSoftness;
        }
        nlStrNCat<char>(szKey, m_szBaseName, " dont use lowest net texture LOD", 0x100);
        bool bDontUseLowest = GetConfigBool(gpConfig, szKey, false);
        NetMesh::SetDontUseLowestNetTextureLOD(bDontUseLowest);
    }

    // PlanarShadow.ini Config
    {
        Config psConfig(Config::ALLOCATE_HIGH);
        psConfig.LoadFromFile("PlanarShadow.ini");
        char szKey[256];
        nlStrNCat<char>(szKey, m_szBaseName, " Shadow Height", 0x100);
        float fShadowHeight = GetConfigFloat(psConfig, szKey, 0.1f);
        SetCoPlanarZ(fShadowHeight);
        nlStrNCat<char>(szKey, m_szBaseName, " Shadow Opacity", 0x100);
        float fShadowOpacity = GetConfigFloat(psConfig, szKey, 0.3f);
        SetPlanarShadowOpacity(fShadowOpacity);
    }

    // PenaltyHelper
    HelperObject* pPenBoxHelper = FindHelperObject(uPenaltyHelperHashID);
    if (pPenBoxHelper != NULL)
    {
        cField::mfPenaltyBoxX = pPenBoxHelper->m_worldMatrix.f.m41;
        cField::mfPenaltyBoxY = pPenBoxHelper->m_worldMatrix.f.m42;
    }

    // Camera flash - Pass 1: Count
    m_NumCameraFlashPositions = 0;
    u32* pIterStack = (u32*)nlMalloc(8, 8, false);
    if (pIterStack != NULL)
    {
        AVLTreeNode* pNode = (AVLTreeNode*)m_helperMap.m_Root;
        pIterStack[0] = (u32)nlMalloc((m_helperMap.m_NumElements + 1) * 4, 8, false);
        pIterStack[1] = 0;
        if (pNode != NULL)
        {
            while (pNode->left != NULL)
            {
                ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pNode;
                pIterStack[1]++;
                pNode = pNode->left;
            }
            ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pNode;
            pIterStack[1]++;
        }
    }
    const char* flashString = "fx_camera_flash";
    while (pIterStack[1] > 0)
    {
        HelperObject* pHelper = ((AVLTreeEntry<unsigned long, HelperObject*>*)((AVLTreeNode**)pIterStack[0])[pIterStack[1] - 1])->value;
        if (nlStrNICmp<char>(pHelper->m_szName, flashString, nlStrLen<char>(flashString)) == 0)
        {
            m_NumCameraFlashPositions++;
        }
        pIterStack[1]--;
        AVLTreeNode* pPopped = ((AVLTreeNode**)pIterStack[0])[pIterStack[1]];
        AVLTreeNode* pRight = pPopped->right;
        if (pRight != NULL)
        {
            while (pRight->left != NULL)
            {
                ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pRight;
                pIterStack[1]++;
                pRight = pRight->left;
            }
            ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pRight;
            pIterStack[1]++;
        }
    }
    if (pIterStack != NULL)
    {
        delete[] (char*)pIterStack[0];
        delete pIterStack;
    }

    // Camera flash - Pass 2: Extract positions
    m_CameraFlashPositions = (nlVector3*)nlMalloc(m_NumCameraFlashPositions * sizeof(nlVector3), 8, false);
    m_NumCameraFlashPositions = 0;
    pIterStack = (u32*)nlMalloc(8, 8, false);
    if (pIterStack != NULL)
    {
        AVLTreeNode* pNode = (AVLTreeNode*)m_helperMap.m_Root;
        pIterStack[0] = (u32)nlMalloc((m_helperMap.m_NumElements + 1) * 4, 8, false);
        pIterStack[1] = 0;
        if (pNode != NULL)
        {
            while (pNode->left != NULL)
            {
                ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pNode;
                pIterStack[1]++;
                pNode = pNode->left;
            }
            ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pNode;
            pIterStack[1]++;
        }
    }
    while (pIterStack[1] > 0)
    {
        HelperObject* pHelper = ((AVLTreeEntry<unsigned long, HelperObject*>*)((AVLTreeNode**)pIterStack[0])[pIterStack[1] - 1])->value;
        if (nlStrNICmp<char>(pHelper->m_szName, flashString, nlStrLen<char>(flashString)) == 0)
        {
            m_CameraFlashPositions[m_NumCameraFlashPositions] = *(nlVector3*)&pHelper->m_worldMatrix.f.m41;
            m_NumCameraFlashPositions++;
        }
        pIterStack[1]--;
        AVLTreeNode* pPopped = ((AVLTreeNode**)pIterStack[0])[pIterStack[1]];
        AVLTreeNode* pRight = pPopped->right;
        if (pRight != NULL)
        {
            while (pRight->left != NULL)
            {
                ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pRight;
                pIterStack[1]++;
                pRight = pRight->left;
            }
            ((AVLTreeNode**)pIterStack[0])[pIterStack[1]] = pRight;
            pIterStack[1]++;
        }
    }
    if (pIterStack != NULL)
    {
        delete[] (char*)pIterStack[0];
        delete pIterStack;
    }

    // Extra ball loop
    {
        const DrawableObject* ball = FindDrawableObject(nlStringHash("gameplay/ball"));
        if (ball != NULL)
        {
            for (int i = 0; i < 2; i++)
            {
                DrawableObject* extraBall = ball->Clone();
                extraBall->m_uObjectFlags &= 0xFFFFFFFE;
                BasicString<char, Detail::TempStringAllocator> name = Format(BasicString<char, Detail::TempStringAllocator>("extra_ball_{0}"), i);
                unsigned long extraHash = nlStringHash(name.c_str());
                AddDrawableObject(extraHash, extraBall);
            }
        }
    }

    // Optimization file loading
    {
        char szOptBin[80];
        nlSNPrintf(szOptBin, 0x50, "%s-GameplayInvisibleModels.bin", m_szBaseName);
        unsigned long fileSize;
        u32* pData = (u32*)nlLoadEntireFile(szOptBin, &fileSize, 0x20, AllocateEnd);
        if (pData != NULL)
        {
            u32* pCur = pData;
            u32 uFlags = eOC_OPTIMIZE_OUT_FROM_GAMEPLAY;
            unsigned long numEntries = fileSize >> 2;
            for (unsigned long j = 0; j < numEntries; j++)
            {
                DrawableObject* pObj = FindDrawableObject(*pCur);
                if (pObj != NULL)
                {
                    DrawableModel* pModel = pObj->AsDrawableModel();
                    pModel->m_uObjectCreationFlags |= uFlags;
                }
                pCur++;
            }
            nlFree(pData);
        }
    }

    return true;
}
/**
 * Offset/Address/Size: 0x8C | 0x8019BE0C | size: 0x25C
 */
void BasicStadium::Update(float dt)
{
    nlMatrix4 mWorld;
    nlMatrix4 mRot;

    World::Update(dt);

    m_fSkyboxRotationAng += dt * (6.2831855f / g_fSkyboxRotationTime);
    m_fCloudRotationAng += dt * (6.2831855f / g_fCloudRotationTime);

    if (m_fSkyboxRotationAng > 6.2831855f)
    {
        m_fSkyboxRotationAng -= 6.2831855f;
    }

    if (m_fCloudRotationAng > 6.2831855f)
    {
        m_fCloudRotationAng -= 6.2831855f;
    }

    if (m_pSkyboxObject != NULL)
    {
        nlMakeRotationMatrixZ(mRot, m_fSkyboxRotationAng);
        nlMultMatrices(mWorld, mRot, mSkyBoxTM);
        m_pSkyboxObject->m_worldMatrix = mWorld;
    }

    if (m_pCloudsObject != NULL)
    {
        nlMakeRotationMatrixZ(mRot, m_fCloudRotationAng);
        nlMultMatrices(mWorld, mRot, mCloudTM);
        m_pCloudsObject->m_worldMatrix = mWorld;
    }

    if (m_bCameraFlashesEnabled)
    {
        if (m_NumCameraFlashPositions != 0)
        {
            u32 randomValue = nlRandom(m_NumCameraFlashPositions, &nlDefaultSeed);
            m_fTimeUntilNextCameraFlash -= dt;
            if (m_fTimeUntilNextCameraFlash < 0.0f)
            {
                EmitCameraFlash(m_CameraFlashPositions[randomValue]);
                m_fTimeUntilNextCameraFlash = 0.01f + nlRandomf(0.05f, &nlDefaultSeed);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x48 | 0x8019BDC8 | size: 0x44
 */
void BasicStadium::UpdateInReplay(float dt)
{
    World::UpdateInReplay(dt);
    mpNPCManager->UpdateNPCs(dt);
}

/**
 * Offset/Address/Size: 0x8 | 0x8019BD88 | size: 0x40
 */
void BasicStadium::Render()
{
    World::Render();
    if (!World::sbIsHyperShootToScoreRenderingEnabled)
    {
        mpNPCManager->RenderNPCs();
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8019BD80 | size: 0x8
 */
BasicStadium* BasicStadium::GetCurrentStadium()
{
    return pBasicStadiumInstance;
}
