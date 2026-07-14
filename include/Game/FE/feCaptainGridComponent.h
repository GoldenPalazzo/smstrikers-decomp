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
    bool IsValid(eTeamID);
    void SetValid(eTeamID, bool);
    void UpdateSuperTeamIconState();
    void Update(eFEINPUT_PAD);
    void RebuildInstanceTable();
    void BuildMapMenu();
    ~ICaptainGridComponent();
    ICaptainGridComponent(TLComponentInstance*, bool);
};

#endif // _FECAPTAINGRIDCOMPONENT_H_
