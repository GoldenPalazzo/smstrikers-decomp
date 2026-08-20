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

struct NameIDEntry
{
    const char* mName;
    int mID;
};

extern NameIDEntry NameTeamTable[9];
extern NameIDEntry NameSidekickTable[4];

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

namespace DoubleHighlite
{
void CloseItem(TLComponentInstance*);
void OpenItem(TLComponentInstance*);
void TempDisableSound();
} // namespace DoubleHighlite

namespace SingleHighlite
{
void CloseItem(TLComponentInstance*);
void OpenItem(TLComponentInstance*);
void TempDisableSound();

extern bool TEMPDISABLESOUND;
} // namespace SingleHighlite

#endif // _FEHELPFUNCS_H_
