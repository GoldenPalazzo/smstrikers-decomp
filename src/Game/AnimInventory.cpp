#include "Game/AnimInventory.h"

#include "NL/nlString.h"
#include "NL/nlMemory.h"
#include "NL/nlFileGC.h"
#include "NL/nlWare.h"

cInventory<cSAnim>* g_pDefaultSAnimInventory = nullptr;

typedef ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*> > > SAnimListBase;
typedef ListContainerBase<char*, NewAdapter<ListEntry<char*> > > FileListBase;

static inline void ClearAnimList(cInventory<cSAnim>* c)
{
    nlWalkList(c->m_lItemList.m_Head, (SAnimListBase*)c, &SAnimListBase::DeleteEntry);
    c->m_lItemList.m_Head = 0;
    c->m_lItemList.m_Tail = 0;
}

static inline void ClearFileList(cInventory<cSAnim>* c)
{
    nlWalkList(c->m_lMemList.m_Head, (FileListBase*)&c->m_lMemList, &FileListBase::DeleteEntry);
    c->m_lMemList.m_Head = 0;
    c->m_lMemList.m_Tail = 0;
}

static inline cSAnim* FindAnim(ListEntry<cSAnim*>* pEntry, unsigned int hash)
{
    for (; pEntry != 0; pEntry = pEntry->next)
    {
        if (hash == pEntry->data->m_uHashID)
            return pEntry->data;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x438 | 0x800073B4 | size: 0xA0
 */
cAnimInventory::cAnimInventory(const AnimProperties* props, int count)
{
    m_count = count;
    m_cont = 0;
    m_anims = 0;
    m_props = props;

    cInventory<cSAnim>* cont = (cInventory<cSAnim>*)nlMalloc(0x1C, 8, 0);
    if (cont)
    {
        cont->m_lItemList.m_Head = 0;
        cont->m_lItemList.m_Tail = 0;
        cont->m_lMemList.m_Head = 0;
        cont->m_lMemList.m_Tail = 0;
        cont->m_nItemCount = 0;
    }
    m_cont = cont;

    if (g_pDefaultSAnimInventory == 0)
        g_pDefaultSAnimInventory = m_cont;

    m_anims = (cSAnim**)nlMalloc((unsigned long)(m_count << 2), 8, 0);
}

/**
 * Offset/Address/Size: 0x29C | 0x80007218 | size: 0x19C
 */
cAnimInventory::~cAnimInventory()
{
    cInventory<cSAnim>* c = m_cont;
    if (c != 0)
    {
        ListEntry<cSAnim*>* anim = c->m_lItemList.m_Head;
        while (anim != 0)
        {
            anim->data->Destroy();
            anim = anim->next;
        }

        ClearAnimList(c);

        ListEntry<char*>** tail = &c->m_lMemList.m_Tail;
        ListEntry<char*>** head = &c->m_lMemList.m_Head;
        while (c->m_lMemList.m_Head != 0)
        {
            ListEntry<char*>* entry = nlListRemoveStart<ListEntry<char*> >(head, tail);
            char* filename;
            if (&filename != 0)
            {
                filename = entry->data;
            }
            delete entry;
            delete filename;
        }

        c->m_nItemCount = 0;
        if (&c->m_lMemList != 0)
        {
            if (&c->m_lMemList != 0)
            {
                ClearFileList(c);
            }
        }

        if (c != 0)
        {
            if (c != 0)
            {
                ClearAnimList(c);
            }
        }
        ::operator delete(c);
    }

    delete[] m_anims;
    g_pDefaultSAnimInventory = 0;
}

/**
 * Offset/Address/Size: 0x88 | 0x80007004 | size: 0x214
 * TODO: 98.27% match - instruction stream is byte-exact; only the callee-saved
 * register assignment differs (this/filename get r27/r28 instead of r30/r31, with
 * pMem/len shifted to r30/r31 instead of r28/r27)
 */
void cAnimInventory::AddAnimBundle(const char* szFilename)
{
    int i;
    int len;
    cInventory<cSAnim>* inv;
    char* pMem = (char*)nlLoadEntireFileToVirtualMemory(szFilename, &len, 0x10000, 0, AllocateStart);
    int bundleLen = len;
    inv = m_cont;

    ListEntry<char*>* pFileEntry = (ListEntry<char*>*)nlMalloc(8, 8, 0);
    if (pFileEntry != 0)
    {
        pFileEntry->next = 0;
        pFileEntry->data = pMem;
    }
    nlListAddStart<ListEntry<char*> >(&inv->m_lMemList.m_Head, pFileEntry, &inv->m_lMemList.m_Tail);

    char* end = pMem + bundleLen;
    while (pMem != end)
    {
        if ((((nlChunk*)pMem)->m_ID & 0x80FFFFFF) == 0x80017000)
        {
            cSAnim* pAnim = cSAnim::Initialize((nlChunk*)pMem);
            ListEntry<cSAnim*>* pAnimEntry = (ListEntry<cSAnim*>*)nlMalloc(8, 8, 0);
            if (pAnimEntry != 0)
            {
                pAnimEntry->next = 0;
                pAnimEntry->data = pAnim;
            }
            nlListAddStart<ListEntry<cSAnim*> >(&inv->m_lItemList.m_Head, pAnimEntry, &inv->m_lItemList.m_Tail);
            inv->m_nItemCount++;
        }
        else
        {
            nlPrintf("Warning: inventory encountered an unknown chunk type\n");
        }

        pMem = (char*)(((nlChunk*)pMem)->m_Size + pMem + 8);
    }

    i = 0;
    while (i < m_count)
    {
        cInventory<cSAnim>* pInv = m_cont;
        unsigned int hash = nlStringHash(m_props[i].name);
        cSAnim* pFound = FindAnim(pInv->m_lItemList.m_Head, hash);

        m_anims[i] = pFound;
        if (m_anims[i] == 0)
        {
            nlPrintf("Warning! Could not find \"%s\" in bundle \"%s\"\n",
                m_props[i].name,
                szFilename);
            cInventory<cSAnim>* pDefaultInv = g_pDefaultSAnimInventory;
            hash = nlStringHash(m_props[i].name);
            pFound = FindAnim(pDefaultInv->m_lItemList.m_Head, hash);

            m_anims[i] = pFound;
            if (m_anims[i] == 0)
            {
                m_anims[i] = m_anims[0];
            }
        }

        i++;
    }
}

/**
 * Offset/Address/Size: 0x78 | 0x80006FF4 | size: 0x10
 */
cSAnim* cAnimInventory::GetAnim(int i)
{
    return m_anims[i];
}

/**
 * Offset/Address/Size: 0x64 | 0x80006FE0 | size: 0x14
 */
ePlayMode cAnimInventory::GetPlayMode(int i)
{
    return m_props[i].playMode;
}

/**
 * Offset/Address/Size: 0x50 | 0x80006FCC | size: 0x14
 */
float cAnimInventory::GetBlendTime(int i)
{
    return m_props[i].blendTime;
}

/**
 * Offset/Address/Size: 0x3C | 0x80006FB8 | size: 0x14
 */
bool cAnimInventory::GetMirrored(int i)
{
    return m_props[i].mirrored;
}

/**
 * Offset/Address/Size: 0x28 | 0x80006FA4 | size: 0x14
 */
int cAnimInventory::GetBallRotationMode(int i)
{
    return m_props[i].ballRotMode;
}

/**
 * Offset/Address/Size: 0x14 | 0x80006F90 | size: 0x14
 */
int cAnimInventory::GetEndPhase(int i)
{
    return m_props[i].endPhase;
}

/**
 * Offset/Address/Size: 0x0 | 0x80006F7C | size: 0x14
 */
u8 cAnimInventory::GetMatchCharacterSpeed(int i)
{
    return m_props[i].matchCharSpd;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80007454 | size: 0x24
//  */
// void ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*>>>::DeleteEntry(ListEntry<cSAnim*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x24 | 0x80007478 | size: 0x24
//  */
// void ListContainerBase<char*, NewAdapter<ListEntry<char*>>>::DeleteEntry(ListEntry<char*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800074F0 | size: 0x68
//  */
// void nlWalkList<ListEntry<char*>, ListContainerBase<char*, NewAdapter<ListEntry<char*>>>>(ListEntry<char*>*, ListContainerBase<char*,
// NewAdapter<ListEntry<char*>>>*, void (ListContainerBase<char*, NewAdapter<ListEntry<char*>>>::*)(ListEntry<char*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x80007558 | size: 0x68
//  */
// void nlWalkList<ListEntry<cSAnim*>, ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*>>>>(ListEntry<cSAnim*>*,
// ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*>>>*, void (ListContainerBase<cSAnim*,
// NewAdapter<ListEntry<cSAnim*>>>::*)(ListEntry<cSAnim*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xD0 | 0x800075C0 | size: 0x44
//  */
// void nlListRemoveStart<ListEntry<char*>>(ListEntry<char*>**, ListEntry<char*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x114 | 0x80007604 | size: 0x28
//  */
// void nlListAddStart<ListEntry<cSAnim*>>(ListEntry<cSAnim*>**, ListEntry<cSAnim*>*, ListEntry<cSAnim*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x13C | 0x8000762C | size: 0x28
//  */
// void nlListAddStart<ListEntry<char*>>(ListEntry<char*>**, ListEntry<char*>*, ListEntry<char*>**)
// {
// }
