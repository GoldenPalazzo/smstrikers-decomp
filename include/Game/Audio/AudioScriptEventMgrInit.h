#ifndef _AUDIOSCRIPTEVENTMGRINIT_H_
#define _AUDIOSCRIPTEVENTMGRINIT_H_

#include "NL/nlQSort.h"

// Static-init half of the NIS event lookup: hash + sort the table; the
// return value seeds g_PendingEvents' (initial, delta) ctor. All of this
// inlines into __sinit_AudioScriptEventMgr_cpp.
static inline int InitNisEventLookup()
{
    for (unsigned int i = 0; i < 4; i++)
    {
        g_NisEventLookup[i].hash = nlStringLowerHash(g_NisEventLookup[i].Name);
    }
    nlQSort<NIS_EVENT_LOOKUP>(g_NisEventLookup, 4, &nlDefaultQSortComparer<NIS_EVENT_LOOKUP>);
    return 0x10;
}

nlListSlotPool<AUDIO_EVENT_RECORD> g_PendingEvents(InitNisEventLookup(), 0x10);

#endif // _AUDIOSCRIPTEVENTMGRINIT_H_
