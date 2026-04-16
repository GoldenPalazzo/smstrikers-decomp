#include "NL/nlFileGC.h"
#include "NL/nlFile.h"
#include "NL/nlMemory.h"
#include "NL/nlFunction.h"
#include "FILE_POS.h"
#include "direct_io.h"
#include "types.h"
#include <string.h>

static AsyncManager* s_pAsyncManager;
static GCFileSystem fileSystem;

static Function<void(int)> g_HandleDVDMessageCallback;
static Function<void(int)> g_HandleDVDAllClearCallback;
static Function<void(int)> g_HandleDVDRetryCB;
static Function<FnVoidVoid> g_CheckForResetCB;

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

namespace nlFileGC
{
AsyncToVirMemBufferLoad asyncToVirMemBufferLoad[4];
u8 asyncToVirMemBuffer[0x4000];
} // namespace nlFileGC

/**
 * Offset/Address/Size: 0x0 | 0x801CED54 | size: 0xEC
 * TODO: 98.6% in scratch - register swap r25/r26/r27 for counter1/counter2/chunkSize.
 * Scratch limitation: nlReadAsync gets inlined from context (defined later in TU).
 * In real build nlReadAsync should NOT inline, likely resolving register allocation.
 */
void nlReadAsyncToVirtualMemory(nlFile* file, void* buffer, int size, ReadAsyncCallback callback, unsigned long param,
    unsigned long chunkSize, void* userData)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        if (nlFileGC::asyncToVirMemBufferLoad[i].numChunksLeft == 0)
        {
            unsigned int numChunks = (unsigned int)size / (unsigned int)chunkSize;
            unsigned long counter2 = i;
            unsigned long counter1 = i;
            unsigned int sz = chunkSize;
            nlFileGC::asyncToVirMemBufferLoad[i].numChunksLeft = numChunks + 1;
            nlFileGC::asyncToVirMemBufferLoad[i].param = param;
            nlFileGC::asyncToVirMemBufferLoad[i].callback = callback;
            nlFileGC::asyncToVirMemBufferLoad[i].size = size;
            int remainder = size - numChunks * chunkSize;
            nlFileGC::asyncToVirMemBufferLoad[i].target = (char*)buffer;

            int j;
            for (j = 0; j < (int)numChunks; j++)
            {
                nlReadAsync(file, userData, sz, nlFileGC::AsyncToVirMemBufferCallback, counter1);
            }

            nlReadAsync(file, userData, remainder, nlFileGC::AsyncToVirMemBufferCallback, counter2);
            return;
        }
    }
}

/**
 * Offset/Address/Size: 0xEC | 0x801CEE40 | size: 0x38
 */
void nlAsyncLoadFileToVirtualMemory(nlFile* file, int size, void* buffer, ReadAsyncCallback callback, unsigned long alignment)
{
    nlReadAsyncToVirtualMemory(file, buffer, size, callback, alignment, 0x4000, nlFileGC::asyncToVirMemBuffer);
}

namespace nlFileGC
{
/**
 * Offset/Address/Size: 0x124 | 0x801CEE78 | size: 0xAC
 */
void AsyncToVirMemBufferCallback(nlFile*, void*, unsigned int, unsigned long)
{
}
} // namespace nlFileGC

/**
 * Offset/Address/Size: 0x1D0 | 0x801CEF24 | size: 0xF4
 */
// nlCancelPendingAsyncReads(nlFile*, void (*)(nlFile*, void*, unsigned int, unsigned long, LoadAsyncCallback))
void nlCancelPendingAsyncReads(nlFile*)
{
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

/**
 * Offset/Address/Size: 0x2F8 | 0x801CF04C | size: 0x2D0
 * TODO: 97.1% match - 1 extra mr instruction (uPos scheduling through r0
 * instead of direct to r23) and remaining callee-saved register permutation
 */
void* nlLoadEntireFileToVirtualMemory(const char* fileName, int* size, unsigned int transferSize, void* target, eAllocType allocType)
{
    void* buffer = NULL;
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

        {
            unsigned int totalSize = fileSize;
            void* tempBuffer = nlMalloc(transferSize, 0x20, false);
            unsigned int offset = 0;

            while (offset < totalSize)
            {
                unsigned int amount = transferSize;
                if (totalSize - offset <= transferSize)
                {
                    amount = totalSize - offset;
                }
                nlRead(pGCFile, tempBuffer, amount);
                memcpy((char*)buffer + offset, tempBuffer, amount);
                offset += amount;
            }

            nlFree(tempBuffer);
        }
        goto alloc_done;

    alloc_fallback:
    {
        unsigned int totalFree = nlVirtualTotalFree();
        OSReport("load into virtual memory failure %d %s %d %d\n", fileSize, fileName, nlVirtualLargestBlock(), totalFree);
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

/**
 * Offset/Address/Size: 0x72C | 0x801CF480 | size: 0x27C
 * TODO: 90.0% match in scratch - remaining diffs are register allocation in
 * both branches and MWCC switch lowering (jump-table target still emitted as
 * compare-chain here).
 */
void nlServiceFileSystem()
{
    extern void glxLoadSaveState(void);
    extern void glxLoadRestoreState(void);

    extern Function<void(int)> g_HandleDVDMessageCallback;
    extern Function<void(int)> g_HandleDVDAllClearCallback;
    extern Function<void(int)> g_HandleDVDRetryCB;
    extern Function<FnVoidVoid> g_CheckForResetCB;

    AsyncManager* manager = s_pAsyncManager;
    AsyncEntry* entry = manager->m_activeEntryList;
    s32 driveStatus;
    bool loadedSaveState;

    if (entry != NULL)
    {
        entry = entry->next;

        if ((OSGetConsoleType() & 0x20000000) != 0)
        {
            OSYieldThread();
        }

        if (((bool (*)(AsyncEntry*))UpdateReadState)(entry))
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

    loadedSaveState = false;

    while (true)
    {
        driveStatus = DVDGetDriveStatus();

        u32 statusPlusOne = (u32)(driveStatus + 1);
        if (statusPlusOne <= 12)
        {
            switch (statusPlusOne)
            {
            case DVD_STATE_FATAL_ERROR + 1:
            case DVD_STATE_END + 1:
            case DVD_STATE_BUSY + 1:
            case DVD_STATE_WAITING + 1:
            case DVD_STATE_PAUSING + 1:
            case DVD_STATE_IGNORED + 1:
            case DVD_STATE_CANCELED + 1:
                break;

            case DVD_STATE_COVER_CLOSED + 1:
            case DVD_STATE_NO_DISK + 1:
            case DVD_STATE_COVER_OPEN + 1:
            case DVD_STATE_WRONG_DISK + 1:
            case DVD_STATE_MOTOR_STOPPED + 1:
                if (!loadedSaveState)
                {
                    glxLoadSaveState();
                }

                if (g_HandleDVDMessageCallback.mTag == 1)
                {
                    g_HandleDVDMessageCallback.mFreeFunction(driveStatus);
                }
                else
                {
                    (*g_HandleDVDMessageCallback.mFunctor)(driveStatus);
                }

                loadedSaveState = true;

                while (driveStatus == DVDGetDriveStatus())
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

            case DVD_STATE_RETRY + 1:
                if (loadedSaveState)
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
            }
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
 * TODO: 89.95% match - register off-by-1 (r4-r10 vs r5-r11) from missing
 * slwi/add residual pointer setup (compiler internal), AsyncManager r30 vs r0
 * for NULL stores
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
            u8* p = (u8*)pFile;

            pAlloc->m_pFree = pFile;

            for (i = 0; i < 31; i++)
            {
                *(TDEVChunkFile**)p = (TDEVChunkFile*)(p + 0x20);
                p += 0x20;
            }

            *(TDEVChunkFile**)((u8*)pFile + 0x3E0) = NULL;
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
            u8* p = (u8*)pFile;

            pAlloc->m_pFree = pFile;

            for (i = 0; i < 31; i++)
            {
                *(DolphinFile**)p = (DolphinFile*)(p + 0x48);
                p += 0x48;
            }

            *(DolphinFile**)((u8*)pFile + 0x8B8) = NULL;
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

            pManager->m_freeEntryList = NULL;
            pManager->m_activeEntryList = NULL;

            for (i = 0; i < 64; i++)
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
 * TODO: 95.0% match in scratch - remaining diffs are in DVD status dispatch:
 * jump-table `bctr` is still emitted as compare/range checks, plus related
 * status temp register allocation (r0/r3) around `statusPlusOne`.
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

        if (((bool (*)(AsyncEntry*))UpdateReadState)(entry))
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
            if (statusPlusOne <= 12)
            {
                if ((u32)(statusPlusOne - 4) <= 4)
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
                }
                else if (statusPlusOne == 12)
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
                }
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
 * TODO: 93.8% match in scratch - remaining diffs are callback/buffer register
 * assignment, loadedSaveState register flow, and DVD status jump-table
 * lowering.
 */
static unsigned char GameCubeReadAsync(GCFile* pFile, ReadAsyncCallback callback, void* pBuffer, unsigned long uSize, unsigned long uParam)
{
    extern void glxLoadSaveState(void);
    extern void glxLoadRestoreState(void);

    extern Function<void(int)> g_HandleDVDMessageCallback;
    extern Function<void(int)> g_HandleDVDAllClearCallback;
    extern Function<void(int)> g_HandleDVDRetryCB;
    extern Function<FnVoidVoid> g_CheckForResetCB;

    u8 loadedSaveState = 0;
    AsyncManager* manager = s_pAsyncManager;

    if (manager->m_freeEntryList != NULL)
    {
        AsyncEntry* entry = nlDLRingRemoveStartAsyncEntry(&manager->m_freeEntryList);

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
            loadedSaveState = (u8)(((u32)(1 - fs) | (u32)(fs - 1)) >> 31);
        }

        nlDLRingAddEnd<AsyncEntry>(&manager->m_activeEntryList, entry);

        if (loadedSaveState)
        {
            entry = manager->m_activeEntryList;

            if (entry != NULL)
            {
                entry = entry->next;

                if ((OSGetConsoleType() & 0x20000000) != 0)
                {
                    OSYieldThread();
                }

                if (((bool (*)(AsyncEntry*))UpdateReadState)(entry))
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
                loadedSaveState = 0;

                while (true)
                {
                    s32 driveStatus = DVDGetDriveStatus();
                    u32 statusPlusOne = (u32)(driveStatus + 1);

                    if (statusPlusOne <= 12)
                    {
                        if ((u32)(statusPlusOne - 4) <= 4)
                        {
                            if (!loadedSaveState)
                            {
                                glxLoadSaveState();
                            }

                            if (g_HandleDVDMessageCallback.mTag == 1)
                            {
                                g_HandleDVDMessageCallback.mFreeFunction(driveStatus);
                            }
                            else
                            {
                                (*g_HandleDVDMessageCallback.mFunctor)(driveStatus);
                            }

                            loadedSaveState = 1;

                            while (driveStatus == DVDGetDriveStatus())
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
                        else if (statusPlusOne == 12)
                        {
                            if (loadedSaveState)
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
                        }
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
void UpdateReadState(AsyncEntry*)
{
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
 * TODO: 93.8% match - register allocation still differs in fread setup
 *   (amountRead/length/remainingBytes in r3/r6 vs target r7/r3/r6), plus
 *   post-fread compare/flag path keeps currentAmount/isComplete in r0/r4
 *   instead of target r4/r0; final shift emits rlwinm instead of srwi.
 */
s32 TDEVChunkFile::GetReadStatus()
{
    fseek(m_pFile, m_CurrentRead.Pos + m_CurrentRead.AmountRead, 0);

    u32 length = m_CurrentRead.Length;
    u32 amountRead = m_CurrentRead.AmountRead;
    u32 remainingBytes = length;
    remainingBytes -= amountRead;
    u8* dest = m_CurrentRead.Buffer + amountRead;

    u32 bytesRead = fread(dest, 1, (remainingBytes <= 0x3000U) ? remainingBytes : 0x3000U, m_pFile);
    u32 nextAmount = m_CurrentRead.AmountRead + bytesRead;
    m_CurrentRead.AmountRead = nextAmount;
    bool isComplete = false;
    u32 currentAmount = m_CurrentRead.AmountRead;
    u32 currentLength = m_CurrentRead.Length;

    if ((currentAmount == currentLength) || ((currentLength == 0x20U) && (currentAmount != 0U)))
    {
        isComplete = true;
    }
    return !isComplete;
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

namespace nlFileGC
{
/**
 * Offset/Address/Size: 0x1E74 | 0x801D0BC8 | size: 0xC
 */
void AsyncToVirMemBufferLoad()
{
}
} // namespace nlFileGC

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
// nlDLRingRemoveStart<AsyncEntry>(AsyncEntry**)
// {
// }

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
// nlDLRingRemove<AsyncEntry>(AsyncEntry**, AsyncEntry*)
// {
// }

/**
 * Offset/Address/Size: 0x94 | 0x801D0EE0 | size: 0x3C
 */
// nlDLRingAddEnd<AsyncEntry>(AsyncEntry**, AsyncEntry*)
// {
// }

/**
 * Offset/Address/Size: 0xD0 | 0x801D0F1C | size: 0x38
 */
// nlDLRingAddStart<AsyncEntry>(AsyncEntry**, AsyncEntry*)
// {
// }

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
