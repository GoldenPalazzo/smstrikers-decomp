#ifndef _NLWALKHELPER_H_
#define _NLWALKHELPER_H_

template <typename KeyType, typename EntryType, typename CallbackType>
class WalkHelper
{
public:
    CallbackType* m_CBClass;
    void (CallbackType::*m_CB)(KeyType*);
#ifdef CHARACTERTEMPLATE_INLINE_WALKHELPER_CALLBACK
    void Callback(EntryType* listEntry)
    {
        FORCE_DONT_INLINE;
        (m_CBClass->*m_CB)(&listEntry->entry);
    }
#else
    void Callback(EntryType*);
#endif
};

#ifndef CHARACTERTEMPLATE_INLINE_WALKHELPER_CALLBACK
template <typename KeyType, typename EntryType, typename CallbackType>
void WalkHelper<KeyType, EntryType, CallbackType>::Callback(EntryType* listEntry)
{
    (m_CBClass->*m_CB)(&listEntry->entry);
}
#endif

#endif // _NLWALKHELPER_H_
