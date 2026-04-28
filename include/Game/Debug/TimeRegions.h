#ifndef _TIMEREGIONS_H_
#define _TIMEREGIONS_H_

#include "NL/nlList.h"
#include "NL/nlMemory.h"
#include "Game/GameTweaks.h" // Add this for nlListAddEnd

void DestroyTimeRegions();
void InitializeTimeRegions();
bool IsShotInProgress();
bool IsBowserAround();
bool IsDuringAutoreplay();
bool IsDuringNIS();
bool RightSideOfField();
bool LeftSideOfField();
bool CentreOfField();
bool IsDuringGameplay();
void WriteFrameRateStatsToFile();
// void nlListAddEnd<ListEntry<TimeRegion*>>(ListEntry<TimeRegion*>**, ListEntry<TimeRegion*>**, ListEntry<TimeRegion*>*);

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
            entry->data = this; // Store the old value
        }
        nlListAddEnd<ListEntry<TimeRegion*> >(&sTimeRegionList.m_Head, &sTimeRegionList.m_Tail, entry);
    }

    virtual ~TimeRegion()
    {
        ListEntry<TimeRegion*>* currentEntry = TimeRegion::sTimeRegionList.m_Head;
        if (currentEntry != NULL)
        {
            if (currentEntry->data == this)
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
                    if (nextEntry->data == this)
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
