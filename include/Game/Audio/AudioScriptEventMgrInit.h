#ifndef _AUDIOSCRIPTEVENTMGRINIT_H_
#define _AUDIOSCRIPTEVENTMGRINIT_H_

#include "NL/nlQSort.h"

namespace
{
// Dummy ctor: precompute hashes for g_NisEventLookup, then sort the table.
// Declared BEFORE g_PendingEvents so MWCC emits the hash/qsort sequence at the
// start of __sinit_AudioScriptEventMgr_cpp.
struct AudioScriptEventMgrLookupInit
{
    AudioScriptEventMgrLookupInit()
    {
        for (unsigned int i = 0; i < 4; i++)
        {
            g_NisEventLookup[i].hash = nlStringLowerHash(g_NisEventLookup[i].Name);
        }
        nlQSort<NIS_EVENT_LOOKUP>(g_NisEventLookup, 4, &nlDefaultQSortComparer<NIS_EVENT_LOOKUP>);
    }
};
AudioScriptEventMgrLookupInit s_audioScriptEventMgrLookupInit;
} // namespace

// g_PendingEvents uses the (initial, delta) ctor which inlines:
//   __ct__12SlotPoolBaseFv, m_Head/m_Tail=0, m_Initial=16, BaseAddNewBlock, m_Delta=16.
// MWCC also automatically emits a __register_global_object call for the dtor.
nlListSlotPool<AUDIO_EVENT_RECORD> g_PendingEvents(0x10, 0x10);

#endif // _AUDIOSCRIPTEVENTMGRINIT_H_
