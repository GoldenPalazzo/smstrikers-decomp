#ifndef _THPSIMPLE_H_
#define _THPSIMPLE_H_

#include "NL/nlFile.h"
#include "dolphin/thp/THPBuffer.h"

struct THPSimpleControl
{
    /* 0x00 */ nlFile* file;
    /* 0x04 */ u8 _pad04[0x04];
    /* 0x08 */ u32 version;
    /* 0x0C */ u8 _pad0C[0x0C];
    /* 0x18 */ u32 numFrames;
    /* 0x1C */ u32 firstFrameSize;
    /* 0x20 */ u8 _pad20[0x04];
    /* 0x24 */ u32 compInfoDataOffsets;
    /* 0x28 */ u8 _pad28[0x04];
    /* 0x2C */ u32 movieDataOffsets;
    /* 0x30 */ u8 _pad30[0x3C];
    /* 0x6C */ u8 playing;
    /* 0x6D */ u8 _pad6D;
    /* 0x6E */ u8 loop;
    /* 0x6F */ u8 _pad6F;
    /* 0x70 */ u32 totalRead;
    /* 0x74 */ s32 unk74;
    /* 0x78 */ u32 dvdBusy;
    /* 0x7C */ u8 _pad7C[0x04];
    /* 0x80 */ s32 readIdx;
    /* 0x84 */ u32 nextSize;
    /* 0x88 */ u32 frameCount;
    /* 0x8C */ u8 _pad8C[0x10];
    /* 0x9C */ THPReadBuffer readBuffers[10];
};

extern THPSimpleControl SimpleControl;
extern s32 NumReadBuffers;

int THPSimpleSetBuffer(unsigned char* buffer);
void THPAudioMixCallback();
void MixAudio(short*, short*, unsigned long);
void __THPSimpleDVDCallback(nlFile*, void*, unsigned int, unsigned long);
void __THPAsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long));
#endif // _THPSIMPLE_H_
