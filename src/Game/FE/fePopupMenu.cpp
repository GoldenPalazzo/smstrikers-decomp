#include "Game/FE/fePopupMenu.h"

#include "Game/BaseGameSceneManager.h"
#include "Game/GameSceneManager.h"
#include "Game/OverlayManager.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/FE/tlImageInstance.h"
#include "Game/FE/feText.h"
#include "NL/nlAlgorithm.h"
#include "NL/nlConfig.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlLocalization.h"
#include "NL/nlFormat.h"
#include "NL/gl/gl.h"
#include "Game/GameInfo.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/DB/SaveLoad.h"
#include "Game/SH/SHOptions.h"
#include "NL/nlString.h"

extern char* optionNames[4];

extern void* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

struct PopupEntry
{
    int mMessageType;
    unsigned long mMessage;
    unsigned long mOptions[4];
    int mInitialHighlight;
};

static const PopupEntry PopupEntries[] = {
    { 0, 0x53FF99B4, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  0 POPUP_END_CUP
    { 2, 0xC5DF4FD6, { 0x56970FCC, 0, 0, 0 }, 0 },          //  1 POPUP_FLOWER_CUP_LOCKED
    { 2, 0x9FEC7DA1, { 0x56970FCC, 0, 0, 0 }, 0 },          //  2 POPUP_STAR_CUP_LOCKED
    { 2, 0xB49B9A79, { 0x56970FCC, 0, 0, 0 }, 0 },          //  3 POPUP_BOWSER_CUP_LOCKED
    { 2, 0xAF3B0976, { 0x56970FCC, 0, 0, 0 }, 0 },          //  4 POPUP_SUPER_CUPS_LOCKED
    { 0, 0x8C8DB4C5, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  5 POPUP_INGAME_FORFEIT_MATCH
    { 1, 0xE3096A19, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  6 POPUP_INGAME_QUIT_MATCH
    { 1, 0xAF2EF7BB, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, //  7 POPUP_INGAME_QUIT_STRIKERS_101
    { 2, 0x83AEBE4A, { 0x56970FCC, 0, 0, 0 }, 0 },          //  8 POPUP_NO_SIDES_CHOSEN
    { 2, 0xBFAB9C0B, { 0x56970FCC, 0, 0, 0 }, 0 },          //  9 POPUP_NO_HUMAN_TOURNAMENT
    { 1, 0xE04E2367, { 0x93F834CC, 0x93F834CD, 0, 0 }, 0 }, // 10 POPUP_START_NEW_CUP
    { 1, 0xE04E2367, { 0xE4FA843C, 0xE4FA843D, 0, 0 }, 0 }, // 11 POPUP_START_NEW_TOURNAMENT
    { 2, 0x01D6E088, { 0x56970FCC, 0, 0, 0 }, 0 },          // 12 POPUP_FILLALLSLOTS
    { 1, 0x7401B743, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 13 POPUP_REVERT_OPTION_CHANGES
    { 1, 0x00768290, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 14 POPUP_TOURNEY_OVER
    { 0, 0x64886630, { 0x56970FCC, 0, 0, 0 }, 0 },          // 15 POPUP_NO_FORFEIT
    { 1, 0x4A4753B1, { 0x29793383, 0x56970FAF, 0, 0 }, 1 }, // 16 POPUP_REALLY_OVERWRITE
    { 0, 0xC19ADE86, { 0, 0, 0, 0 }, 0 },                  // 17 POPUP_APPLYING_AUDIO
    { 1, 0x657FEF59, { 0x6C158828, 0x265190F3, 0, 0 }, 0 }, // 18 POPUP_NO_MEMCARD
    { 1, 0x3BB17094, { 0x6C158828, 0x265190F3, 0xEEE94B74, 0 }, 0 }, // 19 POPUP_MEMCARD_CORRUPTED
    { 1, 0x3BB17094, { 0x6C158828, 0x265190F3, 0xEEE94B74, 0 }, 0 }, // 20 POPUP_MEMCARD_WRONGFORMAT
    { 1, 0xE7B97A1B, { 0x6C158828, 0x265190F3, 0x5474CFE5, 0 }, 0 }, // 21 POPUP_FILE_CORRUPTED
    { 1, 0xF5D2425F, { 0x6C158828, 0x265190F3, 0, 0 }, 0 }, // 22 POPUP_MEMCARD_DAMAGED
    { 1, 0x70C310A0, { 0x6C158828, 0x265190F3, 0, 0 }, 0 }, // 23 POPUP_WRONG_DEVICE
    { 1, 0x4A0F9827, { 0x6C158828, 0x265190F3, 0, 0 }, 0 }, // 24 POPUP_NOT_ENOUGH_SPACE
    { 1, 0x1ED2AA65, { 0x6C158828, 0x265190F3, 0xF6E79F0E, 0 }, 0 }, // 25 POPUP_NOT_ENOUGH_SPACE_CANMANAGE
    { 1, 0x702AE111, { 0x29793383, 0x265190F3, 0, 0 }, 0 }, // 26 POPUP_ABOUTTOSAVE
    { 1, 0xE9210639, { 0x6C158828, 0x265190F3, 0, 0 }, 0 }, // 27 POPUP_NOTSAMECARD
    { 1, 0xE151C8EE, { 0x265190F3, 0xF7B71D79, 0, 0 }, 0 }, // 28 POPUP_MEMCARD_ASK_SAVE_OVERWRITE
    { 1, 0x66620884, { 0xF4D56A69, 0x104AFE8A, 0, 0 }, 0 }, // 29 POPUP_MEMCARD_ASK_LOAD_OVERWRITE
    { 1, 0x533A9343, { 0x29793383, 0x265190F3, 0, 0 }, 0 }, // 30 POPUP_MEMCARD_ASK_SAVE_NO_FILE
    { 1, 0x8187DF41, { 0x265190F3, 0x29793383, 0, 0 }, 0 }, // 31 POPUP_MEMCARD_CONFIRM_FORMAT
    { 3, 0x3A310CB4, { 0x56970FCC, 0, 0, 0 }, 0 },          // 32 POPUP_UNLOCKED_FLOWER_CUP
    { 3, 0x2480233F, { 0x56970FCC, 0, 0, 0 }, 0 },          // 33 POPUP_UNLOCKED_STAR_CUP
    { 3, 0x8B820AD7, { 0x56970FCC, 0, 0, 0 }, 0 },          // 34 POPUP_UNLOCKED_BOWSER_CUP
    { 3, 0x96B3F7E7, { 0x56970FCC, 0, 0, 0 }, 0 },          // 35 POPUP_UNLOCKED_SUPER_CUPS
    { 3, 0x1D38741B, { 0x56970FCC, 0, 0, 0 }, 0 },          // 36 POPUP_UNLOCKED_CUSTOM_POWERUPS
    { 3, 0xD2D06999, { 0x56970FCC, 0, 0, 0 }, 0 },          // 37 POPUP_UNLOCKED_KONGA_STADIUM
    { 3, 0xB567B415, { 0x56970FCC, 0, 0, 0 }, 0 },          // 38 POPUP_UNLOCKED_YOSHI_STADIUM
    { 3, 0xACBF4236, { 0x56970FCC, 0, 0, 0 }, 0 },          // 39 POPUP_UNLOCKED_FORBIDDEN_STADIUM
    { 3, 0x96BCB7B8, { 0x56970FCC, 0, 0, 0 }, 0 },          // 40 POPUP_UNLOCKED_SUPER_STADIUM
    { 3, 0xB1857565, { 0x56970FCC, 0, 0, 0 }, 0 },          // 41 POPUP_UNLOCKED_LEGEND_DIFFICULTY
    { 3, 0x96BD0453, { 0x56970FCC, 0, 0, 0 }, 0 },          // 42 POPUP_UNLOCKED_SUPER_TEAM
    { 3, 0x364A97B2, { 0x56970FCC, 0, 0, 0 }, 0 },          // 43 POPUP_UNLOCKED_CHEAT_GOALIE
    { 3, 0x8A4F1917, { 0x56970FCC, 0, 0, 0 }, 0 },          // 44 POPUP_UNLOCKED_CHEAT_INFINITE
    { 3, 0xEBDC519E, { 0x56970FCC, 0, 0, 0 }, 0 },          // 45 POPUP_UNLOCKED_CHEAT_TILT
    { 3, 0x281B81D4, { 0x56970FCC, 0, 0, 0 }, 0 },          // 46 POPUP_UNLOCKED_ALL_STS
    { 3, 0x5072D40B, { 0x56970FCC, 0, 0, 0 }, 0 },
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
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])));
}

/**
 * Offset/Address/Size: 0x164 | 0x80098410 | size: 0x2FC
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
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])));
    pText->SetAssetColour(mHighlightedOptionColour);

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])));

    nlTextBox::StringDrawInfo drawInfo = pText->m_DrawInfo;

    pHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")));

    textPosition = pText->GetAssetPosition();
    highlightPosition = pHighlight->GetAssetPosition();
    pHighlight->SetAssetPosition(highlightPosition.e[0], textPosition.e[1], highlightPosition.e[2]);
    pHighlight->SetActiveSlide(pHighlight->GetActiveSlide());
    pHighlight->Update(0.0f);

    ((TLInstance*)FEFinder<TLImageInstance, 2>::Find<TLSlide>(
         pHighlight->GetActiveSlide(),
         InlineHasher(nlStringLowerHash("Highlight")),
         InlineHasher(0)))
        ->SetAssetScale(
            mHighlightSize.e[0],
            mHighlightSize.e[1] * (float)drawInfo.RowCount,
            mHighlightSize.e[2]);

    SetOptionTextColourOnCurrent(true);
}

/**
 * Offset/Address/Size: 0x460 | 0x8009870C | size: 0x1C0
 */
void FEPopupMenu::CentrePopup(float totalHeight, float topOfMessageBox)
{
    FORCE_DONT_INLINE;
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
        InlineHasher(nlStringLowerHash("Message")));

    position = pText->GetAssetPosition();
    pText->SetAssetPosition(position.e[0], position.e[1] + offset, position.e[2]);

    for (i = 0; i < mPopup.numOptions; i++)
    {
        pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(optionNames[i])));

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
    float prevOptionHeight = 0.0f;
    float totalHeight = 0.0f;
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
    TLImageInstance* pImage;
    TLComponentInstance* pFinalHighlight;

    presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    pHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")));

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Message")));

    messagePosition = pText->GetAssetPosition();
    const nlFont* pFont = ((const FEText*)pText->m_component)->m_pFeFontResource->m_font;
    struct Copy88
    {
        unsigned long w[22];
    };
    *(Copy88*)&drawInfo = *(Copy88*)&pText->m_DrawInfo;
    messageHeight = (float)(drawInfo.RowCount * pFont->m_Metrics.Height);
    totalHeight += messageHeight;
    topOfMessage = messagePosition.e[1] + (messageHeight / 2.0f);

    if (messageHeight == 0.0)
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
                InlineHasher(nlStringLowerHash(optionNames[i])));

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

    float firstOptionSpacing = GetConfigFloat(Config::Global(), "popup_first_option_spacing", 75.0f);
    float otherOptionSpacing = GetConfigFloat(Config::Global(), "popup_other_option_spacing", 12.5f);

    for (i2 = 0; i2 < mPopup.numOptions; i2++)
    {
        pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(optionNames[i2])));

        optionColour = pText->GetAssetColour();
        optionColour.c[3] = 0xFF;
        pText->SetAssetColour(optionColour);

        {
            const nlFont* pFont = ((const FEText*)pText->m_component)->m_pFeFontResource->m_font;
            nlTextBox::StringDrawInfo info = pText->m_DrawInfo;
            drawInfo = info;
            optionHeight = (float)(drawInfo.RowCount * pFont->m_Metrics.Height);
        }

        totalHeight += optionHeight;

        if (i2 == 0)
        {
            totalHeight += firstOptionSpacing;
            optionY = (messagePosition.e[1] - (messageHeight / 2.0f)) - (optionHeight / 2.0f) - firstOptionSpacing;
        }
        else
        {
            totalHeight += otherOptionSpacing;
            optionY = optionPosition.e[1] - (prevOptionHeight / 2.0f) - (optionHeight / 2.0f) - otherOptionSpacing;
        }

        prevOptionHeight = optionHeight;
        optionPosition = pText->GetAssetPosition();
        optionPosition.e[1] = optionY;
        pText->SetAssetPosition(optionPosition.e[0], optionPosition.e[1], optionPosition.e[2]);

        if (i2 == mHighlightedOption)
        {
            highlightScale = (float)(unsigned int)drawInfo.RowCount;
        }
    }

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash(optionNames[mHighlightedOption])));

    pFinalHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")));

    highlightPosition = pFinalHighlight->GetAssetPosition();

    pImage = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        pFinalHighlight->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Highlight")),
        InlineHasher(0));

    mHighlightSize = pImage->GetAssetScale();

    CentrePopup(totalHeight, topOfMessage);

    optionPosition = pText->GetAssetPosition();
    pFinalHighlight->SetAssetPosition(highlightPosition.e[0], optionPosition.e[1], highlightPosition.e[2]);

    pImage->SetAssetScale(mHighlightSize.e[0], mHighlightSize.e[1] * highlightScale, mHighlightSize.e[2]);

    pFinalHighlight->m_bVisible = true;
    mMenuDisplayed = true;
}

/**
 * Offset/Address/Size: 0x2B24 | 0x8009ADD0 | size: 0x400
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

        if (!mUnknownA64)
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
            InlineHasher(nlStringLowerHash("highlite")));

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
        if (!mUnknownA64)
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
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    int i;
    int k;

    TLTextInstance* pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Message")));

    pText->SetString(mPopup.pMessage->begin());

    if (mUnknownAA5)
    {
        nlVector2 boxSize = pText->m_OverloadedAttributes.BoxSize;
        boxSize.e[0] = 650.0f;
        pText->m_OverloadedAttributes.BoxSize = boxSize;
        pText->m_OverloadFlags |= 4;
    }

    for (i = 0; i < mPopup.numOptions; i++)
    {
        pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(optionNames[i])));

        pText->SetString(mPopup.pOptionLabels[i]->begin());

        if (i == 0)
        {
            mHighlightedOptionColour = pText->GetAssetColour();
        }
    }

    for (k = mPopup.numOptions; k < 4; k++)
    {
        pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(optionNames[k])));

        if (pText != NULL)
        {
            pText->m_bVisible = false;
        }
    }

    FEAudio::EnableSounds(true);

    switch (PopupEntries[mType].mMessageType)
    {
    case 0:
        FEAudio::PlayAnimAudioEvent("sfx_popup_open_normal", false);
        break;
    case 1:
        FEAudio::PlayAnimAudioEvent("sfx_popup_open_question", false);
        break;
    case 2:
        FEAudio::PlayAnimAudioEvent("sfx_popup_open_deny", false);
        break;
    case 3:
        FEAudio::PlayAnimAudioEvent("sfx_popup_open_unlocked", false);
        break;
    default:
        break;
    }

    FEAudio::EnableSounds(false);

    TLComponentInstance* pHighlight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")));

    pHighlight->SetActiveSlide("idle");

    mButtons.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")));

    if (mButtons.mButtonInstance != NULL)
    {
        mButtons.mButtonInstance->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800982AC | size: 0xA8
 */
void FEPopupMenu::SetBackButtonCallback(Function<FnVoidVoid> callback)
{
    mUnknownA64 = callback;
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
        callback();
    }
    else if (mUnknownA1F != false)
    {
        mUnknownA64();
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
    , mUnknownA64()
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
 * TODO: 52.16% match - LookupLoc calls emitted out-of-line here; the target inlines all 41.
 * Inlining them requires an inline-budget raise that also inlines the BasicString destructor,
 * which the target keeps out-of-line (45 calls). See notes fepopup-create-lookuploc-dtor-coupling.
 */
void FEPopupMenu::Create(
    ePopupMenu type,
    Function<FnVoidVoid> option1,
    Function<FnVoidVoid> option2,
    Function<FnVoidVoid> option3,
    Function<FnVoidVoid> option4)
{
    typedef BasicString<unsigned short, Detail::TempStringAllocator> WStr;
    typedef BasicString<char, Detail::TempStringAllocator> NStr;

    if (mMenuCreated)
    {
        return;
    }

    const PopupEntry* popupentry = &PopupEntries[type];

    switch (type)
    {
    case POPUP_START_NEW_TOURNAMENT:
    {
        WStr cupName(LookupLoc(GetLOCModeName(GameInfoManager::GM_TOURNAMENT)));
        WStr message(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new (8, false) WStr(Format<WStr, WStr>(message, cupName));

        WStr tournamentTypeName(LookupLoc(0x81A57D85));
        if (GameInfoManager::s_pInstance->mCustomTournamentInfo.m_tournMode == TM_LEAGUE)
        {
            tournamentTypeName = LookupLoc(0x81A57D85);
        }
        else
        {
            tournamentTypeName = LookupLoc(0x11295359);
        }

        int round = GameInfoManager::s_pInstance->mCustomTournamentInfo.m_cup->mRoundNumber;
        WStr roundString;
        if (round == -4)
        {
            roundString = LookupLoc(0xAC987EEC);
        }
        else if (round == -3)
        {
            roundString = LookupLoc(0x5030ECB6);
        }
        else if (round == -2 || round == -1)
        {
            roundString = LookupLoc(0x051C0F69);
        }
        else if (round < 0 || round >= 14)
        {
            roundString = LookupLoc(GetLOCRank(0));
        }
        else
        {
            roundString = LookupLoc(GetLOCRank(round));
        }

        message = LookupLoc(popupentry->mOptions[0]);
        mPopup.pOptionLabels[0] = new (8, false) WStr(Format<WStr, WStr, WStr, WStr>(message, cupName, tournamentTypeName, roundString));
        message = LookupLoc(popupentry->mOptions[1]);
        mPopup.pOptionLabels[1] = new (8, false) WStr(Format<WStr, WStr>(message, cupName));
        mPopup.pOptionLabels[2] = NULL;
        mPopup.pOptionLabels[3] = NULL;
        mPopup.numOptions = 2;
        break;
    }

    case POPUP_START_NEW_CUP:
    {
        WStr cupName(LookupLoc(GetLOCModeName(GameInfoManager::s_pInstance->mCurrentMode)));
        WStr message(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new (8, false) WStr(Format<WStr, WStr>(message, cupName));

        WStr captainName(LookupLoc(GetLOCTeamName(GameInfoManager::s_pInstance->GetUserSelectedCupTeam())));
        int round = GameInfoManager::s_pInstance->GetCurrentRoundNumber();
        WStr roundString;
        if (round == -4)
        {
            roundString = LookupLoc(0xAC987EEC);
        }
        else if (round == -3)
        {
            roundString = LookupLoc(0x5030ECB6);
        }
        else if (round == -2 || round == -1)
        {
            roundString = LookupLoc(0x051C0F69);
        }
        else if (round < 0 || round >= 14)
        {
            roundString = LookupLoc(GetLOCRank(0));
        }
        else
        {
            roundString = LookupLoc(GetLOCRank(round));
        }

        message = LookupLoc(popupentry->mOptions[0]);
        mPopup.pOptionLabels[0] = new (8, false) WStr(Format<WStr, WStr, WStr, WStr>(message, cupName, captainName, roundString));
        message = LookupLoc(popupentry->mOptions[1]);
        mPopup.pOptionLabels[1] = new (8, false) WStr(Format<WStr, WStr>(message, cupName));
        mPopup.pOptionLabels[2] = NULL;
        mPopup.pOptionLabels[3] = NULL;
        mPopup.numOptions = 2;
        break;
    }

    case POPUP_REALLY_OVERWRITE:
    {
        WStr cupName(LookupLoc(GetLOCModeName(GameInfoManager::s_pInstance->mCurrentMode)));
        WStr message(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new (8, false) WStr(Format<WStr, WStr>(message, cupName));
        mPopup.pOptionLabels[0] = new (8, false) WStr(LookupLoc(popupentry->mOptions[0]));
        mPopup.pOptionLabels[1] = new (8, false) WStr(LookupLoc(popupentry->mOptions[1]));
        mPopup.pOptionLabels[2] = NULL;
        mPopup.pOptionLabels[3] = NULL;
        mPopup.numOptions = 2;
        break;
    }

    case POPUP_END_CUP:
    {
        WStr cupName(LookupLoc(GetLOCModeName(GameInfoManager::s_pInstance->mCurrentMode)));
        WStr message(LookupLoc(popupentry->mMessage));
        mPopup.pMessage = new (8, false) WStr(Format<WStr, WStr>(message, cupName));
        mPopup.pOptionLabels[0] = new (8, false) WStr(LookupLoc(popupentry->mOptions[0]));
        mPopup.pOptionLabels[1] = new (8, false) WStr(LookupLoc(popupentry->mOptions[1]));
        mPopup.pOptionLabels[2] = NULL;
        mPopup.pOptionLabels[3] = NULL;
        mPopup.numOptions = 2;
        break;
    }

    case POPUP_NO_MEMCARD:
    case POPUP_NOT_ENOUGH_SPACE:
    case POPUP_NOT_ENOUGH_SPACE_CANMANAGE:
    case POPUP_ABOUTTOSAVE:
    case POPUP_MEMCARD_ASK_SAVE_NO_FILE:
    {
        WStr unformatted(LookupLoc(popupentry->mMessage));
        int blocks = SaveLoad::GetSaveBlockSize(0);
        NStr numBlocks(LexicalCast<NStr, int>(blocks));
        unsigned short param1[4];
        nlStrToWcs(numBlocks.c_str(), param1, 4);
        mPopup.pMessage = new (8, false) WStr(Format<WStr, unsigned short[4]>(unformatted, param1));

        mPopup.numOptions = 0;
        for (int i = 0; i < 4; i++)
        {
            if (popupentry->mOptions[i] != 0)
            {
                mPopup.pOptionLabels[i] = new (8, false) WStr(LookupLoc(popupentry->mOptions[i]));
                mPopup.numOptions++;
            }
            else
            {
                mPopup.pOptionLabels[i] = NULL;
            }
        }
        break;
    }

    case POPUP_REVERT_OPTION_CHANGES:
    {
        WStr unformattedmessage(LookupLoc(popupentry->mMessage));
        WStr optionname;
        eMenuState menustate = MENUSTATE_INVALID;
        OptionsScene* scene;

        if (GameSceneManager::s_pInstance != NULL)
        {
            scene = (OptionsScene*)GameSceneManager::s_pInstance->GetScene(SCENE_OPTIONS);
            menustate = scene->m_curMenuState;
        }
        else if (OverlayManager::s_pInstance->GetScene(IGSCENE_PAUSE_AUDIO) != NULL)
        {
            menustate = MS_AUDIO;
        }
        else if (OverlayManager::s_pInstance->GetScene(IGSCENE_PAUSE_VISUAL) != NULL)
        {
            menustate = MS_VISUAL;
        }

        switch (menustate)
        {
        case MS_AUDIO:
            optionname = LookupLoc(0x941C139C);
            break;
        case MS_VISUAL:
            optionname = LookupLoc(0xF21F2E3E);
            break;
        case MS_GAMEPLAY:
            optionname = LookupLoc(0x1E76A69A);
            break;
        case MS_CHEATS:
            optionname = LookupLoc(0xA1877C37);
            break;
        default:
            break;
        }

        mPopup.pMessage = new (8, false) WStr(Format<WStr, WStr>(unformattedmessage, optionname));
        if (menustate == MS_GAMEPLAY)
        {
            mUnknownAA5 = true;
        }
        mPopup.pOptionLabels[0] = new (8, false) WStr(LookupLoc(popupentry->mOptions[0]));
        mPopup.pOptionLabels[1] = new (8, false) WStr(LookupLoc(popupentry->mOptions[1]));
        mPopup.pOptionLabels[2] = NULL;
        mPopup.pOptionLabels[3] = NULL;
        mPopup.numOptions = 2;
        break;
    }

    default:
    {
        if (popupentry->mMessage != 0)
        {
            mPopup.pMessage = new (8, false) WStr(LookupLoc(popupentry->mMessage));
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
                mPopup.pOptionLabels[i] = new (8, false) WStr(LookupLoc(popupentry->mOptions[i]));
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

// /**
//  * Offset/Address/Size: 0x14 | 0x8009C410 | size: 0x94
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::~BasicString()
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
