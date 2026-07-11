#ifndef _FEPOPUPMENU_H_
#define _FEPOPUPMENU_H_

#include "Game/FE/fePresentation.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feInput.h"
#include "Game/BaseSceneHandler.h"
#include "NL/nlBasicString.h"
#include "NL/nlColour.h"
#include "NL/nlFunction.h"

// Some target TUs see these callback overloads with by-value C++ mangling.
// MWCC still passes non-trivial Function objects by address at the ABI level,
// so call-site codegen can match the reference form while the symbol name
// changes. Keep this separate from Function assignment inlining controls.
#if defined(FEPOPUPMENU_BYVAL_DECLS) || defined(FEPOPUPMENU_INTERNAL_BYVAL)
typedef Function<FnVoidVoid> _FEPopupMenuCB;
#else
typedef Function<FnVoidVoid>& _FEPopupMenuCB;
#endif

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

    void Create(ePopupMenu);
#if defined(FEPOPUPMENU_BYVAL_DECLS) || defined(FEPOPUPMENU_INTERNAL_BYVAL)
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
#else
    void Create(ePopupMenu, _FEPopupMenuCB);
    void Create(ePopupMenu, _FEPopupMenuCB, _FEPopupMenuCB);
    void Create(ePopupMenu, _FEPopupMenuCB, _FEPopupMenuCB, _FEPopupMenuCB);
    void Create(ePopupMenu, _FEPopupMenuCB, _FEPopupMenuCB, _FEPopupMenuCB, _FEPopupMenuCB);
#endif
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
