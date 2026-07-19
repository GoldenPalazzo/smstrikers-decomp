#ifndef _OVERLAYHANDLERWINNER_H_
#define _OVERLAYHANDLERWINNER_H_

#include "Game/FE/BaseOverlayHandler.h"
#include "Game/FE/feAsyncImage.h"
#include "Game/Team.h"

class WinnerOverlay : public BaseOverlayHandler
{
public:
    WinnerOverlay();
    ~WinnerOverlay();
    void SceneCreated();
    void Update(float);

    /* 0x28 */ unsigned short mScoresBuffer[32];
    /* 0x68 */ unsigned short mWinnerBuffer[32];
    /* 0xA8 */ float mInputDelay;
    /* 0xAC */ bool mDoingOutTransition;
    /* 0xB0 */ AsyncImage* mWinnerActionWhite;
    /* 0xB4 */ AsyncImage* mWinnerAction;
    /* 0xB8 */ AsyncImage* mWinnerActionOutline;
    /* 0xBC */ eTeamID mWinningTeam;
}; // total size: 0xC0

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLComponentInstance, 4>
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

#endif // _OVERLAYHANDLERWINNER_H_
