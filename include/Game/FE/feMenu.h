#ifndef _FEMENU_H_
#define _FEMENU_H_

#include "NL/nlFunction.h"

// #pragma push
// #pragma pack(1)
template <typename T>
struct MenuItem
{
    // nlFunction::Function expects the parameter type (e.g. T*).
    /*  0x00 */ Function<T*> mCallbacks[3];
    /*  0x18 */ T* mType;
    /*  0x1C */ bool mDisabled;
    /*  0x1D */ bool mLocked;

    MenuItem() { };
    ~MenuItem() { };
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
