#ifndef _AUDIOSTREAMVIRTUALS_H_
#define _AUDIOSTREAMVIRTUALS_H_

#include "Game/Sys/GCStream.h"

namespace GCAudioStreaming
{

inline void AudioStream::Purge()
{
    m_State = SS_New;
}

inline AudioStream::~AudioStream()
{
}

inline void AudioStream::WarmReadDone(AudioStreamBuffer* pBuffer)
{
    if (m_Buffers[m_BufferCount - 1] != pBuffer)
    {
        return;
    }

    m_State = SS_Warm;

    if (!(m_Flags & (1 << SF_Play)))
    {
        return;
    }

    if (pBuffer != m_Buffers[m_BufferCount - 1])
    {
        return;
    }

    m_Flags &= ~(1 << SF_Play);
    ActivateBuffers();
    m_State = SS_Playing;
}

inline MonoAudioStream::~MonoAudioStream()
{
    Destructor();
}

inline bool MonoAudioStream::SafeToPurge()
{
    bool result = false;
    if (m_State <= SS_Initd && !nlAsyncReadsPending(m_pFile))
    {
        result = true;
    }
    return result;
}

inline void MonoAudioStream::Purge()
{
    m_State = SS_New;
    nlClose(m_pFile);
}

inline bool StereoAudioStream::SafeToPurge()
{
    bool result = false;
    if (m_State <= SS_Initd && !nlAsyncReadsPending(m_pFile))
    {
        result = true;
    }
    return result;
}

inline void StereoAudioStream::Purge()
{
    m_State = SS_New;
    nlClose(m_pFile);
}

} // namespace GCAudioStreaming

#endif // _AUDIOSTREAMVIRTUALS_H_
