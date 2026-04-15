#include "Game/Audio/AudioScriptEventMgr.h"

#include "NL/nlList.h"
#include "NL/nlSlotPool.h"

struct AUDIO_EVENT_RECORD
{
    /* 0x0 */ AudioScriptEventMgr::AUDIO_EVENT Event : 16;
    /* 0x2 */ AudioScriptEventMgr::AUDIO_EVENT_TEAM Team : 16;
};

template class ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >;

// /**
//  * Offset/Address/Size: 0xD0 | 0x8014B7EC | size: 0x2C
//  */
// void nlListAddEnd<ListEntry<AUDIO_EVENT_RECORD> >(ListEntry<AUDIO_EVENT_RECORD>**, ListEntry<AUDIO_EVENT_RECORD>**, ListEntry<AUDIO_EVENT_RECORD>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x8014B784 | size: 0x68
//  */
// void nlWalkList<ListEntry<AUDIO_EVENT_RECORD>, ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > > >(ListEntry<AUDIO_EVENT_RECORD>*, ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >*, void (ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >::*)(ListEntry<AUDIO_EVENT_RECORD>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B71C | size: 0x68
//  */
// void nlWalkList<ListEntry<AUDIO_EVENT_RECORD>, WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser> >(ListEntry<AUDIO_EVENT_RECORD>*, WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>*, void (WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>::*)(ListEntry<AUDIO_EVENT_RECORD>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B65C | size: 0xC0
//  */
// void 0x8014B71C..0x8014B818 | size : 0xFC
// {
// }

// /**
//  * Offset/Address/Size: 0xE0 | 0x8014B590 | size: 0xCC
//  */
// void nlListSlotPool<AUDIO_EVENT_RECORD>::~nlListSlotPool()
// {
// }

// /**
//  * Offset/Address/Size: 0xB4 | 0x8014B564 | size: 0x2C
//  */
// void nlDefaultQSortComparer<NIS_EVENT_LOOKUP>(const NIS_EVENT_LOOKUP*, const NIS_EVENT_LOOKUP*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8C | 0x8014B53C | size: 0x28
//  */
// void nlQSort<NIS_EVENT_LOOKUP>(NIS_EVENT_LOOKUP*, int, int (*)(const NIS_EVENT_LOOKUP*, const NIS_EVENT_LOOKUP*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B4B0 | size: 0x8C
//  */
// void nlBSearch<NIS_EVENT_LOOKUP, unsigned long>(const unsigned long&, NIS_EVENT_LOOKUP*, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B4A0 | size: 0x10
//  */
// void ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >::DeleteEntry(ListEntry<AUDIO_EVENT_RECORD>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B46C | size: 0x34
//  */
// void WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>::Callback(ListEntry<AUDIO_EVENT_RECORD>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x2248 | 0x8014B39C | size: 0xD0
//  */
// void GetEventTeam<GoalScoredData>(Event*, bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x2178 | 0x8014B2CC | size: 0xD0
//  */
// void GetEventTeam<CollisionBallGoalpostData>(Event*, bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x1BAC | 0x8014AD00 | size: 0x5CC
//  */
// void AudioScriptEventMgr::Init()
// {
// }

// /**
//  * Offset/Address/Size: 0x1B18 | 0x8014AC6C | size: 0x94
//  */
// void AudioScriptEventMgr::Purge()
// {
// }

// /**
//  * Offset/Address/Size: 0x1A48 | 0x8014AB9C | size: 0xD0
//  */
// void AudioScriptEventMgr::Update()
// {
// }

// /**
//  * Offset/Address/Size: 0x19B8 | 0x8014AB0C | size: 0x90
//  */
// void _AudioEventRaiser::RaiseEvent(AUDIO_EVENT_RECORD*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1904 | 0x8014AA58 | size: 0xB4
//  */
// void AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AUDIO_EVENT, AudioScriptEventMgr::AUDIO_EVENT_TEAM)
// {
// }

// /**
//  * Offset/Address/Size: 0x10F8 | 0x8014A24C | size: 0x80C
//  */
// void Poll()
// {
// }

// /**
//  * Offset/Address/Size: 0xFF8 | 0x8014A14C | size: 0x100
//  */
// void RecordExcitingEvent()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80149154 | size: 0xFF8
//  */
// void AudioScriptEventHandler(Event*, void*)
// {
// }
