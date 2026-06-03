#ifndef _NLWARE_H_
#define _NLWARE_H_

// Stubbed engine-wide diagnostic print. Defined inline here (matching DWARF, which
// attributes nlPrintf to nlWare.h) so the owning TU emits it as a weak symbol in its
// own .text subsection; all other TUs reference it externally (UND).
inline int nlPrintf(const char*, ...)
{
    return 0;
}

#endif // _NLWARE_H_
