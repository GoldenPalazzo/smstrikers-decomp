#ifndef _SHLOADING_H_
#define _SHLOADING_H_

#include "types.h"

#include "Game/BaseSceneHandler.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/FE/feAsyncImage.h"

class SuperLoadingScene : public BaseSceneHandler
{
public:
    enum TransitionType
    {
        TT_INVALID = -1,
        TT_IN = 0,
        TT_OUT = 1,
    };

    SuperLoadingScene();
    virtual ~SuperLoadingScene();
    virtual void SceneCreated();
    virtual void Update(float);
    void DisplayCupInfo();
    void BuildPlayerStrings(TLTextInstance*, int, bool);

    /* 0x01C */ TransitionType mType;                   // size 0x4
    /* 0x020 */ float mElapsedTime;                     // size 0x4
    /* 0x024 */ bool mAlreadySwappedTextures;           // size 0x1
    /* 0x026 */ unsigned short mStatsBuffers[4][64];    // size 0x200
    /* 0x228 */ TLImageInstance* mImageInstances[2][1]; // size 0x8
    /* 0x230 */ unsigned long mTextureHandles[2][1];    // size 0x8
    /* 0x238 */ unsigned short mPlayerStrings[2][255];  // size 0x3FC
}; // total size: 0x634

// class BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>::AppendInPlace(const unsigned short*);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class Format<BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>, unsigned short[16]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned
//     short(&)[16]); void TempStringAllocator>, unsigned short[2]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const
//     unsigned short(&)[2]);
// };

// class FormatImpl<BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&);
// };

#endif // _SHLOADING_H_
