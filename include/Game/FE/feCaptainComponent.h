#ifndef _FECAPTAINCOMPONENT_H_
#define _FECAPTAINCOMPONENT_H_

#include "Game/FE/feInput.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feCaptainGridComponent.h"
#include "Game/GameInfo.h"

class TLComponentInstance;
class TLInstance;
class TLSlide;
class InlineHasher;
class FEPresentation;
class AsyncImage;
class ISidekickGridComponent;

enum UpdateResult
{
    UPDATE_OK = 0,
    UPDATE_GO_BACK = 1,
    UPDATE_GO_FORWARD = 2,
};

enum Phase
{
    PHASE_IDLE = 0,
    PHASE_CHOOSING_CAPTAIN = 1,
    PHASE_CHOOSING_SIDEKICK = 2,
    PHASE_READY = 3,
};

class IChooseCaptain
{
public:
    class NameComponent
    {
    public:
        void SetSidekickName(unsigned long);
        void SetTextName(const char*, unsigned long);
        void SetCaptainName(unsigned long);
        void SetCaptainLogo(const char*);

        /* 0x0 */ TLComponentInstance* mComponent;
        /* 0x4 */ const char* mCaptainObjName;
        /* 0x8 */ const char* mSidekickObjName;
    }; // total size: 0xC

    class ComponentState
    {
    public:
        void GotoNextPhase();
        void GotoPreviousPhase();
        void SetCurrentPhase(Phase);

        /* 0x0 */ Phase mCurrentPhase;
        /* 0x4 */ IChooseCaptain* mParent;
        /* 0x8 */ int mHomeAway;
    }; // total size: 0xC

    IChooseCaptain();
    ~IChooseCaptain();
    void Initialize(const char*, const char*);
    void UpdateSound(float);
    UpdateResult Update(float);
    void UpdateAsyncImages();
    void SceneCreated(FEPresentation*);
    void SetupCaptainComponent(TLComponentInstance*, int);
    void StartSidekickMiniHead(int, eSidekickID);
    void CheckForDisconnectedHumanPlayers();
    void FindAliveHumanPlayers();
    void SetupForLastPhase(eFEINPUT_PAD);
    void PushPlayer(eFEINPUT_PAD, int);
    void PopPlayer(eFEINPUT_PAD);
    void ResetPushPlayerData();
    void PushPlayerWithGameInfoDB();
    void MoveHighlightToCurrentCaptain(int);
    void SetupNameComponentToCurrentCaptain(int);
    void SetPhaseReady(int);

    inline bool IsPlayerPushed(const int pad)
    {
        for (int i = 0; i < mNumTotalPushedPlayers; i++)
        {
            if (mAllPushedPlayers[i] == pad)
            {
                return true;
            }
        }
        return false;
    }

    /* 0x00 */ ComponentState mComponentState[2];                   // size 0x18
    /* 0x18 */ unsigned char mIsSinglePlayerInput;                  // size 0x1
    /* 0x1C */ eFEINPUT_PAD mAllPushedPlayers[4];                   // size 0x10
    /* 0x2C */ int mAllPushedPlayerSides[4];                        // size 0x10
    /* 0x3C */ int mNumTotalPushedPlayers;                          // size 0x4
    /* 0x40 */ AsyncImage* mAsyncImage[2][3];                       // size 0x18
    /* 0x58 */ int mHomeAwayTeam[2];                                // size 0x8
    /* 0x60 */ int mHomeAwaySidekicks[2];                           // size 0x8
    /* 0x68 */ bool mDidSwapCaptains[2];                            // size 0x2
    /* 0x6A */ bool mDidSwapSidekicks[2];                           // size 0x2
    /* 0x6C */ TLComponentInstance* mCaptainComponents[2];          // size 0x8
    /* 0x74 */ TLComponentInstance* mSidekickComponents[2];         // size 0x8
    /* 0x7C */ TLComponentInstance* mSidekickMiniHeadComponents[2]; // size 0x8
    /* 0x84 */ float mCaptainSlideDurations[2];                     // size 0x8
    /* 0x8C */ float mSidekickSlideDurations[2];                    // size 0x8
    /* 0x94 */ float mCaptainSoundDelay[2];                         // size 0x8
    /* 0x9C */ ICaptainGridComponent* mCaptainGridComponents[2];    // size 0x8
    /* 0xA4 */ ISidekickGridComponent* mSidekickGridComponents[2];  // size 0x8
    /* 0xAC */ char* mLastCaptainSelectSoundStrPlayed[2];           // size 0x8
    /* 0xB4 */ NameComponent mNameComponents[2];                    // size 0x18
}; // total size: 0xCC

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class IGridComponent<eSidekickID>
// {
// public:
//     void RebindHighliteComponent(const char*);
// };

// class IGridComponent<eTeamID>
// {
// public:
//     void RebindHighliteComponent(const char*);
// };

#endif // _FECAPTAINCOMPONENT_H_
