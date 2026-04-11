#ifndef _FECAPTAINGRIDCOMPONENT_H_
#define _FECAPTAINGRIDCOMPONENT_H_
#include "Game/FE/feInput.h"
#include "Game/FE/feSidekickGridComponent.h"
#include "Game/FE/feMapMenu.h"

#include "Game/Team.h"

class TLComponentInstance;
class TLInstance;
class TLSlide;
class InlineHasher;

class ICaptainGridComponent : public IGridComponent<eTeamID>
{
public:
    void SetAllItemsActive();
    virtual void MoveHighlightToTarget(eTeamID);
    eTeamID GetSelectedItem() const;
    void IsValid(eTeamID);
    void SetValid(eTeamID, bool);
    void UpdateSuperTeamIconState();
    void Update(eFEINPUT_PAD);
    void RebuildInstanceTable();
    void BuildMapMenu();
    ~ICaptainGridComponent();
    ICaptainGridComponent(TLComponentInstance*, bool);
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

// class IGridComponent<eTeamID>
// {
// public:
//     void ~IGridComponent();
//     void IGridComponent(TLComponentInstance*, const char*, bool);
// };

#endif // _FECAPTAINGRIDCOMPONENT_H_
