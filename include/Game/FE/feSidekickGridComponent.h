#ifndef _FESIDEKICKGRIDCOMPONENT_H_
#define _FESIDEKICKGRIDCOMPONENT_H_
#include "Game/FE/feInput.h"
#include "Game/FE/feMapMenu.h"

#include "Game/DB/Cup.h"

class TLComponentInstance;
class TLInstance;
class TLSlide;
class InlineHasher;

template <typename T>
class IGridComponent
{
public:
    virtual ~IGridComponent()
    {
        if (mInstanceTable != NULL)
        {
            delete[] mInstanceTable;
            mInstanceTable = NULL;
        }
        delete mMapMenu;
        mMapMenu = NULL;
    }
    virtual void vfunc_0C() { }
    virtual void vfunc_10() { }
    virtual void vfunc_14() { }
    virtual void MoveHighlightToTarget(T target) { }

    /* 0x00 */ // vtable
    /* 0x04 */ TLInstance** mInstanceTable;
    /* 0x08 */ FEMapMenu* mMapMenu;
    /* 0x0C */ bool mHighliteVisibilityAtAnimEnd;
    /* 0x10 */ TLComponentInstance* mParentComponent;
    /* 0x14 */ TLComponentInstance* mHighliteComponent;
    /* 0x18 */ bool mIsMirrored;
    /* 0x19 */ bool mHasChangedSinceLastUpdate;
}; // total size: 0x1C

class ISidekickGridComponent : public IGridComponent<eSidekickID>
{
public:
    void SetVisibleInstanceTable(bool);
    void MoveHighlightToTarget(eSidekickID);
    eSidekickID GetSelectedItem() const;
    void Update(eFEINPUT_PAD);
    void RebuildInstanceTable();
    void BuildMapMenu();
    ~ISidekickGridComponent();
    ISidekickGridComponent(TLComponentInstance*, bool);
};

// class FEFinder<TLInstance, 2>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class IGridComponent<eSidekickID>
// {
// public:
//     void ~IGridComponent();
//     void IGridComponent(TLComponentInstance*, const char*, bool);
// };

#endif // _FESIDEKICKGRIDCOMPONENT_H_
