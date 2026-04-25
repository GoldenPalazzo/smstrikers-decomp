#include "types.h"
#pragma pool_data off

#include "Game/GameTweaks.h"

#include "NL/nlString.h"
#include "NL/nlConfig.h"
#include "NL/nlMemory.h"
#include "NL/nlPrint.h"

#include "Game/GameInfo.h"

SkillTweaks g_sSkillTweaks[2];
const char* g_sDifficultyFileNames[7] = {
    "DifficultyBraindead.ini",
    "Difficulty0.ini",
    "Difficulty1.ini",
    "Difficulty2.ini",
    "Difficulty3.ini",
    "DifficultySuperhuman.ini",
    "DifficultyHuman.ini",
};

extern char* GetPowerupName(int powerup);

/**
 * Offset/Address/Size: 0x2638 | 0x80042824 | size: 0x50
 */
GameTweaks::GameTweaks(const char* name)
    : TweaksBase(name)
{
    Init();
}

/**
 * Offset/Address/Size: 0x122C | 0x80041418 | size: 0x140C
 */
void GameTweaks::Init()
{
    Config cfg(Config::ALLOCATE_HIGH);
    cfg.LoadFromFile(mszFileName);

    const GameplaySettings& gameSettings = GameInfoManager::s_pInstance->GetGameplayOptions();

    if (Config::Global().Exists("Game Duration"))
    {
        fGameDuration = Config::Global().Get<float>("Game Duration", 0.0f);
    }
    else
    {
        fGameDuration = gameSettings.GameTime;
    }

    fBlockPassLightAttackSpeed = cfg.Get<float>("Block Ball with LAttack under this Speed", 5.0f);
    fFielderAttributeWeight = cfg.Get<float>("Fielder Attributes Weight", 0.2f);
    vGetInPositionKeyFielderDist.f.x = cfg.Get<float>("GetInPosition KeyFielder Min Distance", 2.0f);
    vGetInPositionKeyFielderDist.f.y = cfg.Get<float>("GetInPosition KeyFielder Max Distance", 12.0f);
    vGetInPositionInRadius.f.x = cfg.Get<float>("At Target Position Min Radius", 0.3f);
    vGetInPositionInRadius.f.y = cfg.Get<float>("At Target Position Max Radius", 0.6f);
    vGetInPositionOutRadius.f.x = cfg.Get<float>("At Target Remain At Position Min Radius", 1.0f);
    vGetInPositionOutRadius.f.y = cfg.Get<float>("At Target Remain At Position Max Radius", 4.5f);
    nStrafeToRunInDirectionDelta = (s16)cfg.Get<int>("Run To Strafe Angle", 0x2CEC);
    nBackwardsToStrafeRunInDirectionDelta = (s16)cfg.Get<int>("Backwards To Strafe Angle", 0x59D8);
    nStrafeToRunOutDirectionDelta = (s16)cfg.Get<int>("Strafe To Run Angle", 0x2328);
    nBackwardsToStrafeRunOutDirectionDelta = (s16)cfg.Get<int>("Strafe To Backwards Angle", 0x61A8);
    fSlideAttackRadius = cfg.Get<float>("Do Slide Attack Radius", 2.15f);
    fArrivalInRadius = cfg.Get<float>("Arrived In Radius", 0.45f);
    fArrivalOutRadius = cfg.Get<float>("Arrived Out Radius", 0.85f);
    fNearSeekInRadius = cfg.Get<float>("Near In Radius", 1.55f);
    fNearSeekOutRadius = cfg.Get<float>("Near Out Radius", 2.0f);
    fPassSpeedMaxDist = cfg.Get<float>("Pass Speed Max Dist", 7.5f);
    fPassSpeedMinDist = cfg.Get<float>("Pass Speed Min Dist", 1.0f);
    fSwapControllerTime = cfg.Get<float>("Swap Controller No Swap Back Time", 0.3f);
    fSwapFacingTime = cfg.Get<float>("Swap Controller Dampen Facing Time", 0.5f);
    fGreenShellActiveTime = cfg.Get<float>("Green Shell Time", 2.0f);
    fGreenShellSpeed = cfg.Get<float>("Green Shell Speed", 18.0f);
    fRedShellActiveTime = cfg.Get<float>("Red Shell Time", 1.5f);
    fRedShellSpeed = cfg.Get<float>("Red Shell Speed", 12.5f);
    fSpinyShellActiveTime = cfg.Get<float>("Spiny Shell Time", 1.5f);
    fSpinyShellSpeed = cfg.Get<float>("Spiny Shell Speed", 12.5f);
    fFreezeShellActiveTime = cfg.Get<float>("Freeze Shell Time", 1.5f);
    fFreezeShellSpeed = cfg.Get<float>("Freeze Shell Speed", 12.5f);
    fFreezeShellFrozenTime = cfg.Get<float>("Freeze Shell Frozen Time", 1.5f);
    fBananaActiveTime = cfg.Get<float>("Banana Time", 5.0f);
    fBananaSpeed = cfg.Get<float>("Banana Speed", 8.5f);
    fBobombActiveTime = cfg.Get<float>("Bobomb Time", 8.5f);
    fBobombSpeed = cfg.Get<float>("Bobomb Speed", 8.5f);
    fBobombMaxZSpeed = cfg.Get<float>("Bobomb Max Z Speed", 8.5f);
    fChainChompActiveTime = cfg.Get<float>("ChainChomp Time", 8.5f);
    fChainChompFallTime = cfg.Get<float>("ChainChomp Fall Time", 8.5f);
    fChainChompSpeed = cfg.Get<float>("ChainChomp Speed", 8.5f);
    nChanceForRedShell = cfg.Get<int>("Red Shell Chance", 0);
    nChanceForGreenShell = cfg.Get<int>("Green Shell Chance", 0);
    nChanceForSpinyShell = cfg.Get<int>("Spiny Shell Chance", 0);
    nChanceForFreezeShell = cfg.Get<int>("Freeze Shell Chance", 0);
    nChanceForMushroom = cfg.Get<int>("Mushroom Chance", 0);
    nChanceForBanana = cfg.Get<int>("Banana Chance", 0);
    nChanceForStar = cfg.Get<int>("Star Chance", 0);
    nChanceForBoBomb = cfg.Get<int>("BoBomb Chance", 0);
    nChanceForChainChomp = cfg.Get<int>("Chain Chomp Chance", 0);

    const PowerupSettings& powerupSettings = GameInfoManager::s_pInstance->GetPowerupOptions();

    if ((u32)powerupSettings.RedShells == 0)
    {
        nChanceForRedShell = 0;
    }
    if ((u32)powerupSettings.GreenShells == 0)
    {
        nChanceForGreenShell = 0;
    }
    if ((u32)powerupSettings.BlueShells == 0)
    {
        nChanceForFreezeShell = 0;
    }
    if ((u32)powerupSettings.SpinyShells == 0)
    {
        nChanceForSpinyShell = 0;
    }
    if ((u32)powerupSettings.Starman == 0)
    {
        nChanceForStar = 0;
    }
    if ((u32)powerupSettings.Twister == 0)
    {
        nChanceForRedShell = 0;
    }
    if ((u32)powerupSettings.Bobombs == 0)
    {
        nChanceForBoBomb = 0;
    }
    if ((u32)powerupSettings.Bananas == 0)
    {
        nChanceForBanana = 0;
    }

    nScoreDifferenceMaximum = cfg.Get<int>("Score Difference Maximum", 1);
    nScoreDifferenceMinimum = cfg.Get<int>("Score Difference Minimum", 5);
    nPowerupsNumForPenalty = cfg.Get<int>("Powerups Awarded Penalty", 2);
    fBananaResistance = cfg.Get<float>("Banana Resistance", 5.0f);
    fShellBounceGround = cfg.Get<float>("Shell Bounce Ground", 0.0f);
    fShellBounce = cfg.Get<float>("Shell Bounce Other", 1.0f);
    fShellSmallRadius = cfg.Get<float>("Shell Small Radius", 0.3f);
    fShellMediumRadius = cfg.Get<float>("Shell Medium Radius", 0.6f);
    fShellBigRadius = cfg.Get<float>("Shell Big Radius", 1.0f);
    fShellMediumChance = cfg.Get<float>("Shell Medium Chance", 0.33f);
    fShellBigChance = cfg.Get<float>("Shell Big Chance", 0.33f);
    fShellFiveChance = cfg.Get<float>("Shell Five Chance", 0.33f);
    fShellThreeChance = cfg.Get<float>("Shell Three Chance", 0.33f);
    fShellExplodeChance = cfg.Get<float>("Shell Explode Chance", 0.33f);
    fBananaSmallRadius = cfg.Get<float>("Banana Small Radius", 0.3f);
    fBananaMediumRadius = cfg.Get<float>("Banana Medium Radius", 0.6f);
    fBananaBigRadius = cfg.Get<float>("Banana Big Radius", 1.0f);
    fBananaMediumChance = cfg.Get<float>("Banana Medium Chance", 0.33f);
    fBananaBigChance = cfg.Get<float>("Banana Big Chance", 0.33f);
    fBananaFiveChance = cfg.Get<float>("Banana Five Chance", 0.33f);
    fBananaThreeChance = cfg.Get<float>("Banana Three Chance", 0.33f);
    fBananaExplodeChance = cfg.Get<float>("Banana Explode Chance", 0.33f);
    fBobombSmallRadius = cfg.Get<float>("Bobomb Small Radius", 0.3f);
    fBobombMediumRadius = cfg.Get<float>("Bobomb Medium Radius", 0.6f);
    fBobombBigRadius = cfg.Get<float>("Bobomb Big Radius", 1.0f);
    fBobombMediumChance = cfg.Get<float>("Bobomb Medium Chance", 0.33f);
    fBobombBigChance = cfg.Get<float>("Bobomb Big Chance", 0.33f);
    fBobombFiveChance = cfg.Get<float>("Bobomb Five Chance", 0.33f);
    fBobombThreeChance = cfg.Get<float>("Bobomb Three Chance", 0.33f);
    fBobombMineChance = cfg.Get<float>("Bobomb Mine Chance", 0.33f);
    fPowerupExplosionRadius = cfg.Get<float>("Powerup Explosion Radius", 2.0f);
    fPowerupArrowThrowChance = cfg.Get<float>("Powerup Arrow Throw Chance", 0.3f);
    fPowerupSpreadThrowChance = cfg.Get<float>("Powerup Spread Throw Chance", 0.3f);
    fPowerupSurroundThrowChance = cfg.Get<float>("Powerup Surround Throw Chance", 0.3f);
    fPowerupHorizontalLineThrowChance = cfg.Get<float>("Powerup H Line Throw Chance", 0.3f);
    fPowerupHitWithBallMinAmount = cfg.Get<float>("Powerup Hit With Ball Min", 0.1f);
    fPowerupHitWithBallMaxAmount = cfg.Get<float>("Powerup Hit With Ball Max", 0.15f);
    fPowerupHitNoBallMinAmount = cfg.Get<float>("Powerup Hit No Ball Min", 0.25f);
    fPowerupHitNoBallMaxAmount = cfg.Get<float>("Powerup Hit No Ball Max", 0.35f);
    fPowerupSlideWithBallMinAmount = cfg.Get<float>("Powerup Slide With Ball Min", 0.3f);
    fPowerupSlideWithBallMaxAmount = cfg.Get<float>("Powerup Slide With Ball Max", 0.3f);
    fPowerupSlideNoBallMinAmount = cfg.Get<float>("Powerup Slide No Ball Min", 0.3f);
    fPowerupSlideNoBallMaxAmount = cfg.Get<float>("Powerup Slide No Ball Max", 0.3f);
    fPowerupPowerShotMinAmount = cfg.Get<float>("Powerup Power Shot Min", 0.3f);
    fPowerupPowerShotMaxAmount = cfg.Get<float>("Powerup Power Shot Max", 0.3f);
    fPowerupInterceptPassMinAmount = cfg.Get<float>("Powerup Intercept Pass Min", 0.3f);
    fPowerupInterceptPassMaxAmount = cfg.Get<float>("Powerup Intercept Pass Max", 0.3f);
    fPowerupPerfectPassMinAmount = cfg.Get<float>("Powerup Perfect Pass Min", 0.3f);
    fPowerupPerfectPassMaxAmount = cfg.Get<float>("Powerup Perfect Pass Max", 0.3f);
    fPowerupContextDekeMinAmount = cfg.Get<float>("Powerup Context Deke Min", 0.3f);
    fPowerupContextDekeMaxAmount = cfg.Get<float>("Powerup Context Deke Max", 0.3f);
    fPowerupIconSpeed = cfg.Get<float>("Powerup Icon Speed", 0.2f);
    nPowerupBoxMaxActiveOnField = cfg.Get<int>("Powerup Box Max Amount", 2);
    fPowerupBoxDelayDropTime = cfg.Get<float>("Powerup Box Delay Drop Time", 10.0f);
    fChainChompRadius = cfg.Get<float>("Chain Chomp Radius", 0.3f);
    fDrawPowerupIconTime = cfg.Get<float>("Draw Powerup Icon Time", 1.5f);
    fHitCarrierWeighting = cfg.Get<float>("Hit Carrier Bonus", 1.0f);
    fAngleWeighting = cfg.Get<float>("Powerup Angle Weighting", 0.0025f);
    fDekeAngleWeighting = cfg.Get<float>("Deke Angle Weighting", 0.001f);
    fPassAngleWeighting = cfg.Get<float>("Pass Angle Weighting", 0.0025f);
    fVolleyPassAngleWeighting = cfg.Get<float>("Volley Pass Angle Weighting", 0.0025f);
    fVolleyPassMinDistance = cfg.Get<float>("Volley Pass Min Distance", 3.0f);
    fMushroomEffectTime = cfg.Get<float>("On Mushrooms Time", 1.0f);
    fMushroomSpeed = cfg.Get<float>("On Mushrooms Speed", 1.25f);
    fStarEffectTime = cfg.Get<float>("On Star Time", 1.25f);
    fStarSpeed = cfg.Get<float>("On Star Speed", 1.5f);

    unk1F0 = cfg.Get<float>("Front Audible Surr Dist", 100.0f);
    unk1F4 = cfg.Get<float>("Back Audible Surr Dist", 100.0f);
    unk1F8 = cfg.Get<float>("Speed Of Sound For Doppler", 200.0f);
    unk1FC = cfg.Get<float>("Emitter Dist From Listener Max Vol", 0.0f);
    unk200 = cfg.Get<float>("Max Aubible Emitter Distance", 100.0f);
    unk204 = cfg.Get<float>("Emitter Vol To Distance Value", 0.0f);
    unk208 = cfg.Get<float>("Slow Mo Filter Fade In Time", 0.3f);
    unk20C = cfg.Get<float>("Slow Mo Filter Fade Out Time", 0.2f);
    unk210 = cfg.Get<float>("Filter Min Freq Pct", 0.0f);
    unk214 = cfg.Get<float>("Filter Max Freq Pct", 1.0f);
    unk218 = cfg.Get<float>("Filter Dropoff Delay Time", 0.5f);
    unk21C = cfg.Get<float>("Filter Dropoff Time", 6.0f);
    unk220 = cfg.Get<float>("Dialogue Pitch Min Pct", 0.0f);
    unk224 = cfg.Get<float>("Dialogue Pitch Max Pct", 1.0f);
    unk228 = cfg.Get<float>("Min Bobomb Move SFX Activation Time", 2.5f);
    unk22C = cfg.Get<float>("Perfect Pass SFX Proximity Filter Dist Sq", 16.0f);
    unk230 = cfg.Get<float>("Small Powerup Volume Coefficient", 0.5f);
    unk234 = cfg.Get<float>("Medium Powerup Volume Coefficient", 0.75f);
    unk238 = cfg.Get<float>("Large Powerup Volume Coefficient", 1.0f);
    unk23C = cfg.Get<float>("Perfect Pass Trail SFX Fade Start Time", 0.5f);
    unk240 = cfg.Get<float>("Min Hit Intensity to play BODY_HIT_HARD SFX", 0.9f);
    unk244 = cfg.Get<float>("Slide Attack Hit Reaction Volume", 0.75f);
    unk248 = cfg.Get<float>("S2S Ball Hit Reaction Volume", 0.75f);
    unk24C = cfg.Get<float>("Bomb Hit Reaction Volume", 0.75f);
    unk250 = cfg.Get<float>("Bomb Shockwave Reaction Volume", 0.75f);
    unk254 = cfg.Get<float>("Small Shell Hit Reaction Volume", 0.75f);
    unk258 = cfg.Get<float>("Medium Shell Hit Reaction Volume", 0.75f);
    unk25C = cfg.Get<float>("Large Shell Hit Reaction Volume", 0.75f);
    unk264 = cfg.Get<float>("Goalie Drop Kick Hit Reaction Volume", 0.75f);
    unk268 = cfg.Get<float>("Ball Hit Wall Max Audible Velocity", 35.0f);
    unk26C = cfg.Get<float>("Ball Hit Wall Min Audible Velocity", 2.0f);
    unk270 = cfg.Get<float>("Ball Hit Wall Min Volume", 0.3f);
    unk274 = cfg.Get<float>("Ball Hit Wall Min Time Before Next Audio", 0.2f);
    unk278 = cfg.Get<float>("Ball Hit Net Max Audible Velocity", 35.0f);
    unk27C = cfg.Get<float>("Ball Hit Net Min Audible Velocity", 3.0f);
    unk280 = cfg.Get<float>("Ball Hit Net Min Volume", 0.3f);
    unk284 = cfg.Get<float>("Ball Hit Net Min Time Before Next Audio", 1.5f);
    unk1E4 = cfg.Get<float>("Perfect Pass Slow Mo", 0.5f);
    unk1E8 = cfg.Get<float>("S2S Slow Mo Min", 0.4f);
    unk1EC = cfg.Get<float>("S2S Slow Mo Max", 0.6f);
    unk288 = cfg.Get<float>("S2S Ball Blur Width", 0.3f);
    unk28C = cfg.Get<int>("S2S Ball Blur Length", 0x1E);
    unk290 = cfg.Get<float>("S2S 2 Button Press Yellow Dist", 0.05f);
    unk294 = cfg.Get<float>("S2S Perfect First Button Time", 0.1f);
    unk298 = cfg.Get<float>("S2S Perfect Second Button Time", 0.12f);
    unk29C = cfg.Get<float>("S2S Perfect Distance Min", 0.02f);
    unk2A0 = cfg.Get<float>("S2S Perfect Distance Max", 0.02f);
    unk2A4 = cfg.Get<float>("Slide Attack Maximum Time", 0.35f);
    unk2A8 = cfg.Get<float>("Slide Attack Fail Up Time", 0.35f);
    unk2AC = cfg.Get<float>("Slide Attack Deceleration", 0.5f);
    unk2B0 = cfg.Get<float>("Left Trigger Pressure", 0.7f);
    unk2B4 = cfg.Get<float>("Indicator Distance Above Head", 0.4525f);
    unk2B8 = cfg.Get<float>("Indicator Pixel Clearance", 6.0f);
    unk2BC = cfg.Get<float>("Clear Ball Min Ground Speed", 0.0f);
    unk2C0 = cfg.Get<float>("Clear Ball Max Ground Speed", 0.0f);
    unk2C4 = cfg.Get<float>("Clear Ball Min Z Speed", 0.0f);
    unk2C8 = cfg.Get<float>("Clear Ball Max Z Speed", 0.0f);
    unk2D0 = cfg.Get<float>("Shot Windup Seconds", 0.75f);
    unk2D4 = cfg.Get<float>("Shot Net Open Angle", 45.0f);
    unk2D8 = cfg.Get<float>("Shot Ratings Weight", 0.5f);
    unk2DC = cfg.Get<float>("Shot Net Open Weight", 0.0f);
    unk2E0 = cfg.Get<float>("Shot Player Distance Weight", 0.0f);
    unk2E4 = cfg.Get<float>("Chip Shot Goalie Out Weight", 0.0f);
    unk2E8 = cfg.Get<float>("Chip Shot Net Open Weight", 0.0f);
    unk2EC = cfg.Get<float>("Shot Meter Max Onetimer Speed", 0.0f);
    unk2F0 = cfg.Get<float>("Shot Offset From Post", 0.3f);
    unk2F4 = cfg.Get<float>("Chip Shot Offset From Post", 0.3f);
    unk2F8 = cfg.Get<float>("High Shot Distance", 18.0f);
    unk2FC = cfg.Get<float>("Shot Width Variance", 0.12f);
    unk300 = cfg.Get<float>("Shot Height Variance", 0.06f);
    unk304 = cfg.Get<float>("Bowser Radius", 1.0f);
    unk308 = cfg.Get<float>("Bowser Chance", 0.5f);
    unk30C = cfg.Get<float>("Bowser Start Time", 30.0f);
    unk310 = cfg.Get<float>("Bowser End Time", 60.0f);
    unk314 = cfg.Get<float>("Bowser Min Attack Time", 30.0f);
    unk318 = cfg.Get<float>("Bowser Max Attack Time", 30.0f);
    unk31C = (float)cfg.Get<int>("Bowser Tilt Time", 0xF);
    unk320 = cfg.Get<float>("Bowser Interval Delay", 2.0f);
    unk324 = cfg.Get<float>("Bowser Min Tilt Force", 0.1f);
    unk328 = cfg.Get<float>("Bowser Max Tilt Force", 0.2f);
    unk32C = cfg.Get<float>("Bowser Max Tilt", 4.0f);
    unk330 = cfg.Get<float>("Bowser Gravity", -20.0f);
    unk334 = cfg.Get<float>("Bowser Rebound", -0.4f);
    unk338 = cfg.Get<float>("Bowser Force Multiplier", 1.0f);
    unk33C = cfg.Get<float>("Bowser Big Powerup Boost", 0.4f);
}

/**
 * Offset/Address/Size: 0x2E8 | 0x800404D4 | size: 0xF44
 * TODO: 98.3% match - r28 vs r26 register allocation for node
 */
SkillTweaks::SkillTweaks()
{
    mSkillTweaksList.m_pEnd = NULL;
    mSkillTweaksList.m_pStart = NULL;
    SkillTweak** pEnd;
    void* mem;
    SkillTweak* node;

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_Avoidance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Avoidance Effectiveness");
    }
    pEnd = &mSkillTweaksList.m_pEnd;
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_DekeChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Deke Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_GroundPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Ground Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_VolleyPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Volley Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_ShootingChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shooting Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_ChipShotChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Chip Shot Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_CaptainS2SChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Captain ShootToScore Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_GroundOneTimerChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Ground OneTimer Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_VolleyOneTimerChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Volley OneTimer Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_OneTouchGroundPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "OneTouch Ground Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_OneTouchVolleyPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "OneTouch Volley Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_PassReceiveHitChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "PassReceive Hit Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_PassReceivePowerupChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "PassReceive Powerup Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_WindupDekeChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Windup Deke Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_WindupPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Windup Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_WindupPowerupChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Windup Powerup Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_CutAndBreakChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Cut And Break Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_TurboChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Turbo With Ball Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Off_Reaction;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Reaction");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Def_BlockPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Block Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Def_BlockShotChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Block Shot Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Def_SlideAttackChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Slide Attack Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Def_HeavyAttackChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Heavy Attack Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Def_VolleyPassDefendChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Defend VolleyPass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Def_Marking;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Marking Effectiveness");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Loose_HeavyAttackChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Loose Heavy Attack Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Loose_ShotChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Loose Shot Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Loose_GroundPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Loose Ground Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Loose_VolleyPassChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Loose Volley Pass Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Shoot_CaptainS2SFirstButtonChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Captain S2S Perfect First Button Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &Shoot_CaptainS2SSecondButtonChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Captain S2S Perfect Second Button Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotValue1;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Value 1");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotValue2;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Value 2");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotValue3;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Value 3");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotChance0;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Chance 0");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotChance1;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Chance 1");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotChance2;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Chance 2");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotChance3;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Chance 3");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fShotChance4;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Shot Chance 4");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fSTSWindupTime;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "STS Windup Time");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fAttackCarrierDistance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Attack Carrier Range");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fLooseBallChaseDistance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Loose Ball Range");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fGoalieCanInterceptPass;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Goalie Intercept Pass");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fGoalieDekeChance;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Goalie Deke Chance");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    mem = nlMalloc(0x88, 8, false);
    node = (SkillTweak*)mem;
    if (mem != NULL)
    {
        ((SkillTweak*)mem)->mpValue = &fGoalieDekeSpeed;
        nlSNPrintf(((SkillTweak*)mem)->mNameInFile, 0x7F, "%s", "Goalie Deke Speed");
    }
    nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);

    for (int var_r24 = 0; var_r24 < 3; var_r24++)
    {
        const char* var_r29;
        switch (var_r24)
        {
        case 1:
            var_r29 = "Defensive";
            break;
        case 0:
            var_r29 = "Offensive";
            break;
        default:
            var_r29 = "Loose";
            break;
        }

        float* var_r26 = PowerupUsageChance[var_r24]; // cursor within this category
        for (int var_r23 = 0; var_r23 < 9; var_r23++)
        {
            char sp8[0x40]; // 0x3F usable + NUL
            const char* powerName = GetPowerupName(var_r23);

            nlSNPrintf(sp8, 0x3F, "%s %s Chance", var_r29, powerName);

            node = (SkillTweak*)nlMalloc(0x88, 8, false);
            if (node != NULL)
            {
                node->mpValue = &(var_r26[var_r23]);                   // node->unk84 = &var_r26->unkB8 (cursor)
                nlSNPrintf(node->mNameInFile, 0x7F, "%s", (char*)sp8); // write label into node+0x04
            }
            nlListAddEnd<SkillTweak>(&mSkillTweaksList.m_pStart, pEnd, node);
        }
    }
}

/**
 * Offset/Address/Size: 0x264 | 0x80040450 | size: 0x84
 */
SkillTweaks::~SkillTweaks()
{
}

/**
 * Offset/Address/Size: 0x250 | 0x8004043C | size: 0x14
 */
SkillTweaks* SkillTweaks::GetSkillTweaks(int idx)
{
    return &g_sSkillTweaks[idx];
}

/**
 * Offset/Address/Size: 0x4 | 0x800401F0 | size: 0x24C
 */
void SkillTweaks::Init(eDifficultyID diff, bool blend)
{
    bool bNeedToLoadFile = true;

    nlStrNCpy<char>((char*)mszFileName, g_sDifficultyFileNames[(int)diff], 0x1F);

    if (blend && (int)diff > 1 && (int)diff < 3)
    {
        SkillTweaks minmax[2];
        minmax[0].Init((eDifficultyID)1, 0);
        minmax[1].Init((eDifficultyID)3, 0);

        // float fPercent = (float)(diff - 1) * 0.5f;
        float fPercent = (float)(diff - 1) / 2.f;

        SkillTweak* dst = mSkillTweaksList.m_pStart;
        SkillTweak* min = minmax[0].mSkillTweaksList.m_pStart;
        SkillTweak* max = minmax[1].mSkillTweaksList.m_pStart;

        while (dst)
        {
            *(dst->mpValue) = *(min->mpValue) + (*(max->mpValue) - *(min->mpValue)) * fPercent;

            dst = dst->next;
            min = min->next;
            max = max->next;
        }

        bNeedToLoadFile = false;
    }

    if (bNeedToLoadFile)
    {
        Config cfg(Config::ALLOCATE_HIGH);
        cfg.LoadFromFile((const char*)mszFileName);

        SkillTweak* it = mSkillTweaksList.m_pStart;
        while (it != 0)
        {
            *it->mpValue = GetConfigFloat(cfg, it->mNameInFile, 0.5f);
            it = it->next;
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800401EC | size: 0x4
 */
void SkillTweaks::HookupTweakeables(int)
{
    // EMPTY
}
