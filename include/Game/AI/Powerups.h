#ifndef _POWERUPS_H_
#define _POWERUPS_H_

#include "NL/nlMath.h"
#include "NL/nlTimer.h"
#include "Game/Physics/PhysicsObject.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/ObjectBlur.h"
#include "Game/Sys/eventman.h"

class cFielder;
class cPlayer;
class cTeam;
class BlurHandler;
class SFXEmitter;
class Bowser;

struct PowerupSounds
{
    /* 0x00 */ unsigned long sndAcquire;
    /* 0x04 */ unsigned long sndActivate;
    /* 0x08 */ unsigned long sndInEffect;
    /* 0x0C */ unsigned long sndHit;
    /* 0x10 */ unsigned long sndBounceWall;
    /* 0x14 */ unsigned long sndBounceGround;
    /* 0x18 */ unsigned long sndExplode;
    /* 0x1C */ unsigned long sndEnd;
}; // total size: 0x20

struct PowerupModelPool
{
    void Initialize(int type, unsigned long objHashName);

    /* 0x000 */ int mNum;
    /* 0x004 */ DrawableObject* mObjs[6][25];
    /* 0x25C */ bool mFree[6][25];
}; // total size: 0x2F4

enum ePowerUpType
{
    POWER_UP_NONE = -1,
    POWER_UP_GREEN_SHELL = 0,
    POWER_UP_RED_SHELL = 1,
    POWER_UP_SPINY_SHELL = 2,
    POWER_UP_FREEZE_SHELL = 3,
    POWER_UP_BANANA = 4,
    POWER_UP_BOBOMB = 5,
    POWER_UP_CHAIN_CHOMP = 6,
    NUM_DRAWABLE_POWER_UPS = 6,
    POWER_UP_MUSHROOM = 7,
    POWER_UP_STAR = 8,
    NUM_POWER_UPS = 9,
};

enum ePowerupSize
{
    POWERUPSIZE_SMALL = 0,
    POWERUPSIZE_MEDIUM = 1,
    POWERUPSIZE_LARGE = 2,
};

struct PowerUpTeamType
{
    /* 0x0 */ ePowerUpType eType;
    /* 0x4 */ int nnumOfPowerups;
    /* 0x8 */ unsigned char bIsNew;
}; // total size: 0xC

enum eThrowStyle
{
    THROW_ARROW = 0,
    THROW_HORIZONTAL_LINE = 1,
    THROW_SURROUND = 2,
    THROW_SPREAD = 3,
    NUM_THROW_TYPES = 4,
};

class PowerupBase
{
public:
    enum PowerupSound
    {
        PWRUP_SOUND_ACQUIRE = 0,
        PWRUP_SOUND_ACTIVATE = 1,
        PWRUP_SOUND_IN_EFFECT = 2,
        PWRUP_SOUND_HIT = 3,
        PWRUP_SOUND_BOUNCE_WALL = 4,
        PWRUP_SOUND_BOUNCE_GROUND = 5,
        PWRUP_SOUND_EXPLODE = 6,
        PWRUP_SOUND_END = 7,
    };

    PowerupBase(cFielder*, ePowerUpType, float, ePowerupSize, bool, int);
    /* 0x08 */ virtual ~PowerupBase();
    /* 0x0C */ virtual void Destroy(bool);
    /* 0x10 */ virtual void PreThrow(cFielder*, Bowser*);
    /* 0x18 */ virtual void ThrowAt(cFielder*, Bowser*);
    /* 0x14 */ virtual void Init(cFielder*, Bowser*);
    /* 0x1C */ virtual void Update(float);

    float GetRadius() const;
    static int AwardPowerup(cTeam*);
    static void CollisionCallback(PhysicsObject*, PhysicsObject*, const nlVector3&, void*);
    void UpdateTransform();
    static unsigned long PlayPowerupSound(ePowerUpType, PowerupBase::PowerupSound, const nlVector3&, float);
    static unsigned long PlayPowerupSound(ePowerUpType, PowerupBase::PowerupSound, PhysicsObject*, float);
    static void StopPowerupInEffectSound(SFXEmitter*);

    /* 0x04 */ bool m_bShouldDestroy;
    /* 0x08 */ DrawableObject* m_pDrawableObj;
    /* 0x0C */ PhysicsObject* m_pPhysicsObject;
    /* 0x10 */ cFielder* m_pTarget;
    /* 0x14 */ cFielder* m_pThrower;
    /* 0x18 */ ePowerUpType m_eType;
    /* 0x1C */ Timer mtActiveTimer;
    /* 0x20 */ Timer mtNoHitTimer;
    /* 0x24 */ u16 m_aOrientation;
    /* 0x28 */ f32 m_scale;
    /* 0x2C */ nlVector3 m_v3Position;
    /* 0x38 */ nlVector3 m_v3PrevPosition;
    /* 0x44 */ nlVector3 m_v3Velocity;
    /* 0x50 */ const char* m_szStreakTexture;
    /* 0x54 */ f32 m_fBlurWidth;
    /* 0x58 */ f32 m_fBlurLength;
    /* 0x5C */ u32 m_uVoiceID;
    /* 0x60 */ BlurHandler* m_pBlurHandler;
    /* 0x64 */ s32 m_nIndex;
    /* 0x68 */ s32 m_nThrowerPadID;
    /* 0x6C */ ePowerupSize meSize;
    /* 0x70 */ bool mbExploder;
}; // total size: 0x74

class Bobomb : public PowerupBase
{
public:
    virtual ~Bobomb();
    virtual void Update(float);
    virtual void ThrowAt(cFielder*, Bowser*);
    void Destroy(bool);

    /* 0x74 */ bool mbIsMine;
    /* 0x78 */ SFXEmitter* pMovementEmitter;

    static SlotPool<Bobomb> m_BobombSlotPool;
}; // total size: 0x7C

class FreezeShell : public PowerupBase
{
public:
    virtual ~FreezeShell();
    virtual void Update(float);
    void Destroy(bool);

    static SlotPool<FreezeShell> m_FreezeShellSlotPool;
}; // total size: 0x74

class SpinyShell : public PowerupBase
{
public:
    virtual ~SpinyShell();
    virtual void Update(float);
    void Destroy(bool);

    static SlotPool<SpinyShell> m_SpinyShellSlotPool;
}; // total size: 0x74

class Banana : public PowerupBase
{
public:
    virtual ~Banana();
    virtual void Update(float);
    void Destroy(bool);

    static SlotPool<Banana> m_BananaSlotPool;
}; // total size: 0x74

class RedShell : public PowerupBase
{
public:
    virtual ~RedShell();
    virtual void Update(float);
    void Destroy(bool);
    void SeekTarget();

    static SlotPool<RedShell> m_RedShellSlotPool;
}; // total size: 0x74

class GreenShell : public PowerupBase
{
public:
    virtual ~GreenShell();
    virtual void Update(float);
    void Destroy(bool);

    static SlotPool<GreenShell> m_GreenShellSlotPool;
}; // total size: 0x74

void CompactPowerups();
static void InitializePowerups();
static PowerupBase* FindPowerUp(unsigned long hashOfDrawable);
void PowerupCreateAndThrow(cFielder*, ePowerUpType, int, Bowser*);
void PowerupThrowPosition(int nThrowOrder, eThrowStyle eStyle, PowerupBase* pNewPowerup, PowerupBase* pFirstPowerup);
cFielder* FindPowerupTarget(cFielder*, Bowser*);

// class PhysicsShell
// {
// public:
//     ~PhysicsShell();
// };

// class PowerupModelPool
// {
// public:
//     void Initialize(int, unsigned long);
// };

// class DrawableObject
// {
// public:
//     void Clone() const;
// };

struct PowerupUsedEventData : public EventData
{
    /* 0x04 */ ePowerUpType Type;
    /* 0x08 */ cPlayer* Thrower;
    /* 0x0C */ cPlayer* Target;
    u32 GetID();
};

struct PowerupHitPlayerEventData : public EventData
{
    /* 0x04 */ ePowerUpType Type;
    /* 0x08 */ cPlayer* Thrower;
    /* 0x0C */ cPlayer* Target;
    u32 GetID();
};

// class Format < BasicString < char, Detail
// {
// public:
//     void TempStringAllocator >, int > (const BasicString<char, Detail::TempStringAllocator>&, const int&);
// };

// class FormatImpl < BasicString < char, Detail
// {
// public:
//     void TempStringAllocator >> ::operator% <int>(const int&);
// };

// class SlotPool<Bobomb>
// {
// public:
//     void ~SlotPool();
// };

// class SlotPool<Banana>
// {
// public:
//     void ~SlotPool();
// };

// class SlotPool<RedShell>
// {
// public:
//     void ~SlotPool();
// };

// class SlotPool<SpinyShell>
// {
// public:
//     void ~SlotPool();
// };

// class SlotPool<GreenShell>
// {
// public:
//     void ~SlotPool();
// };

// class SlotPool<FreezeShell>
// {
// public:
//     void ~SlotPool();
// };

#endif // _POWERUPS_H_
