#ifndef _FESIDEKICKGRIDCOMPONENT_H_
#define _FESIDEKICKGRIDCOMPONENT_H_
#include "Game/FE/feInput.h"
#include "Game/FE/feMapMenu.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlComponentInstance.h"
#include "NL/nlMemory.h"

#include "Game/DB/Cup.h"

class TLSlide;

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
    virtual void BuildMapMenu() { }
    virtual void RebuildInstanceTable() { }
    virtual void Update(eFEINPUT_PAD) { }
    virtual void MoveHighlightToTarget(T target) { }
    virtual T GetSelectedItem() const { return (T)-1; }

    IGridComponent(TLComponentInstance* parentcomponent, const char* highlitename, bool ismirrored);
    void RebindHighliteComponent(const char* highlitename);

    /* 0x00 */ // vtable
    /* 0x04 */ TLInstance** mInstanceTable;
    /* 0x08 */ FEMapMenu* mMapMenu;
    /* 0x0C */ bool mHighliteVisibilityAtAnimEnd;
    /* 0x10 */ TLComponentInstance* mParentComponent;
    /* 0x14 */ TLComponentInstance* mHighliteComponent;
    /* 0x18 */ bool mIsMirrored;
    /* 0x19 */ bool mHasChangedSinceLastUpdate;
}; // total size: 0x1C

template <typename T>
void IGridComponent<T>::RebindHighliteComponent(const char* highlitename)
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash(highlitename);
    hB.m_Hash = hash;
    hA.m_Hash = hash;

    TLSlide* slide = mParentComponent->GetActiveSlide();
    mHighliteComponent = findComp.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);
    FEMapMenu* menu = mMapMenu;
    menu->m_highlighter = (TLInstance*)mHighliteComponent;
    menu->UpdateHighlighter();
}

template <typename T>
IGridComponent<T>::IGridComponent(TLComponentInstance* parentcomponent, const char* highlitename, bool ismirrored)
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    mInstanceTable = NULL;
    mMapMenu = NULL;
    mHighliteVisibilityAtAnimEnd = false;
    mParentComponent = parentcomponent;
    mHighliteComponent = NULL;
    mIsMirrored = ismirrored;
    mHasChangedSinceLastUpdate = false;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash(highlitename);
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLSlide* slide = parentcomponent->GetActiveSlide();
    mHighliteComponent = findComp.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    mMapMenu = new (8, false) FEMapMenu((TLInstance*)mHighliteComponent, true);
}

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
