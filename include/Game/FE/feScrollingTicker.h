#ifndef _FESCROLLINGTICKER_H_
#define _FESCROLLINGTICKER_H_

#include "Game/BaseSceneHandler.h"
#include "Game/FE/feIMessenger.h"
#include "Game/FE/feTweener.h"
#include "Game/FE/feTweenFuncs.h"
#include "Game/FE/feScrollText.h"

class TLTextInstance;
class TLInstance;
class feVector3;

class ScrollingTickerScene : public FEIMessenger, public BaseSceneHandler
{
public:
    bool IsMessengerOpen() const;
    void CloseMessengerNow();
    void CloseMessenger();
    void OpenMessengerNow();
    void OpenMessenger();
    void SetDisplayMessage(const BasicString<unsigned short, Detail::TempStringAllocator>&);
    void SetMessageFinishedCB(const Function<FnVoidVoid>& cb)
    {
        m_cbFunc = cb;
        if (m_textScroller != NULL)
        {
            m_textScroller->m_messageFinishedCB = cb;
        }
    }
    virtual ~ScrollingTickerScene();
    virtual void SceneCreated();
    virtual void Update(float);
    ScrollingTickerScene();

    static void tickerClosed(void*);
    static void tickerOpened(void*);
    static void setScaleTweenCallback(void*, const float*);
    static void setSizeTweenCallback(void*, const float*);

    /* 0x020 */ unsigned char m_active;
    /* 0x024 */ feVector3 m_leftBallClosedPos;
    /* 0x030 */ feVector3 m_rightBallClosedPos;
    /* 0x03C */ feVector3 m_leftBallOpenPos;
    /* 0x048 */ feVector3 m_rightBallOpenPos;
    /* 0x054 */ feVector3 m_ballClosedScale;
    /* 0x060 */ feVector3 m_grayClosedScale;
    /* 0x06C */ feVector3 m_grayOpenScale;
    /* 0x078 */ TLTextInstance* m_textBox;
    /* 0x07C */ TLInstance* m_leftBall;
    /* 0x080 */ TLInstance* m_rightBall;
    /* 0x084 */ TLInstance* m_backRectangle;
    /* 0x088 */ Function<FnVoidVoid> m_cbFunc;
    /* 0x090 */ FEScrollText* m_textScroller;
    /* 0x094 */ FETweenManager m_pFETweenManager;
}; // total size: 0xCC

#endif // _FESCROLLINGTICKER_H_
