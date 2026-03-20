#ifndef _CAMERAMAN_H_
#define _CAMERAMAN_H_

#include "types.h"
#include "NL/nlMath.h"

#include "Game/Camera/BaseCamera.h"

enum eCameraTransition
{
    eCT_NONE = 0,
    eCT_EASE_IN = 1,
};

enum eCameraMessage
{
    eCM_ABORTED_BY_POP = 0,
    eCM_ABORTED_BY_PUSH = 1,
    eCM_COMPLETE = 2,
};

class DrawableObject;

void FireCameraRumbleFilter(float, float);

class cCameraManager
{
public:
    static void Startup();
    static void Shutdown();
    void Update(float);
    void UpdateGameCameraType();
    static bool HasCamera(cBaseCamera* pCamera);
    static void PushCamera(cBaseCamera* pCamera);
    static void Remove(const cBaseCamera& camera);
    static void Remove(eCameraType type, bool bDeleteAfterRemoving);
    static void PushCameraWithTransition(cBaseCamera*, float, eCameraTransition, void (*)(eCameraMessage));
    static void PopCameraWithTransition(float, eCameraTransition, void (*)(eCameraMessage));
    static bool IsObjectOccludingField(const DrawableObject*);
    static float GetDistanceFromCameraToObject(const nlVector3& objectPosition);
    static void GetViewVector(nlVector3& viewVector);
    static void GetUpVector(nlVector3& upVector);
    static void SetWorldUpVectorTilt(float fXAxisTilt, float fYAxisTilt);
    static void PushWorldUpVector();
    static void PopWorldUpVector();

    template <typename T>
    static T* GetCamera(eCameraType type);
    // {
    //     if (m_cameraStack == nullptr)
    //     {
    //         return nullptr;
    //     }
    //     cBaseCamera* pCamera = m_cameraStack;
    //     do
    //     {
    //         pCamera = pCamera->m_next;
    //         if (pCamera->GetType() == type)
    //         {
    //             return static_cast<T*>(pCamera);
    //         }
    //     } while (pCamera != m_cameraStack);
    //     return nullptr;
    // }

    static u16 m_aJoystickRemap;
    static cBaseCamera* m_cameraStack;
    static eCameraTransition m_transition;
    static void (*m_pCallback)(enum eCameraMessage);

    static nlMatrix4 m_matView;
    static nlVector3 m_cameraPosition;
    static nlMatrix4 m_matPrevView;

    static nlVector3 m_UpVectorStack[2];
    static int m_UpVectorStackSize;
};

#endif // _CAMERAMAN_H_
