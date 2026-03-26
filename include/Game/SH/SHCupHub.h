#ifndef _SHCUPHUB_H_
#define _SHCUPHUB_H_

#include "Game/BaseSceneHandler.h"
#include "Game/DB/StatsTracker.h"
#include "Game/FE/Cup/CupTickerManager.h"
#include "Game/FE/feAsyncImage.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/tlComponentInstance.h"
#include "NL/nlColour.h"

enum eHubColour
{
    eHubColour_0 = 0
};

enum eHubState
{
    HUB_INVALID = -1,
    HUB_LEAGUE = 0,
    HUB_BOWSER_TRANSITION = 1,
    HUB_KNOCKOUT2 = 2,
    HUB_KNOCKOUT4 = 3,
    HUB_KNOCKOUT8 = 4,
    NUM_HUB_STATES = 5,
};

class CupHubScene : public BaseSceneHandler
{
public:
    CupHubScene(bool, bool);
    ~CupHubScene();
    void SceneCreated();
    void Update(float);
    void EndCup();
    void ReturnToMainMenu();
    unsigned char UpdateDisplayedStat();
    void CreateLeague();
    void CreateBowserLeague();
    void CreateKnockout();
    unsigned char UpdateLeague(float);
    void UpdateKnockout8(float);
    s32 UpdateKnockout4(float);
    void UpdateKnockout2(float);
    void UpdateProgressIndicator();
    void ColourUserRow();
    void HandleButtonComponent();
    void SetRoundColours(eHubColour*, int);
    void UpdateRoundMessage(bool);
    void LoadCaptainImage();

    /* 0x001C */ TeamStats mAllTeamStats[8];                     // size 0x200
    /* 0x021C */ nlColour mTextColour;                           // size 0x4
    /* 0x0220 */ bool mDoAnimations;                             // size 0x1
    /* 0x0221 */ bool mUpdatingStats;                            // size 0x1
    /* 0x0222 */ bool mKnockoutLoserAnimations;                  // size 0x1
    /* 0x0223 */ bool mAllKnockoutAnimations;                    // size 0x1
    /* 0x0224 */ bool mSuperTeamAnimation;                       // size 0x1
    /* 0x0225 */ bool mHasHumanTeamPlayed;                       // size 0x1
    /* 0x0226 */ bool mDoAutoSave;                               // size 0x1
    /* 0x0227 */ bool mPlayPopSound;                             // size 0x1
    /* 0x0228 */ float mRowMovement[8];                          // size 0x20
    /* 0x0248 */ unsigned short mColumnsByRowsBuffers[5][8][32]; // size 0xA00
    /* 0x0C48 */ unsigned short mProgressBuffer[128];            // size 0x100
    /* 0x0D48 */ TLComponentInstance* mAnimComponents[8];        // size 0x20
    /* 0x0D68 */ int mOldRanks[9];                               // size 0x24
    /* 0x0D8C */ int mNewRanks[9];                               // size 0x24
    /* 0x0DB0 */ int mStandingsIndices[8];                       // size 0x20
    /* 0x0DD0 */ int mAnimatingKnockoutTeams[4];                 // size 0x10
    /* 0x0DE0 */ int mCurrentKnockoutAnimationRound;             // size 0x4
    /* 0x0DE4 */ unsigned short mOldStats[8][4];                 // size 0x40
    /* 0x0E24 */ float mStatUpdateDelay;                         // size 0x4
    /* 0x0E28 */ float mSlideSwitchDelay;                        // size 0x4
    /* 0x0E2C */ eHubState mHubState;                            // size 0x4
    /* 0x0E30 */ CupTickerManager mTickerManager;                // size 0x408
    /* 0x1238 */ AsyncImage* mCaptainImage;                      // size 0x4
    /* 0x123C */ ButtonComponent mButtons;                       // size 0x24
}; // total size: 0x1260

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher); void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//     unsigned long); void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class Function0<void>
// {
// public:
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*>>::~FunctorImpl();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*>>::operator()();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*>>::Clone() const;
// };

// class TeamStats
// {
// public:
//     TeamStats();
// };

// class MemFun<CupHubScene, void>(void (CupHubScene
// {
// public:
//     void *)());
// };

// class Bind<void, Detail
// {
// public:
//     void MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*>(Detail::MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*
//     const&);
// };

#endif // _SHCUPHUB_H_
