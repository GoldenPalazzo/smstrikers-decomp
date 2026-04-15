#include "Game/AnimInventory.h"

#include "NL/nlString.h"
#include "NL/nlMemory.h"
#include "NL/nlFileGC.h"

SAnimContainer* g_pDefaultSAnimInventory = nullptr;

/**
 * Offset/Address/Size: 0x438 | 0x800073B4 | size: 0xA0
 */
cAnimInventory::cAnimInventory(const AnimProperties* props, int count)
{
    m_count = count;
    m_cont = 0;
    m_anims = 0;
    m_props = props;

    SAnimContainer* cont = (SAnimContainer*)nlMalloc(0x1C, 8, 0);
    if (cont)
    {
        cont->animHead = 0;
        cont->animTail = 0;
        cont->fileHead = 0;
        cont->fileTail = 0;
        cont->animCount = 0;
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
    typedef ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*> > > SAnimListBase;
    typedef ListContainerBase<char*, NewAdapter<ListEntry<char*> > > FileListBase;

    SAnimContainer* c = m_cont;
    if (c != 0)
    {
        ListEntry<cSAnim*>* anim = c->animHead;
        while (anim != 0)
        {
            anim->data->Destroy();
            anim = anim->next;
        }

        nlWalkList(c->animHead, (SAnimListBase*)c, &SAnimListBase::DeleteEntry);
        c->animHead = 0;
        c->animTail = 0;

        ListEntry<char*>** tail = &c->fileTail;
        ListEntry<char*>** head = &c->fileHead;
        while (c->fileHead != 0)
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

        c->animCount = 0;
        if (&c->unused0C != 0)
        {
            if (&c->unused0C != 0)
            {
                nlWalkList(c->fileHead, (FileListBase*)&c->unused0C, &FileListBase::DeleteEntry);
                c->fileHead = 0;
                c->fileTail = 0;
            }
        }

        if (c != 0)
        {
            if (c != 0)
            {
                nlWalkList(c->animHead, (SAnimListBase*)c, &SAnimListBase::DeleteEntry);
                c->animHead = 0;
                c->animTail = 0;
            }
        }
        delete c;
    }

    delete[] m_anims;
    g_pDefaultSAnimInventory = 0;
}

/**
 * Offset/Address/Size: 0x88 | 0x80007004 | size: 0x214
 * TODO: 91.2% match - callee-saved register allocation differs (params r28/r27
 * vs target r31/r30), search loop pFound placement, pMem increment codegen
 */
void cAnimInventory::AddAnimBundle(const char* szFilename)
{
    int len;
    char* pMem = (char*)nlLoadEntireFileToVirtualMemory(szFilename, &len, 0x10000, 0, AllocateStart);
    int bundleLen = len;
    SAnimContainer* inv = m_cont;

    ListEntry<char*>* pFileEntry = (ListEntry<char*>*)nlMalloc(8, 8, 0);
    if (pFileEntry != 0)
    {
        pFileEntry->next = 0;
        pFileEntry->data = pMem;
    }
    nlListAddStart<ListEntry<char*> >(&inv->fileHead, pFileEntry, &inv->fileTail);

    char* end = pMem + bundleLen;
    while (pMem != end)
    {
        if ((((nlChunk*)pMem)->m_ID & 0xFFFFFF00) == 0x80017000)
        {
            cSAnim* pAnim = cSAnim::Initialize((nlChunk*)pMem);
            ListEntry<cSAnim*>* pAnimEntry = (ListEntry<cSAnim*>*)nlMalloc(8, 8, 0);
            if (pAnimEntry != 0)
            {
                pAnimEntry->next = 0;
                pAnimEntry->data = pAnim;
            }
            nlListAddStart<ListEntry<cSAnim*> >(&inv->animHead, pAnimEntry, &inv->animTail);
            inv->animCount++;
        }
        else
        {
            nlPrintf("Warning: inventory encountered an unknown chunk type\n");
        }

        int chunkSize = ((nlChunk*)pMem)->m_Size;
        pMem += chunkSize + 8;
    }

    int propOffset = 0;
    int i = 0;
    int animOffset = propOffset;
    while (i < m_count)
    {
        SAnimContainer* pInv = m_cont;
        unsigned int hash = nlStringHash(*(const char**)((char*)m_props + propOffset + 4));
        ListEntry<cSAnim*>* pList = pInv->animHead;
        cSAnim* pFound = 0;

        while (pList != 0)
        {
            cSAnim* pEntryAnim = pList->data;
            if (hash == pEntryAnim->m_uHashID)
            {
                pFound = pEntryAnim;
                break;
            }
            pList = pList->next;
        }

        *(cSAnim**)((char*)m_anims + animOffset) = pFound;
        if (*(cSAnim**)((char*)m_anims + animOffset) == 0)
        {
            nlPrintf("Warning! Could not find \"%s\" in bundle \"%s\"\n",
                *(const char**)((char*)m_props + propOffset + 4),
                szFilename);
            SAnimContainer* pDefaultInv = g_pDefaultSAnimInventory;
            hash = nlStringHash(*(const char**)((char*)m_props + propOffset + 4));
            pList = pDefaultInv->animHead;
            pFound = 0;
            while (pList != 0)
            {
                cSAnim* pEntryAnim = pList->data;
                if (hash == pEntryAnim->m_uHashID)
                {
                    pFound = pEntryAnim;
                    break;
                }
                pList = pList->next;
            }

            *(cSAnim**)((char*)m_anims + animOffset) = pFound;
            if (*(cSAnim**)((char*)m_anims + animOffset) == 0)
            {
                *(cSAnim**)((char*)m_anims + animOffset) = m_anims[0];
            }
        }

        propOffset += sizeof(AnimProperties);
        animOffset += sizeof(cSAnim*);
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
//  * Offset/Address/Size: 0x0 | 0x8000749C | size: 0x54
//  */
// void nlPrintf(const char*, ...)
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
