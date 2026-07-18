#ifndef _FEHELPFUNCS_H_
#define _FEHELPFUNCS_H_

#include "Game/GameInfo.h"
#include "Game/Team.h"
#include "Game/TrophyInfo.h"
#include "Game/DB/Cup.h"
#include "Game/DB/UserOptions.h"

#include "Game/FE/tlComponentInstance.h"

#include "Game/FE/tlTextInstance.h"
#include "Game/Render/Nis.h"

struct NameTeamEntry
{
    const char* name;
    eTeamID id;
};

extern NameTeamEntry NameTeamTable[9];

struct NameSidekickEntry
{
    const char* name;
    eSidekickID id;
};

extern NameSidekickEntry NameSidekickTable[4];

void MakeTextBoxReallyWide(TLTextInstance&);
const char* GetCupStreamName(eTrophyType);
const char* GetMemCardDescription();
const char* GetMemCardTitle();
void EnableAutoPressed();
TLInstance* FindComponent(TLSlide*, const char*);
unsigned long GetLOCRank(int);
eSidekickID ConvertToSidekickID(const char*);
eTeamID ConvertToTeamID(const char*);
const char* GetSidekickName(eSidekickID);
const char* GetTeamName(eTeamID);
eCharacterClass ConvertToCharacterClass(eSidekickID);
eCharacterClass ConvertToCharacterClass(eTeamID);
unsigned long GetLOCTrophyName(eTrophyType);
unsigned long GetLOCStandingsName(GameInfoManager::eGameModes);
unsigned long GetLOCModeName(GameInfoManager::eGameModes);
unsigned long GetLOCTeamName(eTeamID);
unsigned long GetLOCSidekickName(eSidekickID);
unsigned long GetLOCCharacterName(eTeamID, bool, bool);
unsigned long GetLOCDifficultyName(GameplaySettings::eSkillLevel);
unsigned long GetStadiumStringID(eStadiumID);

namespace TakeGameMemSnapshot
{
extern unsigned char gTakenSnapshot;
extern float gTimeElapsed;
void WriteToDisk();
void ResetTimers();
void Update(float);
} // namespace TakeGameMemSnapshot

class FECharacterSound
{
public:
    static void PlayCaptainSlideIn(eTeamID);
    static void PlaySidekickName(eSidekickID);
    static const char* PlayCaptainName(eTeamID);
};

class CaptainSidekickFilename
{
public:
    enum Type
    {
        TYPE_0 = 0,
        TYPE_1 = 1,
        TYPE_2 = 2,
        TYPE_3 = 3,
        TYPE_4 = 4,
    };
    static void Build(Type, char*, int, int, int);
};

extern const unsigned char PAD_COLOURS[4][3];

namespace SingleHighlite
{
void CloseItem(TLComponentInstance*);
void OpenItem(TLComponentInstance*);
void TempDisableSound();

extern bool TEMPDISABLESOUND;
} // namespace SingleHighlite

namespace DoubleHighlite
{
void CloseItem(TLComponentInstance*);
void OpenItem(TLComponentInstance*);
void TempDisableSound();
} // namespace DoubleHighlite

// class GetLOCStandingsName(GameInfoManager
// {
// public:
//     void eGameModes);
// };

// class GetLOCModeName(GameInfoManager
// {
// public:
//     void eGameModes);
// };

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

#endif // _FEHELPFUNCS_H_
