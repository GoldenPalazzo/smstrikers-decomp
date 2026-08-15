#ifndef _GOALIESAVE_H_
#define _GOALIESAVE_H_

#include "NL/nlMath.h"
#include "NL/nlList.h"
#include "NL/nlAVLTree.h"

struct SaveInfo;
class Goalie;

class SavePositionData
{
public:
    void Init(Goalie*, int);

    /* 0x00 */ int mnAnimID;
    /* 0x04 */ float mfAnimDistance;
    /* 0x08 */ float mfAnimTime;
    /* 0x0C */ float mfAnimVelocity;
}; // total size: 0x10

class SaveData
{
public:
    static float LookupFatigueValue(Goalie*, const SaveInfo&);
    void Init(Goalie*, const SaveInfo&, unsigned int);
    void PostInit(const SaveInfo&);
    float GetMilestoneTime(int milestone) const
    {
        return mfMilestonePercent[milestone] * mfDuration;
    }

    /* 0x00 */ int mnAnimID;
    /* 0x04 */ SaveData* mpFailAnimData;
    /* 0x08 */ int mnRecoverAnimID;
    /* 0x0C */ unsigned int muSaveType;
    /* 0x10 */ nlVector3 mv3SavePos;
    /* 0x1C */ nlVector3 mv3TakeoffPos;
    /* 0x28 */ float mfDuration;
    /* 0x2C */ float mfMilestonePercent[5];
    /* 0x40 */ float mfFatigueValue;
    /* 0x44 */ SaveData* mpConnectedSaveData[4];
    /* 0x54 */ nlVector3 mv3GroupMinCoords;
    /* 0x60 */ nlVector3 mv3GroupMaxCoords;
    /* 0x6C */ char mszName[16];
    /* 0x7C */ int muIndex;
}; // total size: 0x80

struct SaveBlendInfo
{
public:
    /* 0x00 */ float mfStartTime;
    /* 0x04 */ float mfMilestoneTime[5];
    /* 0x18 */ float mfMilestoneScale[4][5];
    /* 0x68 */ float mfSaveBlendPrimary;
    /* 0x6C */ float mfSaveBlendSecondary;
    /* 0x70 */ float mfSaveBlendComposite;
    /* 0x74 */ class SaveData* mpSaveData[4];
    /* 0x84 */ class nlVector3 mv3BlendedSavePos;
}; // total size: 0x90

class GoalieSave
{
    static void FindVerticalBoundingPoints(SaveData*, const nlVector3&, SaveData**, SaveData**);

public:
    static SaveData* FindSaveData(int);
    static void ClearData();
    static void InitData(Goalie*);
    static SaveData* FindBestSave(SaveBlendInfo& blendInfo, const nlVector3& v3LocalPos, float fTime, bool bDoNearSearch, unsigned int uSaveType, bool bFromTakeoff);
    static SaveData* FindBestInList(SaveBlendInfo&, nlListContainer<SaveData*>&, const nlVector3&, float, unsigned int, bool);
    static SaveData* GetClosestBlendedPos(SaveBlendInfo&, const nlVector3&, SaveData*);
    static SaveData* GetMissChipSaveData(bool, bool);
    static SaveData* GetRandomSTSMissData(bool);
    static SaveData* GetSTSSpinMissData(bool);
    static SaveData* GetRandomSTSSaveData();
    static bool TriggerCallback(float, float, unsigned long, float, void*);
    static void AddAreaToGrid(SaveData*);
    static void AddSegmentToGrid(SaveData*, SaveData*);
    static void AddChainToGrid(SaveData*, bool);
    static void AddToGrid(SaveData*);
    static void ClearGrid();
    static float GridSectionWidth();
    static float GridSectionHeight();

    static float mfCatchAllowDistSq;
    static SaveData* mpSaveTable;
    static unsigned int muNumSaveEntries;
    static unsigned int muNumPositionEntries;
    static unsigned int muSTSMissIndexStart;
    static unsigned int muSTSMissCount;
    static unsigned int muSTSGoalIndexStart;
    static unsigned int muSTSGoalCount;
    static unsigned int muSTSSaveIndexStart;
    static unsigned int muSTSSaveCount;
    static unsigned int muMissChipIndexStart;
    static unsigned int muMissChipCount;
    static float mfCrouchDuration;
    static SavePositionData* mpPositionTable;
    static unsigned char mbInitialized;
};

#endif // _GOALIESAVE_H_
