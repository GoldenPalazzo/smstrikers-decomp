#include "Game/FE/feSidekickGridComponent.h"
#include "Game/FE/feMapMenu.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlComponentInstance.h"

struct CellItem
{
    int mIconType;
    const char* mIconName;
};

extern CellItem SidekickCellItems[4];

/**
 * Offset/Address/Size: 0x0 | 0x800C2810 | size: 0x34
 */
void ISidekickGridComponent::SetVisibleInstanceTable(bool visible)
{
    mInstanceTable[0]->m_bVisible = visible;
    mInstanceTable[1]->m_bVisible = visible;
    mInstanceTable[2]->m_bVisible = visible;
    mInstanceTable[3]->m_bVisible = visible;
}

/**
 * Offset/Address/Size: 0x34 | 0x800C2844 | size: 0x70
 */
void ISidekickGridComponent::MoveHighlightToTarget(eSidekickID id)
{
    int item;
    switch (id)
    {
    case SK_TOAD:
        item = 0;
        break;
    case SK_HAMMERBROS:
        item = 1;
        break;
    case SK_BIRDO:
        item = 2;
        break;
    case SK_KOOPA:
        item = 3;
        break;
    default:
        item = 0;
        break;
    }
    mMapMenu->SetSelectedItem(item);
}

/**
 * Offset/Address/Size: 0xA4 | 0x800C28B4 | size: 0x70
 */
eSidekickID ISidekickGridComponent::GetSelectedItem() const
{
    int position = mMapMenu->GetSelectedItem();

    switch (position)
    {
    case 0:
        return SK_TOAD;
    case 1:
        return SK_HAMMERBROS;
    case 2:
        return SK_BIRDO;
    case 3:
        return SK_KOOPA;
    default:
        return SK_TOAD;
    }
}

/**
 * Offset/Address/Size: 0x114 | 0x800C2924 | size: 0x210
 */
void ISidekickGridComponent::Update(eFEINPUT_PAD pad)
{
    int oldSelected = mMapMenu->GetSelectedItem();
    mHasChangedSinceLastUpdate = false;

    if (g_pFEInput->IsAutoPressed(pad, 0xB, true, NULL))
    {
        if (mIsMirrored)
        {
            mMapMenu->MoveRight(false);
        }
        else
        {
            mMapMenu->MoveLeft(false);
        }

        FEAudio::PlayAnimAudioEvent("sfx_option_scroll_left", false);
    }
    else if (g_pFEInput->IsAutoPressed(pad, 0xC, true, NULL))
    {
        if (mIsMirrored)
        {
            mMapMenu->MoveLeft(false);
        }
        else
        {
            mMapMenu->MoveRight(false);
        }

        FEAudio::PlayAnimAudioEvent("sfx_option_scroll_right", false);
    }
    else if (g_pFEInput->IsAutoPressed(pad, 0xD, true, NULL))
    {
        mMapMenu->MoveUp(false);
        FEAudio::PlayAnimAudioEvent("sfx_option_scroll_up", false);
    }
    else if (g_pFEInput->IsAutoPressed(pad, 0xE, true, NULL))
    {
        mMapMenu->MoveDown(false);
        FEAudio::PlayAnimAudioEvent("sfx_option_scroll_down", false);
    }

    if (oldSelected != mMapMenu->GetSelectedItem())
    {
        mHasChangedSinceLastUpdate = true;
    }

    if (mHighliteVisibilityAtAnimEnd)
    {
        TLSlide* activeSlide = mParentComponent->GetActiveSlide();
        bool shouldShow;

        if (activeSlide == NULL)
        {
            shouldShow = true;
        }
        else if (activeSlide->m_time >= activeSlide->m_start + activeSlide->m_duration)
        {
            shouldShow = true;
        }
        else
        {
            shouldShow = false;
        }

        if (shouldShow)
        {
            mHighliteVisibilityAtAnimEnd = false;
            mHighliteComponent->m_bVisible = true;
        }
    }

    mMapMenu->Update(0.016666668f);
}

/**
 * Offset/Address/Size: 0x324 | 0x800C2B34 | size: 0xE4
 */
void ISidekickGridComponent::RebuildInstanceTable()
{
    TLSlide* activeslide = mParentComponent->GetActiveSlide();

    for (int i = 0; i < 4; i++)
    {
        TLInstance* inst = FEFinder<TLInstance, 2>::Find<TLSlide>(
            activeslide,
            InlineHasher(nlStringLowerHash(SidekickCellItems[i].mIconName)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        int iconType = SidekickCellItems[i].mIconType;
        mInstanceTable[iconType] = inst;
        mMapMenu->ChangeItem(iconType, mInstanceTable[iconType]);
    }
}

/**
 * Offset/Address/Size: 0x408 | 0x800C2C18 | size: 0x1AC
 * TODO: 97.2% match - register allocation: this=r26 instead of r27,
 * activeslide/this swapped, base=r31 instead of r25 in second loop
 */
#pragma opt_strength_reduction off
void ISidekickGridComponent::BuildMapMenu()
{
    CellItem* pItem;
    TLSlide* activeslide = mParentComponent->GetActiveSlide();

    mInstanceTable = (TLInstance**)nlMalloc(0x10, 8, false);

    pItem = SidekickCellItems;
    for (int i = 0; i < 4; i++, pItem++)
    {
        TLInstance* inst = FEFinder<TLInstance, 2>::Find(
            activeslide,
            InlineHasher(nlStringLowerHash(pItem->mIconName)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        mInstanceTable[pItem->mIconType] = inst;
    }

    for (int itemIndexBase = 0, i = 0; i < 2; i++, itemIndexBase += 2)
    {
        int itemIndex = itemIndexBase;

        for (int j = 0; j < 2; j++, itemIndex++)
        {
            int leftIndex = itemIndex - 1;
            if (((itemIndex % 2) - 1) < 0)
            {
                leftIndex = itemIndex + 1;
            }

            int rightIndex = itemIndex + 1;
            if (((itemIndex % 2) + 1) >= 2)
            {
                rightIndex = itemIndex - 1;
            }

            int oppositeIndex = (itemIndex + 2) % 4;
            int iconType = SidekickCellItems[itemIndex].mIconType;

            mMapMenu->AddItem(
                iconType,
                mInstanceTable[iconType],
                SidekickCellItems[leftIndex].mIconType,
                SidekickCellItems[rightIndex].mIconType,
                SidekickCellItems[oppositeIndex].mIconType,
                SidekickCellItems[oppositeIndex].mIconType,
                true);
        }
    }

    mMapMenu->SetSelectedItem(SidekickCellItems[0].mIconType);
}
#pragma opt_strength_reduction on

/**
 * Offset/Address/Size: 0x5B4 | 0x800C2DC4 | size: 0x94
 */
ISidekickGridComponent::~ISidekickGridComponent()
{
}

/**
 * Offset/Address/Size: 0x648 | 0x800C2E58 | size: 0x48
 */
ISidekickGridComponent::ISidekickGridComponent(TLComponentInstance*, bool)
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800C2EA0 | size: 0x38
//  */
// void FEFinder<TLInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800C2ED8 | size: 0x84
//  */
// void FEFinder<TLInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800C2F5C | size: 0x15C
//  */
// void FEFinder<TLInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800C30B8 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800C30F0 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800C3174 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800C32D0 | size: 0x84
//  */
// void IGridComponent<eSidekickID>::~IGridComponent()
// {
// }

// /**
//  * Offset/Address/Size: 0x84 | 0x800C3354 | size: 0xF0
//  */
// void IGridComponent<eSidekickID>::IGridComponent(TLComponentInstance*, const char*, bool)
// {
// }
