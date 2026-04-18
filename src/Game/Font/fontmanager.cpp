#include "Game/Font/fontmanager.h"
#include "NL/nlBundleFile.h"
#include "NL/nlMemory.h"
#include "NL/nlString.h"
#include "NL/gl/glTexture.h"
#include "NL/nlBundleFile.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x80209CC8 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<nlFont*>, DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>>(DLListEntry<nlFont*>*, DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>*, void (DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>::*)(DLListEntry<nlFont*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xB0 | 0x80209C90 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<nlFont*>>(DLListEntry<nlFont*>**, DLListEntry<nlFont*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x74 | 0x80209C54 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<nlFont*>>(DLListEntry<nlFont*>**, DLListEntry<nlFont*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x80209C3C | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<nlFont*>>(DLListEntry<nlFont*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x80209C1C | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<nlFont*>>(DLListEntry<nlFont*>*, DLListEntry<nlFont*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80209BE0 | size: 0x3C
 */
template void nlWalkDLRing<DLListEntry<nlFont*>, DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*> > > >(
    DLListEntry<nlFont*>* head,
    DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*> > >* callback,
    void (DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*> > >::*callbackFunc)(DLListEntry<nlFont*>*));

// /**
//  * Offset/Address/Size: 0x0 | 0x80209BD0 | size: 0x10
//  */
// void DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>::DeleteEntry(DLListEntry<nlFont*>*)
// {
// }

template <>
nlDLListSlotPool<nlFont*>::nlDLListSlotPool()
{
    this->m_Head = NULL;
    this->m_Allocator.m_Initial = 8;
    SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&this->m_Allocator, sizeof(DLListEntry<nlFont*>));
    this->m_Allocator.m_Delta = 0;
}

/**
 * Offset/Address/Size: 0x4CC | 0x80209B60 | size: 0x70
 * TODO: 96.25% match - remaining mismatch is r30/r31 nonvolatile register
 * allocation in the inlined m_fonts slot-pool constructor path.
 */
FontManager::FontManager()
{
}

/**
 * Offset/Address/Size: 0x374 | 0x80209A08 | size: 0x158
 * TODO: 97.97% match - auto-generated m_fonts destructor path still lowers to
 * nlWalkRing with @106/@165 callback constants instead of target nlWalkDLRing
 * calls that reuse @198.
 */
FontManager::~FontManager()
{
    DLListEntry<nlFont*>* head;
    DLListEntry<nlFont*>* current = nlDLRingGetStart(m_fonts.m_Head);
    head = m_fonts.m_Head;

    while (current != NULL)
    {
        delete current->m_data;

        if (nlDLRingIsEnd(head, current) || current == NULL)
        {
            current = NULL;
        }
        else
        {
            current = current->m_next;
        }
    }

    typedef DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*> > > FontListBase;
    typedef void (*WalkFn)(DLListEntry<nlFont*>*, FontListBase*, void (FontListBase::*)(DLListEntry<nlFont*>*));

    void (FontListBase::*func)(DLListEntry<nlFont*>*) = &FontListBase::DeleteEntry;
    WalkFn walk = &nlWalkDLRing<DLListEntry<nlFont*>, FontListBase>;
    walk(m_fonts.m_Head, &m_fonts, func);
    m_fonts.m_Head = NULL;
}

/**
 * Offset/Address/Size: 0x2A8 | 0x8020993C | size: 0xCC
 */
nlFont* FontManager::GetFontByHashID(unsigned long hashID)
{
    DLListEntry<nlFont*>* head;
    DLListEntry<nlFont*>* entry = nlDLRingGetStart(m_fonts.m_Head);
    head = m_fonts.m_Head;

    while (entry != NULL)
    {
        nlFont* font = entry->m_data;
        if (hashID == font->m_Metrics.FontName)
        {
            return font;
        }

        if (nlDLRingIsEnd(head, entry) || entry == NULL)
        {
            entry = NULL;
        }
        else
        {
            entry = entry->m_next;
        }
    }

    nlPrintf("FontManager: Warning, failed to find font 0x%08x\n", hashID);

    DLListEntry<nlFont*>* start = nlDLRingGetStart(m_fonts.m_Head);
    if (start == NULL)
    {
        return NULL;
    }
    return start->m_data;
}

/**
 * Offset/Address/Size: 0x0 | 0x80209694 | size: 0x2A8
 */
static inline bool LoadFontDescription(BundleFile& fileBundle, unsigned long fileHashID, const char* szFontFileName, const char* szFontName, nlFont** pNewFont)
{
    BundleFileDirectoryEntry entry;
    if (!fileBundle.GetFileInfo(fileHashID, &entry, true))
    {
        return false;
    }

    char* fileData = (char*)nlMalloc(entry.m_length, 0x20, true);
    fileBundle.ReadFile(fileHashID, fileData, entry.m_length);

    *pNewFont = new (nlMalloc(sizeof(nlFont), 0x8, false)) nlFont();
    (*pNewFont)->Load(szFontName, fileData, nlStringHash(szFontFileName));

    delete[] fileData;
    return true;
}

static inline void LoadFontTexture(BundleFile& fileBundle, unsigned long fileHashID)
{
    BundleFileDirectoryEntry entry;
    if (fileBundle.GetFileInfo(fileHashID, &entry, true))
    {
        char* textureData = (char*)nlMalloc(entry.m_length, 0x20, true);
        fileBundle.ReadFile(fileHashID, textureData, entry.m_length);
        glTextureAdd(fileHashID, textureData, entry.m_length);
        delete[] textureData;
    }
}

bool FontManager::LoadFont(const char* bundlePath, const char* fontName, const char* fontFileName)
{
    BundleFile bundleFile;
    BundleFileDirectoryEntry entry;
    char nameBuffer[0xFF];
    unsigned long hashID;
    nlFont* newFont = NULL;

    nlStrNCpy(nameBuffer, fontFileName, 0xFF);
    nlToLower(nameBuffer);

    bundleFile.Open(bundlePath);

    hashID = nlStringHash(fontName);
    if (!LoadFontDescription(bundleFile, hashID, nameBuffer, fontName, &newFont))
    {
        return false;
    }

    DLListEntry<nlFont*>* slot = NULL;
    if (m_fonts.m_Allocator.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&m_fonts.m_Allocator, sizeof(DLListEntry<nlFont*>));
    }

    if (m_fonts.m_Allocator.m_FreeList != NULL)
    {
        slot = (DLListEntry<nlFont*>*)m_fonts.m_Allocator.m_FreeList;
        m_fonts.m_Allocator.m_FreeList = m_fonts.m_Allocator.m_FreeList->m_next;
    }

    if (slot != NULL)
    {
        slot->m_next = NULL;
        slot->m_prev = NULL;
        slot->m_data = newFont;
    }

    nlDLRingAddEnd(&m_fonts.m_Head, slot);

    for (unsigned long i = 0; i < newFont->m_PageCount; i++)
    {
        hashID = newFont->m_TextureHandles[i];
        bundleFile.GetFileInfo(hashID, &entry, true);
        LoadFontTexture(bundleFile, hashID);

        if (newFont->m_TextureType == SplitFX)
        {
            hashID = newFont->m_EffectTextureHandles[i];
            bundleFile.GetFileInfo(hashID, &entry, true);
            LoadFontTexture(bundleFile, hashID);
        }
    }

    bundleFile.Close();
    return true;
}
