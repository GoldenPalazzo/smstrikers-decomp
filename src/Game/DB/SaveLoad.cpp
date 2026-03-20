#include "Game/DB/SaveLoad.h"
#include "Game/GameInfo.h"
#include "Game/Sys/gcmemcard.h"
#include "Game/Sys/debug.h"
#include "Game/FE/feHelpFuncs.h"
#include "NL/nlFileGC.h"
#include "NL/nlMain.h"
#include "Game/ResetTask.h"
#include <string.h>

extern void nlFree(void*);

IconDataCache gIconDataCache;

bool InOperation = false;

LoadCallbacks LoadSystem;
SaveCallbacks SaveSystem;
DeleteCallbacks DeleteSystem;
FormatCallbacks FormatSystem;

static MemCard* s_MemCardStorage[2] = { nullptr, nullptr };
void (*g_Callback)(long);
FileExistsCallbacks FileExistsSystem;
MemoryCardIDCallbacks MemoryCardIDSystem;
MemCard** g_MemCards = s_MemCardStorage;
struct MemCardIDInfo
{
    s64 serialID;
    // u32 cardID;
};
MemCardIDInfo mLastKnownMemCardID;
static const char* MarioSoccerFileName;
static s64 mRequiredMemoryCardID;
static unsigned long gIconCRC;

// /**
//  * Offset/Address/Size: 0x0 | 0x8018D40C | size: 0xA4
//  */
// void 0x8028D340..0x8028D344 | size: 0x4
// {
// }

// /**
//  * Offset/Address/Size: 0x104 | 0x8018D3D8 | size: 0x34
//  */
// void MemCardFunctor::MCMemberFunctor<LoadCallbacks>::Call(unsigned long, long)
// {
// }

// /**
//  * Offset/Address/Size: 0xD0 | 0x8018D3A4 | size: 0x34
//  */
// void MemCardFunctor::MCMemberFunctor<SaveCallbacks>::Call(unsigned long, long)
// {
// }

// /**
//  * Offset/Address/Size: 0x9C | 0x8018D370 | size: 0x34
//  */
// void MemCardFunctor::MCMemberFunctor<DeleteCallbacks>::Call(unsigned long, long)
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x8018D33C | size: 0x34
//  */
// void MemCardFunctor::MCMemberFunctor<FormatCallbacks>::Call(unsigned long, long)
// {
// }

// /**
//  * Offset/Address/Size: 0x34 | 0x8018D308 | size: 0x34
//  */
// void MemCardFunctor::MCMemberFunctor<FileExistsCallbacks>::Call(unsigned long, long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8018D2D4 | size: 0x34
//  */
// void MemCardFunctor::MCMemberFunctor<MemoryCardIDCallbacks>::Call(unsigned long, long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38E4 | 0x8018D240 | size: 0x94
//  */
// IconDataCache::~IconDataCache()
// {
// }

/**
 * Offset/Address/Size: 0x38DC | 0x8018D238 | size: 0x8
 */
bool SaveLoad::CardBusy()
{
    return InOperation;
}

/**
 * Offset/Address/Size: 0x3720 | 0x8018D07C | size: 0x1BC
 * TODO: 99.3% match - header-size regalloc still differs (iconFmt/iconCount/bannerFmt map to r0/r11/r10 instead of
 * r4/r0/r11, and accumulation keeps r6 live instead of target r0/r3 add sequence). Literal symbol IDs for "@2009"
 * and "@2010" also remain different in scratch context.
 */
void LoadMemoryCardIconData()
{
    nlFile* pFile1;
    unsigned int iconSize;
    unsigned int bannerSize;

    memset(&gIconDataCache, 0, sizeof(MemCard::ICON_CONFIG));

    gIconDataCache.mIconConfig.IconCount = 1;
    gIconDataCache.mIconConfig.IconFormat = 2;
    gIconDataCache.mIconConfig.IconSpeeds[0] = 3;
    gIconDataCache.mIconConfig.BannerFormat = 2;

    gIconDataCache.mIconConfig.GetValidDataInfo(gIconDataCache.mIconDataInfo);

    u8 bannerFmt = gIconDataCache.mIconConfig.BannerFormat;
    u8 iconCount = gIconDataCache.mIconConfig.IconCount;
    s8 iconFmt = gIconDataCache.mIconConfig.IconFormat;

    u32 headerSize = 0;
    headerSize += ((bannerFmt == 1) ? 0x200 : 0);
    headerSize += bannerFmt * 0xC00;
    headerSize += ((iconFmt == 1) ? 0x200 : 0);
    headerSize += iconCount * (iconFmt << 10);
    headerSize += 0x40;
    gIconDataCache.mIconConfig.HeaderSize = headerSize;

    gIconDataCache.mIconHdrBuffer = nlMalloc(headerSize, 0x20, false);
    gIconDataCache.mIconDataInfo.pHeaderData = (unsigned char*)gIconDataCache.mIconHdrBuffer;
    void* pHdrBuf = gIconDataCache.mIconDataInfo.pHeaderData;

    nlStrNCpy((char*)pHdrBuf, GetMemCardTitle(), 0x20);
    char* pDescDst = (char*)gIconDataCache.mIconDataInfo.pHeaderData + 0x20;
    nlStrNCpy(pDescDst, GetMemCardDescription(), 0x20);

    pFile1 = nlOpen("@2009\0\0\0\0");
    nlFileSize(pFile1, &bannerSize);
    gIconDataCache.mBannerBuffer = nlMalloc(bannerSize, 0x20, true);
    nlRead(pFile1, gIconDataCache.mBannerBuffer, bannerSize);
    nlClose(pFile1);

    nlFile* pFile2 = nlOpen("@2010\0\0\0\0");
    nlFileSize(pFile2, &iconSize);
    gIconDataCache.mIconBuffer = nlMalloc(iconSize, 0x20, true);
    nlRead(pFile2, gIconDataCache.mIconBuffer, iconSize);
    nlClose(pFile2);
}

/**
 * Offset/Address/Size: 0x355C | 0x8018CEB8 | size: 0x1C4
 * TODO: 97.83% match - li r6,0x200 scheduling and header-size calc
 * register allocation differ under -inline deferred context
 */
unsigned long LoadCallbacks::LoadIconDataDoneCB(unsigned long Slot, long Result, void* pUserData)
{
    void* pReadBuf = m_pReadBuffer;
    MemCard::MC_FILE* pFile = (MemCard::MC_FILE*)pUserData;
    int iconFmt = pFile->IconCfg.IconFormat;
    int iconCount = pFile->IconCfg.IconCount;
    int bannerFmt = pFile->IconCfg.BannerFormat;

    u32 headerSize = 0;
    headerSize += ((bannerFmt == 1) ? 0x200 : 0);
    headerSize += bannerFmt * 0xC00;
    headerSize += ((iconFmt == 1) ? 0x200 : 0);
    headerSize += iconCount * (iconFmt << 10);
    headerSize += 0x40;
    pFile->IconCfg.HeaderSize = headerSize;

    u32 crc = nlChecksum32(m_pIconReadBuffer, headerSize);
    m_IconLoadedCRC = crc;
    m_MustFreeBuffers = true;

    if (((unsigned long*)pReadBuf)[2] != m_IconLoadedCRC)
    {
        if (pFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(pFile);
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(-1000);
        m_MustFreeBuffers = true;
        return -1;
    }

    gIconCRC = crc;
    if (m_TestGameID)
    {
        *(u8*)&m_GameIDTestResult = nlSingleton<GameInfoManager>::s_pInstance->CheckSaveIDChanged((void*)((u8*)m_pReadBuffer + 12));
    }
    else
    {
        nlSingleton<GameInfoManager>::s_pInstance->SetMemoryCardData((void*)((u8*)m_pReadBuffer + 12));
    }

    mRequiredMemoryCardID = g_MemCards[Slot]->GetSerialID();
    g_MemCards[Slot]->CloseFile(pFile);

    MemCard* card = g_MemCards[Slot];
    card->m_State = IS_IDLE;
    card->m_CardState = CS_IDLE;
    CARDUnmount(card->m_Slot);
    InOperation = false;
    g_Callback(Result);
    m_MustFreeBuffers = true;
    return (-Result | Result) >> 31;
}

/**
 * Offset/Address/Size: 0x3254 | 0x8018CBB0 | size: 0x308
 * TODO: 85.82% match - prologue memset arg scheduling diffs + rlwinm
 * optimization combining srwi+clrlwi. All -inline deferred issues.
 */
unsigned long LoadCallbacks::ReadDoneCB(unsigned long Slot, long Result, void* pUserData)
{
    MemCardFunctor functor;
    typedef unsigned long (LoadCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    MemCard::ICON_CONFIG localCfg;
    MemCard::MC_FILE* pFile = (MemCard::MC_FILE*)pUserData;

    localCfg.BannerFormat = 0;
    localCfg.IconCount = 0;
    localCfg.IconFormat = 0;
    localCfg.IconAnimType = 0;
    memset(localCfg.IconSpeeds, 0, 8);
    memset(&localCfg, 0, sizeof(MemCard::ICON_CONFIG));

    localCfg.IconCount = 1;
    localCfg.IconFormat = 2;
    localCfg.IconSpeeds[0] = 3;
    localCfg.BannerFormat = 2;

    u8 configValid;
    if (memcmp(&localCfg, &pFile->IconCfg, 1) != 0)
    {
        configValid = 0;
    }
    else
    {
        void* pBuf = m_pReadBuffer;
        configValid = (*(u32*)pBuf == (u32)nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize());
    }

    if (!configValid)
    {
        if (pFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(pFile);
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(-1000);
        m_MustFreeBuffers = true;
        return -1;
    }

    void* pBuf = m_pReadBuffer;
    u32 dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    u32 crc = nlChecksum32((void*)((u8*)m_pReadBuffer + 12), dataSize);
    if (((u32*)pBuf)[1] != crc)
    {
        if (pFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(pFile);
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(-1000);
        m_MustFreeBuffers = true;
        return -1;
    }

    m_pLoadFile = pFile;

    cb = &LoadCallbacks::LoadIconDataDoneCB;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<LoadCallbacks>(this, cb, pFile);

    MemCard::MC_FILE* pLoadFile = m_pLoadFile;
    MemCard** memCards = g_MemCards;
    u8 bannerFmt = pLoadFile->IconCfg.BannerFormat;
    s8 iconFmt = pLoadFile->IconCfg.IconFormat;
    u8 iconCount = pLoadFile->IconCfg.IconCount;

    u32 totalHeader = 0;
    totalHeader += ((bannerFmt == 1) ? 0x200 : 0);
    totalHeader += bannerFmt * 0xC00;
    totalHeader += ((iconFmt == 1) ? 0x200 : 0);
    totalHeader += iconCount * (iconFmt << 10);

    u32 headerSize = totalHeader + 0x40;
    pLoadFile->IconCfg.HeaderSize = headerSize;

    u32 alignedSize = (headerSize + 0x1FF) & ~0x1FF;

    long result = memCards[Slot]->InternalReadFile(pLoadFile, m_pIconReadBuffer, alignedSize, 0, functor);

    if (result != 0)
    {
        if (pFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(pFile);
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        m_MustFreeBuffers = true;
        return -1;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x2DA8 | 0x8018C704 | size: 0x4AC
 * TODO: 86.19% match - remaining diffs from -inline deferred vs -inline auto:
 * r31 caching Slot*4, nor/srawi/and constant folding in icon config,
 * extra cmpwi+ble while loop guards, cascading register allocation.
 */
unsigned long SaveCallbacks::FileWriteCB(unsigned long Slot, long Result, void* pUserData)
{
    if (Result != 0)
    {
        long errorCode = Result;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;

        if (errorCode == -4)
        {
            MemCard* card2 = g_MemCards[Slot];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks = 0;
            int origSize = (dataSize += 12);
            dataSize = (u32)(dataSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            MemCard::ICON_CONFIG IconCfg;
            IconCfg.BannerFormat = 0;
            IconCfg.IconCount = 0;
            IconCfg.IconFormat = 0;
            IconCfg.IconAnimType = 0;
            memset(IconCfg.IconSpeeds, 0, 8);
            memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
            IconCfg.IconCount = 1;
            IconCfg.IconFormat = 2;
            IconCfg.IconSpeeds[0] = 3;
            IconCfg.BannerFormat = 2;
            int iconFormat = IconCfg.IconFormat;
            int iconCount = IconCfg.IconCount;
            int iconSize = (iconFormat << 10) * iconCount;
            int temp = ~(iconCount | -1);
            int bannerClut = (temp >> 31) & 0x200;
            int bannerSize = iconFormat * 0xC00;
            int iconClut = (temp >> 31) & 0x200;
            int total = bannerClut + bannerSize + iconSize + iconClut;
            origSize = (int)(IconCfg.HeaderSize = total + 0x40);
            dataSize = (u32)(origSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[Slot];
            u8 hasSpace;
            if (alignedSize > mc->m_CardInfo.FreeBytes)
                hasSpace = 0;
            else if (mc->m_CardInfo.FreeFiles < 1)
                hasSpace = 0;
            else
                hasSpace = 1;
            if (!hasSpace)
                errorCode = -9;
        }

        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }

    // Result == 0: check serial ID
    if (mRequiredMemoryCardID != 0)
    {
        s64 serialID = g_MemCards[Slot]->GetSerialID();
        if (mRequiredMemoryCardID != serialID)
        {
            long errorCode = -1001;
            if (m_pSaveFile != NULL)
            {
                g_MemCards[Slot]->CloseFile(m_pSaveFile);
                m_pSaveFile = NULL;
            }
            MemCard* card = g_MemCards[Slot];
            card->m_State = IS_IDLE;
            card->m_CardState = CS_IDLE;
            CARDUnmount(card->m_Slot);
            InOperation = false;

            if (errorCode == -4)
            {
                MemCard* card2 = g_MemCards[Slot];
                long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
                int numBlocks = 0;
                int origSize = (dataSize += 12);
                dataSize = (u32)(dataSize + 0x1FFF) >> 13;
                if (origSize > 0)
                {
                    while (dataSize > 0)
                    {
                        numBlocks++;
                        dataSize--;
                    }
                }
                MemCard::ICON_CONFIG IconCfg;
                IconCfg.BannerFormat = 0;
                IconCfg.IconCount = 0;
                IconCfg.IconFormat = 0;
                IconCfg.IconAnimType = 0;
                memset(IconCfg.IconSpeeds, 0, 8);
                memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
                IconCfg.IconCount = 1;
                IconCfg.IconFormat = 2;
                IconCfg.IconSpeeds[0] = 3;
                IconCfg.BannerFormat = 2;
                int iconFormat = IconCfg.IconFormat;
                int iconCount = IconCfg.IconCount;
                int iconSize = (iconFormat << 10) * iconCount;
                int temp = ~(iconCount | -1);
                int bannerClut = (temp >> 31) & 0x200;
                int bannerSize = iconFormat * 0xC00;
                int iconClut = (temp >> 31) & 0x200;
                int total = bannerClut + bannerSize + iconSize + iconClut;
                origSize = (int)(IconCfg.HeaderSize = total + 0x40);
                dataSize = (u32)(origSize + 0x1FFF) >> 13;
                if (origSize > 0)
                {
                    while (dataSize > 0)
                    {
                        numBlocks++;
                        dataSize--;
                    }
                }
                unsigned long sectorSize = card2->m_CardInfo.SectorSize;
                unsigned long bytestosave = numBlocks * sectorSize;
                unsigned long alignedSize = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave);
                MemCard* mc = g_MemCards[Slot];
                u8 hasSpace;
                if (alignedSize > mc->m_CardInfo.FreeBytes)
                    hasSpace = 0;
                else if (mc->m_CardInfo.FreeFiles < 1)
                    hasSpace = 0;
                else
                    hasSpace = 1;
                if (!hasSpace)
                    errorCode = -9;
            }

            m_MustFreeMemory = true;
            g_Callback(errorCode);
            ResetTask::s_resetPaused = false;
            return -1;
        }
    }
    else
    {
        mRequiredMemoryCardID = g_MemCards[Slot]->GetSerialID();
    }

    // Success path
    g_MemCards[Slot]->CloseFile(m_pSaveFile);
    MemCard* card = g_MemCards[Slot];
    card->m_State = IS_IDLE;
    card->m_CardState = CS_IDLE;
    CARDUnmount(card->m_Slot);
    InOperation = false;
    m_MustFreeMemory = true;
    g_Callback(Result);
    ResetTask::s_resetPaused = false;
    return 0;
}

/**
 * Offset/Address/Size: 0x2708 | 0x8018C064 | size: 0x6A0
 * TODO: 71.37% match - stack/regalloc differences and ICON_CONFIG constant
 * folding remain in the duplicated -4 free-space remap paths.
 *
 * TODO: 82.15% match - remaining diffs from -inline deferred vs -inline auto:
 * 1. ICON_CONFIG constant folding: compiler folds ~(iconCount|-1)=0, eliminating nor/srawi/and 0x200 (10+ insns)
 * 2. Extra cmpwi+ble loop guards from (u32) cast creating temp register, breaking addic. CR chain
 * 3. CSE of Slot*4 into callee-saved r30 vs target computing fresh per-block
 * All inherent -inline deferred optimization differences, unfixable with -inline auto.
 */
long SaveCallbacks::DoSave(unsigned long Slot)
{
    extern unsigned int nlRandom(unsigned int, unsigned int*);
    extern unsigned int nlDefaultSeed;

    typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    MemCardFunctor functor;

    MemCard::ICON_CONFIG localCfg;
    localCfg.BannerFormat = 0;
    localCfg.IconCount = 0;
    localCfg.IconFormat = 0;
    localCfg.IconAnimType = 0;
    memset(localCfg.IconSpeeds, 0, 8);
    memset(&localCfg, 0, sizeof(MemCard::ICON_CONFIG));

    localCfg.IconCount = 1;
    localCfg.IconFormat = 2;
    localCfg.IconSpeeds[0] = 3;
    localCfg.BannerFormat = 2;
    u8 configValid = (memcmp(&localCfg, &m_pSaveFile->IconCfg, 1) == 0);
    if (!configValid)
    {
        long errorCode = -1000;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card2 = g_MemCards[Slot];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks = 0;
            int origSize = (dataSize += 12);
            dataSize = (u32)(dataSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            MemCard::ICON_CONFIG IconCfg;
            IconCfg.BannerFormat = 0;
            IconCfg.IconCount = 0;
            IconCfg.IconFormat = 0;
            IconCfg.IconAnimType = 0;
            memset(IconCfg.IconSpeeds, 0, 8);
            memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
            IconCfg.IconCount = 1;
            IconCfg.IconFormat = 2;
            IconCfg.IconSpeeds[0] = 3;
            IconCfg.BannerFormat = 2;
            int iconFormat = IconCfg.IconFormat;
            int iconCount = IconCfg.IconCount;
            int iconSize = (iconFormat << 10) * iconCount;
            int temp = ~(iconCount | -1);
            int bannerClut = (temp >> 31) & 0x200;
            int bannerSize = iconFormat * 0xC00;
            int iconClut = (temp >> 31) & 0x200;
            int total = bannerClut + bannerSize + iconSize + iconClut;
            origSize = (int)(IconCfg.HeaderSize = total + 0x40);
            dataSize = (u32)(origSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[Slot];
            u8 hasSpace;
            if (alignedSize > mc->m_CardInfo.FreeBytes)
                hasSpace = 0;
            else if (mc->m_CardInfo.FreeFiles < 1)
                hasSpace = 0;
            else
                hasSpace = 1;
            if (hasSpace == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }

    if (gIconCRC == 0)
    {
        MemCard::ICON_DATA_INFO localDataInfo1;
        m_pSaveFile->IconCfg.GetValidDataInfo(localDataInfo1);
        IconDataCache* cache = &gIconDataCache;
        void* pHdrBuf = cache->mIconHdrBuffer;
        void* bannerBuf = cache->mBannerBuffer;
        u32 bannerOfs = localDataInfo1.BannerOffset;
        void* destBanner = (u8*)pHdrBuf + bannerOfs;
        u8 bannerFmt = m_pSaveFile->IconCfg.BannerFormat;
        u32 tableOfs = *(u32*)((u8*)bannerBuf + 8);
        u32 entryVal = *(u32*)((u8*)bannerBuf + tableOfs);
        u32 dataOfs = *(u32*)((u8*)bannerBuf + entryVal + 8);
        void* srcBanner = (u8*)bannerBuf + dataOfs;
        int bfm1 = bannerFmt - 1;
        int bfm2 = 1 - bannerFmt;
        int bannerMask = ~(bfm1 | bfm2);
        int bannerClutV = 0x200 & (bannerMask >> 31);
        int banDataV = bannerFmt * 0xC00;
        u32 bannerCopySize = bannerClutV + banDataV;
        memcpy(destBanner, srcBanner, bannerCopySize);
        MemCard::ICON_DATA_INFO localDataInfo2;
        m_pSaveFile->IconCfg.GetValidDataInfo(localDataInfo2);
        void* pHdrBuf2 = cache->mIconHdrBuffer;
        void* iconBuf = cache->mIconBuffer;
        u32 iconOfs = localDataInfo2.IconOffset[0];
        void* destIcon = (u8*)pHdrBuf2 + iconOfs;
        s8 iconFmtS = m_pSaveFile->IconCfg.IconFormat;
        u32 itableOfs = *(u32*)((u8*)iconBuf + 8);
        u32 ientryVal = *(u32*)((u8*)iconBuf + itableOfs);
        u32 idataOfs = *(u32*)((u8*)iconBuf + ientryVal + 8);
        void* srcIcon = (u8*)iconBuf + idataOfs;
        u32 iconCopySize = iconFmtS << 10;
        memcpy(destIcon, srcIcon, iconCopySize);
        MemCard::MC_FILE* pFile = m_pSaveFile;
        u8 bannerFmt2 = pFile->IconCfg.BannerFormat;
        s8 iconFmt2 = pFile->IconCfg.IconFormat;
        u8 iconCount2 = pFile->IconCfg.IconCount;
        int bf1 = bannerFmt2 - 1;
        int bf2 = 1 - bannerFmt2;
        int banClutMask = ~(bf1 | bf2);
        int banClutH = 0x200 & (banClutMask >> 31);
        int banDatH = bannerFmt2 * 0xC00;
        int if1 = iconFmt2 - 1;
        int if2 = 1 - iconFmt2;
        int icnClutMask = ~(if1 | if2);
        int icnClutH = 0x200 & (icnClutMask >> 31);
        u32 icnPixels = iconCount2 * (iconFmt2 << 10);
        int headerTotal = banClutH + banDatH + icnPixels + icnClutH;
        u32 headerSize = headerTotal + 0x40;
        pFile->IconCfg.HeaderSize = headerSize;
        u32 crc = nlChecksum32(pHdrBuf2, headerSize);
        m_IconCRC = crc;
        gIconCRC = m_IconCRC;
    }

    struct MCFILE_HEADER
    {
        unsigned long Size;
        unsigned long CRC;
        unsigned long IconCRC;
    };

    unsigned long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize() + 12;
    m_pSaveGameBuffer = nlMalloc(dataSize, 0x20, true);
    GameInfoManager* pGIM = nlSingleton<GameInfoManager>::s_pInstance;
    pGIM->mUserInfo.mSaveID = nlRandom(-1, &nlDefaultSeed);
    nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardData((void*)((u8*)m_pSaveGameBuffer + 12));
    unsigned long* header = (unsigned long*)m_pSaveGameBuffer;
    header[0] = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    header[1] = nlChecksum32((void*)((u8*)m_pSaveGameBuffer + 12), nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize());
    header[2] = gIconCRC;
    cb = &SaveCallbacks::FileWriteCB;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb, m_pSaveGameBuffer);
    long result = g_MemCards[Slot]->InternalWriteFile(m_pSaveFile, m_pSaveGameBuffer, dataSize, m_pSaveFile->TotalHeaderSize, functor, true);
    if (result != 0)
    {
        long errorCode = result;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card2 = g_MemCards[Slot];
            long ds2 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks2 = 0;
            int origSize2 = (ds2 += 12);
            ds2 = (u32)(ds2 + 0x1FFF) >> 13;
            if (origSize2 > 0)
            {
                while (ds2 > 0)
                {
                    numBlocks2++;
                    ds2--;
                }
            }
            MemCard::ICON_CONFIG IconCfg2;
            IconCfg2.BannerFormat = 0;
            IconCfg2.IconCount = 0;
            IconCfg2.IconFormat = 0;
            IconCfg2.IconAnimType = 0;
            memset(IconCfg2.IconSpeeds, 0, 8);
            memset(&IconCfg2, 0, sizeof(MemCard::ICON_CONFIG));
            IconCfg2.IconCount = 1;
            IconCfg2.IconFormat = 2;
            IconCfg2.IconSpeeds[0] = 3;
            IconCfg2.BannerFormat = 2;
            int iconFormat2 = IconCfg2.IconFormat;
            int iconCount3 = IconCfg2.IconCount;
            int iconSize2 = (iconFormat2 << 10) * iconCount3;
            int temp2 = ~(iconCount3 | -1);
            int bannerClut2 = (temp2 >> 31) & 0x200;
            int bannerSize2 = iconFormat2 * 0xC00;
            int iconClut2 = (temp2 >> 31) & 0x200;
            int total2 = bannerClut2 + bannerSize2 + iconSize2 + iconClut2;
            origSize2 = (int)(IconCfg2.HeaderSize = total2 + 0x40);
            ds2 = (u32)(origSize2 + 0x1FFF) >> 13;
            if (origSize2 > 0)
            {
                while (ds2 > 0)
                {
                    numBlocks2++;
                    ds2--;
                }
            }
            unsigned long sectorSize2 = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave2 = numBlocks2 * sectorSize2;
            unsigned long alignedSize2 = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave2);
            MemCard* mc2 = g_MemCards[Slot];
            u8 hasSpace2;
            if (alignedSize2 > mc2->m_CardInfo.FreeBytes)
                hasSpace2 = 0;
            else if (mc2->m_CardInfo.FreeFiles < 1)
                hasSpace2 = 0;
            else
                hasSpace2 = 1;
            if (hasSpace2 == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x24EC | 0x8018BE48 | size: 0x21C
 * TODO: 87.58% match - icon calc nor/srawi/and constant-folded, CSE of channel*4
 * into r31, extra cmpwi loop guards, numBlocks r28 vs r29. hasSpace condition fixed
 * (FreeFiles < 1 with swapped branches). Remaining diffs are compiler heuristics.
 */
unsigned long SaveCallbacks::FileWriteIconCB(unsigned long channel, long result, void* data)
{
    if (result != 0)
    {
        long errorCode = result;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[channel]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }

        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);

        InOperation = false;

        if (errorCode == -4)
        {
            MemCard* card2 = g_MemCards[channel];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks = 0;
            int origSize = (dataSize += 12);
            dataSize = (u32)(dataSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }

            MemCard::ICON_CONFIG IconCfg;
            IconCfg.BannerFormat = 0;
            IconCfg.IconCount = 0;
            IconCfg.IconFormat = 0;
            IconCfg.IconAnimType = 0;
            memset(IconCfg.IconSpeeds, 0, 8);
            memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));

            IconCfg.IconCount = 1;
            IconCfg.IconFormat = 2;
            IconCfg.IconSpeeds[0] = 3;
            IconCfg.BannerFormat = 2;

            int iconFormat = IconCfg.IconFormat;
            int iconCount = IconCfg.IconCount;
            int iconSize = (iconFormat << 10) * iconCount;
            int temp = ~(iconCount | -1);
            int bannerClut = (temp >> 31) & 0x200;
            int bannerSize = iconFormat * 0xC00;
            int iconClut = (temp >> 31) & 0x200;
            int total = bannerClut + bannerSize + iconSize + iconClut;
            origSize = (int)(IconCfg.HeaderSize = total + 0x40);
            dataSize = (u32)(origSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }

            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytesToSave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[channel]->AlignBytesToSectorSize(bytesToSave);
            MemCard* mc = g_MemCards[channel];
            u8 hasSpace;
            if (alignedSize > mc->m_CardInfo.FreeBytes)
                hasSpace = 0;
            else if (mc->m_CardInfo.FreeFiles < 1)
                hasSpace = 0;
            else
                hasSpace = 1;
            if (hasSpace == 0)
                errorCode = -9;
        }

        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }

    DoSave(channel);
}

/**
 * Offset/Address/Size: 0x1F30 | 0x8018B88C | size: 0x5BC
 * TODO: 77.30% match - rlwinm vs and for 0x200 mask (target uses li+and, compiler
 * optimizes to rlwinm). Extra cmpwi loop guards before unrolled while loops (4x).
 * CSE puts Slot*4 in r30 vs target's r27 mutation. Stack offset +0xC shift.
 */
unsigned long SaveCallbacks::CreateFileCB(unsigned long Slot, long Result, void* pUserData)
{
    if (Result != 0)
    {
        long errorCode = Result;
        m_pSaveFile = NULL;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card2 = g_MemCards[Slot];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks = 0;
            int origSize = (dataSize += 12);
            dataSize = (u32)(dataSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            MemCard::ICON_CONFIG IconCfg;
            IconCfg.BannerFormat = 0;
            IconCfg.IconCount = 0;
            IconCfg.IconFormat = 0;
            IconCfg.IconAnimType = 0;
            memset(IconCfg.IconSpeeds, 0, 8);
            memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
            IconCfg.IconCount = 1;
            IconCfg.IconFormat = 2;
            IconCfg.IconSpeeds[0] = 3;
            IconCfg.BannerFormat = 2;
            int negOne = -1;
            int iconFormat = (int)(unsigned char)IconCfg.IconFormat;
            int iconCount = (int)(unsigned char)IconCfg.IconCount;
            int iconPixelSize = iconFormat << 10;
            negOne = ~(iconCount | negOne);
            int clutSize = 0x200;
            int iconDataSize = iconCount * iconPixelSize;
            int bannerClutMask = negOne >> 31;
            int iconClutMask = negOne >> 31;
            int bannerClutResult = clutSize & bannerClutMask;
            int bannerDataSize = iconFormat * 0xC00;
            int iconClutResult = clutSize & iconClutMask;
            int total = bannerClutResult + bannerDataSize;
            total += iconDataSize;
            total += iconClutResult;
            origSize = (int)(IconCfg.HeaderSize = total + 0x40);
            dataSize = (u32)(origSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[Slot];
            u8 hasSpace;
            if (alignedSize > mc->m_CardInfo.FreeBytes)
                hasSpace = 0;
            else if (mc->m_CardInfo.FreeFiles < 1)
                hasSpace = 0;
            else
                hasSpace = 1;
            if (hasSpace == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }
    MemCard::ICON_DATA_INFO localDataInfo1;
    m_pSaveFile->IconCfg.GetValidDataInfo(localDataInfo1);
    IconDataCache* cache = &gIconDataCache;
    void* pHdrBuf = cache->mIconHdrBuffer;
    void* bannerBuf = cache->mBannerBuffer;
    u32 bannerOfs = localDataInfo1.BannerOffset;
    void* destBanner = (u8*)pHdrBuf + bannerOfs;
    u8 bannerFmt = m_pSaveFile->IconCfg.BannerFormat;
    u32 tableOfs = *(u32*)((u8*)bannerBuf + 8);
    u32 entryVal = *(u32*)((u8*)bannerBuf + tableOfs);
    u32 dataOfs = *(u32*)((u8*)bannerBuf + entryVal + 8);
    void* srcBanner = (u8*)bannerBuf + dataOfs;
    int bfm1 = bannerFmt - 1;
    int bfm2 = 1 - bannerFmt;
    int bannerMask = ~(bfm1 | bfm2);
    int bannerClutV = 0x200 & (bannerMask >> 31);
    int banDataV = bannerFmt * 0xC00;
    u32 bannerCopySize = bannerClutV + banDataV;
    memcpy(destBanner, srcBanner, bannerCopySize);
    MemCard::ICON_DATA_INFO localDataInfo2;
    m_pSaveFile->IconCfg.GetValidDataInfo(localDataInfo2);
    void* pHdrBuf2 = cache->mIconHdrBuffer;
    void* iconBuf = cache->mIconBuffer;
    u32 iconOfs = localDataInfo2.IconOffset[0];
    void* destIcon = (u8*)pHdrBuf2 + iconOfs;
    s8 iconFmtS = m_pSaveFile->IconCfg.IconFormat;
    u32 itableOfs = *(u32*)((u8*)iconBuf + 8);
    u32 ientryVal = *(u32*)((u8*)iconBuf + itableOfs);
    u32 idataOfs = *(u32*)((u8*)iconBuf + ientryVal + 8);
    void* srcIcon = (u8*)iconBuf + idataOfs;
    u32 iconCopySize = iconFmtS << 10;
    memcpy(destIcon, srcIcon, iconCopySize);
    u8 bannerFmt2 = m_pSaveFile->IconCfg.BannerFormat;
    s8 iconFmt2 = m_pSaveFile->IconCfg.IconFormat;
    u8 iconCount2 = m_pSaveFile->IconCfg.IconCount;
    int bf1 = bannerFmt2 - 1;
    int bf2 = 1 - bannerFmt2;
    int banClutMask = ~(bf1 | bf2);
    int banClutH = 0x200 & (banClutMask >> 31);
    int banDatH = bannerFmt2 * 0xC00;
    int if1 = iconFmt2 - 1;
    int if2 = 1 - iconFmt2;
    int icnClutMask = ~(if1 | if2);
    int icnClutH = 0x200 & (icnClutMask >> 31);
    u32 icnPixels = iconCount2 * (iconFmt2 << 10);
    int headerTotal = banClutH + banDatH + icnPixels + icnClutH;
    u32 headerSize = headerTotal + 0x40;
    m_pSaveFile->IconCfg.HeaderSize = headerSize;
    u32 crc = nlChecksum32(cache->mIconHdrBuffer, headerSize);
    m_IconCRC = crc;
    gIconCRC = m_IconCRC;
    typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb2 = &SaveCallbacks::FileWriteIconCB;
    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb2, cache->mIconDataInfo.pHeaderData);
    Result = g_MemCards[m_Slot]->WriteFileIconData(m_pSaveFile, (void*)cache->mIconDataInfo.pHeaderData, functor);
    if (Result != 0)
    {
        long errorCode = Result;
        unsigned long slot2 = m_Slot;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[slot2]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card3 = g_MemCards[slot2];
        card3->m_State = IS_IDLE;
        card3->m_CardState = CS_IDLE;
        CARDUnmount(card3->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card4 = g_MemCards[slot2];
            long dataSize2 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks2 = 0;
            int origSize2 = (dataSize2 += 12);
            dataSize2 = (u32)(dataSize2 + 0x1FFF) >> 13;
            if (origSize2 > 0)
            {
                while (dataSize2 > 0)
                {
                    numBlocks2++;
                    dataSize2--;
                }
            }
            MemCard::ICON_CONFIG IconCfg2;
            IconCfg2.BannerFormat = 0;
            IconCfg2.IconCount = 0;
            IconCfg2.IconFormat = 0;
            IconCfg2.IconAnimType = 0;
            memset(IconCfg2.IconSpeeds, 0, 8);
            memset(&IconCfg2, 0, sizeof(MemCard::ICON_CONFIG));
            IconCfg2.IconCount = 1;
            IconCfg2.IconFormat = 2;
            IconCfg2.IconSpeeds[0] = 3;
            IconCfg2.BannerFormat = 2;
            int negOne2 = -1;
            int iconFormat2 = (int)(unsigned char)IconCfg2.IconFormat;
            int iconCount3 = (int)(unsigned char)IconCfg2.IconCount;
            int iconPixelSize3 = iconFormat2 << 10;
            negOne2 = ~(iconCount3 | negOne2);
            int clutSize2 = 0x200;
            int iconDataSize3 = iconCount3 * iconPixelSize3;
            int bannerClutMask3 = negOne2 >> 31;
            int iconClutMask3 = negOne2 >> 31;
            int bannerClutResult2 = clutSize2 & bannerClutMask3;
            int bannerDataSize2 = iconFormat2 * 0xC00;
            int iconClutResult2 = clutSize2 & iconClutMask3;
            int total2 = bannerClutResult2 + bannerDataSize2;
            total2 += iconDataSize3;
            total2 += iconClutResult2;
            origSize2 = (int)(IconCfg2.HeaderSize = total2 + 0x40);
            dataSize2 = (u32)(origSize2 + 0x1FFF) >> 13;
            if (origSize2 > 0)
            {
                while (dataSize2 > 0)
                {
                    numBlocks2++;
                    dataSize2--;
                }
            }
            unsigned long sectorSize2 = card4->m_CardInfo.SectorSize;
            unsigned long bytestosave2 = numBlocks2 * sectorSize2;
            unsigned long alignedSize2 = g_MemCards[slot2]->AlignBytesToSectorSize(bytestosave2);
            MemCard* mc2 = g_MemCards[slot2];
            u8 hasSpace2;
            if (alignedSize2 > mc2->m_CardInfo.FreeBytes)
                hasSpace2 = 0;
            else if (mc2->m_CardInfo.FreeFiles < 1)
                hasSpace2 = 0;
            else
                hasSpace2 = 1;
            if (hasSpace2 == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
    }
    return Result;
}

/**
 * Offset/Address/Size: 0x1EC4 | 0x8018B820 | size: 0x6C
 */
unsigned long DeleteCallbacks::DeleteDoneCB(unsigned long channel, long result, void* data)
{
    MemCard* card = g_MemCards[channel];
    card->m_State = IS_IDLE;
    card->m_CardState = CS_IDLE;
    CARDUnmount(card->m_Slot);
    InOperation = false;
    g_Callback(result);
    return (-result | result) >> 31;
}

/**
 * Offset/Address/Size: 0x1E58 | 0x8018B7B4 | size: 0x6C
 */
unsigned long FormatCallbacks::FormatDoneCB(unsigned long channel, long result, void* data)
{
    MemCard* card = g_MemCards[channel];
    card->m_State = IS_IDLE;
    card->m_CardState = CS_IDLE;
    CARDUnmount(card->m_Slot);
    InOperation = false;
    g_Callback(result);
    return (-result | result) >> 31;
}

/**
 * Offset/Address/Size: 0x11DC | 0x8018AB38 | size: 0xC7C
 * TODO: 70.01% match - register allocation (r27 vs r29 for Slot) and constant folding
 *   in first error path's HasEnoughFreeSpace inline due to -inline deferred vs -inline auto.
 *   Success path (icon copy, functor construction, inner error paths) matches perfectly.
 */
unsigned long SaveCallbacks::CardMountCB(unsigned long Slot, long Result, void* pUserData)
{
    m_Slot = Slot;
    if (Result != 0)
    {
        long errorCode = Result;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card2 = g_MemCards[Slot];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks = 0;
            int origSize = (dataSize += 12);
            dataSize = (u32)(dataSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            MemCard::ICON_CONFIG IconCfg;
            IconCfg.BannerFormat = 0;
            IconCfg.IconCount = 0;
            IconCfg.IconFormat = 0;
            IconCfg.IconAnimType = 0;
            memset(IconCfg.IconSpeeds, 0, 8);
            memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
            int negOne = -1;
            int iconFormat = 2;
            int iconCount = 1;
            int speed = 3;
            int iconPixelSize = iconFormat << 10;
            IconCfg.IconCount = iconCount;
            negOne = ~(iconCount | negOne);
            int clutSize = 0x200;
            int iconDataSize = iconCount * iconPixelSize;
            IconCfg.IconFormat = iconFormat;
            int bannerClutMask = negOne >> 31;
            int iconClutMask = negOne >> 31;
            IconCfg.IconSpeeds[0] = speed;
            int bannerClutResult = clutSize & bannerClutMask;
            int bannerDataSize = iconFormat * 0xC00;
            int iconClutResult = clutSize & iconClutMask;
            IconCfg.BannerFormat = iconFormat;
            int total = bannerClutResult + bannerDataSize;
            total += iconDataSize;
            total += iconClutResult;
            origSize = (int)(IconCfg.HeaderSize = total + 0x40);
            dataSize = (u32)(origSize + 0x1FFF) >> 13;
            if (origSize > 0)
            {
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize--;
                }
            }
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[Slot];
            u8 hasSpace;
            if (alignedSize > mc->m_CardInfo.FreeBytes)
                hasSpace = 0;
            else if (mc->m_CardInfo.FreeFiles >= 1)
                hasSpace = 1;
            else
                hasSpace = 0;
            if (hasSpace == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }
    long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize() + 12;
    long fileExistsResult = g_MemCards[Slot]->FileExists(MarioSoccerFileName);
    if (fileExistsResult == 0)
    {
        m_pSaveFile = NULL;
        unsigned long openSize;
        long openResult = g_MemCards[Slot]->OpenFile(MarioSoccerFileName, m_pSaveFile, &openSize);
        if (openResult != 0)
        {
            long errorCode = openResult;
            if (m_pSaveFile != NULL)
            {
                g_MemCards[Slot]->CloseFile(m_pSaveFile);
                m_pSaveFile = NULL;
            }
            MemCard* card = g_MemCards[Slot];
            card->m_State = IS_IDLE;
            card->m_CardState = CS_IDLE;
            CARDUnmount(card->m_Slot);
            InOperation = false;
            if (errorCode == -4)
            {
                MemCard* card2 = g_MemCards[Slot];
                long ds = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
                int numBlocks = 0;
                int origSize = (ds += 12);
                ds = (u32)(ds + 0x1FFF) >> 13;
                if (origSize > 0)
                {
                    while (ds > 0)
                    {
                        numBlocks++;
                        ds--;
                    }
                }
                MemCard::ICON_CONFIG IconCfg;
                IconCfg.BannerFormat = 0;
                IconCfg.IconCount = 0;
                IconCfg.IconFormat = 0;
                IconCfg.IconAnimType = 0;
                memset(IconCfg.IconSpeeds, 0, 8);
                memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
                int negOne = -1;
                int iconFormat = 2;
                int iconCount = 1;
                int speed = 3;
                int iconPixelSize = iconFormat << 10;
                IconCfg.IconCount = iconCount;
                negOne = ~(iconCount | negOne);
                int clutSize = 0x200;
                int iconDataSize2 = iconCount * iconPixelSize;
                IconCfg.IconFormat = iconFormat;
                int bannerClutMask = negOne >> 31;
                int iconClutMask = negOne >> 31;
                IconCfg.IconSpeeds[0] = speed;
                int bannerClutResult = clutSize & bannerClutMask;
                int bannerDataSize = iconFormat * 0xC00;
                int iconClutResult = clutSize & iconClutMask;
                IconCfg.BannerFormat = iconFormat;
                int total = bannerClutResult + bannerDataSize;
                total += iconDataSize2;
                total += iconClutResult;
                origSize = (int)(IconCfg.HeaderSize = total + 0x40);
                ds = (u32)(origSize + 0x1FFF) >> 13;
                if (origSize > 0)
                {
                    while (ds > 0)
                    {
                        numBlocks++;
                        ds--;
                    }
                }
                unsigned long sectorSize = card2->m_CardInfo.SectorSize;
                unsigned long bytestosave = numBlocks * sectorSize;
                unsigned long alignedSize = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave);
                MemCard* mc = g_MemCards[Slot];
                u8 hasSpace;
                if (alignedSize > mc->m_CardInfo.FreeBytes)
                    hasSpace = 0;
                else if (mc->m_CardInfo.FreeFiles < 1)
                    hasSpace = 0;
                else
                    hasSpace = 1;
                if (hasSpace == 0)
                    errorCode = -9;
            }
            m_MustFreeMemory = true;
            g_Callback(errorCode);
            ResetTask::s_resetPaused = false;
            return -1;
        }
        MemCard::ICON_DATA_INFO localDataInfo1;
        m_pSaveFile->IconCfg.GetValidDataInfo(localDataInfo1);
        IconDataCache* cache = &gIconDataCache;
        void* pHdrBuf = cache->mIconHdrBuffer;
        void* bannerBuf = cache->mBannerBuffer;
        u32 bannerOfs = localDataInfo1.BannerOffset;
        void* destBanner = (u8*)pHdrBuf + bannerOfs;
        u8 bannerFmt = m_pSaveFile->IconCfg.BannerFormat;
        u32 tableOfs = *(u32*)((u8*)bannerBuf + 8);
        u32 entryVal = *(u32*)((u8*)bannerBuf + tableOfs);
        u32 dataOfs = *(u32*)((u8*)bannerBuf + entryVal + 8);
        void* srcBanner = (u8*)bannerBuf + dataOfs;
        int bfm1 = bannerFmt - 1;
        int bfm2 = 1 - bannerFmt;
        int bannerMask = ~(bfm1 | bfm2);
        int bannerClutV = 0x200 & (bannerMask >> 31);
        int banDataV = bannerFmt * 0xC00;
        u32 bannerCopySize = bannerClutV + banDataV;
        memcpy(destBanner, srcBanner, bannerCopySize);
        MemCard::ICON_DATA_INFO localDataInfo2;
        m_pSaveFile->IconCfg.GetValidDataInfo(localDataInfo2);
        void* pHdrBuf2 = cache->mIconHdrBuffer;
        void* iconBuf = cache->mIconBuffer;
        u32 iconOfs = localDataInfo2.IconOffset[0];
        void* destIcon = (u8*)pHdrBuf2 + iconOfs;
        s8 iconFmtS = m_pSaveFile->IconCfg.IconFormat;
        u32 itableOfs = *(u32*)((u8*)iconBuf + 8);
        u32 ientryVal = *(u32*)((u8*)iconBuf + itableOfs);
        u32 idataOfs = *(u32*)((u8*)iconBuf + ientryVal + 8);
        void* srcIcon = (u8*)iconBuf + idataOfs;
        u32 iconCopySize = iconFmtS << 10;
        memcpy(destIcon, srcIcon, iconCopySize);
        MemCard::MC_FILE* pFile = m_pSaveFile;
        u8 bannerFmt2 = pFile->IconCfg.BannerFormat;
        s8 iconFmt2 = pFile->IconCfg.IconFormat;
        u8 iconCount2 = pFile->IconCfg.IconCount;
        int bf1 = bannerFmt2 - 1;
        int bf2 = 1 - bannerFmt2;
        int banClutMask = ~(bf1 | bf2);
        int banClutH = 0x200 & (banClutMask >> 31);
        int banDatH = bannerFmt2 * 0xC00;
        int if1 = iconFmt2 - 1;
        int if2 = 1 - iconFmt2;
        int icnClutMask = ~(if1 | if2);
        int icnClutH = 0x200 & (icnClutMask >> 31);
        u32 icnPixels = iconCount2 * (iconFmt2 << 10);
        int headerTotal = banClutH + banDatH + icnPixels + icnClutH;
        u32 headerSize = headerTotal + 0x40;
        pFile->IconCfg.HeaderSize = headerSize;
        u32 crc = nlChecksum32(pHdrBuf2, headerSize);
        m_IconCRC = crc;
        gIconCRC = crc;
        typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
        MemberCB cb2 = &SaveCallbacks::FileWriteIconCB;
        MemCardFunctor functor;
        new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb2);
        ((MemCardFunctor::MCInternalFunctorBase*)functor.m_FunctorMem)->m_pData = cache->mIconDataInfo.pHeaderData;
        long writeResult = g_MemCards[Slot]->WriteFileIconData(m_pSaveFile, (void*)cache->mIconDataInfo.pHeaderData, functor);
        if (writeResult == 0)
            return 0;
        long errorCode = writeResult;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card3 = g_MemCards[Slot];
        card3->m_State = IS_IDLE;
        card3->m_CardState = CS_IDLE;
        CARDUnmount(card3->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card4 = g_MemCards[Slot];
            long ds2 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks2 = 0;
            int origSize2 = (ds2 += 12);
            ds2 = (u32)(ds2 + 0x1FFF) >> 13;
            if (origSize2 > 0)
            {
                while (ds2 > 0)
                {
                    numBlocks2++;
                    ds2--;
                }
            }
            MemCard::ICON_CONFIG IconCfg2;
            IconCfg2.BannerFormat = 0;
            IconCfg2.IconCount = 0;
            IconCfg2.IconFormat = 0;
            IconCfg2.IconAnimType = 0;
            memset(IconCfg2.IconSpeeds, 0, 8);
            memset(&IconCfg2, 0, sizeof(MemCard::ICON_CONFIG));
            int negOne2 = -1;
            int iconFormat2 = 2;
            int iconCount3 = 1;
            int speed2 = 3;
            int iconPixelSize2 = iconFormat2 << 10;
            IconCfg2.IconCount = iconCount3;
            negOne2 = ~(iconCount3 | negOne2);
            int clutSize2 = 0x200;
            int iconDataSize3 = iconCount3 * iconPixelSize2;
            IconCfg2.IconFormat = iconFormat2;
            int bannerClutMask2 = negOne2 >> 31;
            int iconClutMask2 = negOne2 >> 31;
            IconCfg2.IconSpeeds[0] = speed2;
            int bannerClutResult2 = clutSize2 & bannerClutMask2;
            int bannerDataSize2 = iconFormat2 * 0xC00;
            int iconClutResult2 = clutSize2 & iconClutMask2;
            IconCfg2.BannerFormat = iconFormat2;
            int total2 = bannerClutResult2 + bannerDataSize2;
            total2 += iconDataSize3;
            total2 += iconClutResult2;
            origSize2 = (int)(IconCfg2.HeaderSize = total2 + 0x40);
            ds2 = (u32)(origSize2 + 0x1FFF) >> 13;
            if (origSize2 > 0)
            {
                while (ds2 > 0)
                {
                    numBlocks2++;
                    ds2--;
                }
            }
            unsigned long sectorSize2 = card4->m_CardInfo.SectorSize;
            unsigned long bytestosave2 = numBlocks2 * sectorSize2;
            unsigned long alignedSize2 = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave2);
            MemCard* mc2 = g_MemCards[Slot];
            u8 hasSpace2;
            if (alignedSize2 > mc2->m_CardInfo.FreeBytes)
                hasSpace2 = 0;
            else if (mc2->m_CardInfo.FreeFiles >= 1)
                hasSpace2 = 1;
            else
                hasSpace2 = 0;
            if (hasSpace2 == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }
    else if (fileExistsResult == -4)
    {
        MemCard::ICON_CONFIG IconCfg;
        IconCfg.BannerFormat = 0;
        IconCfg.IconCount = 0;
        IconCfg.IconFormat = 0;
        IconCfg.IconAnimType = 0;
        memset(IconCfg.IconSpeeds, 0, 8);
        memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
        IconCfg.IconCount = 1;
        IconCfg.IconFormat = 2;
        IconCfg.IconSpeeds[0] = 3;
        IconCfg.BannerFormat = 2;
        m_pSaveFile = NULL;
        typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
        MemberCB cb = &SaveCallbacks::CreateFileCB;
        MemCardFunctor functor;
        new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb);
        long createResult = g_MemCards[Slot]->CreateFile(MarioSoccerFileName, dataSize, &IconCfg, m_pSaveFile, functor);
        mRequiredMemoryCardID = 0;
        if (createResult == 0)
            return 0;
        long errorCode = createResult;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card5 = g_MemCards[Slot];
        card5->m_State = IS_IDLE;
        card5->m_CardState = CS_IDLE;
        CARDUnmount(card5->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card6 = g_MemCards[Slot];
            long ds3 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks3 = 0;
            int origSize3 = (ds3 += 12);
            ds3 = (u32)(ds3 + 0x1FFF) >> 13;
            if (origSize3 > 0)
            {
                while (ds3 > 0)
                {
                    numBlocks3++;
                    ds3--;
                }
            }
            MemCard::ICON_CONFIG IconCfg3;
            IconCfg3.BannerFormat = 0;
            IconCfg3.IconCount = 0;
            IconCfg3.IconFormat = 0;
            IconCfg3.IconAnimType = 0;
            memset(IconCfg3.IconSpeeds, 0, 8);
            memset(&IconCfg3, 0, sizeof(MemCard::ICON_CONFIG));
            int negOne3 = -1;
            int iconFormat3 = 2;
            int iconCount4 = 1;
            int speed3 = 3;
            int iconPixelSize3 = iconFormat3 << 10;
            IconCfg3.IconCount = iconCount4;
            negOne3 = ~(iconCount4 | negOne3);
            int clutSize3 = 0x200;
            int iconDataSize4 = iconCount4 * iconPixelSize3;
            IconCfg3.IconFormat = iconFormat3;
            int bannerClutMask3 = negOne3 >> 31;
            int iconClutMask3 = negOne3 >> 31;
            IconCfg3.IconSpeeds[0] = speed3;
            int bannerClutResult3 = clutSize3 & bannerClutMask3;
            int bannerDataSize3 = iconFormat3 * 0xC00;
            int iconClutResult3 = clutSize3 & iconClutMask3;
            IconCfg3.BannerFormat = iconFormat3;
            int total3 = bannerClutResult3 + bannerDataSize3;
            total3 += iconDataSize4;
            total3 += iconClutResult3;
            origSize3 = (int)(IconCfg3.HeaderSize = total3 + 0x40);
            ds3 = (u32)(origSize3 + 0x1FFF) >> 13;
            if (origSize3 > 0)
            {
                while (ds3 > 0)
                {
                    numBlocks3++;
                    ds3--;
                }
            }
            unsigned long sectorSize3 = card6->m_CardInfo.SectorSize;
            unsigned long bytestosave3 = numBlocks3 * sectorSize3;
            unsigned long alignedSize3 = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave3);
            MemCard* mc3 = g_MemCards[Slot];
            u8 hasSpace3;
            if (alignedSize3 > mc3->m_CardInfo.FreeBytes)
                hasSpace3 = 0;
            else if (mc3->m_CardInfo.FreeFiles >= 1)
                hasSpace3 = 1;
            else
                hasSpace3 = 0;
            if (hasSpace3 == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }
    else
    {
        long errorCode = fileExistsResult;
        if (m_pSaveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(m_pSaveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card7 = g_MemCards[Slot];
        card7->m_State = IS_IDLE;
        card7->m_CardState = CS_IDLE;
        CARDUnmount(card7->m_Slot);
        InOperation = false;
        if (errorCode == -4)
        {
            MemCard* card8 = g_MemCards[Slot];
            long ds4 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks4 = 0;
            int origSize4 = (ds4 += 12);
            ds4 = (u32)(ds4 + 0x1FFF) >> 13;
            if (origSize4 > 0)
            {
                while (ds4 > 0)
                {
                    numBlocks4++;
                    ds4--;
                }
            }
            MemCard::ICON_CONFIG IconCfg4;
            IconCfg4.BannerFormat = 0;
            IconCfg4.IconCount = 0;
            IconCfg4.IconFormat = 0;
            IconCfg4.IconAnimType = 0;
            memset(IconCfg4.IconSpeeds, 0, 8);
            memset(&IconCfg4, 0, sizeof(MemCard::ICON_CONFIG));
            int negOne4 = -1;
            int iconFormat4 = 2;
            int iconCount5 = 1;
            int speed4 = 3;
            int iconPixelSize4 = iconFormat4 << 10;
            IconCfg4.IconCount = iconCount5;
            negOne4 = ~(iconCount5 | negOne4);
            int clutSize4 = 0x200;
            int iconDataSize5 = iconCount5 * iconPixelSize4;
            IconCfg4.IconFormat = iconFormat4;
            int bannerClutMask4 = negOne4 >> 31;
            int iconClutMask4 = negOne4 >> 31;
            IconCfg4.IconSpeeds[0] = speed4;
            int bannerClutResult4 = clutSize4 & bannerClutMask4;
            int bannerDataSize4 = iconFormat4 * 0xC00;
            int iconClutResult4 = clutSize4 & iconClutMask4;
            IconCfg4.BannerFormat = iconFormat4;
            int total4 = bannerClutResult4 + bannerDataSize4;
            total4 += iconDataSize5;
            total4 += iconClutResult4;
            origSize4 = (int)(IconCfg4.HeaderSize = total4 + 0x40);
            ds4 = (u32)(origSize4 + 0x1FFF) >> 13;
            if (origSize4 > 0)
            {
                while (ds4 > 0)
                {
                    numBlocks4++;
                    ds4--;
                }
            }
            unsigned long sectorSize4 = card8->m_CardInfo.SectorSize;
            unsigned long bytestosave4 = numBlocks4 * sectorSize4;
            unsigned long alignedSize4 = g_MemCards[Slot]->AlignBytesToSectorSize(bytestosave4);
            MemCard* mc4 = g_MemCards[Slot];
            u8 hasSpace4;
            if (alignedSize4 > mc4->m_CardInfo.FreeBytes)
                hasSpace4 = 0;
            else if (mc4->m_CardInfo.FreeFiles >= 1)
                hasSpace4 = 1;
            else
                hasSpace4 = 0;
            if (hasSpace4 == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }
}

/**
 * Offset/Address/Size: 0x10D4 | 0x8018AA30 | size: 0x108
 */
long SaveLoad::StartSave(int slot, void (*callback)(long))
{
    nlPrintf("StartSave\n");

    if (SaveSystem.m_MustFreeMemory)
    {
        if (SaveSystem.m_pSaveGameBuffer != nullptr)
        {
            nlFree(SaveSystem.m_pSaveGameBuffer);
            SaveSystem.m_pSaveGameBuffer = nullptr;
        }
    }

    SaveSystem.m_MustFreeMemory = false;
    InOperation = true;
    g_Callback = callback;

    typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &SaveCallbacks::CardMountCB;

    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(&SaveSystem, cb);

    s32 result = g_MemCards[slot]->BeginCardAccess(functor);
    if (result != 0)
    {
        InOperation = false;
    }

    return result;
}

/**
 * Offset/Address/Size: 0xE88 | 0x8018A7E4 | size: 0x24C
 */
unsigned long LoadCallbacks::CardMountCB(unsigned long channel, long result, void* data)
{
    if (result != 0)
    {
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        m_MustFreeBuffers = true;
        return -1;
    }

    MemCard::MC_FILE* pFile;
    unsigned long fileLen;
    result = g_MemCards[channel]->OpenFile(MarioSoccerFileName, pFile, &fileLen);

    if (result != 0)
    {
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        m_MustFreeBuffers = true;
        return -1;
    }

    if (!m_PerformLoad)
    {
        if (pFile != NULL)
        {
            g_MemCards[channel]->CloseFile(pFile);
        }
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        m_MustFreeBuffers = true;
        return 0;
    }

    memset(m_pReadBuffer, 0, m_AlignedReadBufferDataSize);

    MemCard::MC_FILE* pFileLocal = pFile;
    typedef unsigned long (LoadCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &LoadCallbacks::ReadDoneCB;
    MemCardFunctor functor;
    void* functorMem = functor.m_FunctorMem;
    new (functorMem) MemCardFunctor::MCMemberFunctor<LoadCallbacks>(&LoadSystem, cb, pFileLocal);

    result = g_MemCards[channel]->InternalReadFile(pFile, m_pReadBuffer, m_AlignedReadBufferDataSize, pFile->TotalHeaderSize, functor);

    if (result != 0)
    {
        if (pFile != NULL)
        {
            g_MemCards[channel]->CloseFile(pFile);
        }
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        m_MustFreeBuffers = true;
        return -1;
    }

    return 0;
}

/**
 * Offset/Address/Size: 0xBFC | 0x8018A558 | size: 0x28C
 * TODO: 91.2% match - register allocation diffs in icon header size calculation
 * due to -inline deferred vs -inline auto optimization difference in scratch.
 * Code structure and logic is correct.
 */
#pragma push
#pragma opt_propagation off
long SaveLoad::StartLoad(int Slot, void (*pCB)(long), bool PerformLoad, bool testOnly)
{
    MemCardFunctor functor;
    typedef unsigned long (LoadCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    MemCard::ICON_CONFIG IconCfg;

    tDebugPrintManager::Print(DC_FE, "StartLoad\n");

    if (LoadSystem.m_MustFreeBuffers)
    {
        nlFree(LoadSystem.m_pReadBuffer);
        LoadSystem.m_pReadBuffer = NULL;
        nlFree(LoadSystem.m_pIconReadBuffer);
        LoadSystem.m_pIconReadBuffer = NULL;
    }

    LoadSystem.m_MustFreeBuffers = false;

    if (PerformLoad)
    {
        long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize() + 0xC;
        LoadSystem.m_AlignedReadBufferDataSize = dataSize;
        dataSize = (dataSize + 0x1FF) & ~0x1FF;
        LoadSystem.m_AlignedReadBufferDataSize = dataSize;
        LoadSystem.m_pReadBuffer = nlMalloc(dataSize, 0x20, true);
        memset(LoadSystem.m_pReadBuffer, 0, LoadSystem.m_AlignedReadBufferDataSize);

        IconCfg.BannerFormat = 0;
        IconCfg.IconCount = 0;
        IconCfg.IconFormat = 0;
        IconCfg.IconAnimType = 0;
        memset(IconCfg.IconSpeeds, 0, 8);
        memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));

        int negOne = -1;
        int iconFormat = 2;
        int iconCount = 1;
        int speed = 3;
        int iconPixelSize = iconFormat << 10;
        IconCfg.IconCount = iconCount;
        negOne = ~(iconCount | negOne);
        int clutSize = 0x200;
        int iconDataSize = iconCount * iconPixelSize;
        IconCfg.IconFormat = iconFormat;
        int bannerClutNeg = negOne;
        int iconClutNeg = negOne;
        int bannerClutMask = bannerClutNeg >> 31;
        int iconClutMask = iconClutNeg >> 31;
        IconCfg.IconSpeeds[0] = speed;
        int bannerClutResult = clutSize & bannerClutMask;
        int bannerDataSize = iconFormat * 0xC00;
        int iconClutResult = clutSize & iconClutMask;
        IconCfg.BannerFormat = iconFormat;
        int total = bannerClutResult + bannerDataSize;
        total += iconDataSize;
        total += iconClutResult;

        IconCfg.HeaderSize = total + 0x40;

        u32 allocSize = (IconCfg.HeaderSize + 0x1FF) & ~0x1FF;
        LoadSystem.m_pIconReadBuffer = nlMalloc(allocSize, 0x20, true);

        u8 bannerFmt = IconCfg.BannerFormat;
        u8 iconCnt = IconCfg.IconCount;
        s8 iconFmt = IconCfg.IconFormat;

        u32 headerSize = 0;
        headerSize += ((bannerFmt == 1) ? 0x200 : 0);
        headerSize += bannerFmt * 0xC00;
        headerSize += ((iconFmt == 1) ? 0x200 : 0);
        headerSize += iconCnt * (iconFmt << 10);
        headerSize += 0x40;
        IconCfg.HeaderSize = headerSize;

        memset(LoadSystem.m_pIconReadBuffer, 0, headerSize);

        LoadSystem.m_MustFreeBuffers = false;
    }

    InOperation = true;
    g_Callback = pCB;
    LoadSystem.m_TestGameID = testOnly;
    LoadSystem.m_PerformLoad = PerformLoad;

    cb = &LoadCallbacks::CardMountCB;

    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<LoadCallbacks>(&LoadSystem, cb);

    s32 result = g_MemCards[Slot]->BeginCardAccess(functor);
    if (result != 0)
    {
        InOperation = false;
    }

    return result;
}
#pragma pop

/**
 * Offset/Address/Size: 0xBEC | 0x8018A548 | size: 0x10
 */
bool SaveLoad::DidGameIDChange()
{
    return LoadSystem.m_GameIDTestResult;
}

/**
 * Offset/Address/Size: 0xB10 | 0x8018A46C | size: 0xDC
 */
unsigned long DeleteCallbacks::CardMountCB(unsigned long channel, long result, void* data)
{
    if (result != 0)
    {
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        return -1;
    }

    typedef unsigned long (DeleteCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &DeleteCallbacks::DeleteDoneCB;

    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<DeleteCallbacks>(this, cb);

    g_MemCards[channel]->DeleteFile(MarioSoccerFileName, functor);
    return 0;
}

/**
 * Offset/Address/Size: 0xA54 | 0x8018A3B0 | size: 0xBC
 */
long SaveLoad::StartDelete(int slot, void (*callback)(long))
{
    nlPrintf("StartDelete\n");

    InOperation = true;

    typedef unsigned long (DeleteCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &DeleteCallbacks::CardMountCB;

    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<DeleteCallbacks>(&DeleteSystem, cb);

    s32 result = g_MemCards[slot]->BeginCardAccess(functor);
    if (result != 0)
    {
        InOperation = false;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x900 | 0x8018A25C | size: 0x154
 */
unsigned long FormatCallbacks::CardMountCB(unsigned long channel, long result, void* data)
{
    if (result != 0 && result != -13 && result != -6)
    {
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        return -1;
    }

    s64 serialID = g_MemCards[channel]->GetSerialID();
    if (mLastKnownMemCardID.serialID != serialID)
    {
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(-1001);
        return -1;
    }

    typedef unsigned long (FormatCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &FormatCallbacks::FormatDoneCB;

    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<FormatCallbacks>(this, cb);

    g_MemCards[channel]->FormatCard(functor);
    return 0;
}

/**
 * Offset/Address/Size: 0x844 | 0x8018A1A0 | size: 0xBC
 */
long SaveLoad::StartFormat(int slot, void (*callback)(long))
{
    nlPrintf("StartFormat\n");

    InOperation = true;

    typedef unsigned long (FormatCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &FormatCallbacks::CardMountCB;

    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<FormatCallbacks>(&FormatSystem, cb);

    s32 result = g_MemCards[slot]->BeginCardAccess(functor);
    if (result != 0)
    {
        InOperation = false;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x5EC | 0x80189F48 | size: 0x258
 * TODO: 89.93% scratch match - remaining diffs are in the -4 block:
 * register allocation (r29/r31/r30 shuffle), extra cmpwi loop guard,
 * and missing nor/srawi/and clut mask chain under -inline auto.
 */
unsigned long FileExistsCallbacks::CardMountCB(unsigned long channel, long result, void* data)
{
    if (result == 0)
    {
        result = g_MemCards[channel]->FileExists(MarioSoccerFileName);
    }

    if (result != -5)
    {
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
    }

    InOperation = false;

    if (mRequiredMemoryCardID != 0)
    {
        s64 serialID = g_MemCards[channel]->GetSerialID();
        if (mRequiredMemoryCardID != serialID)
        {
            result = -1001;
            goto end;
        }
    }

    if (result == -4)
    {
        MemCard* card = g_MemCards[channel];
        long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
        int numBlocks = 0;

        int origSize = (dataSize += 12);
        dataSize = (u32)(dataSize + 0x1FFF) >> 13;
        if (origSize > 0)
        {
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize--;
            }
        }

        MemCard::ICON_CONFIG IconCfg;
        IconCfg.BannerFormat = 0;
        IconCfg.IconCount = 0;
        IconCfg.IconFormat = 0;
        IconCfg.IconAnimType = 0;
        memset(IconCfg.IconSpeeds, 0, 8);

        memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
        IconCfg.IconCount = 1;
        IconCfg.IconFormat = 2;
        IconCfg.IconSpeeds[0] = 3;
        IconCfg.BannerFormat = 2;

        int iconFormat = IconCfg.IconFormat;
        int iconCount = IconCfg.IconCount;
        int iconPixelSize = iconFormat << 10;
        int iconSize = iconCount * iconPixelSize;
        int negOne = ~(iconCount | -1);
        int clutSize = 0x200;
        int bannerClutMask = negOne >> 31;
        int iconClutMask = negOne >> 31;
        int bannerClut = clutSize & bannerClutMask;
        int bannerSize = iconFormat * 0xC00;
        int iconClut = clutSize & iconClutMask;

        int total = bannerClut + bannerSize;
        total += iconSize;
        total += iconClut;

        origSize = (int)(IconCfg.HeaderSize = total + 0x40);
        dataSize = (u32)(origSize + 0x1FFF) >> 13;
        if (origSize > 0)
        {
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize--;
            }
        }

        unsigned long sectorSize = card->m_CardInfo.SectorSize;
        unsigned long bytestosave = numBlocks * sectorSize;
        unsigned long alignedSize = g_MemCards[channel]->AlignBytesToSectorSize(bytestosave);
        MemCard* mc = g_MemCards[channel];

        u8 hasSpace;
        if (alignedSize > mc->m_CardInfo.FreeBytes)
        {
            hasSpace = 0;
        }
        else if (mc->m_CardInfo.FreeFiles < 1)
        {
            hasSpace = 0;
        }
        else
        {
            hasSpace = 1;
        }

        if (!hasSpace)
        {
            result = -9;
        }
    }

end:
    g_Callback(result);
    return -1;
}

/**
 * Offset/Address/Size: 0x520 | 0x80189E7C | size: 0xCC
 */
long SaveLoad::StartFileExistsCheck(int slot, void (*callback)(long))
{
    nlPrintf("StartFileExistsCheck\n");

    InOperation = true;
    g_Callback = callback;

    typedef unsigned long (FileExistsCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &FileExistsCallbacks::CardMountCB;

    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<FileExistsCallbacks>(&FileExistsSystem, cb);

    s32 result = g_MemCards[slot]->BeginCardAccess(functor);
    if (result != 0)
    {
        InOperation = false;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x45C | 0x80189DB8 | size: 0xC4
 */
unsigned long MemoryCardIDCallbacks::CardMountCB(unsigned long channel, long result, void* data)
{
    if (result == 0)
    {
        if (mRequiredMemoryCardID != 0)
        {
            s64 serialID = g_MemCards[channel]->GetSerialID();
            if (mRequiredMemoryCardID != serialID)
            {
                result = -1001;
            }
        }
    }

    if (result != -5)
    {
        MemCard* card = g_MemCards[channel];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
    }

    InOperation = false;
    g_Callback(result);
    return -1;
}

/**
 * Offset/Address/Size: 0x390 | 0x80189CEC | size: 0xCC
 */
long SaveLoad::StartMemoryCardIDCheck(int slot, void (*callback)(long))
{
    nlPrintf("StartMemoryCardIDCheck\n");

    InOperation = true;
    g_Callback = callback;

    typedef unsigned long (MemoryCardIDCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb = &MemoryCardIDCallbacks::CardMountCB;

    MemCardFunctor functor;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<MemoryCardIDCallbacks>(&MemoryCardIDSystem, cb);

    s32 result = g_MemCards[slot]->BeginCardAccess(functor);
    if (result != 0)
    {
        InOperation = false;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x264 | 0x80189BC0 | size: 0x12C
 * TODO: 77.9% match - extra cmpwi/ble loop-guard checks remain in both block
 * counting loops, and register allocation still differs in the icon header size
 * arithmetic chain.
 */
#pragma push
#pragma opt_propagation off
int SaveLoad::GetSaveBlockSize(int)
{
    int dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    int numBlocks = 0;

    int origSize = (dataSize += 12);
    dataSize = (u32)(dataSize + 0x1FFF) >> 13;
    if (origSize > 0)
    {
        while (dataSize > 0)
        {
            numBlocks++;
            dataSize--;
        }
    }

    MemCard::ICON_CONFIG IconCfg;
    IconCfg.BannerFormat = 0;
    IconCfg.IconCount = 0;
    IconCfg.IconFormat = 0;
    IconCfg.IconAnimType = 0;
    memset(IconCfg.IconSpeeds, 0, 8);
    memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
    int negOne = -1;
    int iconFormat = 2;
    int iconCount = 1;
    int speed = 3;
    int iconPixelSize = iconFormat << 10;
    IconCfg.IconCount = iconCount;
    negOne = ~(iconCount | negOne);
    int clutSize = 0x200;
    int iconDataSize = iconCount * iconPixelSize;
    IconCfg.IconFormat = iconFormat;
    int iconClutMask = negOne >> 31;
    int bannerClutMask = negOne >> 31;
    IconCfg.IconSpeeds[0] = speed;
    int iconClutResult = clutSize & iconClutMask;
    int bannerDataSize = iconFormat * 0xC00;
    int bannerClutResult = clutSize & bannerClutMask;
    IconCfg.BannerFormat = iconFormat;
    int total = iconClutResult + bannerDataSize;
    total += iconDataSize;
    total += bannerClutResult;

    origSize = (int)(IconCfg.HeaderSize = total + 0x40);
    dataSize = (u32)(origSize + 0x1FFF) >> 13;
    if (origSize > 0)
    {
        while (dataSize > 0)
        {
            numBlocks++;
            dataSize--;
        }
    }

    return numBlocks;
}
#pragma pop

static inline MemCard* GetCardBySlot(int slot)
{
    return g_MemCards[slot];
}

/**
 * Offset/Address/Size: 0xD8 | 0x80189A34 | size: 0x18C
 * TODO: 77.9% match - register allocation r29/r30 swapped (slot*4 in r30 vs target r29,
 * numBlocks in r29 vs target r30); compiler constant-folds ~(iconCount|-1) eliminating
 * the nor+srawi+and chain; pre-computed block count adds extra cmpwi/ble in both loops;
 * u8 return adds clrlwi masking. File uses -inline deferred which differs from scratch compiler.
 */
u8 SaveLoad::HasEnoughFreeSpace(int Slot)
{
    int slot = Slot;
    MemCard* card = GetCardBySlot(slot);
    int dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    int numBlocks;
    numBlocks = 0;

    int origSize = (dataSize += 12);
    dataSize = (u32)(dataSize + 0x1FFF) >> 13;
    if (origSize > 0)
    {
        for (; dataSize > 0; dataSize--)
            numBlocks++;
    }

    MemCard::ICON_CONFIG IconCfg;
    IconCfg.BannerFormat = 0;
    IconCfg.IconCount = 0;
    IconCfg.IconFormat = 0;
    IconCfg.IconAnimType = 0;
    memset(IconCfg.IconSpeeds, 0, 8);

    memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));
    IconCfg.IconCount = 1;
    IconCfg.IconFormat = 2;
    IconCfg.IconSpeeds[0] = 3;
    IconCfg.BannerFormat = 2;

    int iconFormat = IconCfg.IconFormat;
    int iconCount = IconCfg.IconCount;

    int iconSize = (iconFormat << 10) * iconCount;
    int temp = ~(iconCount | -1);
    int bannerClut = (temp >> 31) & 0x200;
    int bannerSize = iconFormat * 0xC00;
    int iconClut = (temp >> 31) & 0x200;

    int total = bannerClut + bannerSize + iconSize + iconClut;

    origSize = (int)(IconCfg.HeaderSize = total + 0x40);
    dataSize = (u32)(origSize + 0x1FFF) >> 13;
    if (origSize > 0)
    {
        while (dataSize-- > 0)
            numBlocks++;
    }

    unsigned long sectorSize = card->m_CardInfo.SectorSize;
    unsigned long bytestosave = numBlocks * sectorSize;
    unsigned long alignedSize = GetCardBySlot(slot)->AlignBytesToSectorSize(bytestosave);
    MemCard* mc = GetCardBySlot(slot);
    if (alignedSize > mc->m_CardInfo.FreeBytes)
        return 0;

    if (mc->m_CardInfo.FreeFiles >= 1)
        return 1;

    return 0;
}

/**
 * Offset/Address/Size: 0x34 | 0x80189990 | size: 0xA4
 */
void SaveLoad::FreeAllCallbackMemory()
{
    if (SaveSystem.m_MustFreeMemory)
    {
        if (SaveSystem.m_pSaveGameBuffer != nullptr)
        {
            nlFree(SaveSystem.m_pSaveGameBuffer);
            SaveSystem.m_pSaveGameBuffer = nullptr;
        }
    }
    SaveSystem.m_MustFreeMemory = false;

    if (LoadSystem.m_MustFreeBuffers)
    {
        nlFree(LoadSystem.m_pReadBuffer);
        LoadSystem.m_pReadBuffer = nullptr;
        nlFree(LoadSystem.m_pIconReadBuffer);
        LoadSystem.m_pIconReadBuffer = nullptr;
    }
    LoadSystem.m_MustFreeBuffers = false;
}

/**
 * Offset/Address/Size: 0x0 | 0x8018995C | size: 0x34
 */
void SaveLoad::RememberCurrentMemCardSerialID(int id)
{
    mLastKnownMemCardID.serialID = g_MemCards[id]->GetSerialID();
}
