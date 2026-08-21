#ifndef _FESOUNDKEYFRAMETRIGGER_H_
#define _FESOUNDKEYFRAMETRIGGER_H_

#include "Game/FE/tlSlide.h"

// class TLSlide;

class SoundKeyframeTrigger
{
public:
    void Update(float previoustime, float currenttime);

    /*0x00*/ TLSlide* m_slide; // first word is the slide pointer
};

#endif // _FESOUNDKEYFRAMETRIGGER_H_
