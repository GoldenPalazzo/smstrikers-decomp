#include "Game/FE/feButtonComponent.h"
#include "NL/nlString.h"

#include "Game/FE/feAsyncImage.h"
#include "Game/FE/feFontResource.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feFinder.h"
#include "NL/glx/glxTexture.h"

/**
 * Offset/Address/Size: 0x0 | 0x8010DC04 | size: 0x3F8
 * TODO: 90.96% match - stack frame +0x10 (0x240 vs 0x230), fmuls operand order, instruction scheduling around FCS ctor args
 */
void ButtonComponent::CentreButtons()
{
    float totalLength;
    int i;
    PlatTexture* texture;
    float buttonWidth;
    feVector3 imagePosition;
    feVector3 textPosition;
    unsigned short buffer[128];
    feVector3 labelScale;
    feVector3 componentPosition;

    if (mAlreadyCentred || mButtonInstance == NULL)
    {
        return;
    }

    totalLength = 0.0f;

    for (i = 0; i < mNumButtons; i++)
    {
        unsigned short* pLastChar;
        unsigned short* pCurrentChar;
        bool firstChar;
        int renderedLength;

        texture = glx_GetTex(mButtonImages[i]->m_pTextureResource->m_glTextureHandle, true, true);
        buttonWidth = (float)texture->m_Width;

        imagePosition = mButtonImages[i]->GetAssetPosition();
        float halfWidth = buttonWidth * 0.5f;
        mButtonImages[i]->SetAssetPosition(totalLength + halfWidth, imagePosition.f.y, imagePosition.f.z);
        totalLength += buttonWidth;

        textPosition = mButtonLabels[i]->GetAssetPosition();
        mButtonLabels[i]->SetAssetPosition(totalLength, textPosition.f.y, textPosition.f.z);

        const nlFont* pFont = ((FEFontResource*)mButtonLabels[i]->m_component->pChildren)->m_font;

        nlStrNCpy<unsigned short>(buffer, mButtonLabels[i]->GetString(), 0x80);
        buffer[127] = 0;

        labelScale = mButtonLabels[i]->GetAssetScale();

        renderedLength = 0;
        pCurrentChar = 0;
        firstChar = true;
        FontCharString fcs(buffer, pFont, (unsigned short*)0);
        pLastChar = fcs.m_pString;

        while (*pLastChar != 0)
        {
            renderedLength += pFont->GetCharWidth(*pLastChar, firstChar ? 0 : *pCurrentChar);
            pCurrentChar = pLastChar;
            firstChar = false;
            pLastChar++;
        }

        if (fcs.m_InternalBuffer)
        {
            delete[] fcs.m_pString;
        }

        totalLength += (float)(int)(labelScale.f.x * (float)renderedLength);
        totalLength += 32.0f;
    }

    totalLength -= 32.0f;

    componentPosition = mButtonInstance->GetAssetPosition();
    mButtonInstance->SetAssetPosition(-(totalLength * 0.5f), componentPosition.f.y, componentPosition.f.z);
    mButtonInstance->m_bVisible = true;
    mAlreadyCentred = true;

    TLComponentInstance* pComponent = (TLComponentInstance*)FindComponent(mButtonInstance->GetActiveSlide(), "Component");
    if (pComponent != NULL)
    {
        TLImageInstance* pImage = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            pComponent->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("Group")),
            InlineHasher(nlStringLowerHash("darkblue_frame2")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        if (pImage != 0)
        {
            pImage->SetAssetScale(1024.0f, 1.0f, 1.0f);
        }

        pImage = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            pComponent->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("Group")),
            InlineHasher(nlStringLowerHash("darkblue_frame")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        if (pImage != 0)
        {
            pImage->SetAssetScale(1024.0f, 1.0f, 1.0f);
        }
    }
}

/**
 * Offset/Address/Size: 0x3F8 | 0x8010DFFC | size: 0x8D8
 */
void ButtonComponent::SetState(ButtonComponent::ButtonState buttonstate)
{
    if (mButtonInstance == NULL)
    {
        return;
    }

    mAlreadyCentred = false;

    InlineHasher zero0 = InlineHasher(0);
    InlineHasher zero1 = InlineHasher(0);
    InlineHasher zero2 = InlineHasher(0);
    InlineHasher zero3 = InlineHasher(0);
    InlineHasher zero4 = InlineHasher(0);

    switch (buttonstate)
    {
    case BS_A_AND_B:
        mButtonInstance->SetActiveSlide("a and b");
        mNumButtons = 2;
        mButtonImages[1] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("A_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[1] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("accept")), zero0, zero1, zero2, zero3, zero4);
        mButtonImages[0] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("B_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[0] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("back")), zero0, zero1, zero2, zero3, zero4);
        break;

    case BS_A_ONLY:
        mButtonInstance->SetActiveSlide("a");
        mNumButtons = 1;
        mButtonImages[0] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("A_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[0] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("accept")), zero0, zero1, zero2, zero3, zero4);
        break;

    case BS_B_ONLY:
        mButtonInstance->SetActiveSlide("b");
        mNumButtons = 1;
        mButtonImages[0] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("B_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[0] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("back")), zero0, zero1, zero2, zero3, zero4);
        break;

    case BS_A_AND_B_AND_Y:
        mButtonInstance->SetActiveSlide("a b y");
        mNumButtons = 3;
        mButtonImages[1] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("A_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[1] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("accept")), zero0, zero1, zero2, zero3, zero4);
        mButtonImages[0] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("B_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[0] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("back")), zero0, zero1, zero2, zero3, zero4);
        mButtonImages[2] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("y_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[2] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("OPTIONS")), zero0, zero1, zero2, zero3, zero4);
        break;

    case 4:
        mButtonInstance->SetActiveSlide("a and b and start");
        mNumButtons = 3;
        mButtonImages[0] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("start_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[0] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("start")), zero0, zero1, zero2, zero3, zero4);
        mButtonImages[1] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("B_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[1] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("back")), zero0, zero1, zero2, zero3, zero4);
        mButtonImages[2] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("A_button")), zero0, zero1, zero2, zero3, zero4);
        mButtonLabels[2] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(mButtonInstance->GetActiveSlide(), InlineHasher(nlStringLowerHash("accept")), zero0, zero1, zero2, zero3, zero4);
        break;

    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0xCD0 | 0x8010E8D4 | size: 0x3C
 */
ButtonComponent::~ButtonComponent()
{
}

/**
 * Offset/Address/Size: 0xD0C | 0x8010E910 | size: 0x14
 */
ButtonComponent::ButtonComponent()
{
    mButtonInstance = 0;
    mNumButtons = 0;
    mAlreadyCentred = false;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8010E924 | size: 0x1E4
//  */
// void FontCharString::FontCharString<unsigned short>(const unsigned short*, const nlFont*, unsigned short*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010EB08 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x8010EB40 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x8010EBC4 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x8010ED20 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x8010ED58 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x8010EDDC | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8010EF38 | size: 0x2C
 */
// REMOVE stub once real callers exist
void feButtonComponent_stub()
{
    nlStrLen<unsigned short>((const unsigned short*)0);
    FEFinder<TLTextInstance, 3>::Find<TLSlide>((TLSlide*)0, InlineHasher(0), InlineHasher(0), InlineHasher(0), InlineHasher(0), InlineHasher(0), InlineHasher(0));
}
