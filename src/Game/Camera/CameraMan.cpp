#include "Game/Camera/CameraMan.h"

#include "NL/nlDLRing.h"
#include "NL/nlConfig.h"
#include "NL/nlFormatFwd.h"
#include "NL/nlString.h"
#include "NL/platqmath.h"

#include "Game/Sys/eventman.h"

#include "Game/Camera/animcam.h"
#include "Game/Camera/rumblefilter.h"
#include "Game/Camera/GameplayCam.h"
#include "NL/nlBasicString.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/Field.h"
#include "Game/Net.h"
#include "Game/WorldManager.h"
#include "Game/AI/AiUtil.h"

#include "math.h"

extern float g_fSimulationTick;

static f32 CANT_COLLIDE = *(f32*)__float_max;

eCameraType g_eCurrentCameraType;
cBaseCamera* cCameraManager::m_cameraStack;
float cCameraManager::m_fTransitionSpeed;
float cCameraManager::m_fPrevFOV;
eCameraTransition cCameraManager::m_transition;
u16 cCameraManager::m_aJoystickRemap;
void (*cCameraManager::m_pCallback)(enum eCameraMessage);
int cCameraManager::m_UpVectorStackSize;

nlMatrix4 cCameraManager::m_matView;
nlVector3 cCameraManager::m_cameraPosition;
nlMatrix4 cCameraManager::m_matPrevView;
int cCameraManager::m_pBeginFrameCameraType = 13;
float cCameraManager::m_fTransitionTime = 1.0f;
float cCameraManager::m_fFOV = 50.0f;

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
 * TODO: 99.65% match - second formatted camera-name temporary uses r31/r30 instead of r30/r31
 */
void cCameraManager::Startup()
{
    int i;
    cRumbleFilter* pFilter1;
    cRumbleFilter* pFilter2;
    cBaseCamera* pBaseCamera = (cBaseCamera*)new ((GameplayCamera*)nlMalloc(sizeof(GameplayCamera), 8, false)) GameplayCamera();
    pBaseCamera->m_pFilter = pRumbleFilter = new (nlMalloc(sizeof(cRumbleFilter), 8, false)) cRumbleFilter();

    if (m_transition != eCT_NONE)
    {
        nlPrintf("Camera Transition In Progress\n");
        if (m_pCallback != NULL)
            m_pCallback(eCM_ABORTED_BY_PUSH);
    }
    m_transition = eCT_NONE;

    if (nlDLRingGetStart<cBaseCamera>(m_cameraStack) != NULL)
    {
        if (nlDLRingGetStart<cBaseCamera>(m_cameraStack)->m_pFilter != NULL)
        {
            pFilter1 = nlDLRingGetStart<cBaseCamera>(m_cameraStack)->m_pFilter;
            pFilter2 = nlDLRingGetStart<cBaseCamera>(m_cameraStack)->m_pFilter;
            float dy = pFilter2->v2Pos0.f.y - pFilter1->v2Pos1.f.y;
            float dx = pFilter2->v2Pos0.f.x - pFilter1->v2Pos1.f.x;
            if (nlSqrt(dx * dx + dy * dy, true) > 0.0f)
            {
                g_pEventManager->CreateValidEvent(0x58, 0x14);
            }
        }
    }

    nlDLRingAddStart<cBaseCamera>(&m_cameraStack, pBaseCamera);
    g_eCurrentCameraType = pBaseCamera->GetType();

    cAnimCamera::LoadCameraAnimation("art/cameras/startscreen.cam", "startscreen", true);
    cAnimCamera::LoadCameraAnimation("art/cameras/ShootToScoreCamera.cam", "ShootToScoreCamera", true);

    for (i = 0; i < NUM_TEAMS; i++)
    {
        BasicString<char, Detail::TempStringAllocator> fileName = Format(BasicString<char, Detail::TempStringAllocator>("art/cameras/{0}_shoottoscorecamera.cam"), GetTeamName((eTeamID)i));
        BasicString<char, Detail::TempStringAllocator> camName = Format(BasicString<char, Detail::TempStringAllocator>("{0}_ShootToScoreCamera"), GetTeamName((eTeamID)i));
        cAnimCamera::LoadCameraAnimation(fileName.c_str(), camName.c_str(), true);
    }

    cAnimCamera::LoadCameraAnimation("art/cameras/pause.cam", "pause", true);

    Update(0.017f);
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

void cCameraManager::Update(float fDeltaT)
{
    nlVector3 v3TransTo;
    nlVector3 v3TransFrom;
    nlQuaternion qSlerped;
    nlQuaternion qCur;
    nlQuaternion qPrev;
    nlMatrix4 cameraToWorldMatrix;
    nlMatrix4 prevViewCopy;
    nlMatrix4 curViewCopy;
    nlMatrix4 filteredViewNone;
    nlMatrix4 filteredViewEase;

    if (m_cameraStack == NULL)
        return;
    UpdateGameCameraType();

    cBaseCamera* pCamera = nlDLRingGetStart<cBaseCamera>(m_cameraStack);
    pCamera->mUpVector = m_UpVectorStack[m_UpVectorStackSize];
    pCamera->Update(fDeltaT);
    if (pCamera->m_pFilter != NULL)
        pCamera->m_pFilter->Update(fDeltaT);

    if (m_transition == eCT_NONE)
        goto handle_none;
    switch (m_transition)
    {
    case eCT_EASE_IN:
        goto handle_ease_in;
    default:
        goto handle_end;
    }

handle_ease_in:
{
    float fSimTick = g_fSimulationTick;
    pCamera = nlDLRingGetStart<cBaseCamera>(m_cameraStack);
    if (pCamera != NULL)
    {
        prevViewCopy = m_matPrevView;
        nlMatrixToQuat(qPrev, prevViewCopy);

        v3TransFrom = prevViewCopy.GetTranslation();

        curViewCopy = PeekCamera()->GetViewMatrix();
        if (PeekCamera()->m_pFilter != NULL)
        {
            PeekCamera()->m_pFilter->Filter(curViewCopy, filteredViewEase);
            curViewCopy = filteredViewEase;
        }
        nlMatrixToQuat(qCur, curViewCopy);

        float oneMinusT;
        float t = m_fTransitionTime;
        float smoothT = t * t * t * (t * (6.0f * t + (-15.0f)) + 10.0f);
        v3TransTo = curViewCopy.GetTranslation();
        nlQuatSlerp(qSlerped, qPrev, qCur, smoothT);
        oneMinusT = 1.0f - smoothT;

        nlQuatToMatrix(m_matView, qSlerped);
        m_matView.f.m41 = oneMinusT * v3TransFrom.f.x + smoothT * v3TransTo.f.x;
        m_matView.f.m42 = oneMinusT * v3TransFrom.f.y + smoothT * v3TransTo.f.y;
        m_matView.f.m43 = oneMinusT * v3TransFrom.f.z + smoothT * v3TransTo.f.z;
        m_matView.f.m44 = 1.0f;

        m_fFOV = Interpolate(m_fPrevFOV, pCamera->GetFOV(), smoothT);
        if (m_fFOV < 1.0f)
            m_fFOV = 1.0f;

        m_fTransitionTime = m_fTransitionTime + fSimTick * m_fTransitionSpeed;
        if (m_fTransitionTime > 1.0f)
        {
            m_transition = eCT_NONE;
            if (m_pCallback != NULL)
            {
                m_pCallback(eCM_COMPLETE);
                m_pCallback = NULL;
            }
        }
    }
    nlInvertRotTransMatrix(cameraToWorldMatrix, m_matView);
    m_cameraPosition = cameraToWorldMatrix.GetTranslation();
    goto handle_end;
}

handle_none:
{
    m_matView = pCamera->GetViewMatrix();
    m_cameraPosition = pCamera->GetCameraPosition();
    m_fFOV = pCamera->GetFOV();
    if (m_fFOV < 1.0f)
        m_fFOV = 1.0f;
    if (PeekCamera()->m_pFilter != NULL)
    {
        PeekCamera()->m_pFilter->Filter(m_matView, filteredViewNone);
        m_matView = filteredViewNone;
    }
}

handle_end:
    m_aJoystickRemap = (u16)(int)(nlATan2f(m_matView.f.m23, m_matView.f.m13) * 10430.378f);
    m_aJoystickRemap += 0x8000;
}

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

        WorldManager::s_World->HandleCameraSwitch();
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
            pBaseCamera = (cBaseCamera*)new ((FaceCam*)nlMalloc(0x80, 8, false)) FaceCam(2.0f);
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
 * TODO: 99.91% match - nlPrintf/float literal label relocations and filter pointer r23 vs r30
 */
/* static */ void cCameraManager::PushCameraWithTransition(cBaseCamera* pCamera, float fDuration, eCameraTransition transition, void (*pCallback)(eCameraMessage))
{
    if (cCameraManager::m_transition != eCT_NONE)
    {
        nlPrintf("Camera Transition In Progress\n");
        if (cCameraManager::m_pCallback != NULL)
        {
            (*cCameraManager::m_pCallback)(eCM_ABORTED_BY_PUSH);
        }
    }

    cCameraManager::m_matPrevView = cCameraManager::PeekCamera()->GetViewMatrix();
    cCameraManager::m_fPrevFOV = cCameraManager::PeekCamera()->GetFOV();

    if (cCameraManager::PeekCamera()->m_pFilter != NULL)
    {
        nlMatrix4 matView;
        cCameraManager::PeekCamera()->m_pFilter->Filter(cCameraManager::m_matPrevView, matView);
        cCameraManager::m_matPrevView = matView;
    }

    cCameraManager::m_transition = transition;
    cCameraManager::m_fTransitionSpeed = 1.0f / fDuration;
    cCameraManager::m_fTransitionTime = 0.0f;
    cCameraManager::m_pCallback = pCallback;

    if ((cCameraManager::PeekCamera() != NULL)
        && (cCameraManager::PeekCamera()->m_pFilter != 0))
    {
        cRumbleFilter* filter1 = cCameraManager::PeekCamera()->m_pFilter;
        cRumbleFilter* filter2 = cCameraManager::PeekCamera()->m_pFilter;

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
 */
/* static */ cBaseCamera* cCameraManager::PopCameraWithTransition(float fDuration, eCameraTransition transition, void (*pCallback)(eCameraMessage))
{
    if (cCameraManager::m_transition != eCT_NONE)
    {
        nlPrintf("Camera Transition In Progress\n");
        if (cCameraManager::m_pCallback != NULL)
        {
            (*cCameraManager::m_pCallback)(eCM_ABORTED_BY_POP);
        }
    }

    cCameraManager::m_matPrevView = cCameraManager::PeekCamera()->GetViewMatrix();
    cCameraManager::m_fPrevFOV = cCameraManager::PeekCamera()->GetFOV();

    if (cCameraManager::PeekCamera()->m_pFilter != NULL)
    {
        nlMatrix4 matView;
        cCameraManager::PeekCamera()->m_pFilter->Filter(cCameraManager::m_matPrevView, matView);
        cCameraManager::m_matPrevView = matView;
    }

    cCameraManager::m_transition = transition;
    cCameraManager::m_pCallback = pCallback;
    cCameraManager::m_fTransitionSpeed = 1.0f / fDuration;
    cCameraManager::m_fTransitionTime = 1.0f - cCameraManager::m_fTransitionTime;

    return cCameraManager::PerformCameraPop();
}

/**
 * Offset/Address/Size: 0x200 | 0x801A6888 | size: 0x390
 * TODO: 99.02% match - residual is an MWCC scheduler tiebreak in the cross-product
 * loop: target loads the delta .y pair before the .z pair (ours reversed) and emits
 * the -edgeX fneg after both fmuls (ours between them). Data flow + regs already match.
 */
unsigned char cCameraManager::IsObjectOccludingField(const DrawableObject* drawable)
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
        return false;
    if ((cameraPosition->f.x > goalLineX) && (objectPosition.f.x < -goalLineX))
        return false;
    if ((cameraPosition->f.y < -sidelineY) && (objectPosition.f.y > sidelineY))
        return false;
    if ((cameraPosition->f.y > sidelineY) && (objectPosition.f.y < -sidelineY))
        return false;

    if ((objectPosition.f.z - objectRadius) < 0.0f)
    {
        objectPosition.f.z += objectRadius - objectPosition.f.z;
    }

    nlVector3 fieldCorners[4];
    nlVector3 planeNormals[4];

    fieldCorners[0].f.x = -goalLineX;
    fieldCorners[0].f.y = -sidelineY;
    fieldCorners[0].f.z = 0.0f;
    fieldCorners[1].f.x = -goalLineX;
    fieldCorners[1].f.y = sidelineY;
    fieldCorners[1].f.z = 0.0f;
    fieldCorners[2].f.x = goalLineX;
    fieldCorners[2].f.y = sidelineY;
    fieldCorners[2].f.z = 0.0f;
    fieldCorners[3].f.x = goalLineX;
    fieldCorners[3].f.y = -sidelineY;
    fieldCorners[3].f.z = 0.0f;

    nlVector3* pCorner = &fieldCorners[0];
    nlVector3* pNormal;
    nlVector3* pNormals = &planeNormals[0];
    pNormal = pNormals;
    int i;
    for (i = 0; i < 4; i++)
    {
        int next = (i + 1) % 4;
        nlVector3 edge;
        nlVector3 delta;
        nlVec3Sub(edge, fieldCorners[next], *pCorner);
        nlVec3Sub(delta, *pCorner, *cameraPosition);
        nlVec3Cross(*pNormal, edge, delta);

        float invLength = nlRecipSqrt(
            pNormal->f.x * pNormal->f.x
                + pNormal->f.y * pNormal->f.y
                + pNormal->f.z * pNormal->f.z,
            true);
        nlVec3Scale(*pNormal, invLength);

        pCorner++;
        pNormal++;
    }

    nlVector3 objectDelta;
    nlVec3Sub(objectDelta, objectPosition, *cameraPosition);

    for (i = 0; i < 4; i++)
    {
        if (nlVec3DotProduct(pNormals[i], objectDelta) > objectRadius)
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

    nlVector3* const pUp = &m_UpVectorStack[0];

    pUp->f.x = 0.0f;
    pUp->f.y = fSin;
    pUp->f.z = fCos;

    nlSinCos(&fSin, &fCos, ((s32)(65536.0f * fYAxisTilt)) / 360);

    pUp->f.x = fSin;
    pUp->f.z = pUp->f.z * fCos;

    float xx = pUp->f.x * pUp->f.x;
    float yy = pUp->f.y * pUp->f.y;
    float zz = pUp->f.z * pUp->f.z;
    float temp_f1 = nlRecipSqrt(xx + yy + zz, true);
    nlVec3Scale(*pUp, temp_f1);
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

// Unreferenced helper; with -inline deferred this seeds the float pool in target order.
static void CameraMan_sdata2_stub(
    float& zero, float& angleScale, float& joystickScale, float& one, float& faceDistance, float& easeTen, float& easeNegFifteen, float& easeSix)
{
    zero = 0.0f;
    angleScale = 65536.0f;
    joystickScale = 10430.378f;
    one = 1.0f;
    faceDistance = 2.0f;
    easeTen = 10.0f;
    easeNegFifteen = -15.0f;
    easeSix = 6.0f;
}
