#include "Game/DB/SaveLoad.h"
#include "Game/GameInfo.h"
#include "Game/Sys/gcmemcard.h"
#include "Game/Sys/debug.h"
#include "Game/FE/feHelpFuncs.h"
#include "NL/nlFileGC.h"
#include "NL/nlMain.h"
#include "NL/nlMath.h"
#include "Game/ResetTask.h"
#include <string.h>

extern void nlFree(void*);

IconDataCache gIconDataCache;

static bool InOperation = false;

LoadCallbacks LoadSystem;
SaveCallbacks SaveSystem;
DeleteCallbacks DeleteSystem;
FormatCallbacks FormatSystem;

static void (*g_Callback)(long);
FileExistsCallbacks FileExistsSystem;
MemoryCardIDCallbacks MemoryCardIDSystem;
struct MemCardIDInfo
{
    s64 serialID;
    // u32 cardID;
};
static MemCardIDInfo mLastKnownMemCardID;
static const char* MarioSoccerFileName;
static s64 mRequiredMemoryCardID;
static unsigned long gIconCRC;

// /**
//  * Offset/Address/Size: 0x0 | 0x8018D40C | size: 0xA4
//  */
// void 0x8028D340..0x8028D344 | size: 0x4
// {
// }

/**
 * Offset/Address/Size: 0x104 | 0x8018D3D8 | size: 0x34
 */
void MemCardFunctor::MCMemberFunctor<LoadCallbacks>::Call(unsigned long Slot, long Result)
{
    (m_pObject->*(*(MemberCB*)&m_pFunc))(Slot, Result, m_pData);
}

/**
 * Offset/Address/Size: 0xD0 | 0x8018D3A4 | size: 0x34
 */
void MemCardFunctor::MCMemberFunctor<SaveCallbacks>::Call(unsigned long Slot, long Result)
{
    (m_pObject->*(*(MemberCB*)&m_pFunc))(Slot, Result, m_pData);
}

/**
 * Offset/Address/Size: 0x9C | 0x8018D370 | size: 0x34
 */
void MemCardFunctor::MCMemberFunctor<DeleteCallbacks>::Call(unsigned long Slot, long Result)
{
    (m_pObject->*(*(MemberCB*)&m_pFunc))(Slot, Result, m_pData);
}

/**
 * Offset/Address/Size: 0x68 | 0x8018D33C | size: 0x34
 */
void MemCardFunctor::MCMemberFunctor<FormatCallbacks>::Call(unsigned long Slot, long Result)
{
    (m_pObject->*(*(MemberCB*)&m_pFunc))(Slot, Result, m_pData);
}

/**
 * Offset/Address/Size: 0x34 | 0x8018D308 | size: 0x34
 */
void MemCardFunctor::MCMemberFunctor<FileExistsCallbacks>::Call(unsigned long Slot, long Result)
{
    (m_pObject->*(*(MemberCB*)&m_pFunc))(Slot, Result, m_pData);
}

/**
 * Offset/Address/Size: 0x0 | 0x8018D2D4 | size: 0x34
 */
void MemCardFunctor::MCMemberFunctor<MemoryCardIDCallbacks>::Call(unsigned long Slot, long Result)
{
    (m_pObject->*(*(MemberCB*)&m_pFunc))(Slot, Result, m_pData);
}

IconDataCache::IconDataCache()
{
    gIconCRC = 0;
    mIconConfig.BannerFormat = 0;
    mIconConfig.IconCount = 0;
    mIconConfig.IconFormat = 0;
    mIconConfig.IconAnimType = 0;
    memset(mIconConfig.IconSpeeds, 0, 8);
    mIconHdrBuffer = NULL;
    mIconBuffer = NULL;
    mBannerBuffer = NULL;
}

LoadCallbacks::LoadCallbacks()
    : m_pReadBuffer(NULL)
    , m_pIconReadBuffer(NULL)
    , m_pLoadFile(NULL)
    , m_MustFreeBuffers(false)
    , m_IconLoadedCRC(0)
{
}

SaveCallbacks::SaveCallbacks()
    : m_pSaveFile(NULL)
    , m_pSaveGameBuffer(NULL)
    , m_IconCRC(0)
{
}

/**
 * Offset/Address/Size: 0x38E4 | 0x8018D240 | size: 0x94
 */
IconDataCache::~IconDataCache()
{
    if (mIconHdrBuffer)
    {
        delete[] mIconHdrBuffer;
        mIconHdrBuffer = NULL;
    }
    if (mIconBuffer)
    {
        delete[] mIconBuffer;
        mIconBuffer = NULL;
    }
    if (mBannerBuffer)
    {
        delete[] mBannerBuffer;
        mBannerBuffer = NULL;
    }
}

/**
 * Offset/Address/Size: 0x38DC | 0x8018D238 | size: 0x8
 */
bool SaveLoad::CardBusy()
{
    return InOperation;
}

/**
 * Offset/Address/Size: 0x3720 | 0x8018D07C | size: 0x1BC
 * TODO: 99.68% match - iconFmt/iconCount load registers and product-plus-banner accumulator destination still differ.
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

    u8 iconCount = gIconDataCache.mIconConfig.IconCount;
    int bannerFmt = gIconDataCache.mIconConfig.BannerFormat;
    s8 iconFmt = gIconDataCache.mIconConfig.IconFormat;

    u32 headerSize = 0;
    headerSize += ((bannerFmt == 1) ? 0x200 : 0);
    headerSize += bannerFmt * 0xC00;
    u32 iconClut = ((iconFmt == 1) ? 0x200 : 0);
    headerSize = ((iconFmt << 10) * iconCount) + headerSize;
    headerSize = iconClut + headerSize;
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
 * TODO: 98.67% match - Slot/Result move order, early li r6, and
 * header-size register flow still differ.
 */
unsigned long LoadCallbacks::LoadIconDataDoneCB(unsigned long Slot, long Result, void* pUserData)
{
    void* pReadBuf = m_pReadBuffer;
    MemCard::MC_FILE* pFile = (MemCard::MC_FILE*)pUserData;
    int bannerFmt = pFile->IconCfg.BannerFormat;
    s8 iconFmt = pFile->IconCfg.IconFormat;
    u8 iconCount = pFile->IconCfg.IconCount;

    u32 headerSize = 0;
    headerSize += ((bannerFmt == 1) ? 0x200 : 0);
    headerSize += bannerFmt * 0xC00;
    u32 iconClut = ((iconFmt == 1) ? 0x200 : 0);
    headerSize = ((iconFmt << 10) * iconCount) + headerSize;
    headerSize = iconClut + headerSize;
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

    gIconCRC = ((unsigned long*)pReadBuf)[2];
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
 * TODO: 93.27% match - icon header-size temporaries use different registers
 * and the read-call arguments are prepared earlier.
 */
inline unsigned long LoadCallbacks::ReadDoneCB(unsigned long Slot, long Result, void* pUserData)
{
    union
    {
        MemCardFunctor functor;
    };
    typedef unsigned long (LoadCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    MemCard::ICON_CONFIG localCfg;
    MCFILE_HEADER* header;
    unsigned long calculatedcrc;
    unsigned long filesize;

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

    bool configValid;
    if (memcmp(&localCfg, &((MemCard::MC_FILE*)pUserData)->IconCfg, 1) != 0)
    {
        configValid = 0;
    }
    else
    {
        header = (MCFILE_HEADER*)m_pReadBuffer;
        configValid = (header->Size == (u32)nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize());
    }

    if (!configValid)
    {
        if (pUserData != NULL)
        {
            g_MemCards[Slot]->CloseFile((MemCard::MC_FILE*)pUserData);
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

    header = (MCFILE_HEADER*)m_pReadBuffer;
    filesize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    calculatedcrc = nlChecksum32((MCFILE_HEADER*)m_pReadBuffer + 1, filesize);
    if (header->CRC != calculatedcrc)
    {
        if (pUserData != NULL)
        {
            g_MemCards[Slot]->CloseFile((MemCard::MC_FILE*)pUserData);
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

    m_pLoadFile = (MemCard::MC_FILE*)pUserData;

    cb = &LoadCallbacks::LoadIconDataDoneCB;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<LoadCallbacks>(this, cb, pUserData);

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
        if (pUserData != NULL)
        {
            g_MemCards[Slot]->CloseFile((MemCard::MC_FILE*)pUserData);
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

static inline int BuildDefaultIconHeaderSize(MemCard::ICON_CONFIG& IconCfg)
{
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

    u8 savedIconCount = IconCfg.IconCount;
    int bannerFmt = IconCfg.BannerFormat;
    s8 savedIconFmt = IconCfg.IconFormat;

    unsigned long bannerHeader = 0;
    bannerHeader += ((bannerFmt == 1) ? 0x200 : 0);
    bannerHeader += bannerFmt * 0xC00;
    unsigned long iconClut = ((savedIconFmt == 1) ? 0x200 : 0);
    unsigned long iconHeader = bannerHeader + savedIconCount * (savedIconFmt << 10);
    unsigned long totalHeader = iconHeader + iconClut;

    unsigned long headerSize = totalHeader + 0x40;
    IconCfg.HeaderSize = headerSize;
    return headerSize;
}

/**
 * Offset/Address/Size: 0x2DA8 | 0x8018C704 | size: 0x4AC
 */
#pragma push
#pragma opt_propagation off
inline unsigned long SaveCallbacks::FileWriteCB(unsigned long Slot, long Result, void* pUserData)
{
    if (Result != 0)
    {
        MemCard* card2;
        long errorCode;
        int numBlocks;
        MemCard::MC_FILE* saveFile = m_pSaveFile;
        errorCode = Result;
        if (saveFile != NULL)
        {
            g_MemCards[Slot]->CloseFile(saveFile);
            m_pSaveFile = NULL;
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;

        if (errorCode == -4)
        {
            unsigned long slotOffset = Slot << 2;
            card2 = g_MemCards[slotOffset >> 2];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            numBlocks = 0;
            dataSize += 12;
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize -= 0x2000;
            }
            MemCard::ICON_CONFIG IconCfg;
            dataSize = BuildDefaultIconHeaderSize(IconCfg);
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize -= 0x2000;
            }
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[slotOffset >> 2];
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

    if (mRequiredMemoryCardID != 0)
    {
        s64 serialID = g_MemCards[Slot]->GetSerialID();
        if (mRequiredMemoryCardID != serialID)
        {
            unsigned long slotOffset;
            int numBlocks;
            long serialError = -1001;
            MemCard* card2;
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

            if (serialError == -4)
            {
                slotOffset = Slot << 2;
                card2 = g_MemCards[slotOffset >> 2];
                long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
                numBlocks = 0;
                dataSize += 12;
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize -= 0x2000;
                }
                MemCard::ICON_CONFIG IconCfg;
                dataSize = BuildDefaultIconHeaderSize(IconCfg);
                while (dataSize > 0)
                {
                    numBlocks++;
                    dataSize -= 0x2000;
                }
                unsigned long sectorSize = card2->m_CardInfo.SectorSize;
                unsigned long bytestosave = numBlocks * sectorSize;
                unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
                MemCard* mc = g_MemCards[slotOffset >> 2];
                u8 hasSpace;
                if (alignedSize > mc->m_CardInfo.FreeBytes)
                    hasSpace = 0;
                else if (mc->m_CardInfo.FreeFiles < 1)
                    hasSpace = 0;
                else
                    hasSpace = 1;
                if (!hasSpace)
                    serialError = -9;
            }

            m_MustFreeMemory = true;
            g_Callback(serialError);
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
#pragma pop

/**
 * Offset/Address/Size: 0x2708 | 0x8018C064 | size: 0x6A0
 * TODO: 91.03% match - slot-offset registers, default icon-size arithmetic,
 * and icon-cache/functor load ordering still differ.
 */
long SaveCallbacks::DoSave(unsigned long Slot)
{
    typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    union
    {
        MemCardFunctor functor;
    };

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
        int numBlocks;
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
            unsigned long slotOffset = Slot << 2;
            MemCard* card2 = g_MemCards[slotOffset >> 2];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            numBlocks = 0;
            dataSize += 12;
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize -= 0x2000;
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
            int origSize = (int)(IconCfg.HeaderSize = total + 0x40);
            dataSize = (u32)(origSize + 0x1FFF) >> 13;
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize--;
            }
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[slotOffset >> 2];
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
        localDataInfo1.pHeaderData = (unsigned char*)cache->mIconHdrBuffer;
        void* bannerBuf = cache->mBannerBuffer;
        u32 bannerOfs = localDataInfo1.BannerOffset;
        void* destBanner = localDataInfo1.pHeaderData + bannerOfs;
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
        localDataInfo2.pHeaderData = (unsigned char*)cache->mIconHdrBuffer;
        void* iconBuf = cache->mIconBuffer;
        u32 iconOfs = localDataInfo2.IconOffset[0];
        void* destIcon = localDataInfo2.pHeaderData + iconOfs;
        s8 iconFmtS = m_pSaveFile->IconCfg.IconFormat;
        u32 itableOfs = *(u32*)((u8*)iconBuf + 8);
        u32 ientryVal = *(u32*)((u8*)iconBuf + itableOfs);
        u32 idataOfs = *(u32*)((u8*)iconBuf + ientryVal + 8);
        void* srcIcon = (u8*)iconBuf + idataOfs;
        u32 iconCopySize = iconFmtS << 10;
        memcpy(destIcon, srcIcon, iconCopySize);
        int bannerFmt2 = m_pSaveFile->IconCfg.BannerFormat;
        int iconFmt2 = m_pSaveFile->IconCfg.IconFormat;
        u8 iconCount2 = m_pSaveFile->IconCfg.IconCount;
        int bannerMinus1 = bannerFmt2 - 1;
        int bannerInv = 1 - bannerFmt2;
        int bannerMask2 = ~(bannerMinus1 | bannerInv);
        int clutSizeForHeader = 0x200;
        int bannerClut2 = clutSizeForHeader;
        bannerClut2 &= (bannerMask2 >> 31);
        int bannerData2 = bannerFmt2 * 0xC00;
        int iconPixels2 = iconCount2 * (iconFmt2 << 10);
        int iconMinus1 = iconFmt2 - 1;
        int iconInv = 1 - iconFmt2;
        int iconMask2 = ~(iconMinus1 | iconInv);
        int iconClut2 = clutSizeForHeader;
        iconClut2 &= (iconMask2 >> 31);
        int headerTotal = bannerClut2 + bannerData2;
        headerTotal += iconPixels2;
        headerTotal += iconClut2;
        u32 headerSize = headerTotal + 0x40;
        m_pSaveFile->IconCfg.HeaderSize = headerSize;
        u32 crc = nlChecksum32(localDataInfo2.pHeaderData, headerSize);
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
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb, header);
    long result = g_MemCards[Slot]->InternalWriteFile(m_pSaveFile, m_pSaveGameBuffer, dataSize, m_pSaveFile->TotalHeaderSize, functor, true);
    if (result != 0)
    {
        long errorCode = result;
        int numBlocks2;
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
            unsigned long slotOffset = Slot << 2;
            MemCard* card2 = g_MemCards[slotOffset >> 2];
            long ds2 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            numBlocks2 = 0;
            ds2 += 12;
            while (ds2 > 0)
            {
                numBlocks2++;
                ds2 -= 0x2000;
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
            int origSize2 = (int)(IconCfg2.HeaderSize = total2 + 0x40);
            ds2 = (u32)(origSize2 + 0x1FFF) >> 13;
            while (ds2 > 0)
            {
                numBlocks2++;
                ds2--;
            }
            unsigned long sectorSize2 = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave2 = numBlocks2 * sectorSize2;
            unsigned long alignedSize2 = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave2);
            MemCard* mc2 = g_MemCards[slotOffset >> 2];
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
 * TODO: 95.85% match - icon header arithmetic register/order differences
 * remain.
 */
#pragma push
#pragma opt_propagation off
inline unsigned long SaveCallbacks::FileWriteIconCB(unsigned long Slot, long Result, void* pUserData)
{
    if (Result != 0)
    {
        MemCard* card2;
        long errorCode;
        int numBlocks;
        errorCode = Result;
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
            unsigned long slotOffset = Slot << 2;
            card2 = g_MemCards[slotOffset >> 2];
            long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            numBlocks = 0;
            dataSize += 12;
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize -= 0x2000;
            }

            MemCard::ICON_CONFIG IconCfg;
            dataSize = BuildDefaultIconHeaderSize(IconCfg);
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize -= 0x2000;
            }

            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytesToSave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytesToSave);
            MemCard* mc = g_MemCards[slotOffset >> 2];
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

    DoSave(Slot);
}
#pragma pop

/**
 * Offset/Address/Size: 0x1F30 | 0x8018B88C | size: 0x5BC
 * TODO: 86.81% match - remaining Slot*4 register allocation in duplicated
 * -4 paths, default icon size clut folding, and success-path cache/functor
 * register allocation differ.
 */
unsigned long SaveCallbacks::CreateFileCB(unsigned long Slot, long Result, void* pUserData)
{
    typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    long errorCode;

    if (Result != 0)
    {
        errorCode = Result;
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
            unsigned long slotOffset = Slot << 2;
            MemCard* card2 = g_MemCards[slotOffset >> 2];
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
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[slotOffset >> 2];
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
    localDataInfo1.pHeaderData = (unsigned char*)cache->mIconHdrBuffer;
    void* bannerBuf = cache->mBannerBuffer;
    u32 bannerOfs = localDataInfo1.BannerOffset;
    void* destBanner = localDataInfo1.pHeaderData + bannerOfs;
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
    localDataInfo2.pHeaderData = (unsigned char*)cache->mIconHdrBuffer;
    void* iconBuf = cache->mIconBuffer;
    u32 iconOfs = localDataInfo2.IconOffset[0];
    void* destIcon = localDataInfo2.pHeaderData + iconOfs;
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
    int bannerMinus1 = bannerFmt2 - 1;
    int bannerInv = 1 - bannerFmt2;
    int bannerMask2 = ~(bannerMinus1 | bannerInv);
    int clutSizeForHeader = 0x200;
    int bannerClut2 = clutSizeForHeader;
    bannerClut2 &= (bannerMask2 >> 31);
    int bannerData2 = bannerFmt2 * 0xC00;
    int iconPixels2 = iconCount2 * (iconFmt2 << 10);
    int iconMinus1 = iconFmt2 - 1;
    int iconInv = 1 - iconFmt2;
    int iconMask2 = ~(iconMinus1 | iconInv);
    int iconClut2 = clutSizeForHeader;
    iconClut2 &= (iconMask2 >> 31);
    int headerTotal = bannerClut2 + bannerData2;
    headerTotal += iconPixels2;
    headerTotal += iconClut2;
    u32 headerSize = headerTotal + 0x40;
    m_pSaveFile->IconCfg.HeaderSize = headerSize;
    u32 crc = nlChecksum32(localDataInfo2.pHeaderData, headerSize);
    m_IconCRC = crc;
    gIconCRC = m_IconCRC;
    void* headerData = cache->mIconDataInfo.pHeaderData;
    cb = &SaveCallbacks::FileWriteIconCB;
    union
    {
        MemCardFunctor functor;
    };
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb, headerData);
    Result = g_MemCards[m_Slot]->WriteFileIconData(m_pSaveFile, cache->mIconDataInfo.pHeaderData, functor);
    if (Result != 0)
    {
        errorCode = Result;
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
            unsigned long slotOffset2 = slot2 << 2;
            MemCard* card4 = g_MemCards[slotOffset2 >> 2];
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
            int iconFormat2 = IconCfg2.IconFormat;
            int iconCount3 = IconCfg2.IconCount;
            int iconPixelSize2 = iconFormat2 << 10;
            int iconSize3 = iconCount3 * iconPixelSize2;
            int negOne2 = ~(iconCount3 | -1);
            int clutSize2 = 0x200;
            int bannerClutMask2 = negOne2 >> 31;
            int iconClutMask2 = negOne2 >> 31;
            int bannerClut2 = clutSize2 & bannerClutMask2;
            int bannerSize2 = iconFormat2 * 0xC00;
            int iconClut2 = clutSize2 & iconClutMask2;
            int total2 = bannerClut2 + bannerSize2;
            total2 += iconSize3;
            total2 += iconClut2;
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
            unsigned long alignedSize2 = g_MemCards[slotOffset2 >> 2]->AlignBytesToSectorSize(bytestosave2);
            MemCard* mc2 = g_MemCards[slotOffset2 >> 2];
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
 * TODO: 97.70% match - mount-error result/card registers and icon header
 * copy register flow differ.
 */
#pragma push
#pragma opt_propagation off
unsigned long SaveCallbacks::CardMountCB(unsigned long Slot, long Result, void* pUserData)
{
    MemCard::ICON_CONFIG IconCfg;
    typedef unsigned long (SaveCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb2;
    MemberCB cb;

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
            unsigned long slotOffset = Slot << 2;
            MemCard* card2 = g_MemCards[slotOffset >> 2];
            int dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            int numBlocks = 0;
            dataSize += 12;
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize -= 0x2000;
            }
            MemCard::ICON_CONFIG IconCfg;
            dataSize = BuildDefaultIconHeaderSize(IconCfg);
            while (dataSize > 0)
            {
                numBlocks++;
                dataSize -= 0x2000;
            }
            unsigned long sectorSize = card2->m_CardInfo.SectorSize;
            unsigned long bytestosave = numBlocks * sectorSize;
            unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
            MemCard* mc = g_MemCards[slotOffset >> 2];
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
    long dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize() + 12;
    Result = g_MemCards[Slot]->FileExists(MarioSoccerFileName);
    switch (Result)
    {
    case 0:
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
                MemCard* card2;
                int numBlocks;
                unsigned long slotOffset = Slot << 2;
                card2 = g_MemCards[slotOffset >> 2];
                int ds = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
                numBlocks = 0;
                ds += 12;
                while (ds > 0)
                {
                    numBlocks++;
                    ds -= 0x2000;
                }
                MemCard::ICON_CONFIG IconCfg;
                ds = BuildDefaultIconHeaderSize(IconCfg);
                while (ds > 0)
                {
                    numBlocks++;
                    ds -= 0x2000;
                }
                unsigned long sectorSize = card2->m_CardInfo.SectorSize;
                unsigned long bytestosave = numBlocks * sectorSize;
                unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
                MemCard* mc = g_MemCards[slotOffset >> 2];
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
        localDataInfo1.pHeaderData = (unsigned char*)gIconDataCache.mIconHdrBuffer;
        void* bannerBuf = gIconDataCache.mBannerBuffer;
        u32 bannerOfs = localDataInfo1.BannerOffset;
        void* destBanner = localDataInfo1.pHeaderData + bannerOfs;
        u8 bannerFmt = m_pSaveFile->IconCfg.BannerFormat;
        u32 tableOfs = *(u32*)((u8*)bannerBuf + 8);
        u32 entryVal = *(u32*)((u8*)bannerBuf + tableOfs);
        u32 dataOfs = *(u32*)((u8*)bannerBuf + entryVal + 8);
        void* srcBanner = (u8*)bannerBuf + dataOfs;
        u32 bannerCopySize = ((bannerFmt == 1) ? 0x200 : 0) + bannerFmt * 0xC00;
        memcpy(destBanner, srcBanner, bannerCopySize);
        MemCard::ICON_DATA_INFO localDataInfo2;
        m_pSaveFile->IconCfg.GetValidDataInfo(localDataInfo2);
        localDataInfo2.pHeaderData = (unsigned char*)gIconDataCache.mIconHdrBuffer;
        void* iconBuf = gIconDataCache.mIconBuffer;
        u32 iconOfs = localDataInfo2.IconOffset[0];
        void* destIcon = localDataInfo2.pHeaderData + iconOfs;
        u32 itableOfs = *(u32*)((u8*)iconBuf + 8);
        u32 ientryVal = *(u32*)((u8*)iconBuf + itableOfs);
        s8 iconFmtS = m_pSaveFile->IconCfg.IconFormat;
        u32 idataOfs = *(u32*)((u8*)iconBuf + ientryVal + 8);
        void* srcIcon = (u8*)iconBuf + idataOfs;
        u32 iconCopySize = iconFmtS << 10;
        memcpy(destIcon, srcIcon, iconCopySize);
        u8 bannerFmt2 = m_pSaveFile->IconCfg.BannerFormat;
        s8 iconFmt2 = m_pSaveFile->IconCfg.IconFormat;
        u8 iconCount2 = m_pSaveFile->IconCfg.IconCount;
        int bannerClut2 = ((bannerFmt2 == 1) ? 0x200 : 0);
        int bannerData2 = bannerFmt2 * 0xC00;
        int iconPixels2 = iconCount2 * (iconFmt2 << 10);
        int iconClut2 = ((iconFmt2 == 1) ? 0x200 : 0);
        int headerTotal = bannerClut2 + bannerData2;
        headerTotal += iconPixels2;
        headerTotal += iconClut2;
        u32 headerSize = headerTotal + 0x40;
        m_pSaveFile->IconCfg.HeaderSize = headerSize;
        u32 crc = nlChecksum32(localDataInfo2.pHeaderData, headerSize);
        m_IconCRC = crc;
        gIconCRC = m_IconCRC;
        void* headerData = gIconDataCache.mIconDataInfo.pHeaderData;
        cb2 = &SaveCallbacks::FileWriteIconCB;
        union
        {
            MemCardFunctor functor;
        };
        new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb2, headerData);
        long writeResult = g_MemCards[Slot]->WriteFileIconData(m_pSaveFile, gIconDataCache.mIconDataInfo.pHeaderData, functor);
        if (writeResult != 0)
        {
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
                int numBlocks2;
                MemCard* card4;
                unsigned long slotOffset = Slot << 2;
                card4 = g_MemCards[slotOffset >> 2];
                int ds2 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
                numBlocks2 = 0;
                ds2 += 12;
                while (ds2 > 0)
                {
                    numBlocks2++;
                    ds2 -= 0x2000;
                }
                MemCard::ICON_CONFIG IconCfg2;
                ds2 = BuildDefaultIconHeaderSize(IconCfg2);
                while (ds2 > 0)
                {
                    numBlocks2++;
                    ds2 -= 0x2000;
                }
                unsigned long sectorSize2 = card4->m_CardInfo.SectorSize;
                unsigned long bytestosave2 = numBlocks2 * sectorSize2;
                unsigned long alignedSize2 = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave2);
                MemCard* mc2 = g_MemCards[slotOffset >> 2];
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
    }
    break;
    case -4:
    {
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
        cb = &SaveCallbacks::CreateFileCB;
        union
        {
            MemCardFunctor functor;
        };
        new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<SaveCallbacks>(this, cb);
        long createResult = g_MemCards[Slot]->CreateFile(MarioSoccerFileName, dataSize, &IconCfg, m_pSaveFile, functor);
        mRequiredMemoryCardID = 0;
        if (createResult != 0)
        {
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
                int numBlocks3;
                MemCard* card6;
                unsigned long slotOffset = Slot << 2;
                card6 = g_MemCards[slotOffset >> 2];
                int ds3 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
                numBlocks3 = 0;
                ds3 += 12;
                while (ds3 > 0)
                {
                    numBlocks3++;
                    ds3 -= 0x2000;
                }
                MemCard::ICON_CONFIG IconCfg3;
                ds3 = BuildDefaultIconHeaderSize(IconCfg3);
                while (ds3 > 0)
                {
                    numBlocks3++;
                    ds3 -= 0x2000;
                }
                unsigned long sectorSize3 = card6->m_CardInfo.SectorSize;
                unsigned long bytestosave3 = numBlocks3 * sectorSize3;
                unsigned long alignedSize3 = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave3);
                MemCard* mc3 = g_MemCards[slotOffset >> 2];
                u8 hasSpace3;
                if (alignedSize3 > mc3->m_CardInfo.FreeBytes)
                    hasSpace3 = 0;
                else if (mc3->m_CardInfo.FreeFiles < 1)
                    hasSpace3 = 0;
                else
                    hasSpace3 = 1;
                if (hasSpace3 == 0)
                    errorCode = -9;
            }
            m_MustFreeMemory = true;
            g_Callback(errorCode);
            ResetTask::s_resetPaused = false;
            return -1;
        }
    }
    break;
    default:
    {
        long errorCode = Result;
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
            int numBlocks4;
            MemCard* card8;
            unsigned long slotOffset = Slot << 2;
            card8 = g_MemCards[slotOffset >> 2];
            int ds4 = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
            numBlocks4 = 0;
            ds4 += 12;
            while (ds4 > 0)
            {
                numBlocks4++;
                ds4 -= 0x2000;
            }
            MemCard::ICON_CONFIG IconCfg4;
            ds4 = BuildDefaultIconHeaderSize(IconCfg4);
            while (ds4 > 0)
            {
                numBlocks4++;
                ds4 -= 0x2000;
            }
            unsigned long sectorSize4 = card8->m_CardInfo.SectorSize;
            unsigned long bytestosave4 = numBlocks4 * sectorSize4;
            unsigned long alignedSize4 = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave4);
            MemCard* mc4 = g_MemCards[slotOffset >> 2];
            u8 hasSpace4;
            if (alignedSize4 > mc4->m_CardInfo.FreeBytes)
                hasSpace4 = 0;
            else if (mc4->m_CardInfo.FreeFiles < 1)
                hasSpace4 = 0;
            else
                hasSpace4 = 1;
            if (hasSpace4 == 0)
                errorCode = -9;
        }
        m_MustFreeMemory = true;
        g_Callback(errorCode);
        ResetTask::s_resetPaused = false;
        return -1;
    }
    break;
    }
    return 0;
}
#pragma pop

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

    union
    {
        MemCardFunctor functor;
    };
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
    union
    {
        MemCardFunctor functor;
    };
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

static inline void ConstructIconCfg(MemCard::ICON_CONFIG& IconCfg)
{
    IconCfg.BannerFormat = 0;
    IconCfg.IconCount = 0;
    IconCfg.IconFormat = 0;
    IconCfg.IconAnimType = 0;
    memset(IconCfg.IconSpeeds, 0, 8);
    memset(&IconCfg, 0, sizeof(MemCard::ICON_CONFIG));

    IconCfg.IconFormat = 2;
    IconCfg.IconSpeeds[0] = 3;
    IconCfg.IconCount = 1;
    IconCfg.BannerFormat = 2;

    u8 savedIconCount = IconCfg.IconCount;
    int bannerFmt = IconCfg.BannerFormat;
    s8 savedIconFmt = IconCfg.IconFormat;

    int iconDataSize = savedIconCount * (savedIconFmt << 10);
    int bannerSize = bannerFmt * 0xC00;
    int bannerClut = ((bannerFmt == 1) ? 0x200 : 0);
    int iconClut = ((savedIconFmt == 1) ? 0x200 : 0);
    int total = bannerClut + bannerSize;
    total += iconDataSize;
    total += iconClut;
    IconCfg.HeaderSize = total + 0x40;
}

/**
 * Offset/Address/Size: 0xBFC | 0x8018A558 | size: 0x28C
 * TODO: 97.73% match - remaining register diffs in icon header size calculations
 * and constructor store ordering.
 */
#pragma push
#pragma opt_propagation off
long SaveLoad::StartLoad(int Slot, void (*pCB)(long), bool PerformLoad, bool testOnly)
{
    union
    {
        MemCardFunctor functor;
    };
    typedef unsigned long (LoadCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    MemCard::ICON_CONFIG IconCfg;

    tDebugPrintManager::Print(DC_FE, "Starting memory card load\n");

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

        ConstructIconCfg(IconCfg);

        u32 allocSize = (IconCfg.HeaderSize + 0x1FF) & ~0x1FF;
        LoadSystem.m_pIconReadBuffer = nlMalloc(allocSize, 0x20, true);

        int iconFmt = IconCfg.IconFormat;
        int iconCnt = IconCfg.IconCount;
        int bannerFmt = IconCfg.BannerFormat;

        u32 headerSize = 0;
        headerSize += ((bannerFmt == 1) ? 0x200 : 0);
        headerSize += bannerFmt * 0xC00;
        headerSize += iconCnt * (iconFmt << 10);
        headerSize += ((iconFmt == 1) ? 0x200 : 0);
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

    union
    {
        MemCardFunctor functor;
    };
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

    union
    {
        MemCardFunctor functor;
    };
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

    union
    {
        MemCardFunctor functor;
    };
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

    union
    {
        MemCardFunctor functor;
    };
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
 * TODO: 96.27% match - icon header mask arithmetic still differs in
 * temporary registers and instruction count.
 */
#pragma push
#pragma opt_propagation off
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
        MemCard* card;
        int numBlocks;
        long dataSize;
        unsigned long slotOffset = channel << 2;
        card = g_MemCards[slotOffset >> 2];
        dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
        numBlocks = 0;

        dataSize += 12;
        while (dataSize > 0)
        {
            numBlocks++;
            dataSize -= 0x2000;
        }

        MemCard::ICON_CONFIG IconCfg;
        dataSize = BuildDefaultIconHeaderSize(IconCfg);
        while (dataSize > 0)
        {
            numBlocks++;
            dataSize -= 0x2000;
        }

        unsigned long sectorSize = card->m_CardInfo.SectorSize;
        unsigned long bytestosave = numBlocks * sectorSize;
        unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
        MemCard* mc = g_MemCards[slotOffset >> 2];

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
#pragma pop

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

    union
    {
        MemCardFunctor functor;
    };
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

    union
    {
        MemCardFunctor functor;
    };
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
 * TODO: 92.53% match - inlined icon header still uses different registers and one
 * fewer CLUT mask shift.
 */
#pragma push
#pragma opt_propagation off
int SaveLoad::GetSaveBlockSize(int)
{
    int dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    int numBlocks = 0;

    dataSize += 12;
    while (dataSize > 0)
    {
        numBlocks++;
        dataSize -= 0x2000;
    }

    MemCard::ICON_CONFIG IconCfg;
    dataSize = BuildDefaultIconHeaderSize(IconCfg);
    while (dataSize > 0)
    {
        numBlocks++;
        dataSize -= 0x2000;
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
 */
#pragma push
#pragma opt_propagation off
u8 SaveLoad::HasEnoughFreeSpace(int Slot)
{
    MemCard* card;
    int numBlocks;
    long dataSize;
    unsigned long slotOffset = Slot << 2;

    card = g_MemCards[slotOffset >> 2];
    dataSize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    numBlocks = 0;

    dataSize += 12;
    while (dataSize > 0)
    {
        numBlocks++;
        dataSize -= 0x2000;
    }

    MemCard::ICON_CONFIG IconCfg;
    dataSize = BuildDefaultIconHeaderSize(IconCfg);
    while (dataSize > 0)
    {
        numBlocks++;
        dataSize -= 0x2000;
    }

    unsigned long sectorSize = card->m_CardInfo.SectorSize;
    unsigned long bytestosave = numBlocks * sectorSize;
    unsigned long alignedSize = g_MemCards[slotOffset >> 2]->AlignBytesToSectorSize(bytestosave);
    MemCard* mc = g_MemCards[slotOffset >> 2];
    if (alignedSize > (unsigned long)mc->m_CardInfo.FreeBytes)
        return 0;

    if (mc->m_CardInfo.FreeFiles < 1)
        return 0;

    return 1;
}
#pragma pop

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
