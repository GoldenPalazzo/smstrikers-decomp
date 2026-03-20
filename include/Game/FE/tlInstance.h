#ifndef _TLINSTANCE_H_
#define _TLINSTANCE_H_

#include "NL/nlColour.h"
#include "NL/nlMath.h"

#include "Game/FE/tlComponent.h"
#include "Game/FE/FEAudio.h"

enum eTimeLineAssetType
{
    TLAT_UNKNOWN = -1,
    TLAT_SLIDE = 0,
    TLAT_LAYER = 1,
    TLAT_IMAGE = 2,
    TLAT_TEXT = 3,
    TLAT_COMPONENT = 4,
    TLAT_GROUP = 5,
    TLAT_MAX_TYPES = 6,
};

class TLInstance
{
public:
#ifdef GET_TYPE_IN_TLINSTANCE_TU_DEF
    eTimeLineAssetType GetType() const;
#else
    eTimeLineAssetType GetType() const // only in feRender TU !
    {
        return m_type;
    }
#endif
    bool IsVisible() const
    {
        return m_bVisible;
    }
    FELibObject* GetLibRefObject() const
    {
        return m_component;
    }

    void SetAssetColour(const nlColour&);
    void SetAssetScale(float, float, float);
    void SetAssetRotation(float, float, float);
    void SetAssetPosition(float, float, float);
    bool IsValidAtTime(float);
    const nlColour& GetColour() const;
    const feVector3& GetScale() const;
    const feVector3& GetRotation() const;
    const feVector3& GetPosition() const;
    const nlColour& GetAssetColour() const;
    const feVector3& GetAssetScale() const;
    const feVector3& GetAssetPosition() const;

    /* 0x00 */ TLInstance* m_next;
    /* 0x04 */ TLInstance* m_prev;
    /* 0x08 */ TLInstance* pChildren;
    /* 0x0C */ TLComponent* m_component;
    /* 0x10 */ f32 m_fStartTime;
    /* 0x14 */ f32 m_fDuration;
    /* 0x18 */ char m_szName[32];
    /* 0x38 */ unsigned long m_hashID;
    /* 0x3C */ FELibObjectAttributes m_overloadedAttributes;
    /* 0x74 */ unsigned long m_overloadFlags;
    /* 0x78 */ eTimeLineAssetType m_type;
    /* 0x7C */ unsigned short m_priority;
    /* 0x7E */ bool m_bVisible;
}; // total size: 0x80

#endif // _TLINSTANCE_H_
