#ifndef _NLBUNDLEFILE_H_
#define _NLBUNDLEFILE_H_

extern int nlPrintf(const char*, ...);

#include "NL/nlFileGC.h"

typedef void (*FileReadAsyncCallback)(void*, unsigned long, unsigned long);

typedef struct
{
    /* 0x00 */ u32 m_blockSize;
    /* 0x04 */ u32 m_entryCount;
    /* 0x08 */ u32 m_unk_0x08;
    /* 0x0C */ u32 m_unk_0x0C;
} BundleFileDirectoryHeader;

typedef struct
{
    /* 0x00 */ u32 m_hash;
    /* 0x04 */ u32 m_blockNumber;
    /* 0x08 */ u32 m_length;
} BundleFileDirectoryEntry, *BundleFileDirectoryEntryPtr;

static void cbFileReadAsyncCallback(nlFile*, void*, unsigned int, unsigned long);

class BundleFile
{
public:
    void ReadFileAsync(unsigned long, void*, unsigned long, FileReadAsyncCallback callback, unsigned long);
    void ReadFileAsync(const char*, void*, unsigned long, FileReadAsyncCallback callback, unsigned long);
    void LoadFile(const char*, void*);
    void ReadFileByIndex(unsigned long, void*, unsigned long);
    void ReadFile(unsigned long, void*, unsigned long);
    void ReadFile(const char*, void*, unsigned long);
    bool GetFileInfoByIndex(unsigned long, BundleFileDirectoryEntry*);
    bool GetFileInfo(unsigned long, BundleFileDirectoryEntry*, bool);
    bool GetFileInfo(const char*, BundleFileDirectoryEntry*, bool);
    void Close();
    bool Open(const char*);

    ~BundleFile();
    BundleFile();

public:
    /* 0x00 */ nlFile* m_file;
    /* 0x04 */ u32 m_unk_0x04;
    /* 0x08 */ u32 m_unk_0x08;
    /* 0x0C */ FileReadAsyncCallback m_readAsyncCallback;
    /* 0x10 */ u32 m_unk_0x10;
    /* 0x14 */ BundleFileDirectoryHeader* m_bundleHeader;
    /* 0x18 */ BundleFileDirectoryEntry* m_bundleEntries; // most probably another struct

    // Inline method to find hash index
    inline u32 FindHashIndex(u32 hash) const
    {
        for (u32 i = 0; i < m_bundleHeader->m_entryCount; i++)
        {
            if (hash == m_bundleEntries[i].m_hash)
            {
                return i;
            }
        }
        nlPrintf("ERROR: Failed to find file with hash ID: %d\n", hash);
        return -1U; // Not found
    }

    inline u32 FindHashIndex(u32 hash, bool printError) const
    {
        for (u32 i = 0; i < m_bundleHeader->m_entryCount; i++)
        {
            if (hash == m_bundleEntries[i].m_hash)
            {
                return i;
            }
        }
        if (printError)
        {
            nlPrintf("ERROR: Failed to find file with hash ID: %d\n", hash);
        }
        return -1U; // Not found
    }
};

#endif // _NLBUNDLEFILE_H_
