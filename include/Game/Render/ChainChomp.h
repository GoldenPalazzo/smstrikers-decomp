#ifndef _CHAINCHOMP_H_
#define _CHAINCHOMP_H_

#include "Game/Render/SkinAnimatedMovableNPC.h"

#include "Game/SHierarchy.h"
#include "Game/SAnim.h"
#include "Game/Physics/PhysicsNPC.h"
#include "Game/Team.h"
#include "Game/AI/Fielder.h"
#include "Game/PoseAccumulator.h"
#include "Game/Inventory.h"

enum eChainChompState
{
    CHAIN_STATE_HIDDEN = 0,
    CHAIN_STATE_FALL = 1,
    CHAIN_STATE_RECOVER = 2,
    CHAIN_STATE_CHASE = 3,
    CHAIN_STATE_LEAVE = 4,
};

void UpdateChainEmitter(EmissionController&);

class ChainChomp : public SkinAnimatedMovableNPC
{
public:
    ChainChomp(cSHierarchy&, int, PhysicsNPC&, cInventory<cSAnim>*);
    virtual ~ChainChomp();
    virtual SkinAnimatedNPC_Type GetSkinAnimatedNPC_Type() const { return SkinAnimatedNPC_CHAIN_CHOMP; };
    virtual void Update(float);
    static void CollisionCallback(PhysicsObject*, PhysicsObject*, const nlVector3&);
    void FindTarget(cTeam*);
    void Fall(cFielder*, cFielder*);
    void Hide(bool);
    bool IsHidden() const;
    virtual void Move(float);
    virtual void DrawShadow(const cPoseAccumulator&, const nlMatrix4&);
    void Leave();
    void EmitTrail();

    /* 0x80 */ cSAnim* mpIdleAnim;
    /* 0x84 */ cSAnim* mpRecoverAnim;
    /* 0x88 */ cSAnim* mpDropAnim;
    /* 0x8C */ cFielder* mpTarget;
    /* 0x90 */ eChainChompState meChainChompState;
    /* 0x94 */ Timer mtStateTimer;
    /* 0x98 */ SFXEmitter* mpInEffectSFX;
    /* 0x9C */ cFielder* mpThrower;
    /* 0xA0 */ int mnThrowerPadID;
}; // total size: 0xA4

#endif // _CHAINCHOMP_H_
