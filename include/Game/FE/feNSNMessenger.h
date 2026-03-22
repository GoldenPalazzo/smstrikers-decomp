#ifndef _FENSNMESSENGER_H_
#define _FENSNMESSENGER_H_

#include "NL/nlFunction.h"

#include "Game/BaseSceneHandler.h"
#include "Game/FE/feScrollText.h"
#include "Game/FE/feIMessenger.h"

enum ScreenPosition
{
    POSITION_INVALID = -1,
    POSITION_TOP_LEFT = 0,
    POSITION_TOP_MIDDLE = 1,
    POSITION_TOP_RIGHT = 2,
    POSITION_MIDDLE_LEFT = 3,
    POSITION_MIDDLE_MIDDLE = 4,
    POSITION_MIDDLE_RIGHT = 5,
    POSITION_BOTTOM_LEFT = 6,
    POSITION_BOTTOM_MIDDLE = 7,
    POSITION_BOTTOM_RIGHT = 8,
    POSITION_TOP = 9,
    POSITION_MIDDLE = 10,
    POSITION_BOTTOM = 11,
    POSITION_ALL = 12,
    NUM_POSITIONS = 13,
};

enum eMessengerState
{
    MS_INVALID = -1,
    MS_OPENING = 0,
    MS_OPEN = 1,
    MS_CLOSING = 2,
    MS_CLOSED = 3,
    MS_NUMSTATES = 4,
};

class BaseOverlayHandler : public BaseSceneHandler
{
public:
    BaseOverlayHandler(u32 VisibilityMask, ScreenPosition position = POSITION_TOP)
        : BaseSceneHandler()
    {
        mPosition = position;
        mVisibilityMask = VisibilityMask;
        mWasLastVisible = false;
    }
    virtual ~BaseOverlayHandler() { };

    /* 0x1C */ ScreenPosition mPosition;
    /* 0x20 */ u32 mVisibilityMask;
    /* 0x24 */ bool mWasLastVisible;
}; // total size: 0x28

class NSNMessengerScene : public FEIMessenger, public BaseOverlayHandler
{
public:
    NSNMessengerScene();
    ~NSNMessengerScene();

    virtual void OpenMessenger();
    virtual void OpenMessengerNow();
    virtual void SetDisplayMessage(const BasicString<unsigned short, Detail::TempStringAllocator>&);
    virtual void SetMessageFinishedCB(const Function<FnVoidVoid>& cb);
    virtual void CloseMessenger();
    virtual void CloseMessengerNow();
    virtual bool IsMessengerOpen() const;

    virtual void SceneCreated();
    virtual void Update(float);

    void EnableScrolling(bool);
    void SetDisplayMessage(const char*);
    void ForceMessengerVisibleNow();

    // void @4@SceneCreated();
    // void @4@Update(float);

    /* 0x02C */ Function<NSNMessengerScene> m_messageFinishedCB;
    /* 0x034 */ eMessengerState m_curState;
    /* 0x038 */ float m_messageDisplayTime;
    /* 0x03C */ bool m_messageDisplaying;
    /* 0x03E */ unsigned short m_displayMessage[255];
    /* 0x23C */ FEScrollText* m_scrollText;
}; // total size: 0x240

// class BaseSceneHandler
// {
// public:
//     void SceneCreated();
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long);
// };

#endif // _FENSNMESSENGER_H_
