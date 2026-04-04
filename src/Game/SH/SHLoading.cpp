#include "Game/SH/SHLoading.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/AudioStream.h"
#include "Game/OverlayManager.h"
#include "Game/FE/feInput.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feMusic.h"
#include "Game/FE/fePresentation.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/GameInfo.h"
#include "Game/main.h"
#include "NL/nlBasicString.h"
#include "NL/nlBundleFile.h"
#include "NL/nlTask.h"
#include "NL/nlLocalization.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/gl/glState.h"
#include "NL/gl/glTexture.h"

extern nlLocalization* g_pLocalization;
extern unsigned char PAD_COLOURS[4][3];
extern unsigned long CONTROLLER_TEXT[4];
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

// /**
//  * Offset/Address/Size: 0xE08 | 0x800A9794 | size: 0x118
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[16]>(
//     const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short (&)[16])
// {
// }

// /**
//  * Offset/Address/Size: 0x118 | 0x800A8AA4 | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator% <const unsigned short*>(const unsigned short*
// const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A898C | size: 0x118
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[2]>(
//     const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short (&)[2])
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800A8830 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                      unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800A87AC | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                   unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800A8774 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                  InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800A8618 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800A8594 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800A855C | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800A8400 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800A837C | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A8344 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A8170 | size: 0x1D4
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::AppendInPlace(const unsigned short*)
// {
// }

/**
 * Offset/Address/Size: 0x19A4 | 0x800A8114 | size: 0x5C
 */
SuperLoadingScene::SuperLoadingScene()
    : BaseSceneHandler()
{
    mType = TT_INVALID;
    mElapsedTime = 0.0f;
    mAlreadySwappedTextures = false;
    mImageInstances[0][0] = NULL;
    mImageInstances[1][0] = NULL;
    mTextureHandles[0][0] = -1;
    mTextureHandles[1][0] = -1;
}

/**
 * Offset/Address/Size: 0x1948 | 0x800A80B8 | size: 0x5C
 */
SuperLoadingScene::~SuperLoadingScene()
{
}

/**
 * Offset/Address/Size: 0x1428 | 0x800A7B98 | size: 0x520
 * TODO: 47.58% match - r28/r29 register swap (team1 vs bundleFile),
 * InlineHasher argument copy pattern differs from target.
 * File uses -inline deferred. Compiler-internal allocation.
 */
void SuperLoadingScene::SceneCreated()
{
    FEPresentation* pres = m_pFEScene->m_pFEPackage->GetPresentation();
    if (mType == TT_IN)
    {
        pres->SetActiveSlide("appear");
    }
    else if (mType == TT_OUT)
    {
        pres->SetActiveSlide("disappear");
    }

    eTeamID team1 = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);
    eTeamID team0 = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);

    BundleFile* bundleFile = new (nlMalloc(sizeof(BundleFile), 0x20, true)) BundleFile();
    bundleFile->Open("art/fe/LoadingScreensUI.Res");

    {
        char filename[128] = { };
        BundleFileDirectoryEntry dirEntry;
        CaptainSidekickFilename::Build((CaptainSidekickFilename::Type)0, filename, 0x80, team0, 0);
        bundleFile->GetFileInfo(filename, &dirEntry, true);
        u8* fileData = (u8*)nlMalloc(dirEntry.m_length, 0x20, true);
        bundleFile->ReadFile(filename, fileData, dirEntry.m_length);
        u32 hash = nlStringHash(filename);
        glTextureAdd(hash, fileData, dirEntry.m_length);
        u32 texHandle = glGetTexture(filename);
        delete[] fileData;
        mTextureHandles[0][0] = texHandle;
    }
    {
        char filename[128] = { };
        BundleFileDirectoryEntry dirEntry;
        CaptainSidekickFilename::Build((CaptainSidekickFilename::Type)0, filename, 0x80, team1, 1);
        bundleFile->GetFileInfo(filename, &dirEntry, true);
        u8* fileData = (u8*)nlMalloc(dirEntry.m_length, 0x20, true);
        bundleFile->ReadFile(filename, fileData, dirEntry.m_length);
        u32 hash = nlStringHash(filename);
        glTextureAdd(hash, fileData, dirEntry.m_length);
        u32 texHandle = glGetTexture(filename);
        delete[] fileData;
        mTextureHandles[1][0] = texHandle;
    }

    bundleFile->Close();
    delete bundleFile;

    TLSlide* slide = pres->m_currentSlide;
    InlineHasher h1, h2, h3(0), h4(0), h5(0), h6(0);

    h2.m_Hash = nlStringLowerHash("leftimage");
    h1.m_Hash = nlStringLowerHash("Layer");
    mImageInstances[0][0] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(slide, h1, h2, h3, h4, h5, h6);

    h2.m_Hash = nlStringLowerHash("rightimage");
    h1.m_Hash = nlStringLowerHash("Layer");
    mImageInstances[1][0] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(slide, h1, h2, h3, h4, h5, h6);

    h2.m_Hash = nlStringLowerHash("stadiumname");
    h1.m_Hash = nlStringLowerHash("Layer");
    TLTextInstance* stadiumText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(slide, h1, h2, h3, h4, h5, h6);
    if (stadiumText != NULL)
    {
        stadiumText->m_LocStrId = GetStadiumStringID(nlSingleton<GameInfoManager>::s_pInstance->GetStadium());
        stadiumText->m_OverloadFlags |= 0x8;
    }

    DisplayCupInfo();

    h2.m_Hash = nlStringLowerHash("period");
    h1.m_Hash = nlStringLowerHash("Layer");
    TLComponentInstance* periodComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(slide, h1, h2, h3, h4, h5, h6);
    if (periodComp != NULL)
    {
        if (g_Language == 2)
            periodComp->m_bVisible = true;
        else
            periodComp->m_bVisible = false;
    }

    h2.m_Hash = nlStringLowerHash("playersleft");
    h1.m_Hash = nlStringLowerHash("Layer");
    TLTextInstance* playersLeft = FEFinder<TLTextInstance, 3>::Find<TLSlide>(slide, h1, h2, h3, h4, h5, h6);
    if (playersLeft != NULL)
    {
        BuildPlayerStrings(playersLeft, 0, false);
    }

    h2.m_Hash = nlStringLowerHash("playersright");
    h1.m_Hash = nlStringLowerHash("Layer");
    TLTextInstance* playersRight = FEFinder<TLTextInstance, 3>::Find<TLSlide>(slide, h1, h2, h3, h4, h5, h6);
    if (playersRight != NULL)
    {
        BuildPlayerStrings(playersRight, 1, false);
    }

    FEMusic::StopStream();
}

/**
 * Offset/Address/Size: 0x1324 | 0x800A7A94 | size: 0x104
 */
void SuperLoadingScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);

    if (!mAlreadySwappedTextures)
    {
        bool allReady;
        FETextureResource* texRes;
        TLImageInstance* img0;
        allReady = false;
        texRes = (img0 = mImageInstances[0][0])->m_pTextureResource;
        if (texRes->m_bValid)
        {
            if (mImageInstances[1][0]->m_pTextureResource->m_bValid)
            {
                allReady = true;
            }
        }
        if (allReady)
        {
            texRes = img0->m_pTextureResource;
            texRes->m_glTextureHandle = mTextureHandles[0][0];
            mImageInstances[1][0]->m_pTextureResource->m_glTextureHandle = mTextureHandles[1][0];
            mAlreadySwappedTextures = true;
        }
    }

    TLSlide* slide = m_pFEScene->m_pFEPackage->GetPresentation()->m_currentSlide;
    if (slide->m_time >= slide->m_start + slide->m_duration)
    {
        if (mType == TT_IN)
        {
            AudioLoader::StopStreaming();
            Audio::ConfigureStreamBuffers(2);
            AudioLoader::PlayLoadLoopMusic();
            nlTaskManager::SetNextState(2);
        }
        else if (mType == TT_OUT)
        {
            nlSingleton<OverlayManager>::s_pInstance->Pop();
        }
    }
}

/**
 * Offset/Address/Size: 0x3C4 | 0x800A6B34 | size: 0xF60
 */
void SuperLoadingScene::DisplayCupInfo()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800A6770 | size: 0x3C4
 * TODO: 90.49% match - register allocation mismatch (r28-r31 vs r25-r28 for
 * this/pTextInst/side/i), stack temp offset (sp+0x14 vs sp+0x10 for copy-ctor
 * temporaries). File uses -inline deferred. Compiler-internal allocation.
 */
void SuperLoadingScene::BuildPlayerStrings(TLTextInstance* pTextInst, int side, bool checkConnected)
{
    BasicString<unsigned short, Detail::TempStringAllocator> str;
    char narrowBuf[255] = { };
    unsigned short wideBuf[255] = { };

    for (int i = 0; i < 4; i++)
    {
        if (checkConnected)
        {
            if (!g_pFEInput->IsConnected((eFEINPUT_PAD)i))
                continue;
        }

        if (nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)i) != side)
            continue;

        nlSNPrintf(narrowBuf, 255, "{c:%02x%02x%02x}", PAD_COLOURS[i][0], PAD_COLOURS[i][1], PAD_COLOURS[i][2]);
        nlStrToWcs(narrowBuf, wideBuf, 255);
        str = str.AppendInPlace(wideBuf);

        unsigned long key = CONTROLLER_TEXT[i];
        nlLocalization* loc = g_pLocalization;
        const unsigned short* locText;

        if (loc->m_LookupTable == NULL)
            locText = LocalizationTableNotFound;
        else
        {
            nlLocalization::StringLookup* result = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, loc->m_pFile->StringCount);
            if (result != NULL)
                locText = loc->m_FirstString + result->StringOffset;
            else
                locText = MissingLocString;
        }

        str = str.AppendInPlace(locText);

        static const unsigned short sLineBreak[] = { (unsigned short)'\n', 0 };
        str = str.AppendInPlace(sLineBreak);
    }

    memcpy(side == 0 ? mPlayerStrings[0] : mPlayerStrings[1], str.c_str(), 255);
    pTextInst->SetString(side == 0 ? mPlayerStrings[0] : mPlayerStrings[1]);
}
