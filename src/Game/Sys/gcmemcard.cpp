#include "Game/Sys/gcmemcard.h"
#include "NL/nlString.h"
#include "NL/nlAlgorithm.h"
#include "NL/nlMemory.h"
#include <dolphin/dvd.h>

extern "C" void* memset(void*, int, unsigned long);

MemCard::MemCard(unsigned long slot)
    : m_State(IS_IDLE)
    , m_Slot(slot)
    , m_CardState(CS_IDLE)
    , m_LastTransferSize(0)
    , m_SerialID(0)
{
    DVDDiskID* id = DVDGetCurrentDiskID();
    memcpy(&m_GameId, id, 4);
    memcpy(&m_CompanyId, ((char*)id) + 4, 2);
}

static MemCard* MemCards[2] = { new (8, false) MemCard(0), new (8, false) MemCard(1) };
MemCard** g_MemCards = MemCards;
bool MemCard::s_InitDone;

// /**
//  * Offset/Address/Size: 0x30 | 0x801CB798 | size: 0x24
//  */
// void nlStaticSortedSlot<MemCard::MC_FILE, 16>::GetNewEntry()
// {
// }

// /**
//  * Offset/Address/Size: 0x20 | 0x801CB788 | size: 0x10
//  */
// void nlStaticSortedSlot<MemCard::MC_FILE, 16>::FreeEntry(MemCard::MC_FILE*)
// {
// }

// /**
//  * Offset/Address/Size: 0x14 | 0x801CB77C | size: 0xC
//  */
// void nlStaticSortedSlot<MemCard::MC_FILE, 16>::FreeLookup()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801CB768 | size: 0x14
//  */
// void nlStaticSortedSlot<MemCard::MC_FILE, 16>::ExpandLookup()
// {
// }

// /**
//  * Offset/Address/Size: 0x8C | 0x801CB4DC | size: 0x28C
//  */
// void 0x801CB768..0x801CB7BC | size: 0x54
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801CB450 | size: 0x8C
//  */
// void nlBSearch<nlSortedSlot<MemCard::MC_FILE, 16>::nlSortedSlot<MemCard::MC_FILE, 16>::EntryLookup<MemCard::MC_FILE>, unsigned long>(const unsigned long&, nlSortedSlot<MemCard::MC_FILE, 16>::nlSortedSlot<MemCard::MC_FILE, 16>::EntryLookup<MemCard::MC_FILE>*, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x904 | 0x801CB444 | size: 0xC
//  */
// MemCardFunctor::MemCardFunctor() - moved to header as inline
// {
// }

static inline void DoCardRemovedCleanup(long channel)
{
    unsigned long i = 0;
    MemCard* card = g_MemCards[channel];
    unsigned long lookupOffset = i;

    card->m_State = IS_IDLE;
    card->m_CardState = CS_IDLE;
    card->m_LastTransferSize = 0;

    while (i < card->m_OpenFiles.m_EntryCount)
    {
        card->m_OpenFiles.FreeEntry(card->m_OpenFiles.m_pEntryLookup[i].pEntry);
        lookupOffset += 8;
        i++;
    }

    card->m_OpenFiles.FreeLookup();
    card->m_OpenFiles.m_EntryCount = 0;

    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[0];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, -3);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

inline void MemCard::SetStatusDone(long Result)
{
    if (Result == 0)
    {
        m_State = IS_MOUNTED;
        long Result = InternalWriteFile(m_pFileCB, m_pDataCB, m_pFileCB->TotalHeaderSize, 0, m_CB[8], false);
        if (Result != 0)
        {
            m_State = IS_MOUNTED;
            m_CardState = CS_MOUNTED;
        }
    }

    if (Result != 0)
    {
        MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&m_CB[8];
        unsigned long slot = m_Slot;
        if (*(long*)pFunctor != 0)
        {
            pFunctor->Call(slot, Result);
        }
        else
        {
            nlPrintf("Trying to call unset MC functor");
        }
    }
}

/**
 * Offset/Address/Size: 0x824 | 0x801CB364 | size: 0xE0
 */
void MemCard::CardRemovedCB(long channel, long result)
{
    DoCardRemovedCleanup(channel);
}

/**
 * Offset/Address/Size: 0x6E0 | 0x801CB220 | size: 0x144
 */
void MemCard::MountDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    CARDGetSerialNo(card->m_Slot, (u64*)&card->m_SerialID);

    switch (result)
    {
    case -6: // CARD_RESULT_BROKEN
        card->m_State = IS_CARDCHECK;
        result = CARDCheckAsync(card->m_Slot, CardCheckBrokenDoneCB);
        if (result != 0)
        {
            card->m_State = IS_MOUNTED_ERROR;
            card->m_CardState = CS_MOUNTED_ERROR;
        }
        break;
    case 0: // CARD_RESULT_READY
        card->m_State = IS_CARDCHECK;
        CARDFreeBlocks(card->m_Slot, &card->m_CardInfo.FreeBytes, &card->m_CardInfo.FreeFiles);
        result = CARDCheckAsync(card->m_Slot, CardCheckDoneCB);
        if (result != 0)
        {
            card->m_State = IS_MOUNTED_ERROR;
            card->m_CardState = CS_MOUNTED_ERROR;
        }
        break;
    case -13: // CARD_RESULT_ENCODING
        card->m_State = IS_MOUNTED_ERROR;
        card->m_CardState = CS_MOUNTED_ERROR;
        break;
    default:
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        break;
    }

    if (result != 0)
    {
        MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[1];
        unsigned long slot = card->m_Slot;
        if (*(long*)pFunctor != 0)
        {
            pFunctor->Call(slot, result);
        }
        else
        {
            nlPrintf("Trying to call unset MC functor");
        }
    }
}

/**
 * Offset/Address/Size: 0x540 | 0x801CB080 | size: 0x1A0
 */
void MemCard::CreateFileDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    CARDFreeBlocks(card->m_Slot, &card->m_CardInfo.FreeBytes, &card->m_CardInfo.FreeFiles);

    if (result != 0)
    {
        MC_FILE* pFile = card->m_pFileCB;
        if (pFile != NULL)
        {
            card->m_OpenFiles.DeleteEntry(pFile);
        }
        card->m_pFileCB = NULL;
    }

    card->m_State = IS_MOUNTED;
    card->m_CardState = CS_MOUNTED;
    if (CARDProbeEx(card->m_Slot, &card->m_CardInfo.CardSize, &card->m_CardInfo.SectorSize) != 0)
    {
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
    }
    CARDFreeBlocks(card->m_Slot, &card->m_CardInfo.FreeBytes, &card->m_CardInfo.FreeFiles);
    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[5];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, result);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

/**
 * Offset/Address/Size: 0x498 | 0x801CAFD8 | size: 0xA8
 */
void MemCard::FormatDoneCB(long channel, long result)
{
    long ch = (long)channel;
    MemCard* card = g_MemCards[ch];
    if (result == 0L)
    {
        CARDFreeBlocks(card->m_Slot, &card->m_CardInfo.FreeBytes, &card->m_CardInfo.FreeFiles);
        card->m_State = IS_MOUNTED;
        card->m_CardState = CS_MOUNTED;
    }
    else
    {
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
    }
    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[4];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, result);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

/**
 * Offset/Address/Size: 0x3D0 | 0x801CAF10 | size: 0xC8
 */
void MemCard::DeleteFileDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    CARDFreeBlocks(card->m_Slot, &card->m_CardInfo.FreeBytes, &card->m_CardInfo.FreeFiles);
    card->m_State = IS_MOUNTED;
    card->m_CardState = CS_MOUNTED;
    if (CARDProbeEx(card->m_Slot, &card->m_CardInfo.CardSize, &card->m_CardInfo.SectorSize) != 0)
    {
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
    }
    CARDFreeBlocks(card->m_Slot, &card->m_CardInfo.FreeBytes, &card->m_CardInfo.FreeFiles);
    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[6];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, result);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

/**
 * Offset/Address/Size: 0x328 | 0x801CAE68 | size: 0xA8
 */
void MemCard::ReadFileDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    card->m_State = IS_MOUNTED;
    card->m_CardState = CS_MOUNTED;
    if (CARDProbeEx(card->m_Slot, &card->m_CardInfo.CardSize, &card->m_CardInfo.SectorSize) != 0)
    {
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
    }
    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[7];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, result);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

/**
 * Offset/Address/Size: 0x12C | 0x801CAC6C | size: 0xC4
 */
void MemCard::CardCheckBrokenDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    if (result == 0)
    {
        CARDFreeBlocks(card->m_Slot, &card->m_CardInfo.FreeBytes, &card->m_CardInfo.FreeFiles);
    }
    else
    {
        card->m_State = IS_MOUNTED_ERROR;
        card->m_CardState = CS_MOUNTED_ERROR;
    }
    if (result == 0)
    {
        card->m_State = IS_MOUNTED;
        card->m_CardState = CS_MOUNTED;
    }
    else
    {
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
    }
    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[1];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, result);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

/**
 * Offset/Address/Size: 0xA8 | 0x801CABE8 | size: 0x84
 */
void MemCard::CardCheckDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    if (result == 0)
    {
        card->m_State = IS_MOUNTED;
        card->m_CardState = CS_MOUNTED;
    }
    else
    {
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
    }
    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[1];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, result);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801CAB40 | size: 0xA8
 */
void MemCard::WriteFileDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    card->m_State = IS_MOUNTED;
    card->m_CardState = CS_MOUNTED;
    if (CARDProbeEx(card->m_Slot, &card->m_CardInfo.CardSize, &card->m_CardInfo.SectorSize) != 0)
    {
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
    }
    MemCardFunctor::MCInternalFunctorBase* pFunctor = (MemCardFunctor::MCInternalFunctorBase*)&card->m_CB[8];
    unsigned long slot = card->m_Slot;
    if (*(long*)pFunctor != 0)
    {
        pFunctor->Call(slot, result);
    }
    else
    {
        nlPrintf("Trying to call unset MC functor");
    }
}

/**
 * Offset/Address/Size: 0x12DC | 0x801CAA4C | size: 0xF4
 */
s32 MemCard::BeginCardAccess(const MemCardFunctor& Callback)
{
    if (m_State != IS_IDLE)
    {
        return -100;
    }

    m_CB[1] = Callback;

    s32 result = CARDProbeEx(m_Slot, &m_CardInfo.CardSize, &m_CardInfo.SectorSize);
    if (result != 0)
    {
        m_State = IS_IDLE;
        m_CardState = CS_IDLE;
    }

    if (result != 0)
    {
        return result;
    }

    m_LastTransferSize = 0;
    m_TargetTransferSize = CARD_WORKAREA_SIZE;

    m_State = IS_MOUNTING;
    m_CardState = CS_MOUNTING;

    u8* workArea = m_CardWorkArea + CARD_SEG_SIZE - 1;
    workArea = (u8*)(((u32)workArea) & ~(CARD_SEG_SIZE - 1));

    result = CARDMountAsync(m_Slot, workArea, CardRemovedCB, MountDoneCB);
    if (result != 0)
    {
        m_State = IS_IDLE;
        m_CardState = CS_IDLE;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x10 | 0x801C9780 | size: 0x14
 */
unsigned long MemCard::AlignBytesToSectorSize(unsigned long bytes)
{
    unsigned long sectorSize = m_CardInfo.SectorSize;
    unsigned long mask = sectorSize - 1;
    return (bytes + mask) & ~mask;
}

/**
 * Offset/Address/Size: 0xF28 | 0x801CA698 | size: 0x3B4
 */
long MemCard::CreateFile(const char* FileName, unsigned long FileSize, MemCard::ICON_CONFIG* pIconConfig, MemCard::MC_FILE*& pFile, const MemCardFunctor& Callback)
{
    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    if (m_CardInfo.FreeFiles <= 0)
    {
        return -8;
    }

    if (pIconConfig == NULL)
    {
        pIconConfig = (ICON_CONFIG*)__alloca(sizeof(ICON_CONFIG));
        if (pIconConfig != NULL)
        {
            pIconConfig->BannerFormat = 0;
            pIconConfig->IconCount = 0;
            pIconConfig->IconFormat = 0;
            pIconConfig->IconAnimType = 0;
            memset(pIconConfig->IconSpeeds, 0, 8);
        }
    }

    m_CB[5] = Callback;

    unsigned long hash = nlStringHash(FileName);
    MC_FILE* pNewFile = m_OpenFiles.AddEntry(hash);

    m_pFileCB = pNewFile;
    m_pFileCB->FileInfo.fileNo = -1;

    pFile = m_pFileCB;

    s8 iconFmt = pIconConfig->IconFormat;
    int iconPixels = iconFmt << 10;
    int bannerFmt = pIconConfig->BannerFormat;

    int bannerHeader = 0;
    bannerHeader += ((bannerFmt == 1) ? 0x200 : 0);
    bannerHeader += bannerFmt * 0xC00;
    u32 iconClut = ((iconFmt == 1) ? 0x200 : 0);
    int headerSize = bannerHeader + (pIconConfig->IconCount * iconPixels);
    headerSize = headerSize + iconClut;
    pIconConfig->HeaderSize = headerSize + 0x40;

    MC_FILE* mcFile = m_pFileCB;
    mcFile->IconCfg = *pIconConfig;

    m_pFileCB->TotalHeaderSize = AlignBytesToSectorSize(pIconConfig->HeaderSize);

    unsigned long totalSize = m_pFileCB->TotalHeaderSize + AlignBytesToSectorSize(FileSize);

    m_LastTransferSize = CARDGetXferredBytes(m_Slot);
    m_TargetTransferSize = 0x4000;

    m_State = IS_CREATING;
    m_CardState = CS_CREATING;

    long result = CARDCreateAsync(m_Slot, FileName, totalSize, (CARDFileInfo*)m_pFileCB, CreateFileDoneCB);

    if (result != 0)
    {
        unsigned long searchKey = hash;
        nlSortedSlot<MemCard::MC_FILE, 16>::EntryLookup<MemCard::MC_FILE>* pFound;

        if (m_OpenFiles.m_EntryCount != 0)
        {
            pFound = nlBSearch(searchKey, m_OpenFiles.m_pEntryLookup, m_OpenFiles.m_EntryCount);
        }
        else
        {
            pFound = NULL;
        }

        if (pFound != NULL)
        {
            m_OpenFiles.DeleteEntry(pFound);
        }

        pFile = NULL;
        m_State = IS_MOUNTED;
        m_CardState = CS_MOUNTED;
    }

    return result;
}

/**
 * Offset/Address/Size: 0xBE8 | 0x801CA358 | size: 0x340
 */
extern "C" void* memset(void*, int, unsigned long);

long MemCard::OpenFile(const char* FileName, MemCard::MC_FILE*& pFile, unsigned long* pFileLength)
{
    long result;

    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    unsigned long hash = nlStringHash(FileName);
    MC_FILE* pNewFile = m_OpenFiles.AddEntry(hash);

    pFile = pNewFile;
    pFile->FileInfo.fileNo = -1;
    memset(&pFile->IconCfg.IconSpeeds[0], 0, 8);

    result = CARDOpen(m_Slot, FileName, (CARDFileInfo*)pFile);
    if (result != 0)
    {
        unsigned long searchKey = hash;
        nlSortedSlot<MemCard::MC_FILE, 16>::EntryLookup<MemCard::MC_FILE>* pFound;

        if (m_OpenFiles.m_EntryCount != 0)
        {
            pFound = nlBSearch(searchKey, m_OpenFiles.m_pEntryLookup, m_OpenFiles.m_EntryCount);
        }
        else
        {
            pFound = NULL;
        }

        if (pFound != NULL)
        {
            m_OpenFiles.DeleteEntry(pFound);
        }
    }
    else
    {
        CARDStat stat;
        CARDGetStatus(m_Slot, pFile->FileInfo.fileNo, &stat);

        pFile->IconCfg.BannerFormat = CARDGetBannerFormat(&stat);
        pFile->IconCfg.IconFormat = CARDGetIconFormat(&stat, 0);
        pFile->IconCfg.IconAnimType = CARDGetIconAnim(&stat);
        pFile->IconCfg.IconCount = 0;

        while (pFile->IconCfg.IconCount < 8)
        {
            unsigned long IconSpeed =
                CARDGetIconSpeed(&stat, pFile->IconCfg.IconCount);
            pFile->IconCfg.IconSpeeds[pFile->IconCfg.IconCount] = IconSpeed;
            if (IconSpeed == 0)
            {
                break;
            }
            pFile->IconCfg.IconCount = pFile->IconCfg.IconCount + 1;
        }

        s8 iconFmt;
        MC_FILE* file = pFile;
        iconFmt = file->IconCfg.IconFormat;
        int iconPixels = iconFmt << 10;
        int bannerFmt = file->IconCfg.BannerFormat;

        int bannerHeader = 0;
        bannerHeader += ((bannerFmt == 1) ? 0x200 : 0);
        bannerHeader += bannerFmt * 0xC00;
        u32 iconClut = ((iconFmt == 1) ? 0x200 : 0);
        int headerSize = bannerHeader + (file->IconCfg.IconCount * iconPixels);
        headerSize = headerSize + iconClut;
        headerSize += 0x40;
        file->IconCfg.HeaderSize = headerSize;

        unsigned long sectorMask = m_CardInfo.SectorSize - 1;
        pFile->TotalHeaderSize = (pFile->IconCfg.HeaderSize + sectorMask) & ~sectorMask;

        if (pFileLength != NULL)
        {
            *pFileLength = stat.length - pFile->TotalHeaderSize;
        }
    }

    return result;
}

/**
 * Offset/Address/Size: 0xB28 | 0x801CA298 | size: 0xC0
 */
s32 MemCard::FormatCard(const MemCardFunctor& Callback)
{
    if (m_State != IS_MOUNTED && m_State != IS_MOUNTED_ERROR)
    {
        return -100;
    }

    m_CB[4] = Callback;

    m_State = IS_FORMATTING;
    m_CardState = CS_FORMATTING;
    m_LastTransferSize = CARDGetXferredBytes(m_Slot);
    m_TargetTransferSize = 0xA000;
    s32 Result = CARDFormatAsync(m_Slot, FormatDoneCB);

    if (Result != 0)
    {
        m_State = IS_IDLE;
        m_CardState = CS_IDLE;
    }

    return Result;
}

/**
 * Offset/Address/Size: 0x3060 | 0x801CA0A0 | size: 0x1F8
 */
long MemCard::DeleteFile(const char* FileName, const MemCardFunctor& Callback)
{
    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    m_CB[6] = Callback;

    unsigned long hash = nlStringHash(FileName);

    nlSortedSlot<MemCard::MC_FILE, 16>::EntryLookup<MemCard::MC_FILE>* result;
    if (m_OpenFiles.m_EntryCount != 0)
    {
        result = nlBSearch(hash, m_OpenFiles.m_pEntryLookup, m_OpenFiles.m_EntryCount);
    }
    else
    {
        result = NULL;
    }

    MC_FILE* file;
    if (result != NULL)
    {
        file = result->pEntry;
    }
    else
    {
        file = NULL;
    }

    if (file != NULL)
    {
        file->TotalHeaderSize = 0;
        if (CARDClose(&file->FileInfo) == 0 && file != NULL)
        {
            m_OpenFiles.DeleteEntry(file);
        }
    }

    m_State = IS_DELETING;
    m_CardState = CS_DELETING;
    m_LastTransferSize = CARDGetXferredBytes(m_Slot);
    m_TargetTransferSize = 0x4000;

    long Result = CARDDeleteAsync(m_Slot, FileName, DeleteFileDoneCB);
    if (Result != 0)
    {
        m_State = IS_MOUNTED;
        m_CardState = CS_MOUNTED;
    }

    return Result;
}

/**
 * Offset/Address/Size: 0x860 | 0x801C9FD0 | size: 0xD0
 */
long MemCard::InternalReadFile(MC_FILE* pFile, void* Buffer, unsigned long Length, unsigned long StartAt, const MemCardFunctor& Callback)
{
    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    m_CB[7] = Callback;

    m_State = IS_READING;
    m_CardState = CS_READING;

    m_LastTransferSize = CARDGetXferredBytes(m_Slot);
    m_TargetTransferSize = Length;

    long Result = CARDReadAsync((CARDFileInfo*)pFile, Buffer, (s32)Length, (s32)StartAt, ReadFileDoneCB);
    if (Result != 0)
    {
        m_State = IS_MOUNTED;
        m_CardState = CS_MOUNTED;
    }

    return Result;
}

/**
 * Offset/Address/Size: 0x740 | 0x801C9EB0 | size: 0x120
 */
long MemCard::InternalWriteFile(MC_FILE* pFile, void* Buffer, unsigned long Length, unsigned long StartAt, const MemCardFunctor& Callback, bool ResetTransfer)
{
    unsigned long LengthMisalign;
    long Result;

    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    LengthMisalign = Length % m_CardInfo.SectorSize;
    if (LengthMisalign != 0)
    {
        nlPrintf("MemCard::WriteFile - Length %d not a multiple of card sector size %d, increasing length to %d\n", Length, m_CardInfo.SectorSize, Length + m_CardInfo.SectorSize - LengthMisalign);
        Length += m_CardInfo.SectorSize - LengthMisalign;
    }

    m_CB[8] = Callback;
    m_State = IS_WRITING;
    m_CardState = CS_WRITING;

    if (ResetTransfer)
    {
        m_LastTransferSize = CARDGetXferredBytes(m_Slot);
        m_TargetTransferSize = Length + 0x2000;
    }

    Result = CARDWriteAsync((CARDFileInfo*)pFile, Buffer, (s32)Length, (s32)StartAt, WriteFileDoneCB);
    if (Result != 0)
    {
        m_State = IS_MOUNTED;
        m_CardState = CS_MOUNTED;
    }

    return Result;
}

/**
 * Offset/Address/Size: 0x620 | 0x801C9D90 | size: 0x120
 */
s32 MemCard::CloseFile(MC_FILE* pFile)
{
    pFile->TotalHeaderSize = 0;
    s32 result = CARDClose(&pFile->FileInfo);
    if (result == 0 && pFile != NULL)
    {
        m_OpenFiles.DeleteEntry(pFile);
    }

    return result;
}

/**
 * Offset/Address/Size: 0x5DC | 0x801C9D4C | size: 0x44
 */
s32 MemCard::FileExists(const char* fileName)
{
    CARDFileInfo fileInfo;
    s32 result = CARDOpen(m_Slot, fileName, &fileInfo);
    if (result == 0)
    {
        CARDClose(&fileInfo);
    }
    return result;
}

/**
 * Offset/Address/Size: 0x204 | 0x801C9974 | size: 0x3D8
 */
long MemCard::WriteFileIconData(MemCard::MC_FILE* pFile, void* pData, const MemCardFunctor& functor)
{
    CARDStat stat;
    ICON_DATA_INFO DataInfo;
    unsigned long Icon;

    CARDGetStatus(m_Slot, pFile->FileInfo.fileNo, &stat);

    unsigned char bannerFormat = pFile->IconCfg.BannerFormat;

    DataInfo.Comment1Offset = 0;
    DataInfo.Comment2Offset = 0x20;
    DataInfo.BannerOffset = 0x40;
    DataInfo.BannerCLUTOffset = DataInfo.BannerOffset + ((bannerFormat == 1) ? 0xC00 : 0);
    DataInfo.IconOffset[0] = ((bannerFormat == 1) ? 0x200 : 0) + bannerFormat * 0xC00 + DataInfo.BannerOffset;

    for (Icon = 1; Icon < pFile->IconCfg.IconCount; Icon++)
    {
        DataInfo.IconOffset[Icon] = DataInfo.IconOffset[Icon - 1] + ((s32)pFile->IconCfg.IconFormat << 10);
    }

    for (; Icon < 8; Icon++)
    {
        DataInfo.IconOffset[Icon] = DataInfo.IconOffset[pFile->IconCfg.IconCount - 1];
    }

    DataInfo.IconCLUTOffset = DataInfo.IconOffset[pFile->IconCfg.IconCount - 1] + ((s32)pFile->IconCfg.IconFormat << 10);

    CARDSetCommentAddress(&stat, DataInfo.Comment1Offset);
    CARDSetIconAddress(&stat, DataInfo.BannerOffset);
    CARDSetBannerFormat(&stat, pFile->IconCfg.BannerFormat);

    unsigned long i;
    for (i = 0; i < pFile->IconCfg.IconCount; i++)
    {
        CARDSetIconFormat(&stat, i, (s8)pFile->IconCfg.IconFormat);
        CARDSetIconSpeed(&stat, i, (s8)pFile->IconCfg.IconSpeeds[i]);
    }

    for (; i < 8; i++)
    {
        CARDSetIconFormat(&stat, i, 0);
        CARDSetIconSpeed(&stat, i, 0);
    }

    volatile unsigned char* bannerFmt = (volatile unsigned char*)&stat.bannerFormat;
    unsigned char fmt = *bannerFmt;
    *bannerFmt = (unsigned char)((fmt & ~CARD_STAT_ANIM_MASK) | pFile->IconCfg.IconAnimType);

    m_CB[8] = functor;

    m_State = IS_WRITINGSTATUS;
    m_CardState = CS_WRITING;
    m_LastTransferSize = CARDGetXferredBytes(m_Slot);
    m_TargetTransferSize = pFile->TotalHeaderSize + 0x4000;

    s32 result = CARDSetStatusAsync(m_Slot, pFile->FileInfo.fileNo, &stat, SetStatusDoneCB);

    if (result != 0)
    {
        m_State = IS_MOUNTED;
        m_CardState = CS_MOUNTED;
    }
    else
    {
        m_pFileCB = pFile;
        m_pDataCB = pData;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x24 | 0x801C9794 | size: 0x1E0
 */
void MemCard::ICON_CONFIG::GetValidDataInfo(MemCard::ICON_DATA_INFO& DataInfo) const
{
    unsigned long Icon;

    DataInfo.Comment1Offset = 0;
    DataInfo.Comment2Offset = 0x20;
    DataInfo.BannerOffset = 0x40;
    DataInfo.BannerCLUTOffset = DataInfo.BannerOffset + ((BannerFormat == 1) ? 0xC00 : 0);
    DataInfo.IconOffset[0] = ((BannerFormat == 1) ? 0x200 : 0) + BannerFormat * 0xC00 + DataInfo.BannerOffset;

    for (Icon = 1; Icon < (unsigned long)IconCount; Icon++)
    {
        DataInfo.IconOffset[Icon] = DataInfo.IconOffset[Icon - 1] + ((s32)IconFormat << 10);
    }

    for (; Icon < 8; Icon++)
    {
        DataInfo.IconOffset[Icon] = DataInfo.IconOffset[IconCount - 1];
    }

    DataInfo.IconCLUTOffset = DataInfo.IconOffset[IconCount - 1] + ((s32)IconFormat << 10);
}

/**
 * Offset/Address/Size: 0x0 | 0x801C9770 | size: 0x10
 */
s64 MemCard::GetSerialID() const
{
    return m_SerialID;
}
