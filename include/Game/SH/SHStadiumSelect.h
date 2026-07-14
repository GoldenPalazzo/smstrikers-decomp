#ifndef _SHSTADIUMSELECT_H_
#define _SHSTADIUMSELECT_H_

#include "Game/BaseSceneHandler.h"
#include "Game/DB/Cup.h"
#include "Game/FE/feAsyncImage.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feMenu.h"
#include "Game/FE/feScrollText.h"
#include "Game/FE/tlComponentInstance.h"

struct StadiumEntry
{
    /* 0x0 */ eStadiumID stadiumID;
    /* 0x4 */ const char* imagePath;
}; // total size: 0x8

enum Direction
{
    DIR_LEFT = 0,
    DIR_RIGHT = 1,
};

class StadiumSelectSceneV2 : public BaseSceneHandler
{
public:
    StadiumSelectSceneV2();
    virtual ~StadiumSelectSceneV2();
    virtual void SceneCreated();
    virtual void Update(float);
    void ResetFromRight();
    void ResetFromLeft();

    /* 0x01C */ AsyncImage* mImages[7];                          // size 0x1C
    /* 0x038 */ void* mTempTextureBuffer;                        // size 0x4
    /* 0x03C */ int mTempTextureBufferSize;                      // size 0x4
    /* 0x040 */ FEScrollText* m_pTicker;                         // size 0x4
    /* 0x044 */ int mStadiumIndex;                               // size 0x4
    /* 0x048 */ ButtonComponent mButtons;                        // size 0x24
    /* 0x06C */ MenuList<TLComponentInstance> mMenuItemsLeft;    // size 0x214
    /* 0x280 */ MenuList<TLComponentInstance> mMenuItemsRight;   // size 0x214
    /* 0x494 */ MenuList<TLComponentInstance>* mCurrentMenuList; // size 0x4
    /* 0x498 */ Direction mLastDirection;                        // size 0x4
}; // total size: 0x49C

#endif // _SHSTADIUMSELECT_H_
