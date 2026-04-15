#ifndef _ANIMINVENTORY_H_
#define _ANIMINVENTORY_H_

#include "types.h"
#include "NL/nlList.h"

#include "Game/SAnim.h"

struct SAnimContainer
{
    /* 0x00 */ void* unused0;
    /* 0x04 */ ListEntry<cSAnim*>* animHead;
    /* 0x08 */ ListEntry<cSAnim*>* animTail;
    /* 0x0C */ void* unused0C;
    /* 0x10 */ ListEntry<char*>* fileHead;
    /* 0x14 */ ListEntry<char*>* fileTail;
    /* 0x18 */ int animCount;
};

struct AnimProperties
{
    /* 0x00 */ const char* name0;
    /* 0x04 */ const char* name;
    /* 0x08 */ ePlayMode playMode;
    /* 0x0C */ float blendTime;
    /* 0x10 */ bool mirrored;
    // /* 0x11 */ u8 pad11[3];
    /* 0x14 */ int ballRotMode;
    /* 0x18 */ int endPhase;
    /* 0x1C */ u8 matchCharSpd;
    // /* 0x1D */ u8 pad1d[3];
};

class cAnimInventory
{
public:
    cAnimInventory(const AnimProperties* props, int count);
    ~cAnimInventory();

    // bundle ingest and lookups
    void AddAnimBundle(const char* szFilename);
    cSAnim* GetAnim(int i);
    ePlayMode GetPlayMode(int i);
    float GetBlendTime(int i);
    bool GetMirrored(int i);
    int GetBallRotationMode(int i);
    int GetEndPhase(int i);
    u8 GetMatchCharacterSpeed(int i);

    /* 0x00 */ int m_count;
    /* 0x04 */ SAnimContainer* m_cont;
    /* 0x08 */ cSAnim** m_anims;
    /* 0x0C */ const AnimProperties* m_props;
};

#endif // _ANIMINVENTORY_H_
