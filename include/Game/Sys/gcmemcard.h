#ifndef _GCMEMCARD_H_
#define _GCMEMCARD_H_

// #include "Game/DB/SaveLoad.h"
#include <dolphin/card.h>
#include "NL/nlSortedSlot.h"
#include "NL/nlString.h"

class MemCard;
extern MemCard** g_MemCards;

static inline void DoCardRemovedCleanup(long channel);

enum INTERNAL_STATE
{
    IS_IDLE = 0,
    IS_ERROR = 1,
    IS_MOUNTING = 2,
    IS_CARDCHECK = 3,
    IS_MOUNTED = 4,
    IS_MOUNTED_ERROR = 5,
    IS_FORMATTING = 6,
    IS_CREATING = 7,
    IS_DELETING = 8,
    IS_READING = 9,
    IS_WRITING = 10,
    IS_WRITINGSTATUS = 11,
};

enum CARD_STATE
{
    CS_IDLE = 0,
    CS_MOUNTING = 1,
    CS_MOUNTED = 2,
    CS_MOUNTED_ERROR = 3,
    CS_FORMATTING = 4,
    CS_CREATING = 5,
    CS_DELETING = 6,
    CS_READING = 7,
    CS_WRITING = 8,
    CS_END = 9,
};

struct CARD_INFO
{
    /* 0x0 */ long CardSize;
    /* 0x4 */ long SectorSize;
    /* 0x8 */ long FreeBytes;
    /* 0xC */ long FreeFiles;
}; // total size: 0x10

class MemCardFunctor
{
public:
    MemCardFunctor();

    class MCInternalFunctorBase
    {
    public:
        MCInternalFunctorBase()
            : m_pData(NULL)
        {
        }
        MCInternalFunctorBase(void* pData)
            : m_pData(pData)
        {
        }
        ~MCInternalFunctorBase();
        virtual void Call(unsigned long, long) = 0;
        void Destroy();

        /* 0x04 */ void* m_pData;
    };

    template <class T>
    class MCMemberFunctor : public MCInternalFunctorBase
    {
    public:
        typedef unsigned long (T::*MemberCB)(unsigned long, long, void*);

        MCMemberFunctor(T* obj, const MemberCB& cb)
        {
            m_pFunc = ((void**)&cb)[0];
            m_Slot = ((unsigned long*)&cb)[1];
            m_pfnCB = ((void**)&cb)[2];
            m_pObject = obj;
        }
        MCMemberFunctor(T* obj, const MemberCB& cb, void* pData)
            : MCInternalFunctorBase(pData)
        {
            m_pFunc = ((void**)&cb)[0];
            m_Slot = ((unsigned long*)&cb)[1];
            m_pfnCB = ((void**)&cb)[2];
            m_pObject = obj;
        }
        ~MCMemberFunctor();
        virtual void Call(unsigned long, long);
        void Destroy();

        /* 0x08 */ void* m_pFunc;
        /* 0x0C */ unsigned long m_Slot;
        /* 0x10 */ void* m_pfnCB;
        /* 0x14 */ T* m_pObject;
    };

    /* 0x00 */ mutable unsigned char m_FunctorMem[24];
}; // total size: 0x18

class MemCard
{
public:
    struct ICON_DATA_INFO
    {
        /* 0x00 */ unsigned short Comment1Offset;
        /* 0x02 */ unsigned short Comment2Offset;
        /* 0x04 */ unsigned short BannerOffset;
        /* 0x06 */ unsigned short BannerCLUTOffset;
        /* 0x08 */ unsigned long IconOffset[8];
        /* 0x28 */ unsigned long IconCLUTOffset;
        /* 0x2C */ unsigned char* pHeaderData;
    }; // total size: 0x30

    struct ICON_CONFIG
    {
        void GetValidDataInfo(ICON_DATA_INFO&) const;

        /* 0x0 */ unsigned char BannerFormat;
        /* 0x1 */ unsigned char IconCount;
        /* 0x2 */ char IconFormat;
        /* 0x3 */ char IconAnimType;
        /* 0x4 */ char IconSpeeds[8];
        /* 0xC */ unsigned long HeaderSize;
    }; // total size: 0x10

    struct MC_FILE
    {
        /* 0x00 */ CARDFileInfo FileInfo;
        /* 0x14 */ ICON_CONFIG IconCfg;
        /* 0x24 */ unsigned long TotalHeaderSize;
    }; // total size: 0x28

    MemCard(unsigned long slot);

    static void WriteFileDoneCB(long channel, long result);

    static void CardCheckDoneCB(long channel, long result);

    static void CardCheckBrokenDoneCB(long channel, long result);

    static void SetStatusDoneCB(long channel, long result);

    static void ReadFileDoneCB(long channel, long result);

    static void DeleteFileDoneCB(long channel, long result);

    static void FormatDoneCB(long channel, long result);

    static void CreateFileDoneCB(long channel, long result);

    static void MountDoneCB(long channel, long result);

    static void CardRemovedCB(long channel, long result);

    long BeginCardAccess(const MemCardFunctor&);
    long CreateFile(const char*, unsigned long, MemCard::ICON_CONFIG*, MemCard::MC_FILE*&, const MemCardFunctor&);
    long OpenFile(const char*, MemCard::MC_FILE*&, unsigned long*);
    long FormatCard(const MemCardFunctor&);
    long DeleteFile(const char*, const MemCardFunctor&);
    long InternalReadFile(MemCard::MC_FILE*, void*, unsigned long, unsigned long, const MemCardFunctor&);
    long InternalWriteFile(MemCard::MC_FILE*, void*, unsigned long, unsigned long, const MemCardFunctor&, bool);
    long CloseFile(MemCard::MC_FILE*);
    long FileExists(const char*);
    long WriteFileIconData(MemCard::MC_FILE*, void*, const MemCardFunctor&);
    unsigned long AlignBytesToSectorSize(unsigned long);

private:
    inline void SetStatusDone(long);

public:
    s64 GetSerialID() const;

    /* 0x000  */ INTERNAL_STATE m_State;
    /* 0x004  */ unsigned long m_Slot;
    /* 0x008  */ CARD_STATE m_CardState;
    /* 0x00C  */ CARD_INFO m_CardInfo;
    /* 0x01C */ unsigned long m_LastTransferSize;
    /* 0x020 */ unsigned long m_TargetTransferSize;
    /* 0x024 */ s32 unk_24;
    /* 0x028 */ s64 m_SerialID;
    /* 0x030 */ MemCardFunctor m_CB[9];
    /* 0x108 */ nlStaticSortedSlot<MC_FILE, 16> m_OpenFiles;
    /* 0x41C */ MC_FILE* m_pFileCB;
    /* 0x420 */ void* m_pDataCB;
    /* 0x424 */ unsigned long m_GameId;
    /* 0x428 */ unsigned short m_CompanyId;
    /* 0x42A */ unsigned char m_CardWorkArea[41472];

    static bool s_InitDone;
}; // total size: 0xA620

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

inline void MemCard::WriteFileDoneCB(long channel, long result)
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

inline void MemCard::CardCheckDoneCB(long channel, long result)
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

inline void MemCard::CardCheckBrokenDoneCB(long channel, long result)
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

inline void MemCard::SetStatusDoneCB(long channel, long result)
{
    g_MemCards[channel]->SetStatusDone(result);
}

inline void MemCard::ReadFileDoneCB(long channel, long result)
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

inline void MemCard::DeleteFileDoneCB(long channel, long result)
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

inline void MemCard::FormatDoneCB(long channel, long result)
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

inline void MemCard::CreateFileDoneCB(long channel, long result)
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

inline void MemCard::MountDoneCB(long channel, long result)
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

inline void MemCard::CardRemovedCB(long channel, long result)
{
    DoCardRemovedCleanup(channel);
}

inline MemCardFunctor::MemCardFunctor() { *(unsigned long*)m_FunctorMem = 0; }

#endif // _GCMEMCARD_H_
