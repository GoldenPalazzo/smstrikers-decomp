#ifndef _NLFILEGC_H_
#define _NLFILEGC_H_

#include "NL/nlFile.h"
#include "dolphin/dvd.h"

#include "NL/nlArrayAllocator.h"

enum GCFileSystem
{
    eGC_UNKNOWN = 0,
    eGC_TDEV = 1,
    eGC_DVDOPEN = 2,
};

class GCFile;
enum eReadState
{
    eRS_ISSUE_HEAD_READ = 0,
    eRS_WAIT_HEAD_READ = 1,
    eRS_ISSUE_TAIL_READ = 2,
    eRS_WAIT_TAIL_READ = 3,
    eRS_READ_COMPLETE = 4,
};

class AsyncEntry
{
public:
    /* 0x00 */ AsyncEntry* next;
    /* 0x04 */ AsyncEntry* prev;
    /* 0x08 */ GCFile* m_pFile;
    /* 0x0C */ void (*m_pFunc)(class nlFile*, void*, unsigned int, unsigned long);
    /* 0x10 */ void* m_pBuffer;
    /* 0x14 */ unsigned long m_uSize;
    /* 0x18 */ unsigned long m_uPosition;
    /* 0x1C */ unsigned long m_uParam;
    /* 0x20 */ eReadState Phase;
    /* 0x24 */ int ReadNumBytes;
}; // total size: 0x28

class AsyncManager
{
public:
    /* 0x000 */ AsyncEntry m_asyncEntries[64]; // size 0xA00
    /* 0xA00 */ AsyncEntry* m_freeEntryList;   // size 0x4
    /* 0xA04 */ AsyncEntry* m_activeEntryList; // size 0x4
}; // total size: 0xA08

void nlReadAsyncToVirtualMemory(nlFile*, void*, int, ReadAsyncCallback, unsigned long, unsigned long, void*);
void nlAsyncLoadFileToVirtualMemory(nlFile*, int, void*, ReadAsyncCallback, unsigned long);
// void nlCancelPendingAsyncReads(nlFile*, void (*)(nlFile*, void*, unsigned int, unsigned long, LoadAsyncCallback));
void nlCancelPendingAsyncReads(nlFile* pFile, void (*callback)(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long)));
bool nlAsyncReadsPending(nlFile*);
void* nlLoadEntireFileToVirtualMemory(const char*, int*, unsigned int, void*, eAllocType);
void* nlReadToVirtualMemory(nlFile*, void*, unsigned int, unsigned int);
u32 nlGetFilePosition(nlFile*);
void nlSeek(nlFile*, unsigned int, unsigned long);
void nlReadAsync(nlFile*, void*, unsigned int, ReadAsyncCallback, unsigned long);
void nlServiceFileSystem();
void nlInitFileSystem();
unsigned char GameCubeReadBlocking(GCFile*, void*, unsigned long);
static unsigned char GameCubeReadAsync(GCFile*, ReadAsyncCallback, void*, unsigned long, unsigned long);
void UpdateReadState(AsyncEntry*);
void nlFlushFileCash();
nlFile* nlOpen(const char*);

// void nlDLRingRemoveStart<AsyncEntry>(AsyncEntry**);
// void nlDLRingGetStart<AsyncEntry>(AsyncEntry*);
// void nlDLRingRemove<AsyncEntry>(AsyncEntry**, AsyncEntry*);
// void nlDLRingAddEnd<AsyncEntry>(AsyncEntry**, AsyncEntry*);
// void nlDLRingAddStart<AsyncEntry>(AsyncEntry**, AsyncEntry*);
// void nlRingIsEnd<AsyncEntry>(AsyncEntry*, AsyncEntry*);

namespace nlFileGC
{
void AsyncToVirMemBufferCallback(nlFile*, void*, unsigned int, unsigned long);

struct AsyncToVirMemBufferLoad
{
    /* 0x00 */ int numChunksLeft;
    /* 0x04 */ unsigned long param;
    /* 0x08 */ void (*callback)(class nlFile*, void*, unsigned int, unsigned long);
    /* 0x0C */ char* target;
    /* 0x10 */ int size;

    AsyncToVirMemBufferLoad();
}; // total size: 0x14
// void AsyncToVirMemBufferLoad();
}; // namespace nlFileGC

class GCFile : public nlFile
{
public:
    GCFile()
    {
        PendingAsync.m_Count = 0;
        m_Position = 0;
    }
    virtual ~GCFile() { }
    virtual u32 FileSize(unsigned int*) = 0;
    virtual void Read(void*, unsigned int);
    virtual s32 GetReadStatus() = 0;
    virtual void ReadAsync(void*, unsigned long, unsigned long) = 0;
    virtual u32 GetDiscPosition() = 0;

    /* 0x04 */ Counter PendingAsync;
    /* 0x08 */ unsigned long m_Position;
};

struct CURRENT_READ
{
    /* 0x0 */ unsigned char* Buffer;
    /* 0x4 */ unsigned long Pos;
    /* 0x8 */ unsigned long Length;
    /* 0xC */ unsigned long AmountRead;
}; // total size: 0x10

class TDEVChunkFile : public GCFile
{
public:
    TDEVChunkFile(_FILE* fp)
        : m_pFile(fp)
    {
    }
    virtual ~TDEVChunkFile();
    virtual u32 FileSize(unsigned int*);
    virtual s32 GetReadStatus();
    virtual void ReadAsync(void*, unsigned long, unsigned long);
    virtual u32 GetDiscPosition();

    void* operator new(size_t)
    {
        nlArrayAllocator<TDEVChunkFile>* alloc = s_pAllocator;
        TDEVChunkFile* ptr = alloc->m_pFree;
        if (ptr == NULL)
        {
            ptr = NULL;
        }
        else
        {
            alloc->m_pFree = *(TDEVChunkFile**)ptr;
        }
        return ptr;
    }

    void operator delete(void* ptr)
    {
        nlArrayAllocator<TDEVChunkFile>* alloc = s_pAllocator;
        *(TDEVChunkFile**)ptr = alloc->m_pFree;
        alloc->m_pFree = (TDEVChunkFile*)ptr;
    }

    static nlArrayAllocator<TDEVChunkFile>* s_pAllocator;

    /* 0x0C */ _FILE* m_pFile;
    /* 0x10 */ CURRENT_READ m_CurrentRead;
};

class DolphinFile : public GCFile
{
public:
    DolphinFile(s32 entryNum) { DVDFastOpen(entryNum, &m_fileInfo); }
    virtual ~DolphinFile();
    virtual u32 FileSize(unsigned int*);
    virtual s32 GetReadStatus();
    virtual void ReadAsync(void*, unsigned long, unsigned long);
    virtual u32 GetDiscPosition();

    void* operator new(size_t)
    {
        nlArrayAllocator<DolphinFile>* alloc = s_pAllocator;
        DolphinFile* ptr = alloc->m_pFree;
        if (ptr == NULL)
        {
            ptr = NULL;
        }
        else
        {
            alloc->m_pFree = *(DolphinFile**)ptr;
        }
        return ptr;
    }

    void operator delete(void* ptr)
    {
        nlArrayAllocator<DolphinFile>* alloc = s_pAllocator;
        *(DolphinFile**)ptr = alloc->m_pFree;
        alloc->m_pFree = (DolphinFile*)ptr;
    }

    static nlArrayAllocator<DolphinFile>* s_pAllocator;

    /* 0x0c */ DVDFileInfo m_fileInfo;
};

#endif // _NLFILEGC_H_
