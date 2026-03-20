#include "Game/SH/SHCrossFader.h"

#include "NL/nlConfig.h"
#include "NL/nlColour.h"
#include "NL/nlFileGC.h"
#include "NL/nlMemory.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/GameSceneManager.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/fePresentation.h"
#include "Game/FE/feResourceManager.h"
#include "Game/FE/feSceneManager.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x800BCB38 | size: 0x20
//  */
// void FEResourceManager::Run(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x800BC9DC | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800BC958 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800BC8D4 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800BC89C | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x93C | 0x800BC840 | size: 0x5C
 */
CrossFaderScene::CrossFaderScene()
{
    mNumImages = 0;
    mCurrentImage = 0;
    mImageInstances = NULL;
    mTimer = 0.0f;
    mAlpha = 0.0f;
    mFadeState = FS_INVALID;
    mFadeToBlackTimer = 0.0f;
}

/**
 * Offset/Address/Size: 0x8B8 | 0x800BC7BC | size: 0x84
 */
CrossFaderScene::~CrossFaderScene()
{
    if (mImageInstances != NULL)
    {
        delete[] mImageInstances;
        mImageInstances = NULL;
    }
}

/**
 * Offset/Address/Size: 0x510 | 0x800BC414 | size: 0x3A8
 */
void CrossFaderScene::SceneCreated()
{
    typedef TLImageInstance* (*FindImageByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    FEPresentation* pPresentation;
    bool doCrossFade = true;
    if (Config::Global().Exists("CrossFade"))
    {
        doCrossFade = GetConfigBool(Config::Global(), "CrossFade", false);
    }

    if (!doCrossFade)
    {
        AudioLoader::LoadFEAudioData(true);
        while (!nlSingleton<FESceneManager>::s_pInstance->AreAllScenesValid())
        {
            nlServiceFileSystem();
            nlSingleton<FESceneManager>::s_pInstance->Update(0.0f);
            nlSingleton<FEResourceManager>::s_pInstance->Run(0.0f);
        }
        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_INTRO_MOVIE, SCREEN_NOTHING, false);
    }
    else
    {
        char buf[64];
        nlColour colour;
        volatile unsigned long hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;
        volatile unsigned long s8, s7, s6, s5, s4, s3, s2, s1, s0;
        volatile unsigned long t8, t7, t6, t5, t4, t3, t2, t1, t0;

        pPresentation = m_pFEScene->m_pFEPackage->GetPresentation();
        mNumImages = 0;

        do
        {
            nlSNPrintf(buf, 64, "CrossFadeImage%d", mNumImages);

            h0 = 0;
            h1 = 0;
            h2 = 0;
            h3 = 0;
            h4 = 0;
            h5 = 0;

            unsigned long hash = nlStringLowerHash(buf);
            h6 = hash;
            h7 = hash;
            hash = nlStringLowerHash("Layer1");
            h8 = hash;
            h9 = hash;
            hash = nlStringLowerHash("Slide1");
            hA = hash;
            hB = hash;

            union
            {
                FindImageByValue byValue;
                FindImageByRef byRef;
            } f1;
            f1.byValue = FEFinder<TLImageInstance, 2>::Find<FEPresentation>;
            TLImageInstance* found = f1.byRef(
                pPresentation,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

            if (found == NULL)
            {
                break;
            }
            mNumImages++;
        } while (true);

        mImageInstances = (TLImageInstance**)nlMalloc(mNumImages * 4, 8, false);

        for (int i = 0; i < mNumImages; i++)
        {
            nlSNPrintf(buf, 64, "CrossFadeImage%d", i);

            s0 = 0;
            h1 = 0;
            s1 = 0;
            h3 = 0;
            s2 = 0;
            h5 = 0;

            unsigned long hash = nlStringLowerHash(buf);
            s3 = hash;
            s4 = hash;
            hash = nlStringLowerHash("Layer1");
            s5 = hash;
            s6 = hash;
            hash = nlStringLowerHash("Slide1");
            s7 = hash;
            s8 = hash;

            union
            {
                FindImageByValue byValue;
                FindImageByRef byRef;
            } f2;
            f2.byValue = FEFinder<TLImageInstance, 2>::Find<FEPresentation>;
            mImageInstances[i] = f2.byRef(
                pPresentation,
                (InlineHasher&)s8,
                (InlineHasher&)s6,
                (InlineHasher&)s4,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

            if (i == 0)
            {
                colour.c[0] = 0;
                colour.c[1] = 0;
                colour.c[2] = 0;
                colour.c[3] = 0xFF;
            }
            else
            {
                colour.c[0] = 0xFF;
                colour.c[1] = 0xFF;
                colour.c[2] = 0xFF;
                colour.c[3] = 0;
            }
            mImageInstances[i]->SetAssetColour(colour);
        }

        t0 = 0;
        h1 = 0;
        t1 = 0;
        h3 = 0;
        t2 = 0;
        h5 = 0;

        {
            unsigned long hash = nlStringLowerHash("CrossFadeMain");
            t3 = hash;
            t4 = hash;
            hash = nlStringLowerHash("Layer1");
            t5 = hash;
            t6 = hash;
            hash = nlStringLowerHash("Slide1");
            t7 = hash;
            t8 = hash;
        }

        union
        {
            FindImageByValue byValue;
            FindImageByRef byRef;
        } f3;
        f3.byValue = FEFinder<TLImageInstance, 2>::Find<FEPresentation>;
        mCurrentImageInstance = f3.byRef(
            pPresentation,
            (InlineHasher&)t8,
            (InlineHasher&)t6,
            (InlineHasher&)t4,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        mFadeState = FS_FADE_IN_INIT;
        mCurrentImage = 0;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800BBF04 | size: 0x510
 */
void CrossFaderScene::Update(float)
{
}
