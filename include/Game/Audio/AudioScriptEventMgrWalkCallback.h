#ifndef _AUDIOSCRIPTEVENTMGRWALKCALLBACK_H_
#define _AUDIOSCRIPTEVENTMGRWALKCALLBACK_H_

// Specialized walk callback for the audio event list: ListEntry<T>'s payload
// field is named 'data', while the generic WalkHelper::Callback body in
// NL/nlWalkHelper.h dereferences 'm_data' (the DLListEntry field name).
// Without this specialization MWCC instantiates the generic body and fails.
template <>
inline void WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>::Callback(ListEntry<AUDIO_EVENT_RECORD>* listEntry)
{
    (m_CBClass->*m_CB)(&listEntry->data);
}

#endif // _AUDIOSCRIPTEVENTMGRWALKCALLBACK_H_
