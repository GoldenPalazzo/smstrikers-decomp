#ifndef _FONTMANAGER_H_
#define _FONTMANAGER_H_

#include "NL/nlSingleton.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlFont.h"

// void nlDLRingIsEnd<DLListEntry<nlFont*>>(DLListEntry<nlFont*>*, DLListEntry<nlFont*>*);
// void nlDLRingGetStart<DLListEntry<nlFont*>>(DLListEntry<nlFont*>*);
// void nlDLRingAddEnd<DLListEntry<nlFont*>>(DLListEntry<nlFont*>**, DLListEntry<nlFont*>*);
// void nlDLRingAddStart<DLListEntry<nlFont*>>(DLListEntry<nlFont*>**, DLListEntry<nlFont*>*);

class FontManager : public nlSingleton<FontManager>
{
public:
    FontManager();
    virtual ~FontManager();
    nlFont* GetFontByHashID(unsigned long hashID);
    bool LoadFont(const char* bundlePath, const char* fontName, const char* fontFileName);

    /* 0x4 */ nlDLListSlotPool<nlFont*> m_fonts;
};

// class DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<nlFont*>*);
// };

// class nlWalkDLRing<DLListEntry<nlFont*>, DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>>(DLListEntry<nlFont*>*, DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>*, void (DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>
// {
// public:
//     void *)(DLListEntry<nlFont*>*));
// };

// class nlWalkRing<DLListEntry<nlFont*>, DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>>(DLListEntry<nlFont*>*, DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>*, void (DLListContainerBase<nlFont*, BasicSlotPool<DLListEntry<nlFont*>>>
// {
// public:
//     void *)(DLListEntry<nlFont*>*));
// };

#endif // _FONTMANAGER_H_
