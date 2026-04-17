#include "Game/Sys/gcmemcard.h"
#include "NL/nlString.h"
#include "NL/nlBSearch.h"

extern "C" void* memset(void*, int, unsigned long);

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
// void nlBSearch<nlSortedSlot<MemCard::MC_FILE, 16>::EntryLookup<MemCard::MC_FILE>, unsigned long>(const unsigned long&, nlSortedSlot<MemCard::MC_FILE, 16>::EntryLookup<MemCard::MC_FILE>*, int)
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

static inline EntryLookup<MemCard::MC_FILE>* FindCreateFileLookup(MemCard* self, MemCard::MC_FILE* pFile)
{
    for (long i = 0; (unsigned long)i < self->m_OpenFiles.m_EntryCount; i++)
    {
        if (self->m_OpenFiles.m_pEntryLookup[i].pEntry == pFile)
        {
            return &self->m_OpenFiles.m_pEntryLookup[i];
        }
    }
    return NULL;
}

static inline void ShiftCreateFileLookup(MemCard* self, EntryLookup<MemCard::MC_FILE>* pFoundEntry)
{
    unsigned long total = self->m_OpenFiles.m_EntryCount;
    long idx = (pFoundEntry - self->m_OpenFiles.m_pEntryLookup);

    while ((unsigned long)idx != total)
    {
        long next = idx + 1;
        EntryLookup<MemCard::MC_FILE>* src = &self->m_OpenFiles.m_pEntryLookup[next];
        EntryLookup<MemCard::MC_FILE>* dst = &self->m_OpenFiles.m_pEntryLookup[idx];
        idx = next;
        unsigned long id = src->Id;
        MemCard::MC_FILE* entry = src->pEntry;
        dst->pEntry = entry;
        dst->Id = id;
    }

    self->m_OpenFiles.m_EntryCount = self->m_OpenFiles.m_EntryCount - 1;
}

/**
 * Offset/Address/Size: 0x540 | 0x801CB080 | size: 0x1A0
 * TODO: 99.66% match - entry copy loop in ShiftCreateFileLookup keeps
 * r3/r4/r0 instead of target r4/r3/r5.
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
            EntryLookup<MC_FILE>* pFoundEntry = FindCreateFileLookup(card, pFile);
            card->m_OpenFiles.FreeEntry(pFile);
            ShiftCreateFileLookup(card, pFoundEntry);
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
 * Offset/Address/Size: 0x1F0 | 0x801CAD30 | size: 0x138
 * TODO: 99.62% match - result/file register allocation still swapped
 * (target r29/r30 vs current r30/r29), and remaining rodata label mismatch on
 * the two nlPrintf format strings.
 */
void MemCard::SetStatusDoneCB(long channel, long result)
{
    MemCard* card = g_MemCards[channel];
    MC_FILE* file;
    unsigned long remainder;
    s32 err = 0;

    if (result == 0)
    {
        card->m_State = IS_MOUNTED;
        file = card->m_pFileCB;
        void* data = card->m_pDataCB;
        unsigned long headerSize = file->TotalHeaderSize;

        if (card->m_State != IS_MOUNTED)
        {
            err = -100;
        }
        else
        {
            unsigned long sectorSize = card->m_CardInfo.SectorSize;
            remainder = headerSize % sectorSize;
            if (remainder != 0)
            {
                nlPrintf("MC: Header size (%d) not aligned to sector (%d), rounding to %d\n", headerSize, sectorSize, headerSize + sectorSize - remainder);
                headerSize += card->m_CardInfo.SectorSize - remainder;
            }
            card->m_State = IS_WRITING;
            card->m_CardState = CS_WRITING;
            err = CARDWriteAsync(&file->FileInfo, data, headerSize, 0, WriteFileDoneCB);
            if (err != 0)
            {
                card->m_State = IS_MOUNTED;
                card->m_CardState = CS_MOUNTED;
            }
        }
        if (err != 0)
        {
            card->m_State = IS_MOUNTED;
            card->m_CardState = CS_MOUNTED;
        }
    }
    if (result != 0)
    {
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

    struct FunctorWords
    {
        unsigned long w0;
        unsigned long w1;
        unsigned long w2;
        unsigned long w3;
        unsigned long w4;
        unsigned long w5;
    };

    volatile FunctorWords* dst = (volatile FunctorWords*)&m_CB[1];
    const volatile FunctorWords* src = (const volatile FunctorWords*)&Callback;
    unsigned long b;
    unsigned long a;
    unsigned long e;
    unsigned long d;
    unsigned long c;

    a = src->w0;
    b = src->w1;
    dst->w0 = a;
    dst->w1 = b;

    d = src->w2;
    e = src->w3;
    dst->w2 = d;
    dst->w3 = e;

    b = src->w4;
    a = src->w5;
    dst->w4 = b;
    dst->w5 = a;

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
 * Offset/Address/Size: 0xF28 | 0x801CA698 | size: 0x3B4
 * TODO: 91.88% match - 13 register diffs in shift-up loop. MWCC reallocates
 * high=low+1 from r6 to r4 after binary search, cascading all loop body
 * registers. Same MWCC re-homing quirk as OpenFile's shift loop.
 */
long MemCard::CreateFile(const char* FileName, unsigned long FileSize, MemCard::ICON_CONFIG* pIconConfig, MemCard::MC_FILE*& pFile, const MemCardFunctor& Callback)
{
    EntryLookup<MC_FILE>* nlBSearch(const unsigned long&, EntryLookup<MC_FILE>*, int);

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

    struct FunctorWords
    {
        unsigned long w0;
        unsigned long w1;
        unsigned long w2;
        unsigned long w3;
        unsigned long w4;
        unsigned long w5;
    };

    volatile FunctorWords* fdst = (volatile FunctorWords*)&m_CB[5];
    const volatile FunctorWords* fsrc = (const volatile FunctorWords*)&Callback;
    unsigned long b;
    unsigned long a;
    unsigned long e;
    unsigned long d;
    unsigned long c;
    unsigned long f;

    a = fsrc->w0;
    b = fsrc->w1;
    fdst->w0 = a;
    fdst->w1 = b;

    d = fsrc->w2;
    e = fsrc->w3;
    fdst->w2 = d;
    fdst->w3 = e;

    f = fsrc->w4;
    c = fsrc->w5;
    fdst->w4 = f;
    fdst->w5 = c;

    unsigned long hash = nlStringHash(FileName);
    MC_FILE* pNewFile = m_OpenFiles.GetNewEntry();

    if (m_OpenFiles.m_EntryCount == m_OpenFiles.m_LookupAllocated)
    {
        m_OpenFiles.ExpandLookup();
    }

    long middle;
    long low = -1;
    long high;
    unsigned long count = m_OpenFiles.m_EntryCount;
    high = count;

    while ((high - low) > 1)
    {
        middle = (high + low) >> 1;
        if (m_OpenFiles.m_pEntryLookup[middle].Id > hash)
        {
            high = middle;
        }
        else
        {
            low = middle;
        }
    }

    high = low + 1;
    while (count != (unsigned long)high)
    {
        unsigned long prev = count - 1;
        EntryLookup<MC_FILE>* lookup = m_OpenFiles.m_pEntryLookup;
        EntryLookup<MC_FILE>* src2 = &lookup[prev];
        EntryLookup<MC_FILE>* dst2 = &lookup[count];
        count = prev;
        unsigned long id = src2->Id;
        MC_FILE* entry = src2->pEntry;
        dst2->pEntry = entry;
        dst2->Id = id;
    }

    m_OpenFiles.m_pEntryLookup[high].Id = hash;
    m_OpenFiles.m_pEntryLookup[high].pEntry = pNewFile;
    m_OpenFiles.m_EntryCount = m_OpenFiles.m_EntryCount + 1;

    m_pFileCB = pNewFile;
    m_pFileCB->FileInfo.fileNo = -1;

    pFile = m_pFileCB;

    pIconConfig->HeaderSize = pIconConfig->BannerFormat * 0xC00
                            + ((pIconConfig->BannerFormat == 1) ? 0x200 : 0)
                            + (pIconConfig->IconCount * ((s8)pIconConfig->IconFormat << 10))
                            + (((s8)pIconConfig->IconFormat == 1) ? 0x200 : 0)
                            + 0x40;

    MC_FILE* mcFile = m_pFileCB;
    mcFile->IconCfg.BannerFormat = pIconConfig->BannerFormat;
    mcFile->IconCfg.IconCount = pIconConfig->IconCount;
    mcFile->IconCfg.IconFormat = pIconConfig->IconFormat;
    mcFile->IconCfg.IconAnimType = pIconConfig->IconAnimType;
    *(unsigned long*)&mcFile->IconCfg.IconSpeeds[0] = *(unsigned long*)&pIconConfig->IconSpeeds[0];
    *(unsigned long*)&mcFile->IconCfg.IconSpeeds[4] = *(unsigned long*)&pIconConfig->IconSpeeds[4];
    mcFile->IconCfg.HeaderSize = pIconConfig->HeaderSize;

    mcFile->TotalHeaderSize = (pIconConfig->HeaderSize + (m_CardInfo.SectorSize - 1)) & ~(m_CardInfo.SectorSize - 1);

    unsigned long totalSize = mcFile->TotalHeaderSize + ((FileSize + (m_CardInfo.SectorSize - 1)) & ~(m_CardInfo.SectorSize - 1));

    m_LastTransferSize = CARDGetXferredBytes(m_Slot);
    m_TargetTransferSize = 0x4000;

    m_State = IS_CREATING;
    m_CardState = CS_CREATING;

    long result = CARDCreateAsync(m_Slot, FileName, totalSize, (CARDFileInfo*)m_pFileCB, CreateFileDoneCB);

    if (result != 0)
    {
        unsigned long searchKey = hash;
        EntryLookup<MC_FILE>* pFound;

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
            m_OpenFiles.FreeEntry(pFound->pEntry);

            long idx = pFound - m_OpenFiles.m_pEntryLookup;
            unsigned long total = m_OpenFiles.m_EntryCount;

            while ((unsigned long)idx != total)
            {
                long next = idx + 1;
                EntryLookup<MC_FILE>* lookup = m_OpenFiles.m_pEntryLookup;
                EntryLookup<MC_FILE>* srcE = &lookup[next];
                EntryLookup<MC_FILE>* dstE = &lookup[idx];
                idx = next;
                unsigned long id = srcE->Id;
                MC_FILE* entry = srcE->pEntry;
                dstE->pEntry = entry;
                dstE->Id = id;
            }

            m_OpenFiles.m_EntryCount = m_OpenFiles.m_EntryCount - 1;
        }

        pFile = NULL;
        m_State = IS_MOUNTED;
        m_CardState = CS_MOUNTED;
    }

    return result;
}

/**
 * Offset/Address/Size: 0xBE8 | 0x801CA358 | size: 0x340
 * TODO: 95.96% match - shift-down loop insert variable gets r4 instead of target r6
 * (MWCC register re-homing), nlBSearch mangled name diff (nested vs standalone EntryLookup type).
 */
extern "C" void* memset(void*, int, unsigned long);

long MemCard::OpenFile(const char* FileName, MemCard::MC_FILE*& pFile, unsigned long* pFileLength)
{
    EntryLookup<MC_FILE>* nlBSearch(const unsigned long&, EntryLookup<MC_FILE>*, int);

    long result;

    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    unsigned long hash = nlStringHash(FileName);
    MC_FILE* pNewFile = m_OpenFiles.GetNewEntry();

    if (m_OpenFiles.m_EntryCount == m_OpenFiles.m_LookupAllocated)
    {
        m_OpenFiles.ExpandLookup();
    }

    long middle;
    long low = -1;
    long high;
    unsigned long count = m_OpenFiles.m_EntryCount;
    high = count;

    while ((high - low) > 1)
    {
        middle = (high + low) >> 1;
        if (m_OpenFiles.m_pEntryLookup[middle].Id > hash)
        {
            high = middle;
        }
        else
        {
            low = middle;
        }
    }

    high = low + 1;
    while (count != (unsigned long)high)
    {
        unsigned long prev = count - 1;
        EntryLookup<MC_FILE>* lookup = m_OpenFiles.m_pEntryLookup;
        EntryLookup<MC_FILE>* src = &lookup[prev];
        EntryLookup<MC_FILE>* dst = &lookup[count];
        count = prev;
        unsigned long id = src->Id;
        MC_FILE* entry = src->pEntry;
        dst->pEntry = entry;
        dst->Id = id;
    }

    m_OpenFiles.m_pEntryLookup[high].Id = hash;
    m_OpenFiles.m_pEntryLookup[high].pEntry = pNewFile;
    m_OpenFiles.m_EntryCount = m_OpenFiles.m_EntryCount + 1;

    pFile = pNewFile;
    pFile->FileInfo.fileNo = -1;
    memset(&pFile->IconCfg.IconSpeeds[0], 0, 8);

    result = CARDOpen(m_Slot, FileName, (CARDFileInfo*)pFile);
    if (result != 0)
    {
        unsigned long searchKey = hash;
        EntryLookup<MC_FILE>* pFound;

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
            m_OpenFiles.FreeEntry(pFound->pEntry);

            long idx = pFound - m_OpenFiles.m_pEntryLookup;
            unsigned long total = m_OpenFiles.m_EntryCount;

            while ((unsigned long)idx != total)
            {
                long next = idx + 1;
                EntryLookup<MC_FILE>* lookup = m_OpenFiles.m_pEntryLookup;
                EntryLookup<MC_FILE>* src = &lookup[next];
                EntryLookup<MC_FILE>* dst = &lookup[idx];
                idx = next;
                unsigned long id = src->Id;
                MC_FILE* entry = src->pEntry;
                dst->pEntry = entry;
                dst->Id = id;
            }

            m_OpenFiles.m_EntryCount = m_OpenFiles.m_EntryCount - 1;
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
            unsigned long i = pFile->IconCfg.IconCount;
            unsigned long speed = CARDGetIconSpeed(&stat, i);
            pFile->IconCfg.IconSpeeds[i] = speed;
            if (speed == 0)
            {
                break;
            }
            pFile->IconCfg.IconCount = pFile->IconCfg.IconCount + 1;
        }

        pFile->IconCfg.HeaderSize = pFile->IconCfg.BannerFormat * 0xC00 + ((pFile->IconCfg.BannerFormat == CARD_STAT_BANNER_C8) ? 0x200 : 0) + (pFile->IconCfg.IconCount * ((s8)pFile->IconCfg.IconFormat << 10)) + (((s8)pFile->IconCfg.IconFormat == CARD_STAT_ICON_C8) ? 0x200 : 0) + 0x40;

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

    struct FunctorWords
    {
        unsigned long w0;
        unsigned long w1;
        unsigned long w2;
        unsigned long w3;
        unsigned long w4;
        unsigned long w5;
    };

    volatile FunctorWords* dst = (volatile FunctorWords*)&m_CB[4];
    const volatile FunctorWords* src = (const volatile FunctorWords*)&Callback;
    unsigned long b;
    unsigned long a;
    unsigned long e;
    unsigned long d;
    unsigned long c;

    a = src->w0;
    b = src->w1;
    dst->w0 = a;
    dst->w1 = b;

    d = src->w2;
    e = src->w3;
    dst->w2 = d;
    dst->w3 = e;

    b = src->w4;
    c = src->w5;
    dst->w4 = b;
    dst->w5 = c;

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
 * Offset/Address/Size: 0x930 | 0x801CA0A0 | size: 0x1F8
 * TODO: 96.2% match - 24-byte MemCardFunctor copy still has 6 scheduling/reg
 * diffs, nlBSearch call uses EntryLookup mangling variant, and shift loop keeps a
 * 4-instruction register permutation in entry copy.
 */
static inline EntryLookup<MemCard::MC_FILE>* FindOpenLookup(MemCard* self, MemCard::MC_FILE* file)
{
    long i = 0;
    long byteOff = i;
    while ((unsigned long)i < self->m_OpenFiles.m_EntryCount)
    {
        if (self->m_OpenFiles.m_pEntryLookup[i].pEntry == file)
        {
            return &self->m_OpenFiles.m_pEntryLookup[i];
        }
        byteOff += 8;
        i++;
    }
    return NULL;
}

static inline void ShiftOpenLookup(MemCard* self, EntryLookup<MemCard::MC_FILE>* foundEntry)
{
    s32 next;
    unsigned long total = self->m_OpenFiles.m_EntryCount;
    long idx = (foundEntry - self->m_OpenFiles.m_pEntryLookup);

    while ((unsigned long)idx != total)
    {
        next = idx + 1;
        EntryLookup<MemCard::MC_FILE>* src = &self->m_OpenFiles.m_pEntryLookup[next];
        EntryLookup<MemCard::MC_FILE>* dst = &self->m_OpenFiles.m_pEntryLookup[idx];
        idx = next;
        unsigned long id = src->Id;
        MemCard::MC_FILE* entry = src->pEntry;
        dst->pEntry = entry;
        dst->Id = id;
    }

    self->m_OpenFiles.m_EntryCount = self->m_OpenFiles.m_EntryCount - 1;
}

/**
 * Offset/Address/Size: 0x3060 | 0x801CA0A0 | size: 0x1F8
 * TODO: 96.15% match - struct copy scheduling (m_CB[6]=Callback) differs,
 *   ShiftOpenLookup r3/r4 register allocation swap, nlBSearch symbol name mismatch
 */
long MemCard::DeleteFile(const char* FileName, const MemCardFunctor& Callback)
{
    EntryLookup<MC_FILE>* nlBSearch(const unsigned long&, EntryLookup<MC_FILE>*, int);
    EntryLookup<MC_FILE>* foundEntry;

    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    m_CB[6] = Callback;

    unsigned long hash = nlStringHash(FileName);

    EntryLookup<MC_FILE>* result;
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
            foundEntry = FindOpenLookup(this, file);
            m_OpenFiles.FreeEntry(file);
            ShiftOpenLookup(this, foundEntry);
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
 * TODO: 91.7% match - MWCC struct copy scheduling: compiler pipelines loads
 * early in 24-byte MemCardFunctor copy (lwz before preceding stw), producing
 * 8 instruction scheduling diffs. Same issue affects InternalWriteFile.
 */
long MemCard::InternalReadFile(MC_FILE* pFile, void* Buffer, unsigned long Length, unsigned long StartAt, const MemCardFunctor& Callback)
{
    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    unsigned long* dst = (unsigned long*)&m_CB[7];
    const unsigned long* src = (const unsigned long*)&Callback;
    for (int i = 0; i < 6; i++)
    {
        dst[i] = src[i];
    }
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
    if (m_State != IS_MOUNTED)
    {
        return -100;
    }

    unsigned long sectorSize = m_CardInfo.SectorSize;
    unsigned long LengthMisalign = Length % sectorSize;
    if (LengthMisalign != 0)
    {
        nlPrintf("MemCard::InternalWriteFile - offset %d not sector aligned (%d), changed to %d\n", Length, sectorSize, Length + sectorSize - LengthMisalign);
        Length += m_CardInfo.SectorSize - LengthMisalign;
    }

    struct CopyData
    {
        unsigned long a, b, c, d, e, f;
    };
    *(CopyData*)&m_CB[8] = *(const CopyData*)&Callback;
    m_State = IS_WRITING;
    m_CardState = CS_WRITING;

    if (ResetTransfer)
    {
        m_LastTransferSize = CARDGetXferredBytes(m_Slot);
        m_TargetTransferSize = Length + 0x2000;
    }

    long Result = CARDWriteAsync((CARDFileInfo*)pFile, Buffer, (s32)Length, (s32)StartAt, WriteFileDoneCB);
    if (Result != 0)
    {
        m_State = IS_MOUNTED;
        m_CardState = CS_MOUNTED;
    }

    return Result;
}

/**
 * Offset/Address/Size: 0x620 | 0x801C9D90 | size: 0x120
 * TODO: 99.1% match - first loop keeps i/byteOff in r4/r3 instead of target r3/r4;
 * copy loop keeps slwi/add/lwz+stw on r0 where target keeps those on r3/r5.
 */
static inline EntryLookup<MemCard::MC_FILE>* FindOpenFileLookup(nlStaticSortedSlot<MemCard::MC_FILE, 16>& openFiles, MemCard::MC_FILE* pFile)
{
    for (long i = 0, byteOff = i; (unsigned long)i < openFiles.m_EntryCount; i++, byteOff += 8)
    {
        if (openFiles.m_pEntryLookup[byteOff >> 3].pEntry == pFile)
        {
            return &openFiles.m_pEntryLookup[i];
        }
    }

    return NULL;
}

static inline void ShiftCloseFileEntries(nlStaticSortedSlot<MemCard::MC_FILE, 16>& openFiles, EntryLookup<MemCard::MC_FILE>* pLookup)
{
    s32 next;
    unsigned long total = openFiles.m_EntryCount;
    long idx = (pLookup - openFiles.m_pEntryLookup);

    while ((unsigned long)idx != total)
    {
        next = idx + 1;
        EntryLookup<MemCard::MC_FILE>* dst = &openFiles.m_pEntryLookup[idx];
        EntryLookup<MemCard::MC_FILE>* src = &openFiles.m_pEntryLookup[next];
        idx = next;
        unsigned long id = src->Id;
        MemCard::MC_FILE* entry = src->pEntry;
        dst->pEntry = entry;
        dst->Id = id;
    }

    openFiles.m_EntryCount = openFiles.m_EntryCount - 1;
}

s32 MemCard::CloseFile(MC_FILE* pFile)
{
    pFile->TotalHeaderSize = 0;
    s32 result = CARDClose(&pFile->FileInfo);
    if (result == 0 && pFile != NULL)
    {
        EntryLookup<MC_FILE>* pLookup = FindOpenFileLookup(m_OpenFiles, pFile);
        m_OpenFiles.FreeEntry(pFile);
        ShiftCloseFileEntries(m_OpenFiles, pLookup);
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
 * TODO: 95.2% match - register allocation differs in GetValidDataInfo inline section
 * (bannerFormat r4 vs r11, cascading to all temporaries). -inline deferred file.
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
    DataInfo.IconOffset[0] = DataInfo.BannerOffset + bannerFormat * 0xC00 + ((bannerFormat == 1) ? 0x200 : 0);

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

    CARDSetIconAnim(&stat, pFile->IconCfg.IconAnimType);

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
 * TODO: 89.6% match - prologue register allocation differs (r5/r9 and r7/r8 swaps) and
 * first-loop entry branch compiles as beq (IconCount == 1) instead of target ble (<= 1).
 */
void MemCard::ICON_CONFIG::GetValidDataInfo(MemCard::ICON_DATA_INFO& DataInfo) const
{
    unsigned long Icon;
    unsigned char bannerFormat = BannerFormat;
    unsigned char iconCountMember = IconCount;

    DataInfo.Comment1Offset = 0;
    DataInfo.Comment2Offset = 0x20;
    DataInfo.BannerOffset = 0x40;
    DataInfo.BannerCLUTOffset = DataInfo.BannerOffset + ((bannerFormat == 1) ? 0xC00 : 0);
    DataInfo.IconOffset[0] = DataInfo.BannerOffset + bannerFormat * 0xC00 + ((bannerFormat == 1) ? 0x200 : 0);

    unsigned long iconCount = iconCountMember;
    s32 iconStride = (s32)IconFormat << 10;

    for (Icon = 1; Icon != iconCount; Icon++)
    {
        DataInfo.IconOffset[Icon] = DataInfo.IconOffset[Icon - 1] + iconStride;
    }

    iconCount = IconCount;
    for (; Icon < 8; Icon++)
    {
        DataInfo.IconOffset[Icon] = DataInfo.IconOffset[iconCount - 1];
    }

    DataInfo.IconCLUTOffset = DataInfo.IconOffset[IconCount - 1] + ((s32)IconFormat << 10);
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
 * Offset/Address/Size: 0x0 | 0x801C9770 | size: 0x10
 */
s64 MemCard::GetSerialID() const
{
    return m_SerialID;
}
