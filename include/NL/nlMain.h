#ifndef _NLMAIN_H_
#define _NLMAIN_H_

#include "types.h"

u32 nlChecksum32(const void*, unsigned long);
void nlInit();

class RunningChecksum
{
public:
    void ChecksumData(const void*, unsigned long);
    void ChecksumInt(unsigned long);
    RunningChecksum();

    /* 0x00 */ u32 m_nChecksum;
};

#endif // _NLMAIN_H_
