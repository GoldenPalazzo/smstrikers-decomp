#include "Game/DB/CustomTournament.h"
#include "Game/Sys/debug.h"

// /**
//  * Offset/Address/Size: 0x1010 | 0x8018F618 | size: 0xF4
//  */
// void Cup<3, 3>::DeserializeData(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0xF14 | 0x8018F51C | size: 0xFC
//  */
// void Cup<3, 3>::SerializeData(void*) const
// {
// }

// /**
//  * Offset/Address/Size: 0xF0C | 0x8018F514 | size: 0x8
//  */
// void Cup<3, 3>::GetSaveDataSize() const
// {
// }

// /**
//  * Offset/Address/Size: 0xEFC | 0x8018F504 | size: 0x10
//  */
// void Cup<3, 3>::GetRoundResults(int)
// {
// }

// /**
//  * Offset/Address/Size: 0xEF4 | 0x8018F4FC | size: 0x8
//  */
// void Cup<3, 3>::GetNumRounds()
// {
// }

// /**
//  * Offset/Address/Size: 0xEEC | 0x8018F4F4 | size: 0x8
//  */
// void Cup<3, 3>::GetNumTeams()
// {
// }

// /**
//  * Offset/Address/Size: 0xED8 | 0x8018F4E0 | size: 0x14
//  */
// void Cup<3, 3>::GetTeamStats(int)
// {
// }

// /**
//  * Offset/Address/Size: 0xEBC | 0x8018F4C4 | size: 0x1C
//  */
// void Cup<3, 3>::GetGameInfo(int, int)
// {
// }

// /**
//  * Offset/Address/Size: 0xDC8 | 0x8018F3D0 | size: 0xF4
//  */
// void Cup<5, 5>::DeserializeData(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0xCCC | 0x8018F2D4 | size: 0xFC
//  */
// void Cup<5, 5>::SerializeData(void*) const
// {
// }

// /**
//  * Offset/Address/Size: 0xCC4 | 0x8018F2CC | size: 0x8
//  */
// void Cup<5, 5>::GetSaveDataSize() const
// {
// }

// /**
//  * Offset/Address/Size: 0xCB4 | 0x8018F2BC | size: 0x10
//  */
// void Cup<5, 5>::GetRoundResults(int)
// {
// }

// /**
//  * Offset/Address/Size: 0xCAC | 0x8018F2B4 | size: 0x8
//  */
// void Cup<5, 5>::GetNumRounds()
// {
// }

// /**
//  * Offset/Address/Size: 0xCA4 | 0x8018F2AC | size: 0x8
//  */
// void Cup<5, 5>::GetNumTeams()
// {
// }

// /**
//  * Offset/Address/Size: 0xC90 | 0x8018F298 | size: 0x14
//  */
// void Cup<5, 5>::GetTeamStats(int)
// {
// }

// /**
//  * Offset/Address/Size: 0xC74 | 0x8018F27C | size: 0x1C
//  */
// void Cup<5, 5>::GetGameInfo(int, int)
// {
// }

// /**
//  * Offset/Address/Size: 0xB80 | 0x8018F188 | size: 0xF4
//  */
// void Cup<7, 7>::DeserializeData(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0xA84 | 0x8018F08C | size: 0xFC
//  */
// void Cup<7, 7>::SerializeData(void*) const
// {
// }

// /**
//  * Offset/Address/Size: 0xA7C | 0x8018F084 | size: 0x8
//  */
// void Cup<7, 7>::GetSaveDataSize() const
// {
// }

// /**
//  * Offset/Address/Size: 0xA6C | 0x8018F074 | size: 0x10
//  */
// void Cup<7, 7>::GetRoundResults(int)
// {
// }

// /**
//  * Offset/Address/Size: 0xA64 | 0x8018F06C | size: 0x8
//  */
// void Cup<7, 7>::GetNumRounds()
// {
// }

// /**
//  * Offset/Address/Size: 0xA5C | 0x8018F064 | size: 0x8
//  */
// void Cup<7, 7>::GetNumTeams()
// {
// }

// /**
//  * Offset/Address/Size: 0xA48 | 0x8018F050 | size: 0x14
//  */
// void Cup<7, 7>::GetTeamStats(int)
// {
// }

// /**
//  * Offset/Address/Size: 0xA2C | 0x8018F034 | size: 0x1C
//  */
// void Cup<7, 7>::GetGameInfo(int, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x938 | 0x8018EF40 | size: 0xF4
//  */
// void Cup<3, 6>::DeserializeData(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x83C | 0x8018EE44 | size: 0xFC
//  */
// void Cup<3, 6>::SerializeData(void*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x834 | 0x8018EE3C | size: 0x8
//  */
// void Cup<3, 6>::GetSaveDataSize() const
// {
// }

// /**
//  * Offset/Address/Size: 0x824 | 0x8018EE2C | size: 0x10
//  */
// void Cup<3, 6>::GetRoundResults(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x81C | 0x8018EE24 | size: 0x8
//  */
// void Cup<3, 6>::GetNumRounds()
// {
// }

// /**
//  * Offset/Address/Size: 0x814 | 0x8018EE1C | size: 0x8
//  */
// void Cup<3, 6>::GetNumTeams()
// {
// }

// /**
//  * Offset/Address/Size: 0x800 | 0x8018EE08 | size: 0x14
//  */
// void Cup<3, 6>::GetTeamStats(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x7E4 | 0x8018EDEC | size: 0x1C
//  */
// void Cup<3, 6>::GetGameInfo(int, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x6F0 | 0x8018ECF8 | size: 0xF4
//  */
// void Cup<5, 10>::DeserializeData(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5F4 | 0x8018EBFC | size: 0xFC
//  */
// void Cup<5, 10>::SerializeData(void*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x5EC | 0x8018EBF4 | size: 0x8
//  */
// void Cup<5, 10>::GetSaveDataSize() const
// {
// }

// /**
//  * Offset/Address/Size: 0x5DC | 0x8018EBE4 | size: 0x10
//  */
// void Cup<5, 10>::GetRoundResults(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x5D4 | 0x8018EBDC | size: 0x8
//  */
// void Cup<5, 10>::GetNumRounds()
// {
// }

// /**
//  * Offset/Address/Size: 0x5CC | 0x8018EBD4 | size: 0x8
//  */
// void Cup<5, 10>::GetNumTeams()
// {
// }

// /**
//  * Offset/Address/Size: 0x5B8 | 0x8018EBC0 | size: 0x14
//  */
// void Cup<5, 10>::GetTeamStats(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x59C | 0x8018EBA4 | size: 0x1C
//  */
// void Cup<5, 10>::GetGameInfo(int, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x4A8 | 0x8018EAB0 | size: 0xF4
//  */
// void Cup<7, 14>::DeserializeData(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3AC | 0x8018E9B4 | size: 0xFC
//  */
// void Cup<7, 14>::SerializeData(void*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x3A4 | 0x8018E9AC | size: 0x8
//  */
// void Cup<7, 14>::GetSaveDataSize() const
// {
// }

// /**
//  * Offset/Address/Size: 0x394 | 0x8018E99C | size: 0x10
//  */
// void Cup<7, 14>::GetRoundResults(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x38C | 0x8018E994 | size: 0x8
//  */
// void Cup<7, 14>::GetNumRounds()
// {
// }

// /**
//  * Offset/Address/Size: 0x384 | 0x8018E98C | size: 0x8
//  */
// void Cup<7, 14>::GetNumTeams()
// {
// }

// /**
//  * Offset/Address/Size: 0x370 | 0x8018E978 | size: 0x14
//  */
// void Cup<7, 14>::GetTeamStats(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x354 | 0x8018E95C | size: 0x1C
//  */
// void Cup<7, 14>::GetGameInfo(int, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x260 | 0x8018E868 | size: 0xF4
//  */
// void Knockout<8>::DeserializeData(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x164 | 0x8018E76C | size: 0xFC
//  */
// void Knockout<8>::SerializeData(void*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x15C | 0x8018E764 | size: 0x8
//  */
// void Knockout<8>::GetSaveDataSize() const
// {
// }

// /**
//  * Offset/Address/Size: 0x14C | 0x8018E754 | size: 0x10
//  */
// void Knockout<8>::GetRoundResults(int)
// {
// }

/**
 * Offset/Address/Size: 0x144 | 0x8018E74C | size: 0x8
 */
u16 Knockout<8>::GetNumRounds()
{
    return 3;
}

// /**
//  * Offset/Address/Size: 0x13C | 0x8018E744 | size: 0x8
//  */
// void Knockout<8>::GetNumTeams()
// {
// }

// /**
//  * Offset/Address/Size: 0x128 | 0x8018E730 | size: 0x14
//  */
// void Knockout<8>::GetTeamStats(int)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8018E608 | size: 0x128
//  */
// void Knockout<8>::GetGameInfo(int, int)
// {
// }

// /**
//  * Offset/Address/Size: 0xA9C | 0x8018DF4C | size: 0x6BC
//  */
// void CustomTournament::@class$264CustomTournament_cpp::@class$264CustomTournament_cpp()
// {
// }

/**
 * Offset/Address/Size: 0xA3C | 0x8018DEEC | size: 0x60
 * TODO: 99.79% match - remaining i-diff is helper ctor symbol name in the
 * branch target (`@class$264CustomTournament_cpp` vs generated `@class$319code_c`).
 */
CustomTournament::CustomTournament()
{
    m_fenceBoundary = 0x1234ABCD;
    m_cup = NULL;
    m_tournMode = TM_INVALID;
    m_numTeams = 0;
    m_numGamesPerTeam = 0;
    m_cupConstructed = false;
    m_largestCupSize = sizeof(m_dataSpace);
}

/**
 * Offset/Address/Size: 0x9E8 | 0x8018DE98 | size: 0x54
 */
CustomTournament::~CustomTournament()
{
    if (m_cup != NULL)
    {
        m_cup = NULL;
    }
}

/**
 * Offset/Address/Size: 0x15C | 0x8018D60C | size: 0x88C
 */
BaseCup* CustomTournament::ConstructCup()
{
    if (m_cup != NULL)
    {
        m_cup = NULL;
    }

    switch (m_tournMode)
    {
    case TM_LEAGUE:
        switch (m_numGamesPerTeam)
        {
        case 1:
            switch (m_numTeams)
            {
            case 3:
                m_cup = new (&m_dataSpace.cup31) Cup<3, 3>;
                break;
            case 4:
                m_cup = new (&m_dataSpace.cup41) Cup<4, 3>;
                break;
            case 5:
                m_cup = new (&m_dataSpace.cup51) Cup<5, 5>;
                break;
            case 6:
                m_cup = new (&m_dataSpace.cup61) Cup<6, 5>;
                break;
            case 7:
                m_cup = new (&m_dataSpace.cup71) Cup<7, 7>;
                break;
            case 8:
                m_cup = new (&m_dataSpace.cup81) Cup<8, 7>;
                break;
            }
            break;
        case 2:
            switch (m_numTeams)
            {
            case 3:
                m_cup = new (&m_dataSpace.cup32) Cup<3, 6>;
                break;
            case 4:
                m_cup = new (&m_dataSpace.cup42) Cup<4, 6>;
                break;
            case 5:
                m_cup = new (&m_dataSpace.cup52) Cup<5, 10>;
                break;
            case 6:
                m_cup = new (&m_dataSpace.cup62) Cup<6, 10>;
                break;
            case 7:
                m_cup = new (&m_dataSpace.cup72) Cup<7, 14>;
                break;
            case 8:
                m_cup = new (&m_dataSpace.cup82) Cup<8, 14>;
                break;
            }
            break;
        }
        break;
    case TM_KNOCKOUT:
        switch (m_numTeams)
        {
        case 4:
            m_cup = new (&m_dataSpace.knockout4) Knockout<4>;
            break;
        case 8:
            m_cup = new (&m_dataSpace.knockout8) Knockout<8>;
            break;
        }
        break;
    }

    tDebugPrintManager::Print(DC_FE, "Tournament Cup Constructed\n");
    m_cupConstructed = true;
    return m_cup;
}

/**
 * Offset/Address/Size: 0x150 | 0x8018D600 | size: 0xC
 */
int CustomTournament::GetSaveDataSize() const
{
    return m_largestCupSize + 7;
}

/**
 * Offset/Address/Size: 0xAC | 0x8018D55C | size: 0xA4
 */
void CustomTournament::SerializeData(void* buffer) const
{
    void* ptr = buffer;

    memcpy(ptr, &m_tournMode, 4);
    ptr = (char*)ptr + 4;

    memcpy(ptr, &m_numTeams, 1);
    ptr = (char*)ptr + 1;

    memcpy(ptr, &m_numGamesPerTeam, 1);
    ptr = (char*)ptr + 1;

    memcpy(ptr, &m_cupConstructed, 1);
    ptr = (char*)ptr + 1;

    if (m_cupConstructed)
    {
        m_cup->SerializeData(ptr);
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8018D4B0 | size: 0xAC
 */
void CustomTournament::DeserializeData(void* buffer)
{
    void* ptr = buffer;

    memcpy(&m_tournMode, ptr, 4);
    ptr = (char*)ptr + 4;

    memcpy(&m_numTeams, ptr, 1);
    ptr = (char*)ptr + 1;

    memcpy(&m_numGamesPerTeam, ptr, 1);
    ptr = (char*)ptr + 1;

    memcpy(&m_cupConstructed, ptr, 1);
    ptr = (char*)ptr + 1;

    if (m_cupConstructed)
    {
        ConstructCup();
        m_cup->DeserializeData(ptr);
    }
}
