#ifndef _CHARACTER_H_
#define _CHARACTER_H_

#include "types.h"
#include "NL/nlMath.h"

#include "Game/AI/HeadTrack.h"
#include "Game/SAnim/pnSAnimController.h"

#include "Game/CharacterEffects.h"
#include "Game/Blinker.h"

#include "Game/Physics/PhysicsCharacter.h"
#include "Game/CharacterAudio.h"

#include "Game/PoseNode.h"
#include "Game/PoseAccumulator.h"

#include "Game/ObjectBlur.h"
#include "Game/GL/gluSkinMesh.h"

class Event;

#include "Game/EventDataTypes.h"

// Forward declarations
class cSHierarchy;
class cAnimInventory;
class CharacterPhysicsData;
class AnimRetargetList;
// class cPoseAccumulator;
class GLSkinMesh;
class glModel;
class EmissionController;
class EffectsGroup;
class cTeam;
class cPlayer;

enum eCharacterClass
{
    CHARACTER_CLASS_INVALID = -1,
    BIRDO = 0,
    DAISY = 1,
    DONKEYKONG = 2,
    HAMMERBROS = 3,
    KOOPA = 4,
    LUIGI = 5,
    MARIO = 6,
    PEACH = 7,
    TOAD = 8,
    WALUIGI = 9,
    WARIO = 10,
    YOSHI = 11,
    MYSTERY = 12,
    NUM_FIELDER_CLASSES = 13,
    DAISY_GOALIE = 13,
    DONKEYKONG_GOALIE = 14,
    LUIGI_GOALIE = 15,
    MARIO_GOALIE = 16,
    PEACH_GOALIE = 17,
    WALUIGI_GOALIE = 18,
    WARIO_GOALIE = 19,
    YOSHI_GOALIE = 20,
    SUPERTEAM_GOALIE = 21,
    NUM_CHARACTER_CLASSES = 22,
    NUM_GOALIE_CLASSES = 9,
};

enum eMovementState
{
    MOVEMENT_COAST = 0,
    MOVEMENT_DECELERATE_EXPONENTIAL = 1,
    MOVEMENT_FROM_ANIM = 2,
    MOVEMENT_FROM_ANIM_SEEK = 3,
    MOVEMENT_NONE = 4,
    MOVEMENT_RUNNING = 5,
    MOVEMENT_RUNNING_NO_TURN = 6,
    MOVEMENT_STRAFING = 7,
    MOVEMENT_UNUSED = 8,
};

enum eClassTypes
{
    CHARACTER = 0,
    PLAYER = 1,
    FIELDER = 2,
    GOALIE = 3,
    NUM_CLASSES = 4,
};

enum eCharacterModelType
{
    CharModel_Rigid = 0,
    CharModel_Blend = 1,
    CharModel_Num = 2,
};

#include "Game/Drawable/DrawableObj.h"

class cCharacter //: public PhysicsCharacterBase
{
public:
    virtual ~cCharacter();
    cCharacter(eCharacterClass, const int*, cSHierarchy*, cAnimInventory*, const CharacterPhysicsData*, float, float, AnimRetargetList*,
        eClassTypes);

    virtual void PostPhysicsUpdate();
    virtual void PrePhysicsUpdate(float);
    virtual void PreUpdate(float);
    virtual void SetAnimID(int);

    void AddRandomDirt();
    void SetElectrocutionTextureEnabled(bool);
    void PerformBlinking(GLSkinMesh*, glModel*) const;
    void UpdateBlinking(float);
    void PlayRandomCharDialogue(unsigned long, PosUpdateMethod, float, float);
    int Play3DSFX(Audio::eCharSFX, PosUpdateMethod, float);
    void StopPlayingAllTrackedSFX();
    void StopSFX(Audio::eCharSFX);
    int PlaySFX(Audio::SoundAttributes&);
    void SetSFX(SoundPropAccessor*);
    void UpdateMovementState(float);
    bool IsPlayingEffect(const EffectsGroup*) const;
    void EndEffect(const EffectsGroup*);
    void KillEffect(const EffectsGroup*);
    bool ShouldStartCrossBlend(int);
    void SetVelocity(const nlVector3&);
    virtual void SetPosition(const nlVector3&);
    void SetFacingDirection(unsigned short);
    void SetAnimState(int animID, bool useBlendTime, float fNonDefaultBlendTime, bool bRestartCyclic, bool bForceMirrorSwap);
    float SeekSpeedExponential(float currentValue, float targetValue, float responsiveness, float deltaTime);
    void ResetEffects();
    void CreateWorldMatrix();
    void PoseSkinMesh(cPoseAccumulator*);
    void PoseLocalSpace();
    cPN_SAnimController* NewAnimController(int, bool, bool, void (*)(unsigned int, cPN_SAnimController*), unsigned int);
    static void MatchAnimSpeedToCharacterSpeed(unsigned int nParam, cPN_SAnimController* pController);
    void InitMovementStrafing(float fDirectionSeekSpeed, float fDirectionSeekFalloff, float fAccel, float fDecel);
    void InitMovementRunningNoTurn(float, float);
    void InitMovementRunning(float, float, float, float);
    void InitMovementNone(float, float);
    void InitMovementFromAnimSeek(float, float);
    void InitMovementFromAnim(short, const nlVector3&, float, bool);
    void InitMovementDecelerateExponential(float);
    void InitMovementCoast();
    void EndBlur();
    void InitBlur(int);
    nlVector3& GetPrevJointPosition(int jointIndex);
    void GetCurrentAnimFuture(int, float, nlVector3&, nlVector3&, unsigned short&);
    void GetJointPositionFuture(nlVector3* v3Out, int nAnimIndex, int nJointIndex, float fTime, bool bAddRootTrans, bool bAddRootRot, bool bUsePrevPosition);
    nlVector3& GetJointPosition(int) const;
    s16 GetFacingDeltaToPosition(const nlVector3&);
    s16 CalcAnimTurnAdjust(unsigned short, unsigned short, int);
    void AttachEffect(EmissionController*);
    void AdjustPoseMatrices();
    GLSkinMesh* GetSkinMesh() const;

    int GetHeadJointIndex() const // does not omit code?
    {
        return m_nHeadJointIndex;
    }

    inline bool IsTurboingAnimID()
    {
        bool result = false;
        if (m_eAnimID == 0x1D || m_eAnimID == 0x1E || m_eAnimID == 0x1F || m_eAnimID == 0x20 || m_eAnimID == 0x21 || m_eAnimID == 0x22)
        {
            result = true;
        }
        return result;
    }

    inline bool IsTurboingWithoutBallAnimID()
    {
        bool result = false;
        if (m_eAnimID == 0x10 || m_eAnimID == 0x0F || m_eAnimID == 0x11)
        {
            result = true;
        }
        return result;
    }

    virtual void Update(float);

    /* 0x04 */ eCharacterClass m_eCharacterClass;
    /* 0x08 */ const CharacterPhysicsData* m_pPhysicsData;
    /* 0x0C */ GLSkinMesh* m_pSkinMesh[2];
    /* 0x14 */ PhysicsCharacter* m_pPhysicsCharacter;
    /* 0x18 */ nlVector3 m_v3Position;
    /* 0x24 */ nlVector3 m_v3PrevPosition;
    /* 0x30 */ nlVector3 m_v3Velocity;
    /* 0x3C */ eMovementState m_eMovementState;
    /* 0x40 */ u16 m_aDesiredFacingDirection;
    /* 0x42 */ u16 m_aActualFacingDirection;
    /* 0x44 */ u16 m_aPrevFacingDirection;
    /* 0x46 */ u16 m_aDesiredMovementDirection;
    /* 0x48 */ u16 m_aActualMovementDirection;
    /* 0x4A */ bool m_bFromAnimBlended;
    /* 0x4C */ float m_fAnimAdjustBeginTime;
    /* 0x50 */ float m_fAnimAdjustEndTime;
    /* 0x54 */ float m_fDirectionSeekSpeed;
    /* 0x58 */ float m_fDirectionSeekFalloff;
    /* 0x5C */ float m_fAccel;
    /* 0x60 */ float m_fDecel;
    /* 0x64 */ float m_fDesiredSpeed;
    /* 0x68 */ float m_fActualSpeed;
    /* 0x6C */ float m_fLeanAmount;
    /* 0x70 */ s16 m_nAnimTurnAdjust;
    /* 0x74 */ nlVector3 m_v3AnimMoveAdjust;
    /* 0x80 */ cAnimInventory* m_pAnimInventory;
    /* 0x84 */ cPoseAccumulator* m_pPoseAccumulator;
    /* 0x88 */ cPoseNode* m_pPoseTree;
    /* 0x8C */ cPoseNode** m_pAILayer;
    /* 0x90 */ cPN_SAnimController* m_pCurrentAnimController;
    /* 0x94 */ s32 m_eAnimID;
    /* 0x98 */ AnimRetargetList* m_pAnimRetargetList;
    /* 0x9C */ cHeadTrack* m_pHeadTrack;
    /* 0xA0 */ s32 m_nHeadJointIndex;
    /* 0xA4 */ s32 m_nBip01JointIndex_0xA4; // strange, exists twice... (in character and player)
    /* 0xA8 */ s32 m_nPropJointIndex;
    /* 0xAC */ s32 m_nSpine1JointIndex;
    /* 0xB0 */ const char* m_szEffectsName;
    /* 0xB4 */ eClassTypes m_eClassType;
    /* 0xB8 */ bool m_bIsUsingElectrocutionTexture;
    /* 0xBC */ Audio::cCharacterSFX* m_pCharacterSFX;
    /* 0xC0 */ DrawableObject* m_pPropModel;
    /* 0xC4 */ u32 m_uNormalTextureID;
    /* 0xC8 */ u32 m_uSwapTextureID;

    /* 0xCC */ nlMatrix4 m_m4WorldMatrix;
    /* 0x10C */ float m_Dirt;
    /* 0x110 */ float m_MinDirt;
    /* 0x114 */ BlurHandler* m_pBlurHandler;
    /* 0x118 */ Blinker* m_pBlinker;
    /* 0x11C */ EffectsTexturing* m_pEffectsTexturing;
    /* 0x120 */ nlVector3 m_v3ScreenPosition;

    inline cPoseNode* GetAILayer() const
    {
        return m_pAILayer[0];
    }

    static eCharacterModelType m_ModelType;
};

// class cPN_SAnimController
// {
// public:
//     ~cPN_SAnimController();
// };

// class cHeadTrack
// {
// public:
//     virtual ~cHeadTrack();
// };

// class PhysicsCharacter
// {
// public:
//     ~PhysicsCharacter();
// };

// class GLSkinMesh
// {
// public:
//     virtual ~GLSkinMesh();
//     virtual void fnc_unk08();
//     virtual void PoseSkinMesh(cPoseAccumulator*);
//     // ... todo: wrong place, and more virtual fnc

//     /* 0x04 */ glModel* pModel;
// };

#endif // _CHARACTER_H_
