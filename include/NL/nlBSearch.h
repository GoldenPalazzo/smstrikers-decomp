#ifndef _NLBSEARCH_H_
#define _NLBSEARCH_H_

/** EXAMPLE USAGE
 * Offset/Address/Size: 0x0 | 0x80213820 | size: 0x8C
 */

template <typename T, typename Key>
T* nlBSearch(const Key& key, T* pBase, int count)
{
    const Key* keyPtr = &key;
    int high = count - 1;
    int low = -1;

    while ((high - low) > 1)
    {
        int mid = (high + low) / 2;

        if (pBase[mid].hash > (unsigned long)*keyPtr)
        {
            high = mid;
        }
        else
        {
            low = mid;
        }
    }

    unsigned long highHash = pBase[high].hash;
    if (highHash == (unsigned long)*keyPtr)
    {
        return &pBase[high];
    }

    if (low == -1)
    {
        return nullptr;
    }

    unsigned long lowHash = pBase[low].hash;
    if (lowHash == (unsigned long)*keyPtr)
    {
        return &pBase[low];
    }

    return nullptr;
}

#endif // _NLBSEARCH_H_
