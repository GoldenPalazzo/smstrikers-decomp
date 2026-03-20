#include "Game/SH/SHProgressiveScan.h"

// /**
//  * Offset/Address/Size: 0x3DC | 0x80111B3C | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x358 | 0x80111AB8 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x80111A34 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                              unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x29C | 0x801119FC | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                             InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x801118A0 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                      unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x8011181C | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                   unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x80111798 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80111760 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                         InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x1148 | 0x80111618 | size: 0x148
 */
ProgressiveScanScene::ProgressiveScanScene(bool)
{
}

/**
 * Offset/Address/Size: 0x107C | 0x8011154C | size: 0xCC
 */
ProgressiveScanScene::~ProgressiveScanScene()
{
    delete mUseProgressiveImage[0];
    delete mUseProgressiveImage[1];
    delete mConfirmationImage;
}

struct InlineHasher
{
    unsigned long m_Hash;

    InlineHasher()
    {
    }

    InlineHasher(unsigned long h)
        : m_Hash(h)
    {
    }
};

template <typename T, int N>
class FEFinder
{
public:
    template <typename U>
    static T* Find(U* slide, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6);
};

extern unsigned long nlStringLowerHash(const char*);
extern int nlSNPrintf(char*, unsigned long, const char*, ...);
extern int g_Language;

/**
 * Offset/Address/Size: 0xD08 | 0x801111D8 | size: 0x374
 * TODO: 89.03% match - all instructions correct, stack frame 0x20 too large
 * due to MWCC not reusing InlineHasher parameter copy slots across Find calls.
 */
void ProgressiveScanScene::SceneCreated()
{
    FEPresentation* presentation;

    presentation = m_pFEPresentation;

    mSelectorComponent = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mSelectorComponent->m_bVisible = false;

    if (mUseProgressiveMode)
    {
        mSelectorComponent->SetActiveSlide("Slide1");
    }
    else
    {
        mSelectorComponent->SetActiveSlide("Slide2");
    }

    mSelectorComponent->Update(0.0f);

    TLImageInstance* img = FEFinder<TLImageInstance, 2>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ProgressiveScan_deu")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    mUseProgressiveImage[0]->mImageInstance = img;
    img->m_bVisible = false;

    mUseProgressiveImage[1]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide3")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ProgressiveScan_deu")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mConfirmationImage->mImageInstance = FEFinder<TLImageInstance, 2>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide2")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ProgressiveScan_deu")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    char texturePath[0x40] = "ProgressiveScan_deu";
    const char* language;

    switch (g_Language)
    {
    case 0:
        language = "eng";
        break;
    case 1:
        language = "fre";
        break;
    case 2:
        language = "deu";
        break;
    case 3:
        language = "spa";
        break;
    case 4:
        language = "ita";
        break;
    case 5:
        language = "jpn";
        break;
    case 6:
        language = "uke";
        break;
    default:
        language = "eng";
        break;
    }

    nlSNPrintf(texturePath, sizeof(texturePath), "ProgressiveScan_%s", language);

    mUseProgressiveImage[0]->QueueLoad(texturePath, true);
    mUseProgressiveImage[1]->QueueLoad(texturePath, true);
}

/**
 * Offset/Address/Size: 0x940 | 0x80110E10 | size: 0x3C8
 */
void ProgressiveScanScene::Update(float)
{
}

/**
 * Offset/Address/Size: 0x0 | 0x801104D0 | size: 0x940
 */
void ProgressiveScanScene::SwitchMessageImage()
{
}
