#ifndef _FEPOPUPMENU_H_
#define _FEPOPUPMENU_H_

#include "Game/FE/fePresentation.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feInput.h"
#include "Game/BaseSceneHandler.h"
#include "NL/nlBasicString.h"
#include "NL/nlColour.h"
#include "NL/nlFunction.h"

typedef Function<FnVoidVoid> _FEPopupMenuCB;

// void CastToSomeType<TLInstance>(TLInstance*, void*);
// void CastToSomeType<TLSlide>(TLSlide*, void*);
// void FindItemByHashID<TLInstance>(TLInstance*, unsigned long);
// void FindItemByHashID<TLSlide>(TLSlide*, unsigned long);

enum ePopupMenu
{
    INVALID_TYPE = -1,
    POPUP_END_CUP = 0,
    POPUP_FLOWER_CUP_LOCKED = 1,
    POPUP_STAR_CUP_LOCKED = 2,
    POPUP_BOWSER_CUP_LOCKED = 3,
    POPUP_SUPER_CUPS_LOCKED = 4,
    POPUP_INGAME_FORFEIT_MATCH = 5,
    POPUP_INGAME_QUIT_MATCH = 6,
    POPUP_INGAME_QUIT_STRIKERS_101 = 7,
    POPUP_NO_SIDES_CHOSEN = 8,
    POPUP_NO_HUMAN_TOURNAMENT = 9,
    POPUP_START_NEW_CUP = 10,
    POPUP_START_NEW_TOURNAMENT = 11,
    POPUP_FILLALLSLOTS = 12,
    POPUP_REVERT_OPTION_CHANGES = 13,
    POPUP_TOURNEY_OVER = 14,
    POPUP_NO_FORFEIT = 15,
    POPUP_REALLY_OVERWRITE = 16,
    POPUP_APPLYING_AUDIO = 17,
    POPUP_NO_MEMCARD = 18,
    POPUP_MEMCARD_CORRUPTED = 19,
    POPUP_MEMCARD_WRONGFORMAT = 20,
    POPUP_FILE_CORRUPTED = 21,
    POPUP_MEMCARD_DAMAGED = 22,
    POPUP_WRONG_DEVICE = 23,
    POPUP_NOT_ENOUGH_SPACE = 24,
    POPUP_NOT_ENOUGH_SPACE_CANMANAGE = 25,
    POPUP_ABOUTTOSAVE = 26,
    POPUP_NOTSAMECARD = 27,
    POPUP_MEMCARD_ASK_SAVE_OVERWRITE = 28,
    POPUP_MEMCARD_ASK_LOAD_OVERWRITE = 29,
    POPUP_MEMCARD_ASK_SAVE_NO_FILE = 30,
    POPUP_MEMCARD_CONFIRM_FORMAT = 31,
    POPUP_UNLOCKED_FLOWER_CUP = 32,
    POPUP_UNLOCKED_STAR_CUP = 33,
    POPUP_UNLOCKED_BOWSER_CUP = 34,
    POPUP_UNLOCKED_SUPER_CUPS = 35,
    POPUP_UNLOCKED_CUSTOM_POWERUPS = 36,
    POPUP_UNLOCKED_KONGA_STADIUM = 37,
    POPUP_UNLOCKED_YOSHI_STADIUM = 38,
    POPUP_UNLOCKED_FORBIDDEN_STADIUM = 39,
    POPUP_UNLOCKED_SUPER_STADIUM = 40,
    POPUP_UNLOCKED_LEGEND_DIFFICULTY = 41,
    POPUP_UNLOCKED_SUPER_TEAM = 42,
    POPUP_UNLOCKED_CHEAT_GOALIE = 43,
    POPUP_UNLOCKED_CHEAT_INFINITE = 44,
    POPUP_UNLOCKED_CHEAT_TILT = 45,
    POPUP_UNLOCKED_ALL_STS = 46,
    NUM_POPUP_MENUS = 47,
};

struct Popup
{
    /* 0x00 */ BasicString<unsigned short, Detail::TempStringAllocator>* pMessage;         // size 0x4
    /* 0x04 */ BasicString<unsigned short, Detail::TempStringAllocator>* pOptionLabels[4]; // size 0x10
    /* 0x14 */ int numOptions;                                                             // size 0x4
}; // total size: 0x18

class FEPopupMenu : public BaseSceneHandler
{
public:
    FEPopupMenu();
    virtual ~FEPopupMenu();
    virtual void Update(float);
    virtual void SceneCreated();

    void SetOptionTextColourOnCurrent(bool);
    void ResizeHighlight();
    void CentrePopup(float, float);
    void SetPositions();

    void Create(ePopupMenu type)
    {
        Create(type, Function<FnVoidVoid>(Nothing));
    }
    void Create(ePopupMenu type, Function<FnVoidVoid> option1)
    {
        Create(type, option1, Function<FnVoidVoid>(Nothing));
    }

    void Create(ePopupMenu type, Function<FnVoidVoid> option1, Function<FnVoidVoid> option2)
    {
        Create(type, option1, option2, Function<FnVoidVoid>(Nothing));
    }

    void Create(
        ePopupMenu type,
        Function<FnVoidVoid> option1,
        Function<FnVoidVoid> option2,
        Function<FnVoidVoid> option3)
    {
        Create(type, option1, option2, option3, Function<FnVoidVoid>(Nothing));
    }

    void Create(
        ePopupMenu,
        Function<FnVoidVoid>,
        Function<FnVoidVoid>,
        Function<FnVoidVoid>,
        Function<FnVoidVoid>);
    void SetBackButtonCallback(_FEPopupMenuCB);
    static void Nothing() { }

    /* 0x01C */ unsigned short mMessageBuffer[1024];  // size 0x800
    /* 0x81C */ unsigned short mOptionBuffers[4][64]; // size 0x200
    /* 0xA1C */ bool mMenuDisplayed;                  // size 0x1
    /* 0xA1D */ bool mMenuCreated;                    // size 0x1
    /* 0xA1E */ bool mRunCallBack;                    // size 0x1
    /* 0xA1F */ bool mUnknownA1F;                     // size 0x1
    /* 0xA20 */ int mHighlightedOption;               // size 0x4
    /* 0xA24 */ float mAcceptDelayTime;               // size 0x4
    /* 0xA28 */ Popup mPopup;                         // size 0x18
    /* 0xA40 */ eFEINPUT_PAD mControlInput;           // size 0x4
    /* 0xA44 */ Function<FnVoidVoid> callBacks[4];    // size 0x20
    /* 0xA64 */ Function<FnVoidVoid> mUnknownA64;     // size 0x8
    /* 0xA6C */ nlColour mHighlightedOptionColour;    // size 0x4
    /* 0xA70 */ feVector3 mHighlightSize;             // size 0xC
    /* 0xA7C */ ePopupMenu mType;                     // size 0x4
    /* 0xA80 */ ButtonComponent mButtons;             // size 0x24
    /* 0xAA4 */ bool mUnknownAA4;                     // size 0x1
    /* 0xAA5 */ bool mUnknownAA5;                     // size 0x1
}; // total size: 0xAA8

#endif // _FEPOPUPMENU_H_
