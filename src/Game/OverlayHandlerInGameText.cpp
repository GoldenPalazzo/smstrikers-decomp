#include "Game/OverlayHandlerInGameText.h"
#include "Game/BaseSceneHandler.h"
#include "Game/FE/FEPresentation.h"
#include "Game/FE/tlSlide.h"
#include "Game/OverlayManager.h"
#include "NL/nlSingleton.h"
#include "NL/nlTask.h"
#include "Game/FE/feInput.h"

#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feTemplates.h"

#include "types.h"

extern FEInput* g_pFEInput;

static char* TEAM_SLIDE_NAMES[8] = {
    "DAISY",
    "DK",
    "LUIGI",
    "MARIO",
    "PEACH",
    "WALUIGI",
    "WARIO",
    "YOSHI"
}; // size: 0x20, address: 0x802BFE60

static const char* OVERLAY_HANDLER_LAYER_NAME = "Layer"; // size: 0x4, address: 0x80395EAC

static const struct InGameTextEntry IGTTable[8] = {
    { SLIDE_NAME_TEXT_GOAL, "GOAL!", 0 },
    { SLIDE_NAME_TEXT_KICKOFF, "KICKOFF!", 0 },
    { SLIDE_NAME_TEXT_WINNER, "WINNER!", 1 },
    { SLIDE_NAME_TEXT_PAUSE, "Pause", 1 },
    { SLIDE_NAME_TEXT_TIE, "TIE!", 1 },
    { SLIDE_NAME_TEXT_LOADING, "LOADING...", 1 },
    { SLIDE_NAME_TEXT_SHOOT, "Shoot!", 2 },
    { SLIDE_NAME_TEXT_REPLAY, "REPLAY", 16 }
}; // size: 0x60, address: 0x802AD8E0

/**
 * Offset/Address/Size: 0xBC | 0x800FC998 | size: 0x208
 */
/*
void BasicString<unsigned short, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
{
}
*/
/**
 * Offset/Address/Size: 0x0 | 0x800FC8DC | size: 0xBC
 */
// void BasicString<unsigned short, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<unsigned short, Detail::TempStringAllocator>&) const
//{
// }

/**
 * Offset/Address/Size: 0x678 | 0x800FC780 | size: 0x15C
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x5F4 | 0x800FC6FC | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x570 | 0x800FC678 | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x538 | 0x800FC640 | size: 0x38
 */
// void FEFinder<TLTextInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
//{
// }

/**
 * Offset/Address/Size: 0x3DC | 0x800FC4E4 | size: 0x15C
 */
// void FEFinder<TLInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x358 | 0x800FC460 | size: 0x84
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 3>::_Find<TLSlide>(
    TLSlide* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->m_instances, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->m_instances, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x2D4 | 0x800FC3DC | size: 0x84
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 3>::_Find<FEPresentation>(
    FEPresentation* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLSlide>(pTopLevel->m_slides, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLSlide>(CastToSomeType<TLSlide>(pTopLevel->m_slides, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x29C | 0x800FC3A4 | size: 0x38
 */
// void FEFinder<TLInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
//{
// }

/**
 * Offset/Address/Size: 0x140 | 0x800FC248 | size: 0x15C
 */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0xBC | 0x800FC1C4 | size: 0x84
 */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x38 | 0x800FC140 | size: 0x84
 */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x0 | 0x800FC108 | size: 0x38
 */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
//{
// }

/**
 * Offset/Address/Size: 0x0 | 0x800FC098 | size: 0x70
 */
/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler
 * lays out the related fields to match the original binary.
 */
void OverlayHandlerInGameText_stub()
{
    void (*volatile forceTrack)(ePlayerStats, int, int, int, int, int, int) = &StatsTracker::Track;
    (void)forceTrack;
    TLInstance* (*volatile forceFind)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher) = &FEFinder<TLInstance, 3>::Find<FEPresentation>;
    (void)forceFind;
}

/**
 * Offset/Address/Size: 0xF44 | 0x800FBFF0 | size: 0xA8
 */
InGameTextOverlay::InGameTextOverlay()
    : BaseOverlayHandler(2, POSITION_ALL)
{
    mCurrentSlideName = SLIDE_NAME_INVALID;
    mPendingSlideName = SLIDE_NAME_INVALID;
    this->SetVisible(false);
}

/**
 * Offset/Address/Size: 0xED8 | 0x800FBF84 | size: 0x6C
 */
InGameTextOverlay::~InGameTextOverlay()
{
}

/**
 * Offset/Address/Size: 0xE1C | 0x800FBEC8 | size: 0xBC
 */
void InGameTextOverlay::SetSlide(OverlaySlideName slideName)
{
    this->mPendingSlideName = slideName;
    if (this->mCurrentSlideName != this->mPendingSlideName)
    {
        this->m_pFEScene->m_pFEPackage->GetPresentation()->SetActiveSlide(IGTTable[this->mPendingSlideName].mSlideName);
        TLSlide* CurrentSlide = this->m_pFEScene->m_pFEPackage->GetPresentation()->m_currentSlide;
        if (CurrentSlide != NULL)
        {
            CurrentSlide->m_time = 0.0f;
            CurrentSlide->m_start = 0.0f;
            CurrentSlide->Update(0.0f);
        }
        if (mCurrentSlideName != SLIDE_NAME_INVALID)
        {
            this->m_pFEScene->m_pFEPackage->GetPresentation()->SetActiveSlide(IGTTable[mCurrentSlideName].mSlideName);
        }
    }
}

/**
 * Offset/Address/Size: 0xCB0 | 0x800FBD5C | size: 0x16C
 */
void InGameTextOverlay::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    if (this->mCurrentSlideName != this->mPendingSlideName)
    {
        this->mCurrentSlideName = this->mPendingSlideName;
        this->m_pFEScene->m_pFEPackage->GetPresentation()->SetActiveSlide(IGTTable[this->mCurrentSlideName].mSlideName);
        this->mVisibilityMask = IGTTable[this->mCurrentSlideName].mTaskVisibility;
        if (this->mVisibilityMask & nlTaskManager::m_pInstance->m_CurrState)
        {
            if (mWasLastVisible)
            {
                this->SetVisible(true);
            }
        }
        else
        {
            mWasLastVisible = m_bVisible;
            this->SetVisible(false);
        }

        switch (this->mCurrentSlideName)
        {
        case SLIDE_NAME_TEXT_WINNER:
            DisplayFinalScore();
            break;
        }
    }
    if (this->mCurrentSlideName == SLIDE_NAME_TEXT_WINNER && g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL) && m_bVisible)
    {
        nlSingleton<OverlayManager>::s_pInstance->SetVisible(OVERLAY_TEXT, false, false);
        SummaryOverlay* handler = (SummaryOverlay*)nlSingleton<OverlayManager>::s_pInstance->Push(OVERLAY_SUMMARY, SCREEN_NOTHING, false);
        handler->mButtonState = ButtonComponent::BS_A_ONLY;
    }
}

/**
 * Offset/Address/Size: 0xCAC | 0x800FBD58 | size: 0x4
 */
void InGameTextOverlay::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800FB0AC | size: 0xCAC
 */
void InGameTextOverlay::DisplayFinalScore()
{
    FORCE_DONT_INLINE;
}
