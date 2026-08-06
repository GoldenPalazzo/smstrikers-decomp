#ifndef _FESIDEKICKGRIDCOMPONENT_H_
#define _FESIDEKICKGRIDCOMPONENT_H_
#include "Game/FE/feGridComponent.h"

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

#endif // _FESIDEKICKGRIDCOMPONENT_H_
