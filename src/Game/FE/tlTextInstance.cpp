#include "Game/FE/tlTextInstance.h"

#include "NL/nlString.h"

// nlStrNCmp<char>'s weak body lives in CrowdMood.o; reference it (UND), don't re-emit.
template <>
int nlStrNCmp<char>(const char*, const char*, unsigned long);

/**
 * Offset/Address/Size: 0x0 | 0x80210170 | size: 0x68
 */
void TLTextInstance::SetStringId(const char* id)
{
    if (nlStrNCmp<char>(id, "LOC_", 4) == 0)
    {
        id += 4;
    }
    m_LocStrId = nlStringLowerHash(id);
    m_OverloadFlags |= 0x8u;
}
