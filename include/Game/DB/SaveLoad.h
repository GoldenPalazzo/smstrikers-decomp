#ifndef _SAVELOAD_H_
#define _SAVELOAD_H_

#include <dolphin/card.h>
#include "Game/Sys/gcmemcard.h"

struct MCFILE_HEADER
{
    /* 0x0 */ unsigned long Size;
    /* 0x4 */ unsigned long CRC;
    /* 0x8 */ unsigned long IconCRC;
}; // total size: 0xC

void LoadMemoryCardIconData();

class SaveLoad
{
public:
    static bool CardBusy();
    static long StartSave(int, void (*)(long));
    static long StartLoad(int, void (*)(long), bool, bool);
    static bool DidGameIDChange();
    static long StartDelete(int, void (*)(long));
    static long StartFormat(int, void (*)(long));
    static long StartFileExistsCheck(int, void (*)(long));
    static long StartMemoryCardIDCheck(int, void (*)(long));
    int GetSaveBlockSize(int);
    static u8 HasEnoughFreeSpace(int);
    static void FreeAllCallbackMemory();
    static void RememberCurrentMemCardSerialID(int);
};

class MemoryCardIDCallbacks
{
public:
    unsigned long CardMountCB(unsigned long, long, void*);
};

class FileExistsCallbacks
{
public:
    unsigned long CardMountCB(unsigned long, long, void*);
};

class FormatCallbacks
{
public:
    unsigned long FormatDoneCB(unsigned long, long, void*);
    unsigned long CardMountCB(unsigned long, long, void*);
};

class DeleteCallbacks
{
public:
    unsigned long DeleteDoneCB(unsigned long, long, void*);
    unsigned long CardMountCB(unsigned long, long, void*);
};

class LoadCallbacks
{
public:
    LoadCallbacks();
    unsigned long LoadIconDataDoneCB(unsigned long, long, void*);
    unsigned long ReadDoneCB(unsigned long, long, void*);
    unsigned long CardMountCB(unsigned long, long, void*);

    /* 0x00 */ void* m_pReadBuffer;
    /* 0x04 */ unsigned long m_AlignedReadBufferDataSize;
    /* 0x08 */ void* m_pIconReadBuffer;
    /* 0x0C */ unsigned long m_AlignedIconReadBufferDataSize;
    /* 0x10 */ MemCard::MC_FILE* m_pLoadFile;
    /* 0x14 */ bool m_TestGameID;
    /* 0x15 */ bool m_GameIDTestResult;
    /* 0x16 */ bool m_PerformLoad;
    /* 0x17 */ bool m_MustFreeBuffers;
    /* 0x18 */ unsigned long m_IconLoadedCRC;
}; // total size: 0x1C

class SaveCallbacks
{
public:
    SaveCallbacks();
    unsigned long FileWriteCB(unsigned long, long, void*);
    long DoSave(unsigned long);
    unsigned long FileWriteIconCB(unsigned long, long, void*);
    unsigned long CreateFileCB(unsigned long, long, void*);
    unsigned long CardMountCB(unsigned long, long, void*);

    /* 0x00 */ MemCard::MC_FILE* m_pSaveFile;
    /* 0x04 */ void* m_pSaveGameBuffer;
    /* 0x08 */ unsigned long m_Slot;
    /* 0x0C */ unsigned long m_IconCRC;
    /* 0x10 */ unsigned char m_MustFreeMemory;
}; // total size: 0x14

struct IconDataCache
{
    IconDataCache();
    ~IconDataCache();

    /* 0x00 */ MemCard::ICON_CONFIG mIconConfig;
    /* 0x10 */ MemCard::ICON_DATA_INFO mIconDataInfo;
    /* 0x40 */ void* mIconHdrBuffer;
    /* 0x44 */ void* mIconBuffer;
    /* 0x48 */ void* mBannerBuffer;
}; // total size: 0x4C

extern IconDataCache gIconDataCache;

// class MemCardFunctor
// {
// public:
//     void MCMemberFunctor<LoadCallbacks>::Call(unsigned long, long);
//     void MCMemberFunctor<SaveCallbacks>::Call(unsigned long, long);
//     void MCMemberFunctor<DeleteCallbacks>::Call(unsigned long, long);
//     void MCMemberFunctor<FormatCallbacks>::Call(unsigned long, long);
//     void MCMemberFunctor<FileExistsCallbacks>::Call(unsigned long, long);
//     void MCMemberFunctor<MemoryCardIDCallbacks>::Call(unsigned long, long);
// };

#endif // _SAVELOAD_H_
