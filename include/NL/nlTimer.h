#ifndef _NLTIMER_H_
#define _NLTIMER_H_

#include "types.h"

class Timer
{
public:
    Timer(f32 seconds = 0.0f) { SetSeconds(seconds); }
    bool Countup(float, float);
    bool Countdown(float, float);
    f32 GetSeconds() const;
    void SetSeconds(float);

    /* 0x00 */ u32 m_uPackedTime;
};

#endif // _NLTIMER_H_
