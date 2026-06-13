#ifndef _EVENTDATA_H_
#define _EVENTDATA_H_

#include "types.h"

struct EventData
{
    // public:
    // EventData() { }
    // virtual ~EventData() { }
    virtual u32 GetID() { return -1; }
};

#endif // _EVENTDATA_H_
