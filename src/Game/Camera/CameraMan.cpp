#include "Game/Camera/CameraMan.h"

#include "NL/nlDLRing.h"
#include "NL/nlString.h"

#include "Game/Sys/eventman.h"

#include "Game/Camera/animcam.h"
#include "Game/Camera/rumblefilter.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/Field.h"
#include "Game/Net.h"

#include "math.h"

f32 CANT_COLLIDE = *(f32*)__float_max;

nlMatrix4 cCameraManager::m_matView;
nlVector3 cCameraManager::m_cameraPosition;
nlMatrix4 cCameraManager::m_matPrevView;
eCameraTransition cCameraManager::m_transition;
void (*cCameraManager::m_pCallback)(enum eCameraMessage);

int cCameraManager::m_UpVectorStackSize;

nlVector3 cCameraManager::m_UpVectorStack[2] = { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } };

cRumbleFilter* pRumbleFilter;

/**
 * Offset/Address/Size: 0x1D04 | 0x801A838C | size: 0x68
 */
void FireCameraRumbleFilter(float fRumbleX, float fRumbleY)
{
    nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
    if (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != NULL)
    {
        nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter->Rumble(fRumbleX, fRumbleY);
    }
}

/**
 * Offset/Address/Size: 0x1768 | 0x801A7DF0 | size: 0x59C
 */
void cCameraManager::Startup()
{
}

/**
 * Offset/Address/Size: 0x1730 | 0x801A7DB8 | size: 0x38
 */
void cCameraManager::Shutdown()
{
    nlDeleteDLRing<cBaseCamera>(&cCameraManager::m_cameraStack);
    cCameraManager::m_cameraStack = NULL;
    cAnimCamera::FreeCameraAnimations();
    delete pRumbleFilter;
}

/**
 * Offset/Address/Size: 0x10CC | 0x801A7754 | size: 0x664
 */
void cCameraManager::Update(float)
{
}

extern eCameraType g_eCurrentCameraType;
extern World* s_World__12WorldManager;

class Config
{
public:
    struct TagValuePair
    {
        const char* tag;
        int type;
        union
        {
            bool b;
            int i;
            float f;
            const char* s;
        } value;
    };

    static Config& Global();
    TagValuePair& FindTvp(const char*);
    void Set(const char*, bool);
};

template <typename T, typename U>
T LexicalCast(const U&);

void* nlMalloc(unsigned long, unsigned int, bool);

class cDebugCamera
{
public:
    cDebugCamera();
};

class ReplayCamera
{
public:
    ReplayCamera();
};

class TopDownCamera
{
public:
    TopDownCamera();
};

class cFollowCamera
{
public:
    enum FollowTarget
    {
        FOLLOW_BALL = 0,
        FOLLOW_CHARACTER = 1,
        FOLLOW_SELECTABLE = 2,
        FOLLOW_ANIM_VIEWER_CHARACTER = 3,
    };

    cFollowCamera(FollowTarget);
};

class cKickOffCamera
{
public:
    cKickOffCamera();
};

class GameplayCamera
{
public:
    GameplayCamera();
};

class GoalCamera
{
public:
    GoalCamera();
};

class cShootToScoreCamera
{
public:
    cShootToScoreCamera();
};

class cAnimViewerCamera
{
public:
    cAnimViewerCamera();
};

class FaceCam
{
public:
    FaceCam(float);
};

/**
 * Offset/Address/Size: 0xD78 | 0x801A7400 | size: 0x354
 */
void cCameraManager::UpdateGameCameraType()
{
    cBaseCamera* pBaseCamera = nlDLRingGetEnd(cCameraManager::m_cameraStack);

    if (g_eCurrentCameraType != pBaseCamera->GetType())
    {
        Config& cfg = Config::Global();
        Config::TagValuePair& tvp = cfg.FindTvp("nocameratweakcrash");

        bool noCameraTweakCrash;
        if (tvp.tag == NULL)
        {
            cfg.Set("nocameratweakcrash", false);
            noCameraTweakCrash = false;
        }
        else if (tvp.type == 0)
        {
            noCameraTweakCrash = LexicalCast<bool, bool>(tvp.value.b);
        }
        else if (tvp.type == 1)
        {
            noCameraTweakCrash = LexicalCast<bool, int>(tvp.value.i);
        }
        else if (tvp.type == 2)
        {
            noCameraTweakCrash = LexicalCast<bool, float>(tvp.value.f);
        }
        else if (tvp.type == 3)
        {
            noCameraTweakCrash = LexicalCast<bool, const char*>(tvp.value.s);
        }
        else
        {
            noCameraTweakCrash = false;
        }

        if (noCameraTweakCrash)
        {
            if (g_eCurrentCameraType > eCameraType_Gameplay)
            {
                g_eCurrentCameraType = eCameraType_Gameplay;
            }
        }

        s_World__12WorldManager->HandleCameraSwitch();
        pBaseCamera->m_pFilter = NULL;
        nlDLRingRemoveEnd(&cCameraManager::m_cameraStack);
        delete pBaseCamera;

        switch (g_eCurrentCameraType)
        {
        case eCameraType_Debug:
            pBaseCamera = (cBaseCamera*)new ((cDebugCamera*)nlMalloc(0x8C, 8, false)) cDebugCamera();
            break;
        case eCameraType_Replay:
            pBaseCamera = (cBaseCamera*)new ((ReplayCamera*)nlMalloc(0x8C, 8, false)) ReplayCamera();
            break;
        case eCameraType_TopDown:
            pBaseCamera = (cBaseCamera*)new ((TopDownCamera*)nlMalloc(0x78, 8, false)) TopDownCamera();
            break;
        case eCameraType_FollowCharacter:
            pBaseCamera = (cBaseCamera*)new ((cFollowCamera*)nlMalloc(0xA0, 8, false)) cFollowCamera(cFollowCamera::FOLLOW_CHARACTER);
            break;
        case eCameraType_FollowBall:
            pBaseCamera = (cBaseCamera*)new ((cFollowCamera*)nlMalloc(0xA0, 8, false)) cFollowCamera(cFollowCamera::FOLLOW_BALL);
            break;
        case eCameraType_Animated:
            pBaseCamera = (cBaseCamera*)new ((cAnimCamera*)nlMalloc(0xAC, 8, false)) cAnimCamera();
            break;
        case eCameraType_KickOff:
            pBaseCamera = (cBaseCamera*)new ((cKickOffCamera*)nlMalloc(0x74, 8, false)) cKickOffCamera();
            break;
        case eCameraType_Gameplay:
            pBaseCamera = (cBaseCamera*)new ((GameplayCamera*)nlMalloc(0x14C, 8, false)) GameplayCamera();
            break;
        case eCameraType_Goal:
            pBaseCamera = (cBaseCamera*)new ((GoalCamera*)nlMalloc(0x74, 8, false)) GoalCamera();
            break;
        case eCameraType_ShootToScore:
            pBaseCamera = (cBaseCamera*)new ((cShootToScoreCamera*)nlMalloc(0x74, 8, false)) cShootToScoreCamera();
            break;
        case eCameraType_AnimViewer:
            pBaseCamera = (cBaseCamera*)new ((cAnimViewerCamera*)nlMalloc(0xA4, 8, false)) cAnimViewerCamera();
            break;
        case eCameraType_FaceCloseup:
            pBaseCamera = (cBaseCamera*)new ((FaceCam*)nlMalloc(0x80, 8, false)) FaceCam(0.0f);
            break;
        default:
            break;
        }

        nlDLRingAddEnd(&cCameraManager::m_cameraStack, pBaseCamera);
    }
}

/**
 * Offset/Address/Size: 0xD50 | 0x801A73D8 | size: 0x28
 */
bool cCameraManager::HasCamera(cBaseCamera* pCamera)
{
    return nlDLRingValidateContainsElement<cBaseCamera>(cCameraManager::m_cameraStack, pCamera);
}

/**
 * Offset/Address/Size: 0xC58 | 0x801A72E0 | size: 0xF8
 */
void cCameraManager::PushCamera(cBaseCamera* pCamera)
{
    if (cCameraManager::m_transition != eCT_NONE)
    {
        nlPrintf("Camera Transition In Progress\n");
        if (cCameraManager::m_pCallback != NULL)
        {
            (*cCameraManager::m_pCallback)(eCM_ABORTED_BY_PUSH);
        }
    }

    cCameraManager::m_transition = eCT_NONE;

    if ((nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack) != NULL)
        && (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != 0))
    {
        cRumbleFilter* filter1 = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter;
        cRumbleFilter* filter2 = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter;

        nlVector2 diff_pos;
        nlVec2Set(diff_pos, filter2->v2Pos0.f.x - filter1->v2Pos1.f.x, filter2->v2Pos0.f.y - filter1->v2Pos1.f.y);
        if (nlSqrt((diff_pos.f.x * diff_pos.f.x) + (diff_pos.f.y * diff_pos.f.y), 1) > 0.0f)
        {
            g_pEventManager->CreateValidEvent(0x58, 0x14);
        }
    }

    nlDLRingAddStart<cBaseCamera>(&cCameraManager::m_cameraStack, pCamera);
}

/**
 * Offset/Address/Size: 0xBA8 | 0x801A7230 | size: 0xB0
 */
void cCameraManager::Remove(const cBaseCamera& camera)
{
    bool actuallyRemoved = true;
    while (actuallyRemoved)
    {
        actuallyRemoved = nlDLRingRemoveSafely<cBaseCamera>(&cCameraManager::m_cameraStack, &camera);
        if (actuallyRemoved
            && (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack) != NULL)
            && (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != 0))
        {
            cRumbleFilter* pFilter = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter;
            pFilter->Reset();
            nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->Reactivate();
        }
    }
}

/**
 * Offset/Address/Size: 0xA94 | 0x801A711C | size: 0x114
 */
void cCameraManager::Remove(eCameraType type, bool bDeleteAfterRemoving)
{
    bool actuallyRemoved;
    cBaseCamera* pCamera = cCameraManager::m_cameraStack; // r31

    if (cCameraManager::m_cameraStack != NULL)
    {
        cBaseCamera* pCameraNext; // r30
        do
        {
            pCameraNext = pCamera->m_next;
            if (type == pCamera->GetType())
            {
                actuallyRemoved = true;
                while (actuallyRemoved)
                {
                    actuallyRemoved = nlDLRingRemoveSafely<cBaseCamera>(&cCameraManager::m_cameraStack, pCamera);
                    if (actuallyRemoved
                        && (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack) != NULL)
                        && (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != 0))
                    {
                        cRumbleFilter* pFilter = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter;
                        pFilter->Reset();
                        nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->Reactivate();
                    }
                }

                if (bDeleteAfterRemoving)
                {
                    delete pCamera;
                }
            }
            pCamera = pCameraNext;
        } while (pCameraNext != cCameraManager::m_cameraStack);
    }
}

/**
 * Offset/Address/Size: 0x7F8 | 0x801A6E80 | size: 0x29C
 * TODO: 99.55% match - nlPrintf literal label relocation (@1258 vs @267)
 */
/* static */ void cCameraManager::PushCameraWithTransition(cBaseCamera* pCamera, float fDuration, eCameraTransition transition, void (*pCallback)(eCameraMessage))
{
    extern float m_fTransitionSpeed__14cCameraManager;
    extern float m_fTransitionTime__14cCameraManager;
    extern float m_fPrevFOV__14cCameraManager;

    if (cCameraManager::m_transition != eCT_NONE)
    {
        nlPrintf("Camera Transition In Progress\n");
        if (cCameraManager::m_pCallback != NULL)
        {
            (*cCameraManager::m_pCallback)(eCM_ABORTED_BY_PUSH);
        }
    }

    cCameraManager::m_matPrevView = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->GetViewMatrix();
    m_fPrevFOV__14cCameraManager = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->GetFOV();

    if (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != NULL)
    {
        nlMatrix4 matView;
        nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter->Filter(cCameraManager::m_matPrevView, matView);
        cCameraManager::m_matPrevView = matView;
    }

    cCameraManager::m_transition = transition;
    cCameraManager::m_pCallback = pCallback;
    m_fTransitionSpeed__14cCameraManager = 1.0f / fDuration;
    m_fTransitionTime__14cCameraManager = 0.0f;

    if ((nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack) != NULL)
        && (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != 0))
    {
        cRumbleFilter* filter1 = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter;
        cRumbleFilter* filter2 = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter;

        nlVector2 diff_pos;
        nlVec2Set(diff_pos, filter2->v2Pos0.f.x - filter1->v2Pos1.f.x, filter2->v2Pos0.f.y - filter1->v2Pos1.f.y);
        if (nlSqrt((diff_pos.f.x * diff_pos.f.x) + (diff_pos.f.y * diff_pos.f.y), 1) > 0.0f)
        {
            g_pEventManager->CreateValidEvent(0x58, 0x14);
        }
    }

    nlDLRingAddStart<cBaseCamera>(&cCameraManager::m_cameraStack, pCamera);
}

/**
 * Offset/Address/Size: 0x590 | 0x801A6C18 | size: 0x268
 * TODO: 98.67% match - local string label relocation (@1258 vs @250) on nlPrintf literal.
 */
/* static */ void cCameraManager::PopCameraWithTransition(float fDuration, eCameraTransition transition, void (*pCallback)(eCameraMessage))
{
    extern float m_fTransitionSpeed__14cCameraManager;
    extern float m_fTransitionTime__14cCameraManager;
    extern float m_fPrevFOV__14cCameraManager;

    if (cCameraManager::m_transition != eCT_NONE)
    {
        nlPrintf("Camera Transition In Progress\n");
        if (cCameraManager::m_pCallback != NULL)
        {
            (*cCameraManager::m_pCallback)(eCM_ABORTED_BY_POP);
        }
    }

    cCameraManager::m_matPrevView = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->GetViewMatrix();
    m_fPrevFOV__14cCameraManager = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->GetFOV();

    if (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != NULL)
    {
        nlMatrix4 matView;
        nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter->Filter(cCameraManager::m_matPrevView, matView);
        cCameraManager::m_matPrevView = matView;
    }

    cCameraManager::m_transition = transition;
    cCameraManager::m_pCallback = pCallback;
    m_fTransitionSpeed__14cCameraManager = 1.0f / fDuration;
    m_fTransitionTime__14cCameraManager = 1.0f - m_fTransitionTime__14cCameraManager;

    nlDLRingRemoveStart<cBaseCamera>(&cCameraManager::m_cameraStack);

    if (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter != NULL)
    {
        nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter->Reset();
        nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->Reactivate();
    }
}

/**
 * Offset/Address/Size: 0x200 | 0x801A6888 | size: 0x390
 */
bool cCameraManager::IsObjectOccludingField(const DrawableObject* drawable)
{
    const nlMatrix4& worldMatrix = drawable->GetWorldMatrix();
    nlVector3 objectPosition = *(const nlVector3*)&worldMatrix.f.m41;
    const nlVector3* cameraPosition = &m_cameraPosition;

    float objectRadius = drawable->m_fBoundingRadius;
    float netDepth = cNet::m_fNetDepth;
    float goalLineX = cField::GetGoalLineX(1U);
    goalLineX = goalLineX + netDepth;
    float sidelineY = cField::GetSidelineY(1U);

    if ((cameraPosition->f.x > -goalLineX)
        && (cameraPosition->f.x < goalLineX)
        && (cameraPosition->f.y > -sidelineY)
        && (cameraPosition->f.y < sidelineY))
    {
        bool objectInBounds = false;
        if ((objectPosition.f.x > -goalLineX)
            && (objectPosition.f.x < goalLineX)
            && (objectPosition.f.y > -sidelineY)
            && (objectPosition.f.y < sidelineY))
        {
            objectInBounds = true;
        }

        if (objectInBounds == false)
        {
            return false;
        }
    }

    if ((cameraPosition->f.x < -goalLineX) && (objectPosition.f.x > goalLineX))
    {
        return false;
    }

    if ((cameraPosition->f.x > goalLineX) && (objectPosition.f.x < -goalLineX))
    {
        return false;
    }

    if ((cameraPosition->f.y < -sidelineY) && (objectPosition.f.y > sidelineY))
    {
        return false;
    }

    if ((cameraPosition->f.y > sidelineY) && (objectPosition.f.y < -sidelineY))
    {
        return false;
    }

    if ((objectPosition.f.z - objectRadius) < 0.0f)
    {
        objectPosition.f.z += objectRadius - objectPosition.f.z;
    }

    nlVector3 corners[4];
    nlVector3 normals[4];

    corners[0].f.x = -goalLineX;
    corners[0].f.y = -sidelineY;
    corners[0].f.z = 0.0f;
    corners[1].f.x = -goalLineX;
    corners[1].f.y = sidelineY;
    corners[1].f.z = 0.0f;
    corners[2].f.x = goalLineX;
    corners[2].f.y = sidelineY;
    corners[2].f.z = 0.0f;
    corners[3].f.x = goalLineX;
    corners[3].f.y = -sidelineY;
    corners[3].f.z = 0.0f;

    nlVector3* currentCorner = corners;
    nlVector3* currentNormal = normals;
    for (int i = 0; i < 4; i++)
    {
        int next = (i + 1) % 4;
        nlVector3* nextCorner = &corners[next];

        float deltaY = currentCorner->f.y - cameraPosition->f.y;
        float deltaX = currentCorner->f.x - cameraPosition->f.x;
        float deltaZ = currentCorner->f.z - cameraPosition->f.z;

        float edgeZ = nextCorner->f.z - currentCorner->f.z;
        float edgeX = nextCorner->f.x - currentCorner->f.x;
        float edgeY = nextCorner->f.y - currentCorner->f.y;

        float tmp0 = edgeZ * deltaY;
        float tmp1 = edgeZ * deltaX;
        float negEdgeX = -edgeX;
        float normalX = edgeY * deltaZ - tmp0;
        float tmp2 = edgeY * deltaX;
        float normalY = negEdgeX * deltaZ + tmp1;
        float normalZ = edgeX * deltaY - tmp2;

        currentNormal->f.x = normalX;
        currentNormal->f.y = normalY;
        currentNormal->f.z = normalZ;

        float invLength = nlRecipSqrt(
            currentNormal->f.x * currentNormal->f.x
                + currentNormal->f.y * currentNormal->f.y
                + currentNormal->f.z * currentNormal->f.z,
            true);

        currentNormal->f.x = invLength * currentNormal->f.x;
        currentNormal->f.y = invLength * currentNormal->f.y;
        currentNormal->f.z = invLength * currentNormal->f.z;

        currentCorner++;
        currentNormal++;
    }

    float objectDeltaY = objectPosition.f.y - cameraPosition->f.y;
    float objectDeltaX = objectPosition.f.x - cameraPosition->f.x;
    float objectDeltaZ = objectPosition.f.z - cameraPosition->f.z;

    if ((normals[0].f.x * objectDeltaX + normals[0].f.y * objectDeltaY + normals[0].f.z * objectDeltaZ) > objectRadius)
    {
        return false;
    }

    if ((normals[1].f.x * objectDeltaX + normals[1].f.y * objectDeltaY + normals[1].f.z * objectDeltaZ) > objectRadius)
    {
        return false;
    }

    if ((normals[2].f.x * objectDeltaX + normals[2].f.y * objectDeltaY + normals[2].f.z * objectDeltaZ) > objectRadius)
    {
        return false;
    }

    if ((normals[3].f.x * objectDeltaX + normals[3].f.y * objectDeltaY + normals[3].f.z * objectDeltaZ) > objectRadius)
    {
        return false;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x1A4 | 0x801A682C | size: 0x5C
 */
float cCameraManager::GetDistanceFromCameraToObject(const nlVector3& objectPosition)
{
    nlVector3 diff;
    nlVec3Set(diff,
        objectPosition.f.x - cCameraManager::m_cameraPosition.f.x,
        objectPosition.f.y - cCameraManager::m_cameraPosition.f.y,
        objectPosition.f.z - cCameraManager::m_cameraPosition.f.z);

    return nlSqrt(((diff.f.x) * (diff.f.x)) + ((diff.f.y) * (diff.f.y)) + ((diff.f.z) * (diff.f.z)), 1);
}

/**
 * Offset/Address/Size: 0x174 | 0x801A67FC | size: 0x30
 */
void cCameraManager::GetViewVector(nlVector3& viewVector)
{
    nlVec3Set(viewVector,
        -cCameraManager::m_matView.f.m13,
        -cCameraManager::m_matView.f.m23,
        -cCameraManager::m_matView.f.m33);
}

/**
 * Offset/Address/Size: 0x150 | 0x801A67D8 | size: 0x24
 */
void cCameraManager::GetUpVector(nlVector3& upVector)
{
    nlVec3Set(upVector,
        cCameraManager::m_matView.f.m12,
        cCameraManager::m_matView.f.m22,
        cCameraManager::m_matView.f.m32);
}

/**
 * Offset/Address/Size: 0x1C | 0x801A66A4 | size: 0x134
 */
void cCameraManager::SetWorldUpVectorTilt(float fXAxisTilt, float fYAxisTilt)
{
    float fSin; // r1+0xC
    float fCos; // r1+0x8

    nlSinCos(&fSin, &fCos, ((s32)(65536.0f * fXAxisTilt)) / 360);
    nlVec3Set(cCameraManager::m_UpVectorStack[0], 0.0f, fSin, fCos);
    nlSinCos(&fSin, &fCos, ((s32)(65536.0f * fYAxisTilt)) / 360);

    cCameraManager::m_UpVectorStack[0].f.x = fSin;
    cCameraManager::m_UpVectorStack[0].f.z = cCameraManager::m_UpVectorStack[0].f.z * fCos;

    float xx = cCameraManager::m_UpVectorStack[0].f.x * cCameraManager::m_UpVectorStack[0].f.x;
    float yy = cCameraManager::m_UpVectorStack[0].f.y * cCameraManager::m_UpVectorStack[0].f.y;
    float zz = cCameraManager::m_UpVectorStack[0].f.z * cCameraManager::m_UpVectorStack[0].f.z;
    float temp_f1 = nlRecipSqrt(xx + yy + zz, true);

    nlVec3Set(cCameraManager::m_UpVectorStack[0],
        temp_f1 * cCameraManager::m_UpVectorStack[0].f.x,
        temp_f1 * cCameraManager::m_UpVectorStack[0].f.y,
        temp_f1 * cCameraManager::m_UpVectorStack[0].f.z);
}

/**
 * Offset/Address/Size: 0xC | 0x801A6694 | size: 0x10
 */
void cCameraManager::PushWorldUpVector()
{
    cCameraManager::m_UpVectorStackSize++;
}

/**
 * Offset/Address/Size: 0x0 | 0x801A6688 | size: 0xC
 */
void cCameraManager::PopWorldUpVector()
{
    cCameraManager::m_UpVectorStackSize = 0;
}
