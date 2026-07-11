#include "NL/nlFileGC.h"
#include "NL/nlFile.h"
#include "NL/nlMemory.h"
#include "NL/nlFunction.h"
#include "NL/glx/glxSwap.h"
#include "FILE_POS.h"
#include "direct_io.h"
#include "types.h"
#include <string.h>

int nlSNPrintf(char*, unsigned long, const char*, ...);
void nlBreak();

static AsyncManager* s_pAsyncManager;
static GCFileSystem fileSystem;

static Function<void(int)> g_HandleDVDMessageCallback;
static Function<void(int)> g_HandleDVDAllClearCallback;
static Function<void(int)> g_HandleDVDRetryCB;
static Function<FnVoidVoid> g_CheckForResetCB;

namespace
{
void AsyncToVirMemBufferCallback(nlFile*, void*, unsigned int, unsigned long);
}
static unsigned char UpdateReadState(AsyncEntry*);

static AsyncEntry* nlDLRingRemoveStartAsyncEntry(AsyncEntry** head)
{
    AsyncEntry* entry = (*head)->next;

    if (entry->next == entry)
    {
        *head = NULL;
    }
    else
    {
        entry->prev->next = entry->next;
        entry->next->prev = entry->prev;

        if (*head == entry)
        {
            *head = entry->prev;
        }
    }

    return entry;
}

namespace
{
struct AsyncToVirMemBufferLoad
{
    /* 0x00 */ int numChunksLeft;
    /* 0x04 */ unsigned long param;
    /* 0x08 */ void (*callback)(class nlFile*, void*, unsigned int, unsigned long);
    /* 0x0C */ char* target;
    /* 0x10 */ int size;

    AsyncToVirMemBufferLoad();
}; // total size: 0x14
} // namespace

static AsyncToVirMemBufferLoad asyncToVirMemBufferLoad[4];
static char readBuffer32ByteLength[32];
static u8 asyncToVirMemBuffer[0x4000];

/**
 * Offset/Address/Size: 0x0 | 0x801CED54 | size: 0xEC
 */
void nlReadAsyncToVirtualMemory(nlFile* file, void* buffer, int size, ReadAsyncCallback callback, unsigned long param,
    unsigned long chunkSize, void* userData)
{
    FORCE_DONT_INLINE;
    int i;
    for (i = 0; i < 4; i++)
    {
        if (asyncToVirMemBufferLoad[i].numChunksLeft == 0)
        {
            unsigned int numChunks = (unsigned int)size / (unsigned int)chunkSize;
            unsigned long counter1;
            unsigned int sz;
            unsigned long counter2;
            counter2 = i;
            counter1 = i;
            sz = chunkSize;
            asyncToVirMemBufferLoad[i].numChunksLeft = numChunks + 1;
            asyncToVirMemBufferLoad[i].param = param;
            asyncToVirMemBufferLoad[i].callback = callback;
            asyncToVirMemBufferLoad[i].size = size;
            int remainder = size - numChunks * chunkSize;
            asyncToVirMemBufferLoad[i].target = (char*)buffer;

            int j;
            for (j = 0; j < (int)numChunks; j++)
            {
                nlReadAsync(file, userData, sz, AsyncToVirMemBufferCallback, counter1);
            }

            nlReadAsync(file, userData, remainder, AsyncToVirMemBufferCallback, counter2);
            return;
        }
    }
}

/**
 * Offset/Address/Size: 0xEC | 0x801CEE40 | size: 0x38
 */
void nlAsyncLoadFileToVirtualMemory(nlFile* file, int size, void* buffer, ReadAsyncCallback callback, unsigned long alignment)
{
    nlReadAsyncToVirtualMemory(file, buffer, size, callback, alignment, 0x4000, asyncToVirMemBuffer);
}

/**
 * Offset/Address/Size: 0x124 | 0x801CEE78 | size: 0xAC
 */
namespace
{
void AsyncToVirMemBufferCallback(nlFile* pFile, void* buffer, unsigned int size, unsigned long param)
{
    memcpy(asyncToVirMemBufferLoad[param].target, (char*)buffer - size, size);
    asyncToVirMemBufferLoad[param].target += size;
    asyncToVirMemBufferLoad[param].numChunksLeft--;
    if (asyncToVirMemBufferLoad[param].numChunksLeft == 0)
    {
        asyncToVirMemBufferLoad[param].callback(pFile, asyncToVirMemBufferLoad[param].target, asyncToVirMemBufferLoad[param].size, asyncToVirMemBufferLoad[param].param);
    }
}
} // namespace

/**
 * Offset/Address/Size: 0x1D0 | 0x801CEF24 | size: 0xF4
 */
void nlCancelPendingAsyncReads(nlFile* pFile, void (*callback)(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long)))
{
    AsyncEntry* pEntry;
    AsyncEntry* pNextEntry;

    if (pFile == NULL)
    {
        return;
    }

    AsyncManager* pMgr = s_pAsyncManager;

    if (((GCFile*)pFile)->PendingAsync.m_Count == 0)
    {
        return;
    }

    if (pMgr->m_activeEntryList == NULL)
    {
        return;
    }

    pNextEntry = nlDLRingGetStart<AsyncEntry>(pMgr->m_activeEntryList);

    do
    {
        pEntry = pNextEntry->next;

        if (pNextEntry->m_pFile == (GCFile*)pFile)
        {
            u8 beingServiced;
            if (pNextEntry != NULL)
            {
                beingServiced = (u8)(pNextEntry->Phase != 0);
            }
            else
            {
                beingServiced = 0;
            }

            if (!beingServiced)
            {
                ((GCFile*)pFile)->PendingAsync.m_Count--;

                if (callback != NULL)
                {
                    callback((nlFile*)pNextEntry->m_pFile, pNextEntry->m_pBuffer, pNextEntry->m_uSize, pNextEntry->m_uParam, pNextEntry->m_pFunc);
                }

                nlDLRingRemove<AsyncEntry>(&pMgr->m_activeEntryList, pNextEntry);
                nlDLRingAddEnd<AsyncEntry>(&pMgr->m_freeEntryList, pNextEntry);
            }
        }

        if (nlRingIsEnd<AsyncEntry>(pMgr->m_activeEntryList, pNextEntry))
        {
            break;
        }

        pNextEntry = pEntry;
    } while (true);
}

/**
 * Offset/Address/Size: 0x2C4 | 0x801CF018 | size: 0x34
 */
bool nlAsyncReadsPending(nlFile* file)
{
    if (file != NULL)
    {
        return ((GCFile*)file)->PendingAsync.m_Count != 0;
    }
    return s_pAsyncManager->m_activeEntryList != nullptr;
}

static inline nlFile* nlLoadEntireFileOpen(const char* fileName)
{
    nlFile* pGCFile;

    if (fileSystem == eGC_TDEV)
    {
        FILE* pFile = fopen(fileName, "rb");
        if (pFile == NULL)
        {
            pGCFile = NULL;
        }
        else
        {
            long uPos = ftell(pFile);
            fseek(pFile, 0, 2);
            pGCFile = (nlFile*)ftell(pFile);
            fseek(pFile, uPos, 0);

            if ((unsigned long)pGCFile == 0xFFFFFFFF)
            {
                pGCFile = NULL;
            }
            else
            {
                pGCFile = new TDEVChunkFile(pFile);
                while (pFile == NULL)
                {
                }
            }
        }
    }
    else
    {
        s32 fileEntrynum = DVDConvertPathToEntrynum(fileName);
        if (fileEntrynum == -1)
        {
            pGCFile = NULL;
        }
        else
        {
            pGCFile = new DolphinFile(fileEntrynum);
            while (pGCFile == NULL)
            {
            }
        }
    }

    return pGCFile;
}

static inline void* nlReadToVirtualMemoryInline(nlFile* file, void* buffer, unsigned int size, unsigned int chunkSize)
{
    unsigned int readSize;
    void* tempBuffer;
    unsigned int offset;

    tempBuffer = nlMalloc(chunkSize, 0x20, false);
    offset = 0;

    while (offset < size)
    {
        readSize = chunkSize;
        if (size - offset <= chunkSize)
        {
            readSize = size - offset;
        }
        nlRead(file, tempBuffer, readSize);
        memcpy((u8*)buffer + offset, tempBuffer, readSize);
        offset += readSize;
    }

    nlFree(tempBuffer);
    return buffer;
}

/**
 * Offset/Address/Size: 0x2F8 | 0x801CF04C | size: 0x2D0
 * TODO: 97.9% match - remaining saved-register cycle between file,
 * buffer, size, target, and open-path temporaries
 */
void* nlLoadEntireFileToVirtualMemory(const char* fileName, int* size, unsigned int transferSize, void* target, eAllocType allocType)
{
    void* buffer = NULL;
    nlFile* pGCFile = nlLoadEntireFileOpen(fileName);

    if (pGCFile != NULL)
    {
        unsigned int fileSize = 0;
        nlFileSize(pGCFile, &fileSize);

        unsigned int maxRequiredMemory = fileSize + 0x40;
        if (target == NULL)
        {
            if (maxRequiredMemory > nlVirtualLargestBlock())
            {
                goto alloc_fallback;
            }
        }

        if (target == NULL)
        {
            if (allocType == AllocateEnd)
            {
                buffer = nlVirtualAlloc(fileSize, true);
            }
            else
            {
                buffer = nlVirtualAlloc(fileSize, false);
            }
        }
        else
        {
            buffer = target;
        }

        nlReadToVirtualMemoryInline(pGCFile, buffer, fileSize, transferSize);
        goto alloc_done;

    alloc_fallback:
    {
        OSReport("VIRTUAL MEMORY WARNING ~ nlLoadEntireFileToVirtualMemory had to fall back to MRAM\n\tsize: %d file: %s\n\tLargest block: %d Total free: %d\n", fileSize, fileName, nlVirtualLargestBlock(), nlVirtualTotalFree());
        buffer = nlMalloc(fileSize, 0x20, false);
        nlRead(pGCFile, buffer, fileSize);
    }

    alloc_done:
        *size = fileSize;
        nlClose(pGCFile);
    }

    return buffer;
}

/**
 * Offset/Address/Size: 0x5C8 | 0x801CF31C | size: 0x9C
 */
void* nlReadToVirtualMemory(nlFile* file, void* buffer, unsigned int size, unsigned int chunkSize)
{
    unsigned int readSize;
    void* tempBuffer;
    unsigned int offset;

    tempBuffer = nlMalloc(chunkSize, 0x20, false);
    offset = 0;

    while (offset < size)
    {
        readSize = chunkSize;
        if (size - offset <= chunkSize)
        {
            readSize = size - offset;
        }
        nlRead(file, tempBuffer, readSize);
        memcpy((u8*)buffer + offset, tempBuffer, readSize);
        offset += readSize;
    }

    nlFree(tempBuffer);
    return buffer;
}

/**
 * Offset/Address/Size: 0x664 | 0x801CF3B8 | size: 0x8
 */
u32 nlGetFilePosition(nlFile* file)
{
    return ((GCFile*)file)->m_Position;
}

/**
 * Offset/Address/Size: 0x66C | 0x801CF3C0 | size: 0x8C
 */
void nlSeek(nlFile* file, unsigned int offset, unsigned long origin)
{
    GCFile* gcFile = (GCFile*)file;
    switch (origin)
    { /* irregular */
    case 0:
        gcFile->m_Position = offset;
        return;
    case 1:
        gcFile->m_Position = (s32)(gcFile->m_Position + offset);
        return;
    case 2:
        gcFile->m_Position = (s32)(gcFile->FileSize(NULL) - offset);
        return;
    }
}

/**
 * Offset/Address/Size: 0x6F8 | 0x801CF44C | size: 0x34
 */
void nlReadAsync(nlFile* file, void* buffer, unsigned int size, ReadAsyncCallback callback, unsigned long arg4)
{
    FORCE_DONT_INLINE;
    GameCubeReadAsync((GCFile*)file, callback, buffer, (u32)size, arg4);
}

template <>
void nlDLRingRemove<AsyncEntry>(AsyncEntry**, AsyncEntry*);
template <>
void nlDLRingAddEnd<AsyncEntry>(AsyncEntry**, AsyncEntry*);
template <>
void nlDLRingAddStart<AsyncEntry>(AsyncEntry**, AsyncEntry*);
template <>
AsyncEntry* nlDLRingGetStart<AsyncEntry>(AsyncEntry* current);
template <>
AsyncEntry* nlDLRingRemoveStart<AsyncEntry>(AsyncEntry**);

/**
 * Offset/Address/Size: 0x72C | 0x801CF480 | size: 0x27C
 * TODO: 99.87% match - r30 vs r27 register allocation for manager (s_pAsyncManager)
 */
void nlServiceFileSystem()
{
    extern void glxLoadSaveState(void);
    extern void glxLoadRestoreState(void);

    extern Function<void(int)> g_HandleDVDMessageCallback;
    extern Function<void(int)> g_HandleDVDAllClearCallback;
    extern Function<void(int)> g_HandleDVDRetryCB;
    extern Function<FnVoidVoid> g_CheckForResetCB;

    AsyncManager* const manager = s_pAsyncManager;
    AsyncEntry* entry = manager->m_activeEntryList;
    s32 driveStatus;
    u8 loadedSaveState;

    if (entry != NULL)
    {
        entry = entry->next;

        if ((OSGetConsoleType() & 0x20000000) != 0)
        {
            OSYieldThread();
        }

        if (UpdateReadState(entry))
        {
            nlDLRingRemove<AsyncEntry>(&manager->m_activeEntryList, entry);
            entry->m_pFile->PendingAsync.m_Count--;

            if (entry->m_pFunc != NULL)
            {
                entry->m_pFunc(entry->m_pFile, entry->m_pBuffer, entry->m_uSize, entry->m_uParam);
            }

            nlDLRingAddEnd<AsyncEntry>(&manager->m_freeEntryList, entry);
        }

        return;
    }

    loadedSaveState = 0;
    Function<void(int)>* handleDVDMessageCallback = &g_HandleDVDMessageCallback;
    Function<FnVoidVoid>* checkForResetCB = &g_CheckForResetCB;
    Function<void(int)>* handleDVDRetryCB = &g_HandleDVDRetryCB;

    while (true)
    {
        driveStatus = DVDGetDriveStatus();

        switch (driveStatus)
        {
        case DVD_STATE_FATAL_ERROR:
        case DVD_STATE_NO_DISK:
        case DVD_STATE_COVER_OPEN:
        case DVD_STATE_WRONG_DISK:
        case DVD_STATE_RETRY:
            if (!loadedSaveState)
            {
                glxLoadSaveState();
            }

            if (g_HandleDVDMessageCallback.mTag == 1)
            {
                handleDVDMessageCallback->mFreeFunction(driveStatus);
            }
            else
            {
                (*handleDVDMessageCallback->mFunctor)(driveStatus);
            }

            loadedSaveState = 1;

            while (driveStatus == DVDGetDriveStatus())
            {
                OSYieldThread();

                if (g_CheckForResetCB.mTag != 0)
                {
                    if (g_CheckForResetCB.mTag == 1)
                    {
                        checkForResetCB->mFreeFunction();
                    }
                    else
                    {
                        checkForResetCB->mFunctor->operator()();
                    }
                }
            }
            break;

        case DVD_STATE_BUSY:
            if (loadedSaveState)
            {
                if (g_HandleDVDRetryCB.mTag != 0)
                {
                    if (g_HandleDVDRetryCB.mTag == 1)
                    {
                        handleDVDRetryCB->mFreeFunction(1);
                    }
                    else
                    {
                        (*handleDVDRetryCB->mFunctor)(1);
                    }
                }

                while (DVDGetDriveStatus() == DVD_STATE_BUSY)
                {
                    OSYieldThread();

                    if (g_CheckForResetCB.mTag != 0)
                    {
                        if (g_CheckForResetCB.mTag == 1)
                        {
                            checkForResetCB->mFreeFunction();
                        }
                        else
                        {
                            checkForResetCB->mFunctor->operator()();
                        }
                    }
                }
            }
            break;
        }

        if ((driveStatus == DVD_STATE_END) || (driveStatus == DVD_STATE_FATAL_ERROR))
        {
            break;
        }
    }

    if (loadedSaveState)
    {
        glxLoadRestoreState();
    }

    if (loadedSaveState && (g_HandleDVDAllClearCallback.mTag != 0))
    {
        if (g_HandleDVDAllClearCallback.mTag == 1)
        {
            g_HandleDVDAllClearCallback.mFreeFunction(0);
        }
        else
        {
            (*g_HandleDVDAllClearCallback.mFunctor)(0);
        }
    }
}

/**
 * Offset/Address/Size: 0x9A8 | 0x801CF6FC | size: 0x35C
 */
void nlInitFileSystem()
{
    if ((OSGetConsoleType() & 0x20000000) != 0)
    {
        TDEVChunkFile* pFile;
        nlArrayAllocator<TDEVChunkFile>* pAlloc;

        fileSystem = eGC_TDEV;
        pFile = (TDEVChunkFile*)nlMalloc(0x400, 8, false);
        pAlloc = (nlArrayAllocator<TDEVChunkFile>*)nlMalloc(4, 8, false);

        if (pAlloc != NULL)
        {
            u32 i;

            pAlloc->m_pFree = pFile;

            for (i = 0; i < 31; i++)
            {
                *(TDEVChunkFile**)(pFile + i) = pFile + i + 1;
            }

            *(TDEVChunkFile**)(pFile + 31) = NULL;
        }

        TDEVChunkFile::s_pAllocator = pAlloc;
    }
    else
    {
        DolphinFile* pFile;
        nlArrayAllocator<DolphinFile>* pAlloc;

        fileSystem = eGC_DVDOPEN;
        pFile = (DolphinFile*)nlMalloc(0x900, 8, false);
        pAlloc = (nlArrayAllocator<DolphinFile>*)nlMalloc(4, 8, false);

        if (pAlloc != NULL)
        {
            u32 i;

            pAlloc->m_pFree = pFile;

            for (i = 0; i < 31; i++)
            {
                *(DolphinFile**)(pFile + i) = pFile + i + 1;
            }

            *(DolphinFile**)(pFile + 31) = NULL;
        }

        DolphinFile::s_pAllocator = pAlloc;
    }

    DVDInit();

    if (s_pAsyncManager == NULL)
    {
        AsyncManager* pManager;

        pManager = (AsyncManager*)nlMalloc(0xA08, 8, false);
        if (pManager != NULL)
        {
            s32 i;
            AsyncEntry* pEntry = (AsyncEntry*)pManager;

            pManager->m_activeEntryList = pManager->m_freeEntryList = (AsyncEntry*)(i = 0);

            for (; i < 64; i++)
            {
                nlDLRingAddStart<AsyncEntry>(&pManager->m_freeEntryList, pEntry);
                pEntry = (AsyncEntry*)((u8*)pEntry + 0x28);
            }
        }

        s_pAsyncManager = pManager;
    }
}

/**
 * Offset/Address/Size: 0xD04 | 0x801CFA58 | size: 0x2E0
 */
unsigned char GameCubeReadBlocking(GCFile* pFile, void* pBuffer, unsigned long uSize)
{
    extern void glxLoadSaveState(void);
    extern void glxLoadRestoreState(void);

    extern Function<void(int)> g_HandleDVDMessageCallback;
    extern Function<void(int)> g_HandleDVDAllClearCallback;
    extern Function<void(int)> g_HandleDVDRetryCB;
    extern Function<FnVoidVoid> g_CheckForResetCB;
    GameCubeReadAsync(pFile, NULL, pBuffer, uSize, 0);

    Function<FnVoidVoid>* checkForResetCB = &g_CheckForResetCB;
    Function<void(int)>* handleDVDMessageCallback = &g_HandleDVDMessageCallback;
    Function<void(int)>* handleDVDRetryCB = &g_HandleDVDRetryCB;
    Function<void(int)>* handleDVDAllClearCallback = &g_HandleDVDAllClearCallback;

    goto loop_check;

loop_wait:
    OSYieldThread();

    if (g_CheckForResetCB.mTag != 0)
    {
        if (g_CheckForResetCB.mTag == 1)
        {
            checkForResetCB->mFreeFunction();
        }
        else
        {
            checkForResetCB->mFunctor->operator()();
        }
    }

loop_check:
    AsyncManager* const manager = s_pAsyncManager;
    AsyncEntry* entry = manager->m_activeEntryList;

    if (entry != NULL)
    {
        entry = entry->next;

        if ((OSGetConsoleType() & 0x20000000) != 0)
        {
            OSYieldThread();
        }

        if (UpdateReadState(entry))
        {
            nlDLRingRemove<AsyncEntry>(&manager->m_activeEntryList, entry);
            entry->m_pFile->PendingAsync.m_Count--;

            if (entry->m_pFunc != NULL)
            {
                entry->m_pFunc(entry->m_pFile, entry->m_pBuffer, entry->m_uSize, entry->m_uParam);
            }

            nlDLRingAddEnd<AsyncEntry>(&manager->m_freeEntryList, entry);
        }
    }
    else
    {
        s32 driveStatus;
        u8 loadedSaveState = 0;

        while (true)
        {
            driveStatus = DVDGetDriveStatus();

            u32 statusPlusOne = (u32)(driveStatus + 1);
            switch (statusPlusOne)
            {
            case DVD_STATE_FATAL_ERROR + 1:
            case DVD_STATE_NO_DISK + 1:
            case DVD_STATE_COVER_OPEN + 1:
            case DVD_STATE_WRONG_DISK + 1:
            case DVD_STATE_RETRY + 1:
            {
                if (!loadedSaveState)
                {
                    glxLoadSaveState();
                }

                if (g_HandleDVDMessageCallback.mTag == 1)
                {
                    handleDVDMessageCallback->mFreeFunction(driveStatus);
                }
                else
                {
                    (*handleDVDMessageCallback->mFunctor)(driveStatus);
                }

                loadedSaveState = 1;

                while (driveStatus == DVDGetDriveStatus())
                {
                    OSYieldThread();

                    if (g_CheckForResetCB.mTag != 0)
                    {
                        if (g_CheckForResetCB.mTag == 1)
                        {
                            checkForResetCB->mFreeFunction();
                        }
                        else
                        {
                            checkForResetCB->mFunctor->operator()();
                        }
                    }
                }
                break;
            }

            case DVD_STATE_BUSY + 1:
            {
                if (loadedSaveState)
                {
                    if (g_HandleDVDRetryCB.mTag != 0)
                    {
                        if (g_HandleDVDRetryCB.mTag == 1)
                        {
                            handleDVDRetryCB->mFreeFunction(1);
                        }
                        else
                        {
                            (*handleDVDRetryCB->mFunctor)(1);
                        }
                    }

                    while (DVDGetDriveStatus() == DVD_STATE_BUSY)
                    {
                        OSYieldThread();

                        if (g_CheckForResetCB.mTag != 0)
                        {
                            if (g_CheckForResetCB.mTag == 1)
                            {
                                checkForResetCB->mFreeFunction();
                            }
                            else
                            {
                                checkForResetCB->mFunctor->operator()();
                            }
                        }
                    }
                }
                break;
            }

            default:
                break;
            }

            if ((driveStatus == DVD_STATE_END) || (driveStatus == DVD_STATE_FATAL_ERROR))
            {
                break;
            }
        }

        if (loadedSaveState)
        {
            glxLoadRestoreState();
        }

        if (loadedSaveState && (g_HandleDVDAllClearCallback.mTag != 0))
        {
            if (g_HandleDVDAllClearCallback.mTag == 1)
            {
                handleDVDAllClearCallback->mFreeFunction(0);
            }
            else
            {
                (*handleDVDAllClearCallback->mFunctor)(0);
            }
        }
    }

    if (s_pAsyncManager->m_activeEntryList != NULL)
    {
        goto loop_wait;
    }
    return 1;
}

/**
 * Offset/Address/Size: 0xFE4 | 0x801CFD38 | size: 0x324
 */
static unsigned char GameCubeReadAsync(GCFile* pFile, ReadAsyncCallback callback, void* pBuffer, unsigned long uSize, unsigned long uParam)
{
    extern void glxLoadSaveState(void);
    extern void glxLoadRestoreState(void);

    extern Function<void(int)> g_HandleDVDMessageCallback;
    extern Function<void(int)> g_HandleDVDAllClearCallback;
    extern Function<void(int)> g_HandleDVDRetryCB;
    extern Function<FnVoidVoid> g_CheckForResetCB;

    u32 loadedSaveState = 0;
    AsyncManager* manager = s_pAsyncManager;

    if (manager->m_freeEntryList != NULL)
    {
        AsyncEntry* entry = nlDLRingRemoveStart<AsyncEntry>(&manager->m_freeEntryList);

        entry->m_pFile = pFile;
        entry->m_pFunc = callback;
        entry->m_pBuffer = pBuffer;
        entry->m_uSize = uSize;
        entry->m_uParam = uParam;
        entry->m_uPosition = pFile->m_Position;
        entry->ReadNumBytes = uSize;
        entry->Phase = eRS_ISSUE_HEAD_READ;
        entry->m_pFile->PendingAsync.m_Count++;

        if (manager->m_activeEntryList == NULL)
        {
            GCFileSystem fs = fileSystem;
            loadedSaveState = (((u32)(1 - fs) | (u32)(fs - 1)) >> 31);
        }

        nlDLRingAddEnd<AsyncEntry>(&manager->m_activeEntryList, entry);

        if ((u8)loadedSaveState)
        {
            entry = manager->m_activeEntryList;

            if (entry != NULL)
            {
                entry = entry->next;

                if ((OSGetConsoleType() & 0x20000000) != 0)
                {
                    OSYieldThread();
                }

                if (UpdateReadState(entry))
                {
                    nlDLRingRemove<AsyncEntry>(&manager->m_activeEntryList, entry);
                    entry->m_pFile->PendingAsync.m_Count--;

                    if (entry->m_pFunc != NULL)
                    {
                        entry->m_pFunc(entry->m_pFile, entry->m_pBuffer, entry->m_uSize, entry->m_uParam);
                    }

                    nlDLRingAddEnd<AsyncEntry>(&manager->m_freeEntryList, entry);
                }
            }
            else
            {
                u8 loadedSaveState = 0;
                s32 driveStatus;
                Function<void(int)>* handleDVDMessageCB = &g_HandleDVDMessageCallback;
                Function<FnVoidVoid>* checkForResetCB = &g_CheckForResetCB;
                Function<void(int)>* handleDVDRetryCB = &g_HandleDVDRetryCB;

                while (true)
                {
                    driveStatus = DVDGetDriveStatus();

                    switch (driveStatus)
                    {
                    case DVD_STATE_FATAL_ERROR:
                    case DVD_STATE_NO_DISK:
                    case DVD_STATE_COVER_OPEN:
                    case DVD_STATE_WRONG_DISK:
                    case DVD_STATE_RETRY:
                        if (!loadedSaveState)
                        {
                            glxLoadSaveState();
                        }

                        if (g_HandleDVDMessageCallback.mTag == 1)
                        {
                            handleDVDMessageCB->mFreeFunction(driveStatus);
                        }
                        else
                        {
                            (*handleDVDMessageCB->mFunctor)(driveStatus);
                        }

                        loadedSaveState = 1;

                        while (driveStatus == DVDGetDriveStatus())
                        {
                            OSYieldThread();

                            if (g_CheckForResetCB.mTag != 0)
                            {
                                if (g_CheckForResetCB.mTag == 1)
                                {
                                    checkForResetCB->mFreeFunction();
                                }
                                else
                                {
                                    checkForResetCB->mFunctor->operator()();
                                }
                            }
                        }
                        break;

                    case DVD_STATE_BUSY:
                        if (loadedSaveState)
                        {
                            if (g_HandleDVDRetryCB.mTag != 0)
                            {
                                if (g_HandleDVDRetryCB.mTag == 1)
                                {
                                    handleDVDRetryCB->mFreeFunction(1);
                                }
                                else
                                {
                                    (*handleDVDRetryCB->mFunctor)(1);
                                }
                            }

                            while (DVDGetDriveStatus() == DVD_STATE_BUSY)
                            {
                                OSYieldThread();

                                if (g_CheckForResetCB.mTag != 0)
                                {
                                    if (g_CheckForResetCB.mTag == 1)
                                    {
                                        checkForResetCB->mFreeFunction();
                                    }
                                    else
                                    {
                                        checkForResetCB->mFunctor->operator()();
                                    }
                                }
                            }
                        }
                        break;
                    }

                    if ((driveStatus == DVD_STATE_END) || (driveStatus == DVD_STATE_FATAL_ERROR))
                    {
                        break;
                    }
                }

                if (loadedSaveState)
                {
                    glxLoadRestoreState();
                }

                if (loadedSaveState && (g_HandleDVDAllClearCallback.mTag != 0))
                {
                    if (g_HandleDVDAllClearCallback.mTag == 1)
                    {
                        g_HandleDVDAllClearCallback.mFreeFunction(0);
                    }
                    else
                    {
                        (*g_HandleDVDAllClearCallback.mFunctor)(0);
                    }
                }
            }
        }
    }

    pFile->m_Position += uSize;
    return 1;
}

/**
 * Offset/Address/Size: 0x1308 | 0x801D005C | size: 0x6E0
 */
static inline unsigned char CheckDVDStatus()
{
    long Status;
    unsigned char WasAProblem = 0;

    while (true)
    {
        Status = DVDGetDriveStatus();
        u32 statusPlusOne = (u32)(Status + 1);

        switch (statusPlusOne)
        {
        case DVD_STATE_FATAL_ERROR + 1:
        case DVD_STATE_NO_DISK + 1:
        case DVD_STATE_COVER_OPEN + 1:
        case DVD_STATE_WRONG_DISK + 1:
        case DVD_STATE_RETRY + 1:
            if (!WasAProblem)
            {
                glxLoadSaveState();
            }

            if (g_HandleDVDMessageCallback.mTag == 1)
            {
                g_HandleDVDMessageCallback.mFreeFunction(Status);
            }
            else
            {
                (*g_HandleDVDMessageCallback.mFunctor)(Status);
            }

            WasAProblem = 1;

            while (Status == DVDGetDriveStatus())
            {
                OSYieldThread();

                if (g_CheckForResetCB.mTag != 0)
                {
                    if (g_CheckForResetCB.mTag == 1)
                    {
                        g_CheckForResetCB.mFreeFunction();
                    }
                    else
                    {
                        g_CheckForResetCB.mFunctor->operator()();
                    }
                }
            }
            break;

        case DVD_STATE_BUSY + 1:
            if (WasAProblem)
            {
                if (g_HandleDVDRetryCB.mTag != 0)
                {
                    if (g_HandleDVDRetryCB.mTag == 1)
                    {
                        g_HandleDVDRetryCB.mFreeFunction(1);
                    }
                    else
                    {
                        (*g_HandleDVDRetryCB.mFunctor)(1);
                    }
                }

                while (DVDGetDriveStatus() == DVD_STATE_BUSY)
                {
                    OSYieldThread();

                    if (g_CheckForResetCB.mTag != 0)
                    {
                        if (g_CheckForResetCB.mTag == 1)
                        {
                            g_CheckForResetCB.mFreeFunction();
                        }
                        else
                        {
                            g_CheckForResetCB.mFunctor->operator()();
                        }
                    }
                }
            }
            break;

        default:
            break;
        }

        if ((Status == DVD_STATE_END) || (Status == DVD_STATE_FATAL_ERROR))
        {
            break;
        }
    }

    return WasAProblem;
}

static inline void HandleGCIOErrors(GCFile* pFile)
{
    long Status;
    unsigned char WasAProblem = CheckDVDStatus();

    if (WasAProblem)
    {
        glxLoadRestoreState();
    }

    while (true)
    {
        char message[0x100];

        Status = pFile->GetReadStatus();
        if ((Status < 3) && (Status >= 0))
        {
            break;
        }

        nlSNPrintf(message, 0x100, "Read error %d. File start addr %d\n", Status, pFile->GetDiscPosition());
        OSReport(message);
        nlBreak();

        if (g_CheckForResetCB.mTag != 0)
        {
            if (g_CheckForResetCB.mTag == 1)
            {
                g_CheckForResetCB.mFreeFunction();
            }
            else
            {
                g_CheckForResetCB.mFunctor->operator()();
            }
        }

        OSYieldThread();
    }

    if (WasAProblem && (g_HandleDVDAllClearCallback.mTag != 0))
    {
        if (g_HandleDVDAllClearCallback.mTag == 1)
        {
            g_HandleDVDAllClearCallback.mFreeFunction(0);
        }
        else
        {
            (*g_HandleDVDAllClearCallback.mFunctor)(0);
        }
    }
}

static unsigned char UpdateReadState(AsyncEntry* pEntry)
{
    long nStatus;
    unsigned long uNumRead;
    GCFile* pFile;
    unsigned long readSize;

    pFile = pEntry->m_pFile;

    while (true)
    {
        switch (pEntry->Phase)
        {
        case eRS_ISSUE_HEAD_READ:
            uNumRead = pEntry->ReadNumBytes & ~31;
            if (uNumRead >= 0x20)
            {
                pFile->ReadAsync(pEntry->m_pBuffer, uNumRead, pEntry->m_uPosition);
                pEntry->Phase = eRS_WAIT_HEAD_READ;
            }
            else
            {
                pEntry->Phase = eRS_ISSUE_TAIL_READ;
            }
            break;

        case eRS_WAIT_HEAD_READ:
            nStatus = pFile->GetReadStatus();
            switch (nStatus)
            {
            case DVD_STATE_BUSY:
                return 0;

            case DVD_STATE_END:
                readSize = pEntry->ReadNumBytes & ~31;
                pEntry->m_uPosition += readSize;
                pEntry->m_pBuffer = (char*)pEntry->m_pBuffer + readSize;
                pEntry->Phase = eRS_ISSUE_TAIL_READ;
                continue;

            default:
                HandleGCIOErrors(pFile);
                goto return_false;
            }

        case eRS_ISSUE_TAIL_READ:
            readSize = pEntry->ReadNumBytes - (pEntry->ReadNumBytes & ~31);
            if (readSize != 0)
            {
                pFile->ReadAsync(readBuffer32ByteLength, 0x20, pEntry->m_uPosition);
                pEntry->Phase = eRS_WAIT_TAIL_READ;
            }
            else
            {
                pEntry->Phase = eRS_READ_COMPLETE;
            }
            break;

        case eRS_WAIT_TAIL_READ:
            nStatus = pFile->GetReadStatus();
            switch (nStatus)
            {
            case DVD_STATE_BUSY:
                return 0;

            case DVD_STATE_END:
                uNumRead = pEntry->ReadNumBytes - (pEntry->ReadNumBytes & ~31);
                memcpy(pEntry->m_pBuffer, readBuffer32ByteLength, uNumRead);
                pEntry->m_pBuffer = (char*)pEntry->m_pBuffer + uNumRead;
                pEntry->m_uPosition += uNumRead;
                pEntry->Phase = eRS_READ_COMPLETE;
                continue;

            default:
                HandleGCIOErrors(pFile);
                goto return_false;
            }

        case eRS_READ_COMPLETE:
            return 1;

        default:
            goto return_false;
        }
    }

return_false:
    return 0;
}

/**
 * Offset/Address/Size: 0x19E8 | 0x801D073C | size: 0x4
 */
void nlFlushFileCash()
{
    // EMPTY
}

static GCFile* TDEVChunkFileOpen(const char* fileName)
{
    GCFile* pGCFile;
    _FILE* pFile;

    pFile = fopen(fileName, "rb");
    if (pFile == NULL)
    {
        pGCFile = NULL;
    }
    else
    {
        pGCFile = (GCFile*)ftell(pFile);
        fseek(pFile, 0, 2);
        unsigned long uSize = ftell(pFile);
        fseek(pFile, (unsigned long)pGCFile, 0);

        if (uSize == 0xFFFFFFFF)
        {
            pGCFile = NULL;
        }
        else
        {
            pGCFile = new TDEVChunkFile(pFile);
            while (pFile == NULL)
            {
            }
        }
    }

    return pGCFile;
}

static GCFile* DolphinFileOpen(const char* fileName)
{
    s32 fileEntrynum;
    GCFile* pFile;

    fileEntrynum = DVDConvertPathToEntrynum(fileName);
    if (fileEntrynum == -1)
    {
        pFile = NULL;
    }
    else
    {
        pFile = new DolphinFile(fileEntrynum);
        while (pFile == NULL)
        {
        }
    }

    return pFile;
}

/**
 * Offset/Address/Size: 0x19EC | 0x801D0740 | size: 0x18C
 * TODO: 99.9% match - remaining diffs are a scratch-only fopen mode string
 * label mismatch ("rb") and the DVD entry -1 fast path using li r3,0 /
 * direct return instead of li r29,0 through the shared return block.
 */
nlFile* nlOpen(const char* fileName)
{
    GCFile* file;

    if (fileSystem == eGC_TDEV)
    {
        file = TDEVChunkFileOpen(fileName);
    }
    else
    {
        s32 fileEntrynum;

        fileEntrynum = DVDConvertPathToEntrynum(fileName);
        if (fileEntrynum == -1)
        {
            file = NULL;
        }
        else
        {
            DolphinFile* pDolphinFile;

            pDolphinFile = new DolphinFile(fileEntrynum);
            while (pDolphinFile == NULL)
            {
            }
            file = pDolphinFile;
        }
    }

    return file;
}

/**
 * Offset/Address/Size: 0x1B78 | 0x801D08CC | size: 0xB0
 */
s32 TDEVChunkFile::GetReadStatus()
{
    fseek(m_pFile, m_CurrentRead.Pos + m_CurrentRead.AmountRead, 0);

    u32 remainingBytes;
    u32 length = m_CurrentRead.Length;
    u32 amountRead = m_CurrentRead.AmountRead;
    remainingBytes = length - amountRead;
    u8* dest = m_CurrentRead.Buffer + amountRead;

    u32 bytesRead = fread(dest, 1, (remainingBytes <= 0x3000U) ? remainingBytes : 0x3000U, m_pFile);
    u32 nextAmount = m_CurrentRead.AmountRead + bytesRead;
    m_CurrentRead.AmountRead = nextAmount;
    u32 currentLength;
    u32 currentAmount = m_CurrentRead.AmountRead;
    currentLength = m_CurrentRead.Length;
    bool isComplete = (currentAmount == currentLength) || ((currentLength == 0x20U) && (currentAmount != 0U));
    enum ReadStatusEnum
    {
        ReadStatusDone = 0,
        ReadStatusBusy = 1
    };
    ReadStatusEnum status = isComplete ? ReadStatusDone : ReadStatusBusy;

    return status;
}

/**
 * Offset/Address/Size: 0x1C28 | 0x801D097C | size: 0x40
 */
void TDEVChunkFile::ReadAsync(void* buffer, unsigned long length, unsigned long offset)
{
    m_CurrentRead.Buffer = (u8*)buffer;
    m_CurrentRead.Pos = offset;
    m_CurrentRead.Length = length;
    m_CurrentRead.AmountRead = 0;
    GetReadStatus();
}

/**
 * Offset/Address/Size: 0x1C68 | 0x801D09BC | size: 0x20
 */
void GCFile::Read(void* buffer, unsigned int size)
{
    GameCubeReadBlocking(this, buffer, size);
}

/**
 * Offset/Address/Size: 0x1C88 | 0x801D09DC | size: 0xA4
 */
void nlRegCheckForResetFromFSCB(const Function<FnVoidVoid>& cb)
{
    g_CheckForResetCB = cb;
}

/**
 * Offset/Address/Size: 0x1D2C | 0x801D0A80 | size: 0xA4
 */
void nlRegHandleDVDAllClearCB(const Function<void(int)>& cb)
{
    g_HandleDVDAllClearCallback = cb;
}

/**
 * Offset/Address/Size: 0x1DD0 | 0x801D0B24 | size: 0xA4
 */
void nlRegHandleDVDMessageCB(const Function<void(int)>& cb)
{
    g_HandleDVDMessageCallback = cb;
}

/**
 * Offset/Address/Size: 0x1E74 | 0x801D0BC8 | size: 0xC
 */
AsyncToVirMemBufferLoad::AsyncToVirMemBufferLoad()
{
    numChunksLeft = 0;
}

/**
 * Offset/Address/Size: 0x0 | 0x801D0BD4 | size: 0x90
 */
TDEVChunkFile::~TDEVChunkFile()
{
    fclose(m_pFile);
    m_pFile = NULL;
}

/**
 * Offset/Address/Size: 0x90 | 0x801D0C64 | size: 0x8C
 */
static inline unsigned int FileSize_helper(_FILE* pFile)
{
    unsigned long uPos = ftell(pFile);
    fseek(pFile, 0, 2);
    unsigned long uSize = ftell(pFile);
    fseek(pFile, uPos, 0);
    return uSize;
}

u32 TDEVChunkFile::FileSize(unsigned int* pSize)
{
    unsigned long Size = FileSize_helper(m_pFile);
    if (pSize != NULL)
    {
        *pSize = Size;
    }
    return Size;
}

/**
 * Offset/Address/Size: 0x11C | 0x801D0CF0 | size: 0x8
 */
u32 TDEVChunkFile::GetDiscPosition()
{
    return 0;
}

/**
 * Offset/Address/Size: 0x184 | 0x801D0D58 | size: 0x88
 */
DolphinFile::~DolphinFile()
{
    DVDClose(&m_fileInfo);
}

/**
 * Offset/Address/Size: 0x20C | 0x801D0DE0 | size: 0x14
 */
u32 DolphinFile::FileSize(unsigned int* size)
{
    u32 s = m_fileInfo.length;
    if (size != NULL)
    {
        *size = s;
    }
    return s;
}

/**
 * Offset/Address/Size: 0x220 | 0x801D0DF4 | size: 0x24
 */
s32 DolphinFile::GetReadStatus()
{
    return DVDGetCommandBlockStatus(&m_fileInfo.cb);
}

/**
 * Offset/Address/Size: 0x244 | 0x801D0E18 | size: 0x2C
 */
void DolphinFile::ReadAsync(void* addr, unsigned long length, unsigned long offset)
{
    DVDReadAsyncPrio(&m_fileInfo, addr, (s32)length, (s32)offset, 0, 2);
}

/**
 * Offset/Address/Size: 0x270 | 0x801D0E44 | size: 0x8
 */
u32 DolphinFile::GetDiscPosition()
{
    return m_fileInfo.startAddr; // 0x3c
}

/**
 * Offset/Address/Size: 0x0 | 0x801D0E4C | size: 0x38
 */
template <>
AsyncEntry* nlDLRingRemoveStart<AsyncEntry>(AsyncEntry** current)
{
    AsyncEntry* temp_r31;
    temp_r31 = (*current)->next;
    nlDLRingRemove<AsyncEntry>(current, temp_r31);
    return temp_r31;
}

/**
 * Offset/Address/Size: 0x38 | 0x801D0E84 | size: 0x18
 */
template <>
AsyncEntry* nlDLRingGetStart<AsyncEntry>(AsyncEntry* current)
{
    if (current == NULL)
    {
        return NULL;
    }
    return current->next;
}

/**
 * Offset/Address/Size: 0x50 | 0x801D0E9C | size: 0x44
 */
template <>
void nlDLRingRemove<AsyncEntry>(AsyncEntry** head, AsyncEntry* current)
{
    AsyncEntry* tmp_node = current->next;

    if (tmp_node == current)
    {
        *head = NULL;
        return;
    }

    current->prev->next = tmp_node;
    current->next->prev = current->prev;

    if (*head == current)
    {
        *head = current->prev;
    }
}

/**
 * Offset/Address/Size: 0x94 | 0x801D0EE0 | size: 0x3C
 */
template <>
void nlDLRingAddEnd<AsyncEntry>(AsyncEntry** head, AsyncEntry* newNode)
{
    nlDLRingAddStart<AsyncEntry>(head, newNode);
    *head = newNode;
}

/**
 * Offset/Address/Size: 0xD0 | 0x801D0F1C | size: 0x38
 */
template <>
void nlDLRingAddStart<AsyncEntry>(AsyncEntry** head, AsyncEntry* newNode)
{
    AsyncEntry* temp;

    temp = *head;
    if (temp == NULL)
    {
        *head = newNode;
        newNode->next = newNode;
        newNode->prev = newNode;
        return;
    }

    temp->next->prev = newNode;
    newNode->next = temp->next;
    newNode->prev = temp;
    temp->next = newNode;
}

/**
 * Offset/Address/Size: 0x0 | 0x801D0F54 | size: 0x20
 */
// nlRingIsEnd<AsyncEntry>(AsyncEntry*, AsyncEntry*)
// {
// }

/**
 * Offset/Address/Size: 0x20 | 0x801D0F74 | size: 0xCC
 */
// 0x8028D538..0x8028D53C | size: 0x4
// {
// }
