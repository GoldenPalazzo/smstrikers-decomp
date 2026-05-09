#include "Game/OverlayHandlerHUD.h"
#include "Game/Team.h"
#include "Game/Sys/eventman.h"
#include "Game/FE/tlComponentInstance.h"
#include "NL/nlLexicalCast.h"

extern cTeam* g_pTeams[];

struct PowerupAcquireEventData : public EventData
{
    virtual u32 GetID();

    int mHomeAway;
};

struct InlineHasher
{
    InlineHasher() { }
    InlineHasher(unsigned long h)
        : m_Hash(h)
    {
    }
    unsigned long m_Hash;
};

template <typename T, int N>
class FEFinder
{
public:
    template <typename U>
    static T* Find(U*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
};

#ifndef NL_SINGLETON_H
template <class T>
class nlSingleton
{
public:
    static T* s_pInstance;
};
#endif

class GameInfoManager
{
public:
    virtual ~GameInfoManager();
    eTeamID GetTeam(short) const;

    char _padding[0x4955];
    bool mIsInStrikers101Mode;
};

unsigned long GetLOCCharacterName(eTeamID, bool, bool);

class HUDGameSettings
{
public:
    char _padding[0x24];
    float mClockStart;
};

class cGame
{
public:
    char _padding0[0x4];
    HUDGameSettings* mGameSettings;

    float GetGameTime();
};

extern cGame* g_pGame;

class StatsTracker
{
public:
    char _padding[0x4C1];
    bool mIsOvertime;
};

namespace Audio
{

enum eWorldSFX
{
    WORLDSFX_HUD_ACCEPT = 83,
};

class cWorldSFX
{
public:
    unsigned long Play(eWorldSFX, float, float, bool, float);
};

extern cWorldSFX gWorldSFX;

} // namespace Audio

extern int nlSNPrintf(char*, unsigned long, const char*, ...);

static const char* HUD_SLIDE_IN_NAME = "IN";
static const char* HUD_SLIDE_OUT_NAME = "OUT";
static const char* LAYER_NAME = "Layer";
static const char* CLOCK_NAME = "clock";
static char* LEFT_POWER_UP_IMAGE_NAMES[2] = {
    "left_powerup1",
    "left_powerup2",
};
static char* RIGHT_POWER_UP_IMAGE_NAMES[2] = {
    "right_powerup1",
    "right_powerup2",
};
static char* LEFT_FLARE_IMAGE_NAMES[2] = {
    "left_flare1",
    "left_flare2",
};
static char* RIGHT_FLARE_IMAGE_NAMES[2] = {
    "right_flare1",
    "right_flare2",
};
static char* LEFT_POWER_UP_TEXT_NAMES[2] = {
    "POWERUP NUMBER LEFT 1",
    "POWERUP NUMBER LEFT 2",
};
static char* RIGHT_POWER_UP_TEXT_NAMES[2] = {
    "POWERUP NUMBER RIGHT 1",
    "POWERUP NUMBER RIGHT 2",
};

#define FIND_IMAGE_PRESENTATION(presentation, name, slideName) FEFinder<TLImageInstance, 2>::Find<FEPresentation>( \
    presentation,                                                                                                  \
    InlineHasher(nlStringLowerHash(slideName)),                                                                    \
    InlineHasher(nlStringLowerHash(LAYER_NAME)),                                                                   \
    InlineHasher(nlStringLowerHash(name)),                                                                         \
    InlineHasher(0),                                                                                               \
    InlineHasher(0),                                                                                               \
    InlineHasher(0))

#define FIND_COMPONENT_PRESENTATION(presentation, name, slideName) FEFinder<TLComponentInstance, 4>::Find<FEPresentation>( \
    presentation,                                                                                                          \
    InlineHasher(nlStringLowerHash(slideName)),                                                                            \
    InlineHasher(nlStringLowerHash(LAYER_NAME)),                                                                           \
    InlineHasher(nlStringLowerHash(name)),                                                                                 \
    InlineHasher(0),                                                                                                       \
    InlineHasher(0),                                                                                                       \
    InlineHasher(0))

/**
 * Offset/Address/Size: 0x124 | 0x800FA3BC | size: 0xCF0
 */

// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800FA298 | size: 0x124
 */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[8], unsigned short[8]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[8], const unsigned short(&)[8])
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800FA290 | size: 0x8
 */
// u32 PowerupAcquireEventData::GetID()
// {
//     return 0x1C3;
// }

/**
 * Offset/Address/Size: 0x7C0 | 0x800FA20C | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x788 | 0x800FA1D4 | size: 0x38
 */
// void FEFinder<TLTextInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x62C | 0x800FA078 | size: 0x15C
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x5A8 | 0x800F9FF4 | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x570 | 0x800F9FBC | size: 0x38
 */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x4EC | 0x800F9F38 | size: 0x84
 */
// void FEFinder<TLImageInstance, 2>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x4B4 | 0x800F9F00 | size: 0x38
 */
// void FEFinder<TLImageInstance, 2>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x358 | 0x800F9DA4 | size: 0x15C
 */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x2D4 | 0x800F9D20 | size: 0x84
 */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x250 | 0x800F9C9C | size: 0x84
 */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x218 | 0x800F9C64 | size: 0x38
 */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0xBC | 0x800F9B08 | size: 0x15C
 */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x38 | 0x800F9A84 | size: 0x84
 */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800F9A4C | size: 0x38
 */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x36B8 | 0x800F9998 | size: 0xB4
 */
HUDOverlay::HUDOverlay()
    : BaseOverlayHandler(2)
{
    mSeconds = -1;
    mTenths = 0;
    mClockColourChanged = false;
    mOvertimeSFXPlayed = false;
    mStartScoreAnimation = false;
    mNumFlareCycles[0][0] = -1;
    mScore[0] = 0;
    mNewScore[0] = 0;
    mScoreUpdateDelay[0] = 0.0f;
    mNumFlareCycles[0][1] = -1;
    mScore[0] = 0;
    mNewScore[0] = 0;
    mScoreUpdateDelay[0] = 0.0f;
    mNumFlareCycles[0][2] = -1;
    mScore[1] = 0;
    mNewScore[1] = 0;
    mScoreUpdateDelay[1] = 0.0f;
    mNumFlareCycles[0][3] = -1;
    mScore[1] = 0;
    mNewScore[1] = 0;
    mScoreUpdateDelay[1] = 0.0f;
}

/**
 * Offset/Address/Size: 0x35FC | 0x800F98DC | size: 0xBC
 */
HUDOverlay::~HUDOverlay()
{
    delete this->mAsyncImage[0];
    delete this->mAsyncImage[1];
}

/**
 * Offset/Address/Size: 0x2A1C | 0x800F8CFC | size: 0xBE0
 */
void HUDOverlay::Update(float fDeltaT)
{
    struct HUDPresentationTime
    {
        char _padding[0x18];
        float mTime;
    };

    extern unsigned char g_hudVisible;

    BaseSceneHandler::Update(fDeltaT);
    mAsyncImage[0]->Update(true);
    mAsyncImage[1]->Update(true);

    if (!g_hudVisible)
    {
        SetVisible(false);
    }

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    for (int i = 0; i < 2; ++i)
    {
        if (!mIsHUDSlideIn)
        {
            continue;
        }

        if (mScoreUpdateDelay[i] <= 0.0f)
        {
            continue;
        }

        if (((HUDPresentationTime*)presentation)->mTime > 1.0f)
        {
            continue;
        }

        if (mStartScoreAnimation)
        {
            TLComponentInstance* pScoreComp;
            if (i == 0)
            {
                pScoreComp = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
                    presentation,
                    InlineHasher(nlStringLowerHash("IN")),
                    InlineHasher(nlStringLowerHash(LAYER_NAME)),
                    InlineHasher(nlStringLowerHash("left_score")),
                    InlineHasher(0),
                    InlineHasher(0),
                    InlineHasher(0));
            }
            else
            {
                pScoreComp = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
                    presentation,
                    InlineHasher(nlStringLowerHash("IN")),
                    InlineHasher(nlStringLowerHash(LAYER_NAME)),
                    InlineHasher(nlStringLowerHash("right_score")),
                    InlineHasher(0),
                    InlineHasher(0),
                    InlineHasher(0));
            }

            TLSlide* activeSlide = pScoreComp->GetActiveSlide();
            if (activeSlide->m_time >= activeSlide->m_start + activeSlide->m_duration)
            {
                pScoreComp->SetActiveSlide("Slide1");
                pScoreComp->Update(0.0f);
                mStartScoreAnimation = false;
            }
        }
        else
        {
            mScoreUpdateDelay[i] -= fDeltaT;

            if (mScoreUpdateDelay[i] <= 0.0f)
            {
                mScoreUpdateDelay[i] = 0.0f;
                mScore[i] += 1;

                if (mScore[i] < mNewScore[i])
                {
                    mScoreUpdateDelay[i] = 0.5f;
                    mStartScoreAnimation = true;
                }

                BasicString<char, Detail::TempStringAllocator> scoreString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mScore[i]);
                nlStrToWcs(scoreString.c_str(), mScoreBuffer[i], 0x20);
                m_pTextInstanceScore[0][i]->SetString(mScoreBuffer[i]);
                m_pTextInstanceScore[1][i]->SetString(mScoreBuffer[i]);
            }
        }
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
    {
        DisplayPowerUps();
        return;
    }

    bool isOvertime = nlSingleton<StatsTracker>::s_pInstance->mIsOvertime;

    float gameTime = g_pGame->GetGameTime();
    float gameClockStart = g_pGame->mGameSettings->mClockStart;
    float overtimeTime = 59999.0f;
    float fTime = gameTime - gameClockStart;
    float fRemainingTime = gameClockStart - gameTime;

    if (fTime <= overtimeTime)
    {
        overtimeTime = fTime;
    }

    unsigned long time = (unsigned long)fRemainingTime;
    unsigned long remainingTime = isOvertime ? (unsigned long)overtimeTime : time;
    unsigned long newMinutes = remainingTime / 60;
    unsigned long newSeconds = remainingTime - (newMinutes * 60);
    unsigned long newTenths = 0;

    if (fRemainingTime < 30.0f || isOvertime)
    {
        if (!mClockColourChanged)
        {
            mClockColourChanged = true;

            nlColour clockColour;
            clockColour.c[0] = 0xCC;
            clockColour.c[1] = 0x33;
            clockColour.c[2] = 0x33;
            clockColour.c[3] = 0xFF;

            m_pTextInstanceClock[0]->SetAssetColour(clockColour);
            m_pTextInstanceClock[1]->SetAssetColour(clockColour);

            Audio::gWorldSFX.Play(Audio::WORLDSFX_HUD_ACCEPT, 100.0f, -1.0f, true, 100.0f);
            Audio::gWorldSFX.Play(Audio::WORLDSFX_HUD_ACCEPT, 100.0f, 0.25f, true, 100.0f);
        }
    }

    if (newMinutes == 0 && fRemainingTime < 30.0f && !isOvertime)
    {
        newTenths = (unsigned long)((fRemainingTime - (float)newSeconds) * 10.0f);
    }

    if (!isOvertime && (float)remainingTime == gameClockStart && mClockColourChanged)
    {
        mClockColourChanged = false;
        mOvertimeSFXPlayed = false;
        m_pTextInstanceClock[0]->SetAssetColour(mOriginalClockColour);
        m_pTextInstanceClock[1]->SetAssetColour(mOriginalClockColour);
    }

    if (isOvertime)
    {
        if (!mOvertimeSFXPlayed)
        {
            Audio::gWorldSFX.Play((Audio::eWorldSFX)0xCB, 100.0f, -1.0f, true, 100.0f);
            mOvertimeSFXPlayed = true;
        }

        mSuddenDeath[0]->m_bVisible = true;
        mSuddenDeath[1]->m_bVisible = true;
    }
    else
    {
        mSuddenDeath[0]->m_bVisible = false;
        mSuddenDeath[1]->m_bVisible = false;
    }

    if (newSeconds != mSeconds || newMinutes != mMinutes || newTenths != mTenths)
    {
        if (time < 5 && !isOvertime && newSeconds != mSeconds && mTenths == 0 && mSeconds != 0)
        {
            Audio::gWorldSFX.Play(Audio::WORLDSFX_HUD_ACCEPT, 100.0f, -1.0f, true, 100.0f);
        }

        mSeconds = newSeconds;
        mMinutes = newMinutes;
        mTenths = newTenths;

        char minutesString[8];
        char secondsString[8];
        char timeString[32];

        if (mMinutes == 0 && fRemainingTime < 30.0f && !isOvertime)
        {
            nlSNPrintf(minutesString, 8, "%d", mSeconds);
            nlSNPrintf(secondsString, 8, "%d", mTenths);
            nlSNPrintf(timeString, 32, "%s.%s", minutesString, secondsString);
        }
        else
        {
            if (mSeconds < 10)
            {
                nlSNPrintf(secondsString, 8, "0%d", mSeconds);
            }
            else
            {
                nlSNPrintf(secondsString, 8, "%d", mSeconds);
            }

            nlSNPrintf(minutesString, 8, "%d", mMinutes);
            nlSNPrintf(timeString, 32, "%s:%s", minutesString, secondsString);
        }

        nlStrToWcs(timeString, mClockBuffer, 0x20);
        m_pTextInstanceClock[0]->SetString(mClockBuffer);
        m_pTextInstanceClock[1]->SetString(mClockBuffer);
    }

    DisplayPowerUps();
}

/**
 * Offset/Address/Size: 0x2120 | 0x800F8400 | size: 0x8FC
 */
void HUDOverlay::SceneCreated()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    TLComponentInstance* pScoreComp;
    TLSlide* pScoreSlide;
    eTeamID team;
    TLTextInstance* pTeamName;

    m_pTextInstanceClock[0] = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash(CLOCK_NAME)),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    m_pTextInstanceClock[1] = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash(CLOCK_NAME)),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_OUT_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mOriginalClockColour = m_pTextInstanceClock[0]->GetColour();

    pScoreComp = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("left_score")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pScoreSlide = pScoreComp->GetActiveSlide();
    pScoreComp->Update(pScoreSlide->m_start + pScoreSlide->m_duration);
    m_pTextInstanceScore[0][0] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pScoreSlide,
        InlineHasher(nlStringLowerHash("left_score")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pScoreComp = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("right_score")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pScoreSlide = pScoreComp->GetActiveSlide();
    pScoreComp->Update(pScoreSlide->m_start + pScoreSlide->m_duration);
    m_pTextInstanceScore[0][1] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pScoreSlide,
        InlineHasher(nlStringLowerHash("right_score")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pScoreComp = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("left_score")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_OUT_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pScoreSlide = pScoreComp->GetActiveSlide();
    pScoreComp->Update(pScoreSlide->m_start + pScoreSlide->m_duration);
    m_pTextInstanceScore[1][0] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pScoreSlide,
        InlineHasher(nlStringLowerHash("left_score")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pScoreComp = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("right_score")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_OUT_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pScoreSlide = pScoreComp->GetActiveSlide();
    pScoreComp->Update(pScoreSlide->m_start + pScoreSlide->m_duration);
    m_pTextInstanceScore[1][1] = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pScoreSlide,
        InlineHasher(nlStringLowerHash("right_score")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mSuddenDeath[0] = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("SUDDEN DEATH")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mSuddenDeath[1] = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("SUDDEN DEATH")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_OUT_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mSuddenDeath[0]->m_bVisible = false;
    mSuddenDeath[1]->m_bVisible = false;

    LoadHUDTextures();
    SetTeamIcons();

    team = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);

    pTeamName = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("LEFT NAME")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pTeamName->m_LocStrId = GetLOCCharacterName(team, true, false);
    pTeamName->m_OverloadFlags |= 8;

    pTeamName = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("LEFT NAME")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_OUT_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pTeamName->m_LocStrId = GetLOCCharacterName(team, true, false);
    pTeamName->m_OverloadFlags |= 8;

    team = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);

    pTeamName = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("RIGHT NAME")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pTeamName->m_LocStrId = GetLOCCharacterName(team, true, false);
    pTeamName->m_OverloadFlags |= 8;

    pTeamName = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("RIGHT NAME")),
        InlineHasher(nlStringLowerHash(LAYER_NAME)),
        InlineHasher(nlStringLowerHash(HUD_SLIDE_OUT_NAME)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pTeamName->m_LocStrId = GetLOCCharacterName(team, true, false);
    pTeamName->m_OverloadFlags |= 8;

    if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
    {
        m_pTextInstanceClock[0]->m_bVisible = false;
        m_pTextInstanceClock[1]->m_bVisible = false;
    }

    presentation->SetActiveSlide(HUD_SLIDE_OUT_NAME);
    mIsHUDSlideIn = false;

    for (int i = 0; i < 2; i++)
    {
        mScore[i] = 0;
        mNewScore[i] = 0;

        BasicString<char, Detail::TempStringAllocator> scoreStr = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mScore[i]);

        nlStrToWcs(scoreStr.c_str(), mScoreBuffer[i], 0x20);
        m_pTextInstanceScore[0][i]->SetString(mScoreBuffer[i]);
        m_pTextInstanceScore[1][i]->SetString(mScoreBuffer[i]);
    }

    mStartScoreAnimation = false;
}

/**
 * Offset/Address/Size: 0x20DC | 0x800F83BC | size: 0x44
 */
void HUDOverlay::SetSlideIn()
{
    FEPresentation* presentation;
    presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    presentation->SetActiveSlide("IN");
    mIsHUDSlideIn = true;
}

/**
 * Offset/Address/Size: 0x2098 | 0x800F8378 | size: 0x44
 */
void HUDOverlay::SetSlideOut()
{
    FEPresentation* presentation;
    presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    presentation->SetActiveSlide("OUT");
    mIsHUDSlideIn = false;
}

/**
 * Offset/Address/Size: 0x14BC | 0x800F779C | size: 0xBDC
 */
void HUDOverlay::LoadHUDTextures()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    TLImageInstance* pImageInstance;
    int i;
    TLComponentInstance* pComp;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "star", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pStar = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "shell_green", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pShellGreen = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "shell_red", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pShellRed = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "banana", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pBanana = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "mushroom", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pMushroom = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "shell_blue", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pShellBlue = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "shell_spike", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pShellSpike = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "bobomb", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pBobomb = pImageInstance->m_pTextureResource;

    pImageInstance = FIND_IMAGE_PRESENTATION(presentation, "chomp", HUD_SLIDE_IN_NAME);
    pImageInstance->m_bVisible = false;
    m_pChomp = pImageInstance->m_pTextureResource;

    for (i = 0; i < 2; i++)
    {
        pComp = FIND_COMPONENT_PRESENTATION(presentation, LEFT_POWER_UP_IMAGE_NAMES[i], HUD_SLIDE_IN_NAME);
        m_pImagePowerUps[0][0][i] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            pComp->GetActiveSlide(),
            InlineHasher(nlStringLowerHash(LEFT_POWER_UP_IMAGE_NAMES[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        pComp = FIND_COMPONENT_PRESENTATION(presentation, RIGHT_POWER_UP_IMAGE_NAMES[i], HUD_SLIDE_IN_NAME);
        m_pImagePowerUps[0][1][i] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            pComp->GetActiveSlide(),
            InlineHasher(nlStringLowerHash(RIGHT_POWER_UP_IMAGE_NAMES[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        pComp = FIND_COMPONENT_PRESENTATION(presentation, LEFT_FLARE_IMAGE_NAMES[i], HUD_SLIDE_IN_NAME);
        m_pComponentFlares[0][i] = pComp;
        m_pImageFlares[0][0][i] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            pComp->GetActiveSlide(),
            InlineHasher(nlStringLowerHash(LEFT_FLARE_IMAGE_NAMES[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        m_pImageFlares[0][0][i]->m_bVisible = false;

        pComp = FIND_COMPONENT_PRESENTATION(presentation, RIGHT_FLARE_IMAGE_NAMES[i], HUD_SLIDE_IN_NAME);
        m_pComponentFlares[1][i] = pComp;
        m_pImageFlares[0][1][i] = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            pComp->GetActiveSlide(),
            InlineHasher(nlStringLowerHash(RIGHT_FLARE_IMAGE_NAMES[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        m_pImageFlares[0][1][i]->m_bVisible = false;

        m_pPowerupTextComponents[0][0][i] = FIND_COMPONENT_PRESENTATION(presentation, LEFT_POWER_UP_TEXT_NAMES[i], HUD_SLIDE_IN_NAME);
        m_pPowerupTextComponents[0][1][i] = FIND_COMPONENT_PRESENTATION(presentation, RIGHT_POWER_UP_TEXT_NAMES[i], HUD_SLIDE_IN_NAME);

        m_pImagePowerUps[1][0][i] = FIND_IMAGE_PRESENTATION(presentation, LEFT_POWER_UP_IMAGE_NAMES[i], HUD_SLIDE_OUT_NAME);
        m_pImagePowerUps[1][1][i] = FIND_IMAGE_PRESENTATION(presentation, RIGHT_POWER_UP_IMAGE_NAMES[i], HUD_SLIDE_OUT_NAME);

        m_pImageFlares[1][0][i] = FIND_IMAGE_PRESENTATION(presentation, LEFT_FLARE_IMAGE_NAMES[i], HUD_SLIDE_OUT_NAME);
        m_pImageFlares[1][0][i]->m_bVisible = false;

        m_pImageFlares[1][1][i] = FIND_IMAGE_PRESENTATION(presentation, RIGHT_FLARE_IMAGE_NAMES[i], HUD_SLIDE_OUT_NAME);
        m_pImageFlares[1][1][i]->m_bVisible = false;

        m_pPowerupTextComponents[1][0][i] = FIND_COMPONENT_PRESENTATION(presentation, LEFT_POWER_UP_TEXT_NAMES[i], HUD_SLIDE_OUT_NAME);
        m_pPowerupTextComponents[1][1][i] = FIND_COMPONENT_PRESENTATION(presentation, RIGHT_POWER_UP_TEXT_NAMES[i], HUD_SLIDE_OUT_NAME);
    }
}

#undef FIND_IMAGE_PRESENTATION
#undef FIND_COMPONENT_PRESENTATION

/**
 * Offset/Address/Size: 0x1168 | 0x800F7448 | size: 0x354
 */
void HUDOverlay::DisplayPowerUps()
{
    FETextureResource* texture[2];

    for (int homeAway = 0; homeAway < 2; homeAway++)
    {
        for (int i = 0; i < 2; i++)
        {
            int numPowerups = g_pTeams[homeAway]->GetPowerUpByIndex(i).nnumOfPowerups;

            switch (g_pTeams[homeAway]->GetPowerUpByIndex(i).eType)
            {
            case POWER_UP_NONE:
                texture[i] = NULL;
                break;
            case POWER_UP_GREEN_SHELL:
                texture[i] = m_pShellGreen;
                break;
            case POWER_UP_SPINY_SHELL:
                texture[i] = m_pShellSpike;
                break;
            case POWER_UP_FREEZE_SHELL:
                texture[i] = m_pShellBlue;
                break;
            case POWER_UP_RED_SHELL:
                texture[i] = m_pShellRed;
                break;
            case POWER_UP_MUSHROOM:
                texture[i] = m_pMushroom;
                break;
            case POWER_UP_BANANA:
                texture[i] = m_pBanana;
                break;
            case POWER_UP_BOBOMB:
                texture[i] = m_pBobomb;
                break;
            case POWER_UP_STAR:
                texture[i] = m_pStar;
                break;
            case POWER_UP_CHAIN_CHOMP:
                texture[i] = m_pChomp;
                break;
            }

            if (texture[i] == NULL)
            {
                m_pImagePowerUps[0][homeAway][i]->m_bVisible = false;
                m_pImagePowerUps[1][homeAway][i]->m_bVisible = false;
                if (mNumFlareCycles[homeAway][i] >= 0)
                {
                    mNumFlareCycles[homeAway][i] = -1;
                    m_pImageFlares[0][homeAway][i]->m_bVisible = false;
                }
                m_pPowerupTextComponents[0][homeAway][i]->SetActiveSlide("1");
                m_pPowerupTextComponents[1][homeAway][i]->SetActiveSlide("1");
            }
            else
            {
                if (g_pTeams[homeAway]->GetPowerUpByIndex(i).bIsNew && mNumFlareCycles[homeAway][i] == -1)
                {
                    m_pImageFlares[0][homeAway][i]->m_bVisible = true;
                    m_pComponentFlares[homeAway][i]->SetActiveSlide("Slide1");
                    m_pComponentFlares[homeAway][i]->Update(0.0f);
                    mNumFlareCycles[homeAway][i] = 20;
                }
                else if (mNumFlareCycles[homeAway][i] != -1)
                {
                    TLSlide* activeSlide = m_pComponentFlares[homeAway][i]->GetActiveSlide();
                    if (activeSlide->m_time >= activeSlide->m_start + activeSlide->m_duration - 0.1f)
                    {
                        m_pImagePowerUps[0][homeAway][i]->m_bVisible = true;
                        m_pImagePowerUps[1][homeAway][i]->m_bVisible = true;
                        m_pImageFlares[0][homeAway][i]->m_bVisible = false;
                        m_pImageFlares[1][homeAway][i]->m_bVisible = false;
                        g_pTeams[homeAway]->SetIsPowerUpNew(i, false);
                        mNumFlareCycles[homeAway][i] = -1;

                        if (mIsHUDSlideIn)
                        {
                            PowerupAcquireEventData* data = new ((u8*)g_pEventManager->CreateValidEvent(0x69, 0x1C) + 0x10) PowerupAcquireEventData();
                            data->mHomeAway = homeAway;
                        }
                    }
                }
            }

            m_pImagePowerUps[0][homeAway][i]->m_component->pChildren = (TLSlide*)texture[i];
            m_pImagePowerUps[1][homeAway][i]->m_component->pChildren = (TLSlide*)texture[i];

            if (mNumFlareCycles[homeAway][i] == -1 && texture[i] != NULL)
            {
                m_pImagePowerUps[0][homeAway][i]->m_bVisible = true;
                m_pImagePowerUps[1][homeAway][i]->m_bVisible = true;
            }

            if (mNumFlareCycles[homeAway][i] != -1 || numPowerups == 1 || numPowerups == 0)
            {
                m_pPowerupTextComponents[0][homeAway][i]->SetActiveSlide("1");
                m_pPowerupTextComponents[1][homeAway][i]->SetActiveSlide("1");
            }
            else if (numPowerups == 3)
            {
                m_pPowerupTextComponents[0][homeAway][i]->SetActiveSlide("X3");
                m_pPowerupTextComponents[1][homeAway][i]->SetActiveSlide("X3");
            }
            else if (numPowerups == 5)
            {
                m_pPowerupTextComponents[0][homeAway][i]->SetActiveSlide("X5");
                m_pPowerupTextComponents[1][homeAway][i]->SetActiveSlide("X5");
            }
        }
    }
}

u32 PowerupAcquireEventData::GetID()
{
    return 0x1C3;
}

/**
 * Offset/Address/Size: 0x3C0 | 0x800F66A0 | size: 0xDA8
 */
void HUDOverlay::SetTeamIcons()
{
    TLComponentInstance* pCompLeft = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CAPTAIN SYMBOL L")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    TLComponentInstance* pCompRight = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash(HUD_SLIDE_IN_NAME)),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CAPTAIN SYMBOL R")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    const char* filename = "art/fe/CaptainIconsUI.res";

    AsyncImage* pImage = (AsyncImage*)nlMalloc(sizeof(AsyncImage), 8, false);
    if (pImage != 0)
    {
        pImage = new (pImage) AsyncImage(filename, 0);
    }
    mAsyncImage[0] = pImage;
    mAsyncImage[0]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        pCompLeft->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("CAPTAIN_ICONS_MARIO")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pImage = (AsyncImage*)nlMalloc(sizeof(AsyncImage), 8, false);
    if (pImage != 0)
    {
        pImage = new (pImage) AsyncImage(filename, 0);
    }
    mAsyncImage[1] = pImage;
    mAsyncImage[1]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        pCompRight->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("CAPTAIN_ICONS_DK_r")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    BasicString<char, Detail::TempStringAllocator> iconfilename[2];

    for (int i = 0; i < 2; i++)
    {
        eTeamID teamid = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(i);

        switch (teamid)
        {
        case TEAM_DAISY:
            iconfilename[i] = "fe/captain_icons/captain_icons_daisy";
            break;
        case TEAM_DONKEYKONG:
            if (i == 0)
            {
                iconfilename[i] = "fe/captain_icons/captain_icons_dk";
            }
            else
            {
                iconfilename[i] = "fe/captain_icons/captain_icons_dk_r";
            }
            break;
        case TEAM_LUIGI:
            iconfilename[i] = "fe/captain_icons/captain_icons_luigi";
            break;
        case TEAM_MARIO:
            iconfilename[i] = "fe/captain_icons/captain_icons_mario";
            break;
        case TEAM_PEACH:
            iconfilename[i] = "fe/captain_icons/captain_icons_peach";
            break;
        case TEAM_WALUIGI:
            iconfilename[i] = "fe/captain_icons/captain_icons_waluigi";
            break;
        case TEAM_WARIO:
            iconfilename[i] = "fe/captain_icons/captain_icons_wario";
            break;
        case TEAM_YOSHI:
            iconfilename[i] = "fe/captain_icons/captain_icons_yoshi";
            break;
        case TEAM_MYSTERY:
            iconfilename[i] = "fe/captain_icons/captain_icons_super";
            break;
        }

        mAsyncImage[i]->QueueLoad(iconfilename[i].c_str(), true);
    }
}

/**
 * Offset/Address/Size: 0x39C | 0x800F667C | size: 0x24
 */
void HUDOverlay::UpdateScore()
{
    mNewScore[0] = g_pTeams[0]->m_nScore;
    mNewScore[1] = g_pTeams[1]->m_nScore;
}

/**
 * Offset/Address/Size: 0x2F0 | 0x800F65D0 | size: 0xAC
 */
void HUDOverlay::DisplayNewScore()
{
    for (int team = 0; team < 2; team++)
    {
        if (mNewScore[team] != mScore[team])
        {
            mScoreUpdateDelay[team] = 0.0f;
            mStartScoreAnimation = true;
        }

        for (int flare = 0; flare < 2; flare++)
        {
            if (mNumFlareCycles[team][flare] != -1)
            {
                mNumFlareCycles[team][flare] = 20;
                m_pComponentFlares[team][flare]->SetActiveSlide("Slide1");
                m_pComponentFlares[team][flare]->Update(0.0f);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x188 | 0x800F6468 | size: 0x168
 */
void HUDOverlay::ResetScores()
{
    for (int i = 0; i < 2; i++)
    {
        mScore[i] = 0;
        mNewScore[i] = 0;
        BasicString<char, Detail::TempStringAllocator> scoreStr = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mScore[i]);
        nlStrToWcs(scoreStr.c_str(), mScoreBuffer[i], 0x20);
        m_pTextInstanceScore[0][i]->SetString(mScoreBuffer[i]);
        m_pTextInstanceScore[1][i]->SetString(mScoreBuffer[i]);
    }
    mStartScoreAnimation = false;
}

/**
 * Offset/Address/Size: 0x0 | 0x800F62E0 | size: 0x188
 */
void HUDOverlay::SwapPowerUps(int homeAway)
{
    int temp = mNumFlareCycles[homeAway][0];
    mNumFlareCycles[homeAway][0] = mNumFlareCycles[homeAway][1];
    mNumFlareCycles[homeAway][1] = temp;

    f32 time0 = m_pComponentFlares[homeAway][0]->GetActiveSlide()->m_time;
    f32 time1 = m_pComponentFlares[homeAway][1]->GetActiveSlide()->m_time;

    m_pComponentFlares[homeAway][0]->SetActiveSlide("Slide1");
    m_pComponentFlares[homeAway][0]->Update(time1);
    m_pComponentFlares[homeAway][1]->SetActiveSlide("Slide1");
    m_pComponentFlares[homeAway][1]->Update(time0);

    for (int i = 0; i < 2; i++)
    {
        if (mNumFlareCycles[homeAway][i] == -1)
        {
            m_pImageFlares[0][homeAway][i]->m_bVisible = false;
            m_pImageFlares[1][homeAway][i]->m_bVisible = false;
            m_pImagePowerUps[0][homeAway][i]->m_bVisible = true;
            m_pImagePowerUps[1][homeAway][i]->m_bVisible = true;
        }
        else
        {
            m_pImageFlares[0][homeAway][i]->m_bVisible = true;
            m_pImageFlares[1][homeAway][i]->m_bVisible = true;
            m_pImagePowerUps[0][homeAway][i]->m_bVisible = false;
            m_pImagePowerUps[1][homeAway][i]->m_bVisible = false;
        }
    }
}
