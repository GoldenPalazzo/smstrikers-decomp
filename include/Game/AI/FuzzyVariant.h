#ifndef _FUZZYVARIANT_H_
#define _FUZZYVARIANT_H_

#include "Game/AI/Variant.h"

// void 0x8028D28C..0x8028D290 | size: 0x4;

class FuzzyVariant : public Variant
{
public:
    FuzzyVariant() { Reset(); };
    FuzzyVariant(const FuzzyVariant& other)
    {
        Reset();
        *this = other;
    }

    template <typename T>
    FuzzyVariant(const T& value)
        : Variant(VariantTypeOf(value), value)
    {
        ExtraData.Reset();
        Confidence = 0.0f;
        SelectionChance = 1.0f;
    }

    virtual void Reset()
    {
        mType = FT_UNSPECIFIED;
        mData.vector.f.x = 0.0f;
        mData.vector.f.y = 0.0f;
        mData.vector.f.z = 0.0f;
        ExtraData.Reset();
        Confidence = 0.0f;
        SelectionChance = 1.0f;
    };

    float Confidence;      // offset 0x14, size 0x4
    float SelectionChance; // offset 0x18, size 0x4
    Variant ExtraData;     // offset 0x1C, size 0x14
}; // total size: 0x30

extern FuzzyVariant fvNotSet;

#endif // _FUZZYVARIANT_H_
