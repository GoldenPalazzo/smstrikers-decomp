#include "Game/Sys/GCStream.h"

namespace GCAudioStreaming
{

nlArrayAllocator<AudioStream::READ_CB_INFO> AudioStream::READ_CB_INFO::s_AllocPool;

}

// /**
//  * Offset/Address/Size: 0x4AC | 0x801C9630 | size: 0x140
//  */
// void 0x8028D52C..0x8028D530 | size: 0x4
// {
// }

/**
 * Offset/Address/Size: 0x49C | 0x801C9620 | size: 0x10
 */
void GCAudioStreaming::MonoAudioStream::_AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long uParam, void (*)(nlFile*, void*, unsigned int, unsigned long))
{
    AudioStream::READ_CB_INFO* pCBInfo = (AudioStream::READ_CB_INFO*)uParam;
    pCBInfo->m_next = AudioStream::READ_CB_INFO::s_AllocPool.m_pFree;
    AudioStream::READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
}

// /**
//  * Offset/Address/Size: 0x470 | 0x801C95F4 | size: 0x2C
//  */
// void GCAudioStreaming::MonoAudioStream::CancelPendingReads()
// {
// }

// /**
//  * Offset/Address/Size: 0x434 | 0x801C95B8 | size: 0x3C
//  */
// void GCAudioStreaming::MonoAudioStream::GetUpdateReadLength()
// {
// }

// /**
//  * Offset/Address/Size: 0x2E4 | 0x801C9468 | size: 0x150
//  */
// void GCAudioStreaming::AudioStreamBuffer::_UpdateHandler(void*, unsigned long, void*, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2B8 | 0x801C943C | size: 0x2C
//  */
// void GCAudioStreaming::StereoAudioStream::GetUpdateReadLength()
// {
// }

// /**
//  * Offset/Address/Size: 0x27C | 0x801C9400 | size: 0x3C
//  */
// void GCAudioStreaming::StereoAudioStream::_InterleavedHdrReadCB(nlFile*, void*, unsigned int, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x238 | 0x801C93BC | size: 0x44
//  */
// void GCAudioStreaming::StereoAudioStream::_AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long))
// {
// }

// /**
//  * Offset/Address/Size: 0x20C | 0x801C9390 | size: 0x2C
//  */
// void GCAudioStreaming::StereoAudioStream::CancelPendingReads()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801C9184 | size: 0x20C
//  */
// void GCAudioStreaming::AudioStream::Stop()
// {
// }

// /**
//  * Offset/Address/Size: 0x1984 | 0x801C9134 | size: 0x50
//  */
// void ___blank(const char*, ...)
// {
// }

// /**
//  * Offset/Address/Size: 0x187C | 0x801C902C | size: 0x108
//  */
// void GCAudioStreaming::AudioStream::_HdrReadCB(nlFile*, void*, unsigned int, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x17BC | 0x801C8F6C | size: 0xC0
//  */
// void GCAudioStreaming::AudioStream::_WarmReadCB(nlFile*, void*, unsigned int, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x1704 | 0x801C8EB4 | size: 0xB8
//  */
// void GCAudioStreaming::AudioStream::_UpdateReadCB(nlFile*, void*, unsigned int, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x1364 | 0x801C8B14 | size: 0x3A0
//  */
// void GCAudioStreaming::MonoAudioStream::Warm(bool)
// {
// }

// /**
//  * Offset/Address/Size: 0xDCC | 0x801C857C | size: 0x598
//  */
// void GCAudioStreaming::MonoAudioStream::DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*)
// {
// }

// /**
//  * Offset/Address/Size: 0xA48 | 0x801C81F8 | size: 0x384
//  */
// void GCAudioStreaming::StereoAudioStream::Warm(bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x7E8 | 0x801C7F98 | size: 0x260
//  */
// void GCAudioStreaming::StereoAudioStream::InterleavedHdrReadCB(nlFile*, void*, unsigned int)
// {
// }

// /**
//  * Offset/Address/Size: 0x2A8 | 0x801C7A58 | size: 0x540
//  */
// void GCAudioStreaming::StereoAudioStream::DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*)
// {
// }

// /**
//  * Offset/Address/Size: 0x268 | 0x801C7A18 | size: 0x40
//  */
// void GCAudioStreaming::AudioBufferMgr::Init(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x134 | 0x801C78E4 | size: 0x134
//  */
// void GCAudioStreaming::AudioBufferMgr::CreateBuffers(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x98 | 0x801C7848 | size: 0x9C
//  */
// void GCAudioStreaming::AudioBufferMgr::DeleteBuffers()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801C77B0 | size: 0x98
//  */
// void GCAudioStreaming::AudioBufferMgr::FreeBuffer(GCAudioStreaming::AudioStreamBuffer*)
// {
// }
