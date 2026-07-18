#ifndef _CHARACTERTRIGGERS_H_
#define _CHARACTERTRIGGERS_H_

#include "NL/nlMath.h"

enum eBallShotEffectType
{
    BALL_EFFECT_S2S_SUPER_SHOT = 0,
    BALL_EFFECT_S2S_SHOT = 1,
    BALL_EFFECT_PERFECT_SHOT = 2,
    BALL_EFFECT_PERFECT_PASS = 3,
    BALL_EFFECT_REGULAR_SHOT = 4,
    BALL_EFFECT_ONETIMER_SHOT = 5,
    BALL_EFFECT_CHIP_SHOT = 6,
    NUM_BALL_EFFECTS = 7,
};

class cCharacter;
class cPlayer;
class cFielder;
class EmissionController;

void KillSlideTackleTrail(cCharacter*);
void EmitSlideTackleTrail(cCharacter*);
void EmitTackleImpact(cPlayer*);
void EmitChainBite(cFielder*);
void KillStar(cFielder*);
void EmitStar(cFielder*);
void KillMushroom(cFielder*);
void EmitMushroom(cFielder*);
void EmitDust(cPlayer*, const char*);
void EmitTurbo(cPlayer*, const char*);
void KillWindup(cCharacter*, const char*, bool);
void KillWindups(cCharacter*);
void EmitWindupAtCharacter(cCharacter*, const char*);
void EmitWindupAtBall(cCharacter*, const char*);
void EmitShootToScoreHyperStrike(cFielder*);
void EmitGoalieCatch(cPlayer*, const char*, bool);
void EmitBallWallHit(const char*);
void CharacterElectrocutionEffect(cCharacter* pCharacter, const nlVector3& v3Position, const nlVector3& v3Normal);
void EmitUnFreeze(cPlayer*);
void EmitFreeze(cPlayer*);
void KillDaze(cPlayer*);
void EmitDaze(cPlayer*);
void EmitElectrocutionExplosion(cCharacter*);
void EmitSolidRumble(cPlayer*);
void KillBallShot(const char*, bool);
void EmitBallShot(cPlayer*, eBallShotEffectType, cPlayer*, bool);
void EmitBallPass(cPlayer*);
void EmitBallImpact(cPlayer*, bool);
float GetCurrentAnimTriggerTime(cCharacter*, unsigned long, unsigned int);
void GetAnimTriggerInfo(cCharacter*, int, bool (*)(float, float, unsigned long, float, void*), void*);
void CharacterTriggerHandler(unsigned int);
EmissionController* EmitGeneric(cCharacter*, const char*, const char*);
void UpdateEmitterFromBall(EmissionController&);
void UpdateEmitterFromCharacterIdxWithCoordSys(EmissionController&, int);
void UpdateEmitterFromCharacterIdxWithoutAnimController(EmissionController&, int);

#endif // _CHARACTERTRIGGERS_H_
