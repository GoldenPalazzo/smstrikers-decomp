#define FEPOPUPMENU_INTERNAL_BYVAL
#include "Game/FE/fePopupMenu.h"

#include "Game/BaseGameSceneManager.h"
#include "Game/GameSceneManager.h"
#include "Game/OverlayManager.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/FE/feTemplates.h"
#include "NL/nlBSearch.h"
#include "NL/nlConfig.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlLocalization.h"
#include "NL/nlFormat.h"
#include "NL/gl/gl.h"
#include "Game/GameInfo.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/DB/SaveLoad.h"
#include "NL/nlString.h"

extern char* optionNames[4];

extern void* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

// Format with one substitution arg, used by the special-case popup arms.
template <typename StringType, typename T1>
StringType Format(const StringType& format, const T1& value1);

struct PopupEntry
{
    int mMessageType;
    unsigned long mMessage;
    unsigned long mOptions[4];
    int mInitialHighlight;
};

static const PopupEntry PopupEntries[31] = {
    { 0, 0x53FF99B4, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  0 POPUP_END_CUP
    { 2, 0xC5DF4FD6, { 0x56970FCC, 0, 0, 0 }, 0 },          //  1 POPUP_FLOWER_CUP_LOCKED
    { 2, 0x9FEC7DA1, { 0x56970FCC, 0, 0, 0 }, 0 },          //  2 POPUP_STAR_CUP_LOCKED
    { 2, 0xB49B9A79, { 0x56970FCC, 0, 0, 0 }, 0 },          //  3 POPUP_BOWSER_CUP_LOCKED
    { 2, 0xAF3B0976, { 0x56970FCC, 0, 0, 0 }, 0 },          //  4 POPUP_SUPER_CUPS_LOCKED
    { 0, 0x8C8DB4C5, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  5 POPUP_INGAME_FORFEIT_MATCH
    { 1, 0xE3096A19, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  6 POPUP_INGAME_QUIT_MATCH
    { 1, 0xCF0DA0B0, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  7 POPUP_INGAME_QUIT_STRIKERS_101
    { 0, 0xF5BCD42D, { 0x56970FCC, 0, 0, 0 }, 0 },          //  8 POPUP_NO_SIDES_CHOSEN
    { 0, 0x59C0BC5B, { 0x56970FCC, 0, 0, 0 }, 0 },          //  9 POPUP_NO_HUMAN_TOURNAMENT
    { 0, 0xFB7A0CFB, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 10 POPUP_START_NEW_CUP
    { 0, 0xFB7A0CFB, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 11 POPUP_START_NEW_TOURNAMENT
    { 0, 0x36BB72B1, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 12 POPUP_FILLALLSLOTS
    { 0, 0x892C84F9, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 13 POPUP_REVERT_OPTION_CHANGES
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 14 POPUP_TOURNEY_OVER
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 15 POPUP_NO_FORFEIT
    { 0, 0x892C84F9, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 16 POPUP_REALLY_OVERWRITE
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 17 POPUP_APPLYING_AUDIO
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 18 POPUP_NO_MEMCARD
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 19 POPUP_MEMCARD_CORRUPTED
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 20 POPUP_MEMCARD_WRONGFORMAT
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 21 POPUP_FILE_CORRUPTED
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 22 POPUP_MEMCARD_DAMAGED
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 23 POPUP_WRONG_DEVICE
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 24 POPUP_NOT_ENOUGH_SPACE
    { 0, 0x892C84F9, { 0x56970FCC, 0, 0, 0 }, 0 },          // 25 POPUP_NOT_ENOUGH_SPACE_CANMANAGE
    { 0, 0x892C84F9, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 26 POPUP_ABOUTTOSAVE
    { 0, 0x892C84F9, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 27 POPUP_NOTSAMECARD
    { 0, 0x892C84F9, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 28 POPUP_MEMCARD_ASK_SAVE_OVERWRITE
    { 0, 0, { 0, 0, 0, 0 }, 0 },
    { 0, 0, { 0, 0, 0, 0 }, 0 },
};

static inline const unsigned short* LookupLoc(unsigned long key)
{
    nlLocalization* loc = (nlLocalization*)g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, loc->m_pFile->StringCount);
    if (entry != 0)
    {
        return loc->m_FirstString + entry->StringOffset;
    }
    return MissingLocString;
}

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
    feVector3 optionPosition;
    float optionHeight;
    float prevOptionHeight;
    float totalHeight;
    float topOfMessage;
    FEPresentation* presentation;
    TLTextInstance* pText;
    TLComponentInstance* pHighlight;
    feVector3 messagePosition;
    nlTextBox::StringDrawInfo drawInfo;
    float messageHeight;
    float highlightScale;
    nlColour colour;
    int i;
    nlColour colour2;
    int i2;
    float optionY;
    nlColour optionColour;
    feVector3 highlightPosition;
    TLInstance* pImage;

    presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    pHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Message")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    messagePosition = pText->GetAssetPosition();
    drawInfo = pText->m_DrawInfo;
    messageHeight = (float)(drawInfo.RowCount * drawInfo.pFont->m_Metrics.RenderHeight);
    prevOptionHeight = messageHeight * 0.5f;
    totalHeight = messageHeight;
    topOfMessage = messagePosition.e[1] + prevOptionHeight;

    if (messageHeight == 0.0f)
    {
        pHighlight->m_bVisible = false;

        colour = pText->GetAssetColour();
        colour.c[3] = 0;
        pText->SetAssetColour(colour);

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

            colour = pText->GetAssetColour();
            colour.c[3] = 0;
            pText->SetAssetColour(colour);
        }

        glDiscardFrame(1);
        return;
    }

    colour2 = pText->GetAssetColour();
    colour2.c[3] = 0xFF;
    pText->SetAssetColour(colour2);

    Config& cfg = Config::Global();
    TagValuePair& firstOptionSpacingTvp = cfg.FindTvp("popup_first_option_spacing");
    float firstOptionSpacing;

    if (firstOptionSpacingTvp.tag == NULL)
    {
        cfg.Set("popup_first_option_spacing", 75.0f);
        firstOptionSpacing = 75.0f;
    }
    else if (firstOptionSpacingTvp.type == _BOOL)
    {
        firstOptionSpacing = LexicalCast<float, bool>(firstOptionSpacingTvp.value.b);
    }
    else if (firstOptionSpacingTvp.type == _INT)
    {
        firstOptionSpacing = LexicalCast<float, int>(firstOptionSpacingTvp.value.i);
    }
    else if (firstOptionSpacingTvp.type == _FLOAT)
    {
        firstOptionSpacing = LexicalCast<float, float>(firstOptionSpacingTvp.value.f);
    }
    else if (firstOptionSpacingTvp.type == _STRING)
    {
        firstOptionSpacing = LexicalCast<float, const char*>(firstOptionSpacingTvp.value.s);
    }
    else
    {
        firstOptionSpacing = 0.0f;
    }

    Config& cfg2 = Config::Global();
    TagValuePair& otherOptionSpacingTvp = cfg2.FindTvp("popup_other_option_spacing");
    float otherOptionSpacing;

    if (otherOptionSpacingTvp.tag == NULL)
    {
        cfg2.Set("popup_other_option_spacing", 12.5f);
        otherOptionSpacing = 12.5f;
    }
    else if (otherOptionSpacingTvp.type == _BOOL)
    {
        otherOptionSpacing = LexicalCast<float, bool>(otherOptionSpacingTvp.value.b);
    }
    else if (otherOptionSpacingTvp.type == _INT)
    {
        otherOptionSpacing = LexicalCast<float, int>(otherOptionSpacingTvp.value.i);
    }
    else if (otherOptionSpacingTvp.type == _FLOAT)
    {
        otherOptionSpacing = LexicalCast<float, float>(otherOptionSpacingTvp.value.f);
    }
    else if (otherOptionSpacingTvp.type == _STRING)
    {
        otherOptionSpacing = LexicalCast<float, const char*>(otherOptionSpacingTvp.value.s);
    }
    else
    {
        otherOptionSpacing = 0.0f;
    }

    highlightScale = 0.0f;

    for (i2 = 0; i2 < mPopup.numOptions; i2++)
    {
        pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(optionNames[i2])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        optionColour = pText->GetAssetColour();
        optionColour.c[3] = 0xFF;
        pText->SetAssetColour(optionColour);

        drawInfo = pText->m_DrawInfo;
        optionHeight = (float)(drawInfo.RowCount * drawInfo.pFont->m_Metrics.RenderHeight);

        totalHeight += optionHeight;

        if (i2 == 0)
        {
            totalHeight += firstOptionSpacing;
            optionY = (messagePosition.e[1] - prevOptionHeight) - (optionHeight * 0.5f) - firstOptionSpacing;
        }
        else
        {
            totalHeight += otherOptionSpacing;
            optionY = optionPosition.e[1] - (prevOptionHeight * 0.5f) - (optionHeight * 0.5f) - otherOptionSpacing;
        }

        prevOptionHeight = optionHeight;

        optionPosition = pText->GetAssetPosition();
        pText->SetAssetPosition(optionPosition.e[0], optionY, optionPosition.e[2]);

        if (i2 == mHighlightedOption)
        {
            highlightScale = (float)drawInfo.RowCount;
        }
    }

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    highlightPosition = pHighlight->GetAssetPosition();

    pImage = (TLInstance*)FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        pHighlight->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Highlight")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mHighlightSize = pImage->GetAssetScale();

    CentrePopup(totalHeight, topOfMessage);

    optionPosition = pText->GetAssetPosition();
    pHighlight->SetAssetPosition(highlightPosition.e[0], optionPosition.e[1], highlightPosition.e[2]);

    pImage->SetAssetScale(mHighlightSize.e[0], mHighlightSize.e[1] * highlightScale, mHighlightSize.e[2]);

    pHighlight->m_bVisible = true;
    mMenuDisplayed = true;
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

        if (mUnknownA64.mTag == 0)
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
        if (mUnknownA64.mTag == 0)
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
 * TODO: 17.23% match - option-label string copy/update loop and popup-open event
 * selection path are still incomplete.
 */
void FEPopupMenu::SceneCreated()
{
    struct PopupStringData
    {
        unsigned short* mData;
        int mSize;
        int mCapacity;
        int mRefCount;
    };

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    TLTextInstance* pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Message")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    PopupStringData*& textData = reinterpret_cast<PopupStringData*&>(mPopup.pMessage);
    PopupStringData* oldData = textData;

    if (oldData == NULL)
    {
        PopupStringData* data = (PopupStringData*)nlMalloc(0x10, 8, true);
        if (data != NULL)
        {
            data->mData = (unsigned short*)nlMalloc(2, 8, true);
            data->mSize = 1;
            data->mCapacity = 1;
            data->mRefCount = 1;
            data->mData[0] = 0;
        }
        textData = data;
    }
    else if (oldData->mRefCount != 1)
    {
        PopupStringData* data = (PopupStringData*)nlMalloc(0x10, 8, true);
        if (data != NULL)
        {
            data->mData = (unsigned short*)nlMalloc(oldData->mSize * 2, 8, true);
            data->mSize = oldData->mSize;
            data->mCapacity = oldData->mSize;

            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = oldData->mData[i];
            }

            data->mRefCount = 1;
        }

        if (--oldData->mRefCount == 0)
        {
            if (oldData != NULL)
            {
                delete[] oldData->mData;
            }
            if (oldData != NULL)
            {
                nlFree(oldData);
            }
        }

        oldData = data;
        textData = oldData;
    }

    pText->SetString((textData != NULL) ? textData->mData : NULL);

    if (mUnknownAA5)
    {
        pText->m_OverloadedAttributes.BoxSize.e[0] = 650.0f;
        pText->m_OverloadFlags |= 4;
    }

    for (int i = mPopup.numOptions; i < 4; i++)
    {
        pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(optionNames[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        if (pText != NULL)
        {
            pText->m_bVisible = false;
        }
    }

    FEAudio::EnableSounds(true);
    FEAudio::EnableSounds(false);

    TLComponentInstance* pHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pHighlight->SetActiveSlide("idle");

    mButtons.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        pHighlight->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (mButtons.mButtonInstance != NULL)
    {
        mButtons.mButtonInstance->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x35DC | 0x8009B888 | size: 0x258
 */
FEPopupMenu::~FEPopupMenu()
{
    for (int i = 0; i < mPopup.numOptions; i++)
    {
        delete mPopup.pOptionLabels[i];
    }

    if (mPopup.pMessage != NULL)
    {
        delete mPopup.pMessage;
    }

    g_pFEInput->PopExclusiveInputLock(this);
    FEAudio::EnableSounds(true);
    FEAudio::PlayAnimAudioEvent("sfx_back", false);
    FEAudio::EnableSounds(false);

    if (mRunCallBack == true)
    {
        Function<FnVoidVoid>& callback = callBacks[mHighlightedOption];
        if (callback.mTag == FREE_FUNCTION)
        {
            callback.mFreeFunction();
        }
        else
        {
            callback.mFunctor->operator()();
        }
    }
    else if (mUnknownA1F != false)
    {
        if (mUnknownA64.mTag == FREE_FUNCTION)
        {
            mUnknownA64.mFreeFunction();
        }
        else
        {
            mUnknownA64.mFunctor->operator()();
        }
    }

    FEAudio::EnableSounds(true);
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

/**
 * Offset/Address/Size: 0x3000 | 0x8009916C | size: 0x1BB8
 *
 * Current repo match: ~34.5%. Implemented so far:
 *   - prolog + mMenuCreated early-out
 *   - bounds check (type > 30 -> default) + PopupEntries[type] lookup
 *   - jump-table dispatch with 5 distinct arms emitted
 *   - case 0  (POPUP_END_CUP): LookupLoc + Format with current mode name
 *   - case 10 (POPUP_START_NEW_CUP): same Format-with-mode pattern
 *   - case 11 (POPUP_START_NEW_TOURNAMENT): Format with GM_TOURNAMENT mode name
 *                  - top half only; cup-state-conditional option strings TODO
 *   - case 16 (POPUP_REALLY_OVERWRITE): same Format-with-mode pattern
 *   - default arm (covers 22 of 31 enum values): plain LookupLoc + options
 *   - tail: callBacks[0..3] = optionN; mMenuCreated/mType/mHighlightedOption
 *
 * Special arms still TODO:
 *   case 13 -> .L_8009A6DC (REVERT_OPTION_CHANGES) - scene-state-dependent
 *               text via GameSceneManager::GetScene(0x26/0x3b/0x3c) + 5-way
 *               localized template dispatch on r30 = 0..5
 *   case 18/24/25/26/30 -> .L_8009A498 (memcard family) - needs
 *               SaveLoad::GetSaveBlockSize + LexicalCast<BString<char>,int>
 *               + nlStrToWcs + Format<WStr, unsigned short[4]>. Attempted
 *               implementation regressed function match (instruction
 *               alignment quirk) - needs more careful matching pass.
 *   case 11 deep tail: cup-type dispatch on `mCustomTournamentInfo.m_cup`
 *               field (-4/-3/-2/-1/normal) + GetLOCRank-based option
 *               formatting. ~1500 bytes of additional codegen.
 *
 * Mangling note: by-value parameter mangling is locked in via the
 * FEPOPUPMENU_INTERNAL_BYVAL header hack at the top of this TU. Other TUs
 * see Function<FnVoidVoid>& and emit R-prefixed unresolved bl references,
 * which objdiff treats as equivalent - so the 100% matches in
 * confirmNewTourn/UpdateForSubOptionMenus/Create(ePopupMenu) are preserved.
 */
void FEPopupMenu::Create(
    ePopupMenu type,
    Function<FnVoidVoid> option1,
    Function<FnVoidVoid> option2,
    Function<FnVoidVoid> option3,
    Function<FnVoidVoid> option4)
{
    if (mMenuCreated)
    {
        return;
    }

    const PopupEntry* popupentry = &PopupEntries[type];

    switch (type)
    {
    case POPUP_END_CUP: // 0 - .L_8009A230
    {
        typedef BasicString<unsigned short, Detail::TempStringAllocator> WStr;
        WStr modeStr(LookupLoc(GetLOCModeName(GameInfoManager::s_pInstance->mCurrentMode)));
        WStr msgStr(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new WStr(Format<WStr, WStr>(msgStr, modeStr));
        mPopup.numOptions = 0;
        for (int i = 0; i < 4; i++)
        {
            if (popupentry->mOptions[i] != 0)
            {
                mPopup.pOptionLabels[i] = new WStr(LookupLoc(popupentry->mOptions[i]));
                mPopup.numOptions++;
            }
            else
            {
                mPopup.pOptionLabels[i] = NULL;
            }
        }
        break;
    }
    case POPUP_FLOWER_CUP_LOCKED:        // 1
    case POPUP_STAR_CUP_LOCKED:          // 2
    case POPUP_BOWSER_CUP_LOCKED:        // 3
    case POPUP_SUPER_CUPS_LOCKED:        // 4
    case POPUP_INGAME_FORFEIT_MATCH:     // 5
    case POPUP_INGAME_QUIT_MATCH:        // 6
    case POPUP_INGAME_QUIT_STRIKERS_101: // 7
    case POPUP_NO_SIDES_CHOSEN:          // 8
    case POPUP_NO_HUMAN_TOURNAMENT:      // 9
    case POPUP_START_NEW_CUP:            // 10 - .L_8009994C
    {
        typedef BasicString<unsigned short, Detail::TempStringAllocator> WStr;
        WStr modeStr(LookupLoc(GetLOCModeName(GameInfoManager::s_pInstance->mCurrentMode)));
        WStr msgStr(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new WStr(Format<WStr, WStr>(msgStr, modeStr));
        mPopup.numOptions = 0;
        for (int i = 0; i < 4; i++)
        {
            if (popupentry->mOptions[i] != 0)
            {
                mPopup.pOptionLabels[i] = new WStr(LookupLoc(popupentry->mOptions[i]));
                mPopup.numOptions++;
            }
            else
            {
                mPopup.pOptionLabels[i] = NULL;
            }
        }
        break;
    }
    case POPUP_START_NEW_TOURNAMENT: // 11 - .L_800991D0
    {
        typedef BasicString<unsigned short, Detail::TempStringAllocator> WStr;
        WStr teamStr(LookupLoc(GetLOCModeName(GameInfoManager::GM_TOURNAMENT)));
        WStr msgStr(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new WStr(Format<WStr, WStr>(msgStr, teamStr));
        mPopup.numOptions = 0;
        for (int i = 0; i < 4; i++)
        {
            if (popupentry->mOptions[i] != 0)
            {
                mPopup.pOptionLabels[i] = new WStr(LookupLoc(popupentry->mOptions[i]));
                mPopup.numOptions++;
            }
            else
            {
                mPopup.pOptionLabels[i] = NULL;
            }
        }
        break;
    }
    case POPUP_FILLALLSLOTS:          // 12
    case POPUP_REVERT_OPTION_CHANGES: // 13 - .L_8009A6DC  TODO
    case POPUP_TOURNEY_OVER:          // 14
    case POPUP_NO_FORFEIT:            // 15
    case POPUP_REALLY_OVERWRITE:      // 16 - .L_80099FC8
    {
        typedef BasicString<unsigned short, Detail::TempStringAllocator> WStr;
        WStr modeStr(LookupLoc(GetLOCModeName(GameInfoManager::s_pInstance->mCurrentMode)));
        WStr msgStr(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new WStr(Format<WStr, WStr>(msgStr, modeStr));
        mPopup.numOptions = 0;
        for (int i = 0; i < 4; i++)
        {
            if (popupentry->mOptions[i] != 0)
            {
                mPopup.pOptionLabels[i] = new WStr(LookupLoc(popupentry->mOptions[i]));
                mPopup.numOptions++;
            }
            else
            {
                mPopup.pOptionLabels[i] = NULL;
            }
        }
        break;
    }
    case POPUP_APPLYING_AUDIO:             // 17
    case POPUP_NO_MEMCARD:                 // 18 - .L_8009A498  TODO
    case POPUP_MEMCARD_CORRUPTED:          // 19
    case POPUP_MEMCARD_WRONGFORMAT:        // 20
    case POPUP_FILE_CORRUPTED:             // 21
    case POPUP_MEMCARD_DAMAGED:            // 22
    case POPUP_WRONG_DEVICE:               // 23
    case POPUP_NOT_ENOUGH_SPACE:           // 24 - .L_8009A498  TODO
    case POPUP_NOT_ENOUGH_SPACE_CANMANAGE: // 25 - .L_8009A498  TODO
    case POPUP_ABOUTTOSAVE:                // 26 - .L_8009A498  TODO
    case POPUP_NOTSAMECARD:                // 27
    case POPUP_MEMCARD_ASK_SAVE_OVERWRITE: // 28
    case 29:
    case 30: // - .L_8009A498  TODO
    default:
    {
        if (popupentry->mMessage != 0)
        {
            mPopup.pMessage = new BasicString<unsigned short, Detail::TempStringAllocator>(LookupLoc(popupentry->mMessage));
        }
        else
        {
            mPopup.pMessage = NULL;
        }

        mPopup.numOptions = 0;
        for (int i = 0; i < 4; i++)
        {
            if (popupentry->mOptions[i] != 0)
            {
                mPopup.pOptionLabels[i] = new BasicString<unsigned short, Detail::TempStringAllocator>(LookupLoc(popupentry->mOptions[i]));
                mPopup.numOptions++;
            }
            else
            {
                mPopup.pOptionLabels[i] = NULL;
            }
        }
        break;
    }
    }

    callBacks[0] = option1;
    callBacks[1] = option2;
    callBacks[2] = option3;
    callBacks[3] = option4;

    mMenuCreated = true;
    mType = type;
    mHighlightedOption = popupentry->mInitialHighlight;
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
