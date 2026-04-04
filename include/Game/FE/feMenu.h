#ifndef _FEMENU_H_
#define _FEMENU_H_

#include "NL/nlFunction.h"

enum MenuAction
{
    ON_INVALID = -1,
    ON_APPLY = 0,
    ON_HIGHLIGHT = 1,
    ON_UNHIGHLIGHT = 2,
    NUM_ACTIONS = 3,
};

enum MenuResult
{
    RES_ERROR = 0,
    RES_OK = 1,
    RES_NOT_CHANGED = 2,
    RES_ITEM_DISABLED = 3,
    RES_NO_CALLBACK_FUNC = 4,
};

// #pragma push
// #pragma pack(1)
template <typename T>
struct MenuItem
{
    /*  0x00 */ Function<T*> mCallbacks[3];
    /*  0x18 */ T* mType;
    /*  0x1C */ bool mDisabled;
    /*  0x1D */ bool mLocked;

    MenuItem()
        : mType(0)
        , mDisabled(false)
        , mLocked(false) { };
    ~MenuItem() { };

    inline void ApplyAction(MenuAction action)
    {
        if (mCallbacks[action].mTag == EMPTY)
            return;
        if (action == ON_APPLY && mDisabled)
            return;
        mCallbacks[action](mType);
    }
}; // total size: 0x20

template <typename T>
class MenuList
{
public:
    MenuList()
        : mCurrentIndex(0)
        , mNumItemsAdded(0)
        , mFlags(0) { };
    virtual ~MenuList() { };

    /* 0x004 */ MenuItem<T> mMenuItems[16];
    /* 0x204 */ int mCurrentIndex;
    /* 0x208 */ int mNumItemsAdded;
    /* 0x20C */ bool mWrapList;
    /* 0x210 */ int mFlags;
}; // total size: 0x214
// #pragma pop

#endif // _FESLIDEMENU_H_
