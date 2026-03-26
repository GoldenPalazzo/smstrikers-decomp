#include "Game/FE/feTweener.h"
#include "NL/nlMemory.h"

/**
 * Offset/Address/Size: 0x0 | 0x800A2254 | size: 0x9C
 */
void FETweenManager::startTween(FETweener* pTweener)
{
    DLListEntry<FETweener*>* pEntry = NULL;

    pTweener->m_tweenActive = 1;

    // Ensure we have slots available in the active tween list
    if (((SlotPoolBase*)&m_activeTweenList)->m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&m_activeTweenList, sizeof(DLListEntry<FETweener*>));
    }

    // Get a free entry from the pool
    if (((SlotPoolBase*)&m_activeTweenList)->m_FreeList != NULL)
    {
        pEntry = (DLListEntry<FETweener*>*)((SlotPoolBase*)&m_activeTweenList)->m_FreeList;
        ((SlotPoolBase*)&m_activeTweenList)->m_FreeList = ((SlotPoolBase*)&m_activeTweenList)->m_FreeList->m_next;
    }

    // Initialize the entry
    if (pEntry != NULL)
    {
        pEntry->m_next = NULL;
        pEntry->m_prev = NULL;
        pEntry->m_data = pTweener;
    }

    // Add to active tween list
    nlDLRingAddEnd(&m_activeTweenList.m_Head, pEntry);
}

/**
 * Offset/Address/Size: 0x9C | 0x800A22F0 | size: 0x104
 */
void FETweenManager::clearTweensOnObj(void* obj)
{
    DLListEntry<FETweener*>* head = m_activeTweenList.m_Head;
    DLListEntry<FETweener*>* entry = nlDLRingGetStart(head);
    head = m_activeTweenList.m_Head;

    while (entry != NULL)
    {
        FETweener* tweener = entry->m_data;
        if (obj == tweener->m_applyObj)
        {
            tweener->m_tweenActive = 1;
            float accumulated = tweener->m_delay + (tweener->m_startTime + tweener->m_duration);
            FETweener* next = tweener;
            while (next->m_nextTween != NULL)
            {
                next = next->m_nextTween;
                accumulated += next->m_duration + next->m_delay;
            }
            tweener->m_curTime = 1.0f + accumulated;
            Update(0.0f);
            head = m_activeTweenList.m_Head;
            entry = nlDLRingGetStart(head);
            head = m_activeTweenList.m_Head;
        }
        else
        {
            if (nlDLRingIsEnd(head, entry) || entry == NULL)
            {
                entry = NULL;
            }
            else
            {
                entry = entry->m_next;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1A0 | 0x800A23F4 | size: 0xE8
 */
void FETweenManager::clearTweens()
{
    DLListEntry<FETweener*>* head = m_activeTweenList.m_Head;
    DLListEntry<FETweener*>* entry = nlDLRingGetStart(head);
    head = m_activeTweenList.m_Head;

    while (entry != NULL)
    {
        FETweener* tweener = entry->m_data;
        tweener->m_tweenActive = 1;

        float accumulated = tweener->m_delay + (tweener->m_startTime + tweener->m_duration);
        FETweener* next = tweener;
        while (next->m_nextTween != NULL)
        {
            next = next->m_nextTween;
            accumulated += next->m_duration + next->m_delay;
        }

        tweener->m_curTime = 1.0f + accumulated;
        Update(0.0f);

        if (nlDLRingIsEnd(head, entry) || entry == NULL)
        {
            entry = NULL;
        }
        else
        {
            entry = entry->m_next;
        }
    }
}

/**
 * Offset/Address/Size: 0x288 | 0x800A24DC | size: 0x468
 * TODO: 99.86% match - remaining blockers are static-local symbol index diffs for `m_tempValArray`
 * (`m_tempValArray$181` vs compiler-emitted local index in scratch context).
 */
void FETweenManager::Update(float fDeltaT)
{
    static float m_tempValArray[4];

    FETweener* nextTween;
    DLListEntry<FETweener*>* head;
    DLListEntry<FETweener*>* entry;
    DLListEntry<FETweener*>* startEntry;
    DLListEntry<FETweener*>** activeListHead;
    FETweener* curTween;
    DLListEntry<FETweener*>* pEntry;
    DLListEntry<FETweener*>* savedEntry;
    DLListEntry<FETweener*>* tweenHead;
    DLListEntry<FETweener*>* tweenEntry;

    startEntry = nlDLRingGetStart(m_activeTweenList.m_Head);
    head = m_activeTweenList.m_Head;
    activeListHead = &m_activeTweenList.m_Head;
    entry = startEntry;

    while (entry != NULL)
    {
        curTween = entry->m_data;

        unsigned char active = curTween->m_tweenActive;
        unsigned char done;
        if (active != 0)
        {
            done = (curTween->m_curTime - (curTween->m_startTime + curTween->m_delay)) >= curTween->m_duration;
        }
        else
        {
            done = 0;
        }

        if (done)
        {
            nextTween = curTween->m_nextTween;

            // findTweenValue (inlined)
            float val;
            unsigned char done2;
            if (active != 0)
            {
                done2 = (curTween->m_curTime - (curTween->m_startTime + curTween->m_delay)) >= curTween->m_duration;
            }
            else
            {
                done2 = 0;
            }

            if (done2)
            {
                val = 1.0f;
            }
            else if ((curTween->m_curTime - curTween->m_startTime) < curTween->m_delay)
            {
                val = 0.0f;
            }
            else
            {
                val = curTween->m_tweenFunc(
                    curTween->m_curTime - (curTween->m_startTime + curTween->m_delay),
                    0.0f,
                    1.0f,
                    curTween->m_duration);
            }

            // applyTweenValue (inlined)
            {
                unsigned char i = 0;
                while (i < curTween->m_arraySize)
                {
                    m_tempValArray[i] = curTween->m_diffVal[i] * val + curTween->m_startVal[i];
                    i++;
                }
            }
            curTween->m_setterFunc(curTween->m_applyObj, m_tempValArray);

            if (curTween->m_doneFunc != NULL)
            {
                curTween->m_doneFunc(curTween->m_doneFuncParam);
            }

            // Chain next tween
            if (nextTween != NULL)
            {
                float st = curTween->m_startTime;
                float dur = curTween->m_duration;
                float dl = curTween->m_delay;
                nextTween->m_startTime = dl + (st + dur);
                nextTween->m_curTime = curTween->m_curTime;
                nextTween->m_tweenActive = 1;

                pEntry = NULL;
                if (((SlotPoolBase*)&m_activeTweenList)->m_FreeList == NULL)
                {
                    SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&m_activeTweenList, sizeof(DLListEntry<FETweener*>));
                }
                if (((SlotPoolBase*)&m_activeTweenList)->m_FreeList != NULL)
                {
                    pEntry = (DLListEntry<FETweener*>*)((SlotPoolBase*)&m_activeTweenList)->m_FreeList;
                    ((SlotPoolBase*)&m_activeTweenList)->m_FreeList = ((SlotPoolBase*)&m_activeTweenList)->m_FreeList->m_next;
                }
                if (pEntry != NULL)
                {
                    pEntry->m_next = NULL;
                    pEntry->m_prev = NULL;
                    pEntry->m_data = nextTween;
                }
                nlDLRingAddEnd(activeListHead, pEntry);
            }

            // If at end, restart iteration
            if (nlDLRingIsEnd(head, entry))
            {
                nlDLRingGetStart(m_activeTweenList.m_Head);
                head = m_activeTweenList.m_Head;
            }

            savedEntry = entry;
            pEntry = entry;

            if (nlDLRingIsEnd(head, entry) || entry == NULL)
            {
                entry = NULL;
            }
            else
            {
                entry = entry->m_next;
            }

            // Remove from active list and return to free pool
            nlDLRingRemove(activeListHead, savedEntry);
            pEntry->m_next = (DLListEntry<FETweener*>*)((SlotPoolBase*)&m_activeTweenList)->m_FreeList;
            ((SlotPoolBase*)&m_activeTweenList)->m_FreeList = (SlotPoolEntry*)pEntry;

            // Find and remove from tween list
            tweenEntry = nlDLRingGetStart(m_tweenList.m_Head);
            tweenHead = m_tweenList.m_Head;

            while (tweenEntry != NULL)
            {
                if (tweenEntry->m_data == curTween)
                {
                    savedEntry = tweenEntry;
                    pEntry = tweenEntry;

                    if (nlDLRingIsEnd(tweenHead, tweenEntry) || tweenEntry == NULL)
                    {
                        tweenEntry = NULL;
                    }
                    else
                    {
                        tweenEntry = tweenEntry->m_next;
                    }

                    nlDLRingRemove(&m_tweenList.m_Head, savedEntry);
                    pEntry->m_next = (DLListEntry<FETweener*>*)((SlotPoolBase*)&m_tweenList)->m_FreeList;
                    ((SlotPoolBase*)&m_tweenList)->m_FreeList = (SlotPoolEntry*)pEntry;
                }
                else
                {
                    if (nlDLRingIsEnd(tweenHead, tweenEntry) || tweenEntry == NULL)
                    {
                        tweenEntry = NULL;
                    }
                    else
                    {
                        tweenEntry = tweenEntry->m_next;
                    }
                }
            }

            // Delete completed tween
            delete curTween;
        }
        else
        {
            // Not done - update tween
            if (active != 0)
            {
                curTween->m_curTime += fDeltaT;

                unsigned char done3;
                if (curTween->m_tweenActive != 0)
                {
                    done3 = (curTween->m_curTime - (curTween->m_startTime + curTween->m_delay)) >= curTween->m_duration;
                }
                else
                {
                    done3 = 0;
                }

                float val;
                if (done3)
                {
                    val = 1.0f;
                }
                else if ((curTween->m_curTime - curTween->m_startTime) < curTween->m_delay)
                {
                    val = 0.0f;
                }
                else
                {
                    val = curTween->m_tweenFunc(
                        curTween->m_curTime - (curTween->m_startTime + curTween->m_delay),
                        0.0f,
                        1.0f,
                        curTween->m_duration);
                }

                {
                    unsigned char i = 0;
                    while (i < curTween->m_arraySize)
                    {
                        m_tempValArray[i] = curTween->m_diffVal[i] * val + curTween->m_startVal[i];
                        i++;
                    }
                }
                curTween->m_setterFunc(curTween->m_applyObj, m_tempValArray);
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
    }
}

/**
 * Offset/Address/Size: 0x6F0 | 0x800A2944 | size: 0x160
 */
FETweener* FETweenManager::createTween(float* startVals, float* endVals, float duration, float delay, unsigned char arraySize,
    float (*tweenFunc)(float, float, float, float), void* applyObj, void (*setterFunc)(void*, const float*))
{
    FETweener* retTweener = new (nlMalloc(0x50, 0x20, true)) FETweener(arraySize, startVals, endVals, duration, delay, tweenFunc, applyObj, (void (*)(void*, float*))setterFunc);

    DLListEntry<FETweener*>* pEntry = NULL;

    if (((SlotPoolBase*)&m_tweenList)->m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&m_tweenList, sizeof(DLListEntry<FETweener*>));
    }

    if (((SlotPoolBase*)&m_tweenList)->m_FreeList != NULL)
    {
        pEntry = (DLListEntry<FETweener*>*)((SlotPoolBase*)&m_tweenList)->m_FreeList;
        ((SlotPoolBase*)&m_tweenList)->m_FreeList = ((SlotPoolBase*)&m_tweenList)->m_FreeList->m_next;
    }

    if (pEntry != NULL)
    {
        pEntry->m_next = NULL;
        pEntry->m_prev = NULL;
        pEntry->m_data = retTweener;
    }

    nlDLRingAddEnd(&m_tweenList.m_Head, pEntry);

    return retTweener;
}

/**
 * Offset/Address/Size: 0x850 | 0x800A2AA4 | size: 0x18C
 */
FETweenManager::~FETweenManager()
{
    DLListEntry<FETweener*>* head = m_tweenList.m_Head;
    DLListEntry<FETweener*>* entry = nlDLRingGetStart(head);
    head = m_tweenList.m_Head;

    while (entry != NULL)
    {
        FETweener* tweener = entry->m_data;
        delete tweener;

        if (nlDLRingIsEnd(head, entry) || entry == NULL)
        {
            entry = NULL;
        }
        else
        {
            entry = entry->m_next;
        }
    }
}

/**
 * Offset/Address/Size: 0x9DC | 0x800A2C30 | size: 0x9C
 * TODO: 94.62% match - MWCC inline constructor scheduling quirk: `mr r31, r30` scheduled
 * after `stw r29` instead of before (prologue scheduling), and missing `mr r31, r29` for
 * second nlDLListSlotPool member's inline body (compiler doesn't reuse r31 for subobject this).
 */
FETweenManager::FETweenManager()
{
}

/**
 * Offset/Address/Size: 0xA78 | 0x800A2CCC | size: 0xC
 */
void FETweener::setDoneCallFunc(void (*doneFunc)(void*), void* doneFuncParam)
{
    m_doneFunc = doneFunc;
    m_doneFuncParam = doneFuncParam;
}

/**
 * Offset/Address/Size: 0xA84 | 0x800A2CD8 | size: 0x8
 */
void FETweener::setNextTween(FETweener* next)
{
    m_nextTween = next;
}

/**
 * Offset/Address/Size: 0xA8C | 0x800A2CE0 | size: 0x48
 */
FETweener::~FETweener()
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800A2D28 | size: 0x10
//  */
// void DLListContainerBase<FETweener*, BasicSlotPool<DLListEntry<FETweener*>>>::DeleteEntry(DLListEntry<FETweener*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800A2D38 | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before `stw r0, 0x24(r1)`.
 */
template void nlWalkDLRing<DLListEntry<FETweener*>, DLListContainerBase<FETweener*, BasicSlotPool<DLListEntry<FETweener*> > > >(
    DLListEntry<FETweener*>* head,
    DLListContainerBase<FETweener*, BasicSlotPool<DLListEntry<FETweener*> > >* callback,
    void (DLListContainerBase<FETweener*, BasicSlotPool<DLListEntry<FETweener*> > >::*callbackFunc)(DLListEntry<FETweener*>*));

// /**
//  * Offset/Address/Size: 0x3C | 0x800A2D74 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<FETweener*>>(DLListEntry<FETweener*>*, DLListEntry<FETweener*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x800A2D94 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<FETweener*>>(DLListEntry<FETweener*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x74 | 0x800A2DAC | size: 0x44
//  */
// void nlDLRingRemove<DLListEntry<FETweener*>>(DLListEntry<FETweener*>**, DLListEntry<FETweener*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB8 | 0x800A2DF0 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<FETweener*>>(DLListEntry<FETweener*>**, DLListEntry<FETweener*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xF4 | 0x800A2E2C | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<FETweener*>>(DLListEntry<FETweener*>**, DLListEntry<FETweener*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A2E64 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<FETweener*>, DLListContainerBase<FETweener*,
// BasicSlotPool<DLListEntry<FETweener*>>>>(DLListEntry<FETweener*>*, DLListContainerBase<FETweener*,
// BasicSlotPool<DLListEntry<FETweener*>>>*, void (DLListContainerBase<FETweener*,
// BasicSlotPool<DLListEntry<FETweener*>>>::*)(DLListEntry<FETweener*>*))
// {
// }
