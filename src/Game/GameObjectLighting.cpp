#include "Game/GameObjectLighting.h"

#include "Game/GameInfo.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Drawable/DrawableCharacter.h"
#include "NL/gl/glState.h"
#include "NL/gl/glLightUserData.h"
#include "NL/gl/glUserData.h"
#include "types.h"

struct StadiumLightingParams
{
    u32 lightRamp;          // +0x00
    u8 pad1[0x18];          // +0x04
    f32 keyLightIntensity;  // +0x1C
    f32 fillLightIntensity; // +0x20
    u8 pad2[0x18];          // +0x24
}; // total size: 0x3C

struct GameObjectLightArray
{
    u32 numLights;             // +0x00
    GLLightUserData lights[2]; // +0x04
}; // total size: 0x54

StadiumLightingParams gStadiumGameObjectLightingParams[7] = {
    { glGetTexture("MarioStadiumPlayerLightRamp") },
    { glGetTexture("PeachToadStadiumPlayerLightRamp") },
    { glGetTexture("DKDaisyStadiumPlayerLightRamp") },
    { glGetTexture("WarioStadiumPlayerLightRamp") },
    { glGetTexture("YoshiStadiumPlayerLightRamp") },
    { glGetTexture("SuperStadiumPlayerLightRamp") },
    { glGetTexture("ForbiddenDomePlayerLightRamp") }
};

void* g_pCameraRelativeLightData = NULL;

/**
 * Offset/Address/Size: 0xB0C | 0x8012B348 | size: 0x8
 */
void* GetCameraRelativeLightData()
{
    return g_pCameraRelativeLightData;
}

/**
 * Offset/Address/Size: 0xB04 | 0x8012B340 | size: 0x8
 */
void* GetInGameLightData()
{
    extern void* g_pInGameLightData;
    return g_pInGameLightData;
}

bool gAlwaysUseCameraRelativeCharacterLighting;

/**
 * Offset/Address/Size: 0xAFC | 0x8012B338 | size: 0x8
 */
bool AlwaysUseCameraRelativeCharacterLighting()
{
    return gAlwaysUseCameraRelativeCharacterLighting;
}

/**
 * Offset/Address/Size: 0x244 | 0x8012AA80 | size: 0x8B8
 */
void InitializeGameObjectLighting()
{
    struct StadiumLightingParamsInit
    {
        /* 0x00 */ u32 lightRamp;
        /* 0x04 */ s32 rampStartR;
        /* 0x08 */ s32 rampStartG;
        /* 0x0C */ s32 rampStartB;
        /* 0x10 */ s32 rampEndR;
        /* 0x14 */ s32 rampEndG;
        /* 0x18 */ s32 rampEndB;
        /* 0x1C */ f32 cameraRelativeKeyIntensity;
        /* 0x20 */ f32 cameraRelativeFillIntensity;
        /* 0x24 */ f32 inGameKeyIntensity;
        /* 0x28 */ f32 inGameFillIntensity;
        /* 0x2C */ f32 inGameKeyRotYDeg;
        /* 0x30 */ f32 inGameKeyRotZDeg;
        /* 0x34 */ f32 inGameFillRotYDeg;
        /* 0x38 */ f32 inGameFillRotZDeg;
    };

    extern void* g_pInGameLightData;
    extern PlatTexture* g_pGameObjectLightRamp;

    GameObjectLightArray* pLightArray;
    s32 i;
    nlMatrix4 matY;
    nlMatrix4 matZ;
    nlVector3 initialDirection;
    nlVector3 keyDirection;
    nlVector3 fillDirection;
    s32 stadium;
    StadiumLightingParamsInit* pParams;
    PlatTexture* pRampTexture;
    u8* pTextureData;
    s32 deltaR;
    s32 deltaG;
    s32 deltaB;

    g_pCameraRelativeLightData = glUserAlloc(GLUD_Light, 0x54, true);
    pLightArray = (GameObjectLightArray*)glUserGetData(g_pCameraRelativeLightData);
    pLightArray->numLights = 2;

    for (i = 0; i < 2; i++)
    {
        nlZeroMemory(&pLightArray->lights[i], 0x28);
        pLightArray->lights[i].colour.c[0] = 1.0f;
        pLightArray->lights[i].colour.c[1] = 0.0f;
        pLightArray->lights[i].colour.c[2] = 0.0f;
        pLightArray->lights[i].colour.c[3] = 1.0f;
        pLightArray->lights[i].intensity = 1.0f;
        pLightArray->lights[i].innerRadius = 0.0f;
        pLightArray->lights[i].outerRadius = 0.0f;
    }

    g_pInGameLightData = glUserAlloc(GLUD_Light, 0x54, true);
    pLightArray = (GameObjectLightArray*)glUserGetData(g_pInGameLightData);
    pLightArray->numLights = 2;

    for (i = 0; i < 2; i++)
    {
        nlZeroMemory(&pLightArray->lights[i], 0x28);
        pLightArray->lights[i].colour.c[0] = 1.0f;
        pLightArray->lights[i].colour.c[1] = 0.0f;
        pLightArray->lights[i].colour.c[2] = 0.0f;
        pLightArray->lights[i].colour.c[3] = 1.0f;
        pLightArray->lights[i].intensity = 1.0f;
        pLightArray->lights[i].innerRadius = 0.0f;
        pLightArray->lights[i].outerRadius = 0.0f;
    }

    pLightArray = (GameObjectLightArray*)glUserGetData(g_pInGameLightData);
    stadium = nlSingleton<GameInfoManager>::s_pInstance->GetStadium();
    pParams = (StadiumLightingParamsInit*)&gStadiumGameObjectLightingParams[stadium];

    nlVec3Set(initialDirection, 1.0f, 0.0f, 0.0f);

    nlMakeRotationMatrixY(matY, (3.1415927f * pParams->inGameKeyRotYDeg) / 180.0f);
    nlMakeRotationMatrixZ(matZ, (3.1415927f * pParams->inGameKeyRotZDeg) / 180.0f);
    nlMultDirVectorMatrix(keyDirection, initialDirection, matY);
    nlMultDirVectorMatrix(keyDirection, keyDirection, matZ);

    nlMakeRotationMatrixY(matY, (3.1415927f * pParams->inGameFillRotYDeg) / 180.0f);
    nlMakeRotationMatrixZ(matZ, (3.1415927f * pParams->inGameFillRotZDeg) / 180.0f);
    nlMultDirVectorMatrix(fillDirection, initialDirection, matY);
    nlMultDirVectorMatrix(fillDirection, fillDirection, matZ);

    nlVec3Set(pLightArray->lights[0].worldPosition, -keyDirection.f.x, -keyDirection.f.y, -keyDirection.f.z);
    pLightArray->lights[0].intensity = pParams->inGameKeyIntensity;

    nlVec3Set(pLightArray->lights[1].worldPosition, -fillDirection.f.x, -fillDirection.f.y, -fillDirection.f.z);
    pLightArray->lights[1].intensity = pParams->inGameFillIntensity;

    stadium = nlSingleton<GameInfoManager>::s_pInstance->GetStadium();
    pRampTexture = glx_CreatePlatTexture();
    g_pGameObjectLightRamp = pRampTexture;
    glx_AddTex(gStadiumGameObjectLightingParams[stadium].lightRamp, pRampTexture);
    g_pGameObjectLightRamp->Create(0x100, 4, GXTex_RGBA8, 1, true, false);

    stadium = nlSingleton<GameInfoManager>::s_pInstance->GetStadium();
    pParams = (StadiumLightingParamsInit*)&gStadiumGameObjectLightingParams[stadium];

    deltaR = pParams->rampEndR - pParams->rampStartR;
    deltaG = pParams->rampEndG - pParams->rampStartG;
    deltaB = pParams->rampEndB - pParams->rampStartB;

    pTextureData = (u8*)g_pGameObjectLightRamp->m_LinearData;
    for (i = 0; i < 0x100; i++)
    {
        f32 t = (f32)i * (1.0f / 256.0f);
        pTextureData[0] = (u8)((t * (f32)deltaR) + (f32)pParams->rampStartR);
        pTextureData[1] = (u8)((t * (f32)deltaG) + (f32)pParams->rampStartG);
        pTextureData[2] = (u8)((t * (f32)deltaB) + (f32)pParams->rampStartB);
        pTextureData[3] = 0xFF;
        pTextureData += 4;
    }

    for (i = 1; i < 4; i++)
    {
        memcpy((u8*)g_pGameObjectLightRamp->m_LinearData + (i * 0x400), g_pGameObjectLightRamp->m_LinearData, 0x400);
    }

    g_pGameObjectLightRamp->Swizzle(false);
    g_pGameObjectLightRamp->Prepare();
}

/**
 * Offset/Address/Size: 0x34 | 0x8012A870 | size: 0x210
 * TODO: 99.4% match in scratch - remaining diffs are label/address `i` diffs
 * from static local variable numbering and constant pool labels.
 */
void UpdateGameObjectLighting()
{
    if (!DrawableCharacter::sCameraRelativeLighting && !gAlwaysUseCameraRelativeCharacterLighting)
        return;

    static nlVector3 keyLightInViewSpace;
    static nlVector3 fillLightInViewSpace;
    static bool initedLightInViewSpace;
    static s8 init;

    nlVector3 transformedDir;
    nlVector3 viewVec;
    nlMatrix4 viewRotMat;
    nlMatrix4 matZ;
    nlMatrix4 matY;

    void* pLightData = g_pCameraRelativeLightData;

    if (!init)
    {
        initedLightInViewSpace = false;
        init = true;
    }

    if (!initedLightInViewSpace)
    {
        nlVector3 initialDirection = { 1.0f, 0.0f, 0.0f };

        nlMakeRotationMatrixY(matY, 0.7853982f);
        nlMakeRotationMatrixZ(matZ, -0.69813174f);
        nlMultDirVectorMatrix(keyLightInViewSpace, initialDirection, matY);
        nlMultDirVectorMatrix(keyLightInViewSpace, keyLightInViewSpace, matZ);

        nlMakeRotationMatrixY(matY, 0.5235988f);
        nlMakeRotationMatrixZ(matZ, 0.34906587f);
        nlMultDirVectorMatrix(fillLightInViewSpace, initialDirection, matY);
        nlMultDirVectorMatrix(fillLightInViewSpace, fillLightInViewSpace, matZ);

        initedLightInViewSpace = true;
    }

    cCameraManager::GetViewVector(viewVec);

    f32 angle = nlATan2f(viewVec.f.y, viewVec.f.x);
    u16 u16Angle = (u16)(s32)(angle * 10430.378f);
    f32 radAngle = (f32)u16Angle * 0.0000958738f;

    nlMakeRotationMatrixZ(viewRotMat, radAngle);

    s32 stadium = nlSingleton<GameInfoManager>::s_pInstance->GetStadium();
    StadiumLightingParams* params = &gStadiumGameObjectLightingParams[stadium];

    GameObjectLightArray* pLights = (GameObjectLightArray*)glUserGetData(pLightData);

    nlMultDirVectorMatrix(transformedDir, keyLightInViewSpace, viewRotMat);

    nlVec3Set(pLights->lights[0].worldPosition, -transformedDir.f.x, -transformedDir.f.y, -transformedDir.f.z);
    pLights->lights[0].intensity = params->keyLightIntensity;

    nlMultDirVectorMatrix(transformedDir, fillLightInViewSpace, viewRotMat);

    nlVec3Set(pLights->lights[1].worldPosition, -transformedDir.f.x, -transformedDir.f.y, -transformedDir.f.z);
    pLights->lights[1].intensity = params->fillLightIntensity;
}

/**
 * Offset/Address/Size: 0x0 | 0x8012A83C | size: 0x34
 */
u32 GetGameObjectLightRamp()
{
    return gStadiumGameObjectLightingParams[nlSingleton<GameInfoManager>::s_pInstance->GetStadium()].lightRamp;
}
