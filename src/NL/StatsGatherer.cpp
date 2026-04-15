#define NO_BASICSTRING_IMPL
#include "NL/StatsGatherer.h"

/**
 * Offset/Address/Size: 0x0 | 0x80064398 | size: 0x14
 */
template <>
BasicString<char, Detail::TempStringAllocator>&
BasicString<char, Detail::TempStringAllocator>::operator=(BasicString<char, Detail::TempStringAllocator> other)
{
    BasicStringData<char>* tmp = m_data;
    m_data = other.m_data;
    other.m_data = tmp;
    return *this;
}

/**
 * Offset/Address/Size: 0x0 | 0x80064394 | size: 0x4
 */
void nlTask::StateTransition(unsigned int, unsigned int)
{
}
