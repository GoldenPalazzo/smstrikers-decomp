#ifndef _LOADFRAME_H_
#define _LOADFRAME_H_

class LoadFrame
{
public:
    template <int N, typename T>
    void Replayable(T& current);

    template <int N, typename T>
    void Replayable(T& current, NotReplayablePod);

    template <int N, typename T>
    void ReplayablePolymorphicPtr(T*& ptr);

    /* 0x0 */ int mInterval;
    /* 0x4 */ ReadByteStream mStream;
    /* 0xC */ ReplayNonBlendables mReplayNonBlendables;
    /* 0x10 */ float mNonBlendableAheadOfFrame;
}; // total size: 0x14

template <int N, typename T>
inline void ReplayLoadFrameValue(LoadFrame& frame, T& current)
{
    NotReplayablePod pod;
    frame.Replayable<N>(current, pod);
}

template <int N, typename T>
void LoadFrame::Replayable(T& current)
{
    ReplayLoadFrameValue<N>(*this, current);
}

template <int N>
void Replayable(LoadFrame& frame, char typeId, cPoseNode*& poseNode);

template <int N, typename T>
void LoadFrame::ReplayablePolymorphicPtr(T*& current)
{
    FORCE_DONT_INLINE;
    if (N == 0 || mInterval == N)
    {
        unsigned char notNull = 1;
        memcpy(&notNull, mStream.mStorage, 1);
        mStream.mStorage++;

        if (notNull)
        {
            char typeId = 0;
            memcpy(&typeId, mStream.mStorage, 1);
            mStream.mStorage++;

            if (typeId < 0 || typeId > 4)
                nlBreak();

            ::Replayable<N>(*this, typeId, current);
        }
        else
        {
            current = 0;
        }
    }
}

#endif // _LOADFRAME_H_

// The retained EmissionController member instantiations belong to
// EmissionManager.o in the reconstructed target split.
#ifdef LOADFRAME_EMISSIONCONTROLLER_SPECIALIZATIONS

template <>
void LoadFrame::Replayable<0, EmissionController>(EmissionController& current, NotReplayablePod)
{
    FORCE_DONT_INLINE;
    ::Replayable<0>(*this, (unsigned int&)current.m_pPose);
    ::Replayable<0>(*this, (unsigned int&)current.m_pAnimController);
    memcpy(&current.m_uUserData, mStream.mStorage, sizeof(unsigned long));
    mStream.mStorage += sizeof(unsigned long);
    ::Replayable<0>(*this, current.m_fGround);
    memcpy(&current.m_aFacing, mStream.mStorage, sizeof(unsigned short));
    mStream.mStorage += sizeof(unsigned short);
    ::Replayable<0>(*this, (char&)current.m_GlView);
    ReplayControllerFloats(*this, current);
    current.m_Replaying = true;
    ReplayControllerState(*this, current);
}

template <>
void LoadFrame::Replayable<0, EmissionController>(EmissionController& current)
{
    FORCE_DONT_INLINE;
    ReplayLoadFrameValue<0>(*this, current);
}

#endif
