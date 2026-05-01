#include "Game/FE/fePopupMenu.h"

#include "Game/BaseGameSceneManager.h"
#include "Game/GameSceneManager.h"
#include "Game/OverlayManager.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/FE/feTemplates.h"
#include "NL/nlBSearch.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlLocalization.h"
#include "NL/nlFormat.h"
#include "NL/gl/gl.h"

extern char* optionNames[4];

/**
 * Offset/Address/Size: 0xA8 | 0x80098354 | size: 0xBC
 */
void FEPopupMenu::SetOptionTextColourOnCurrent(bool)
{
    FORCE_DONT_INLINE;
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
}

/**
 * Offset/Address/Size: 0x164 | 0x80098410 | size: 0x2FC
 * TODO: 99.5% match - stack frame 0x150 vs 0x130: MWCC not reusing InlineHasher(0)
 * argument copy stack locations across Find calls (all diffs are stack offsets only).
 */
void FEPopupMenu::ResizeHighlight()
{
    FEPresentation* presentation;
    TLTextInstance* pText;
    TLComponentInstance* pHighlight;
    feVector3 textPosition;
    feVector3 highlightPosition;

    presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pText->SetAssetColour(mHighlightedOptionColour);

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    nlTextBox::StringDrawInfo drawInfo = pText->m_DrawInfo;

    pHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    textPosition = pText->GetAssetPosition();
    highlightPosition = pHighlight->GetAssetPosition();
    pHighlight->SetAssetPosition(highlightPosition.e[0], textPosition.e[1], highlightPosition.e[2]);
    pHighlight->SetActiveSlide(pHighlight->GetActiveSlide());
    pHighlight->Update(0.0f);

    ((TLInstance*)FEFinder<TLImageInstance, 2>::Find<TLSlide>(
         pHighlight->GetActiveSlide(),
         InlineHasher(nlStringLowerHash("Highlight")),
         InlineHasher(0),
         InlineHasher(0),
         InlineHasher(0),
         InlineHasher(0),
         InlineHasher(0)))
        ->SetAssetScale(
            mHighlightSize.e[0],
            mHighlightSize.e[1] * (float)drawInfo.RowCount,
            mHighlightSize.e[2]);

    SetOptionTextColourOnCurrent(true);
}

/**
 * Offset/Address/Size: 0x460 | 0x8009870C | size: 0x1C0
 * TODO: 93.76% match - stack frame 0xa0 vs 0x90: MWCC not reusing InlineHasher(0) argument copy
 * stack locations between pre-loop and loop Find calls.
 */
void FEPopupMenu::CentrePopup(float totalHeight, float topOfMessageBox)
{
    float half;
    float offset;
    FEPresentation* presentation;
    TLTextInstance* pText;
    feVector3 position;
    int i;

    half = totalHeight;
    half *= 0.5f;
    offset = half - topOfMessageBox;
    presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Message")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    position = pText->GetAssetPosition();
    pText->SetAssetPosition(position.e[0], position.e[1] + offset, position.e[2]);

    for (i = 0; i < mPopup.numOptions; i++)
    {
        pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(optionNames[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        position = pText->GetAssetPosition();
        pText->SetAssetPosition(position.e[0], position.e[1] + offset, position.e[2]);
    }
}

/**
 * Offset/Address/Size: 0x620 | 0x800988CC | size: 0x8A0
 */
void FEPopupMenu::SetPositions()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x2B24 | 0x8009ADD0 | size: 0x400
 * TODO: 98.4% match - 4 missing instructions (b+lwz merge pattern) in
 * decrement/increment wrap-around blocks due to -inline deferred scratch flag
 */
void FEPopupMenu::Update(float fDeltaT)
{
    if (!mMenuCreated)
    {
        return;
    }

    if (!mMenuDisplayed)
    {
        SetPositions();

        if (mUnknownA64 == 0)
        {
            if (mButtons.mButtonInstance != NULL)
            {
                mButtons.mButtonInstance->m_bVisible = false;
            }
        }
        else
        {
            if (mButtons.mButtonInstance != NULL)
            {
                mButtons.mButtonInstance->m_bVisible = true;
            }
            mButtons.SetState(ButtonComponent::BS_A_AND_B);
            mButtons.CentreButtons();
        }

        if (!mUnknownAA4)
        {
            m_pFEPresentation->m_fadeDuration = 999.9f;
        }
    }

    BaseSceneHandler::Update(fDeltaT);

    if (m_pFEPresentation->m_currentSlide->m_time < m_pFEPresentation->m_currentSlide->m_start + m_pFEPresentation->m_currentSlide->m_duration)
    {
        return;
    }

    if (mAcceptDelayTime > 0.0f)
    {
        mAcceptDelayTime -= fDeltaT;
        if (mAcceptDelayTime <= 0.0f)
        {
            mAcceptDelayTime = 0.0f;

            BaseGameSceneManager* manager = nlSingleton<GameSceneManager>::s_pInstance;
            if (manager != NULL)
            {
                manager->Pop();
            }
            else
            {
                manager = nlSingleton<OverlayManager>::s_pInstance;
                if (manager != NULL)
                {
                    manager->Pop();
                }
            }

            mRunCallBack = true;
            glDiscardFrame(3);
        }

        return;
    }

    if (mPopup.numOptions <= 0)
    {
        return;
    }

    if (g_pFEInput->JustPressed(mControlInput, 0x100, false, NULL))
    {
        TLComponentInstance* pComponent = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
            m_pFEPresentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("highlite")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        pComponent->SetActiveSlide("accept");
        pComponent->Update(0.0f);
        TLSlide* pSlide = pComponent->GetActiveSlide();
        mAcceptDelayTime = pSlide->m_start + pSlide->m_duration;

        ResizeHighlight();
        FEAudio::EnableSounds(true);
        FEAudio::PlayAnimAudioEvent("sfx_popup_accept", false);
        FEAudio::EnableSounds(false);
        return;
    }

    if (g_pFEInput->JustPressed(mControlInput, 0x200, false, NULL))
    {
        if (mUnknownA64 == 0)
        {
            return;
        }

        FEAudio::EnableSounds(true);
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
        FEAudio::EnableSounds(false);

        BaseGameSceneManager* manager = nlSingleton<GameSceneManager>::s_pInstance;
        if (manager != NULL)
        {
            manager->Pop();
        }
        else
        {
            manager = nlSingleton<OverlayManager>::s_pInstance;
            if (manager != NULL)
            {
                manager->Pop();
            }
        }

        mUnknownA1F = true;
        return;
    }

    if (mPopup.numOptions <= 1)
    {
        return;
    }

    if (g_pFEInput->IsAutoPressed(mControlInput, 0xD, true, NULL))
    {
        SetOptionTextColourOnCurrent(false);

        int option = mHighlightedOption - 1;
        mHighlightedOption = option;
        mHighlightedOption = (option < 0) ? (mPopup.numOptions - 1) : mHighlightedOption;

        ResizeHighlight();
        FEAudio::EnableSounds(true);
        FEAudio::PlayAnimAudioEvent("sfx_popup_toggle_up", false);
        FEAudio::EnableSounds(false);
        return;
    }

    if (g_pFEInput->IsAutoPressed(mControlInput, 0xE, true, NULL))
    {
        SetOptionTextColourOnCurrent(false);

        int option = mHighlightedOption + 1;
        mHighlightedOption = option;
        mHighlightedOption = (option > mPopup.numOptions - 1) ? 0 : mHighlightedOption;

        ResizeHighlight();
        FEAudio::EnableSounds(true);
        FEAudio::PlayAnimAudioEvent("sfx_popup_toggle_down", false);
        FEAudio::EnableSounds(false);
    }
}

/**
 * Offset/Address/Size: 0x2F24 | 0x8009B1D0 | size: 0x6B8
 */
void FEPopupMenu::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x35DC | 0x8009B888 | size: 0x258
 */
FEPopupMenu::~FEPopupMenu()
{
}

/**
 * Offset/Address/Size: 0x3834 | 0x8009BAE0 | size: 0xFC
 */
FEPopupMenu::FEPopupMenu()
    : mMenuDisplayed(false)
    , mMenuCreated(false)
    , mRunCallBack(false)
    , mUnknownA1F(false)
    , mHighlightedOption(0)
    , mAcceptDelayTime(0.0f)
    , mControlInput(FE_ALL_PADS)
    , mUnknownA64(0)
    , mType(INVALID_TYPE)
    , mButtons()
    , mUnknownAA4(true)
    , mUnknownAA5(false)
{
    mPopup.numOptions = 0;
    mPopup.pMessage = NULL;
    mPopup.pOptionLabels[0] = NULL;
    mPopup.pOptionLabels[1] = NULL;
    mPopup.pOptionLabels[2] = NULL;
    mPopup.pOptionLabels[3] = NULL;

    g_pFEInput->PushExclusiveInputLock(this, 0x1B);
    FEAudio::EnableSounds(false);
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8009BBDC | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x8009BC14 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x8009BC98 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x8009BD1C | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x29C | 0x8009BE78 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x8009BEB0 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x358 | 0x8009BF34 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x4B4 | 0x8009C090 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x8009C0C8 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x570 | 0x8009C14C | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x5F4 | 0x8009C1D0 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x750 | 0x8009C32C | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x788 | 0x8009C364 | size: 0x8
 */
template TLInstance* CastToSomeType<TLInstance>(TLInstance*, void*);

/**
 * Offset/Address/Size: 0x790 | 0x8009C36C | size: 0x8
 */
template TLSlide* CastToSomeType<TLSlide>(TLSlide*, void*);

/**
 * Offset/Address/Size: 0x798 | 0x8009C374 | size: 0x44
 */
template TLInstance* FindItemByHashID<TLInstance>(TLInstance*, unsigned long);

/**
 * Offset/Address/Size: 0x7DC | 0x8009C3B8 | size: 0x44
 */
template TLSlide* FindItemByHashID<TLSlide>(TLSlide*, unsigned long);

/**
 * Offset/Address/Size: 0x0 | 0x8009C3FC | size: 0x14
 */
template <>
BasicString<unsigned short, Detail::TempStringAllocator>&
BasicString<unsigned short, Detail::TempStringAllocator>::operator=(BasicString<unsigned short, Detail::TempStringAllocator> other)
{
    BasicStringData<unsigned short>* tmp = m_data;
    m_data = other.m_data;
    other.m_data = tmp;
    return *this;
}

// /**
//  * Offset/Address/Size: 0x14 | 0x8009C410 | size: 0x94
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::~BasicString()
// {
// }

// /**
//  * Offset/Address/Size: 0xA8 | 0x8009C4A4 | size: 0xF8
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::BasicString(const unsigned short*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1A0 | 0x8009C59C | size: 0x6B4
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::insert(unsigned short*, const unsigned short*, const unsigned short*)
// {
// }

// /**
//  * Offset/Address/Size: 0x854 | 0x8009CC50 | size: 0x1EC
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::erase(const unsigned short*, const unsigned short*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8009CE3C | size: 0x8C
//  */
// void nlBSearch<nlLocalization::StringLookup, unsigned long>(const unsigned long&, nlLocalization::StringLookup*, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8009CEC8 | size: 0x2C
//  */
// void LexicalCast<BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short,
// Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x2C | 0x8009CEF4 | size: 0x28
//  */
// void Detail::LexicalCastImpl<BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short,
// Detail::TempStringAllocator>>::Do(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x54 | 0x8009CF1C | size: 0x2C
//  */
// void LexicalCast<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*>(const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x8009CF48 | size: 0xF4
//  */
// void Detail::LexicalCastImpl<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*>::Do(const unsigned short*
// const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8009D03C | size: 0x13C
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short, Detail::TempStringAllocator>,
// BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short, Detail::TempStringAllocator>>(const
// BasicString<unsigned short, Detail::TempStringAllocator>&, const BasicString<unsigned short, Detail::TempStringAllocator>&, const
// BasicString<unsigned short, Detail::TempStringAllocator>&, const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x13C | 0x8009D178 | size: 0x28
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator BasicString<unsigned short,
// Detail::TempStringAllocator>() const
// {
// }

// /**
//  * Offset/Address/Size: 0x164 | 0x8009D1A0 | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<BasicString<unsigned short,
// Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0xE54 | 0x8009DE90 | size: 0x114
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short, Detail::TempStringAllocator>>(const
// BasicString<unsigned short, Detail::TempStringAllocator>&, const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0xF68 | 0x8009DFA4 | size: 0x118
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[4]>(const BasicString<unsigned short,
// Detail::TempStringAllocator>&, const unsigned short(&)[4])
// {
// }

// /**
//  * Offset/Address/Size: 0x1080 | 0x8009E0BC | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&)
// {
// }

// Stub to force template instantiations -- REMOVE once real callers exist.
void fePopupMenu_stub()
{
    WideBasicString s;
    unsigned short value[4] = { 0, 0, 0, 0 };
    typedef WideBasicString (*FmtFn)(const WideBasicString&, const unsigned short (&)[4]);
    volatile FmtFn fn = &Format<WideBasicString, unsigned short[4]>;
    LexicalCast<WideBasicString, WideBasicString>(s);
    const unsigned short* p = 0;
    LexicalCast<WideBasicString, const unsigned short*>(p);
    WideBasicString s3 = fn(s, value);
    FormatImpl<WideBasicString> fi;
    WideBasicString s2 = (WideBasicString)fi;

    unsigned long key = 0;
    nlBSearch<nlLocalization::StringLookup, unsigned long>(key, (nlLocalization::StringLookup*)0, 0);
}
