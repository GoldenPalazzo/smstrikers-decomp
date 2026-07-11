#ifndef _TLSLIDE_H_
#define _TLSLIDE_H_

#include "types.h"

#include "Game/FE/tlInstance.h"
#include "Game/FE/feAnimation.h"

enum eTimeLinePlayMode
{
    TLPM_STOP_AT_END = 0,
    TLPM_LOOPING = 1,
};

class TLSlide
{
public:
#ifdef FERENDER_INLINE_ACCESSOR_IMPLS
    // Plain body (no FORCE_DONT_INLINE): the call site uses inline_depth(0),
    // so the weak copy defers to the end-of-TU flush and its bucket becomes
    // the LAST linkonce section, with __sinit right behind it.
    f32 GetCurrentTime() const
    {
        return m_time;
    }
#else
    f32 GetCurrentTime() const;
#endif
    void Update(float);
    void UpdateAsset(TLInstance*, float);

    /* 0x00 */ TLSlide* m_next;
    /* 0x04 */ char pad0[0x4];
    /* 0x08 */ TLInstance* m_instances;
    /* 0x0C */ FEAnimation* m_animations;
    /* 0x10 */ f32 m_start;
    /* 0x14 */ f32 m_duration;
    /* 0x18 */ f32 m_time;
    /* 0x1C */ eTimeLinePlayMode m_uPlayMode;
    /* 0x20 */ char m_szName[32];
    /* 0x40 */ u32 m_hash;
};

#ifdef FERENDER_INLINE_ACCESSOR_IMPLS
// Phantom referenced only by feRender_stub: appends to this header's linkonce
// bucket at end of TU so __sinit lands after GetCurrentTime; both the stub and
// this weak copy are stripped by mwld.
inline void feRenderTlSlideStub()
{
}
#endif

#endif // _TLSLIDE_H_
