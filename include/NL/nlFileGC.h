#ifndef _NLFILEGC_H_
#define _NLFILEGC_H_

#include "NL/nlFile.h"
#include "NL/nlFunction.h"
#include "dolphin/dvd.h"

#include "NL/nlArrayAllocator.h"

enum GCFileSystem
{
    eGC_UNKNOWN = 0,
    eGC_TDEV = 1,
    eGC_DVDOPEN = 2,
};

class GCFile;

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
void nlFlushFileCash();
nlFile* nlOpen(const char*);
void nlRegHandleDVDMessageCB(const Function<void(int)>&);
void nlRegHandleDVDAllClearCB(const Function<void(int)>&);
void nlRegCheckForResetFromFSCB(const Function<FnVoidVoid>&);

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
