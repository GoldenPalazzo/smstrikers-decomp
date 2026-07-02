#ifndef _TIMEREGIONS_H_
#define _TIMEREGIONS_H_

#include "NL/nlList.h"
#include "NL/nlMemory.h"
#include "Game/GameTweaks.h"

void DestroyTimeRegions();
void InitializeTimeRegions();
void WriteFrameRateStatsToFile();

class TimeRegion
{
public:
    static nlListContainer<TimeRegion*> sTimeRegionList;

    TimeRegion(const char* pName, bool (*pConditionFunc)())
        : m_pName(pName)
        , m_pConditionFunc(pConditionFunc)
        , m_fThreshold(0.0f)
        , m_unk10(0)
        , m_unk14(0)
    {
        ListEntry<TimeRegion*>* entry = (ListEntry<TimeRegion*>*)nlMalloc(8, 8, false);
        if (entry != nullptr)
        {
            entry->next = nullptr;
            entry->entry = this;
        }
        nlListAddEnd<ListEntry<TimeRegion*> >(&sTimeRegionList.m_Head, &sTimeRegionList.m_Tail, entry);
    }

    /**
     * Offset/Address/Size: 0x888 | 0x801626A8 | size: 0x100
     * TODO: 96.25% match - r4/r5 register swap for currentEntry, hoisted newHead=NULL init
     */
    virtual ~TimeRegion()
    {
        ListEntry<TimeRegion*>* currentEntry = TimeRegion::sTimeRegionList.m_Head;
        if (currentEntry != NULL)
        {
            if (currentEntry->entry == this)
            {
                ListEntry<TimeRegion*>* newHead = NULL;
                if (currentEntry == TimeRegion::sTimeRegionList.m_Tail)
                {
                    TimeRegion::sTimeRegionList.m_Tail = newHead;
                }
                else
                {
                    newHead = currentEntry->next;
                }

                delete TimeRegion::sTimeRegionList.m_Head;
                TimeRegion::sTimeRegionList.m_Head = newHead;
            }
            else
            {
                ListEntry<TimeRegion*>* previousEntry = currentEntry;
                ListEntry<TimeRegion*>* nextEntry = currentEntry->next;
                while (nextEntry != NULL)
                {
                    if (nextEntry->entry == this)
                    {
                        previousEntry->next = nextEntry->next;
                        if (nextEntry == TimeRegion::sTimeRegionList.m_Tail)
                            TimeRegion::sTimeRegionList.m_Tail = previousEntry;
                        delete nextEntry;
                        break;
                    }
                    previousEntry = nextEntry;
                    nextEntry = nextEntry->next;
                }
            }
        }
    }

    const char* m_pName;
    bool (*m_pConditionFunc)();
    float m_fThreshold;
    int m_unk10;
    int m_unk14;
};

#endif // _TIMEREGIONS_H_
