#include "Game/FormationDefines.h"
#include "NL/nlConfig.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

/**
 * Offset/Address/Size: 0x64 | 0x8003BE64 | size: 0x1C
 */
FormationSet::FormationSet()
{
    m_ID = -1;
    m_NumFormationDefs = 0;
    m_FormationDefArray = NULL;
    m_AutoDelete = false;
}

/**
 * Offset/Address/Size: 0x50 | 0x8003BE50 | size: 0x14
 */
FormationPos::FormationPos()
{
    m_Location.f.x = 0.0f;
    m_Location.f.y = 0.0f;
    m_CaptainPreference = 0.0f;
}

/**
 * Offset/Address/Size: 0x0 | 0x8003BE00 | size: 0x50
 */
FormationSpec::FormationSpec()
{
    m_ID = -1;
}

static inline float Remap(float value, float fromMin, float fromMax, float toMin, float toMax)
{
    float percent = (value - fromMin) / (fromMax - fromMin);
    if (percent > 1.0f)
        percent = 1.0f;
    if (percent < 0.0f)
        percent = 0.0f;
    return toMin + percent * (toMax - toMin);
}

/**
 * Offset/Address/Size: 0xE10 | 0x8003BC20 | size: 0xB4
 */
void FieldLocToAILoc(nlVector3& dest, const nlVector3& field_location, eTeamSide nTeamSide)
{
    float fMaxFromY, fMinFromX, fMaxFromX, fMinFromY;
    fMinFromX = -20.60211f;
    fMaxFromX = 20.60211f;
    fMinFromY = -12.0825f;
    fMaxFromY = 12.0825f;

    if (nTeamSide == AWAY)
    {
        fMinFromX = -fMinFromX;
        fMaxFromX = -fMaxFromX;
        fMinFromY = -fMinFromY;
        fMaxFromY = -fMaxFromY;
    }

    dest.f.x = Remap(field_location.f.x, fMinFromX, fMaxFromX, 0.0f, 4.0f);
    dest.f.y = Remap(field_location.f.y, fMinFromY, fMaxFromY, -1.0f, 1.0f);
    dest.f.z = 0.0f;
}

/**
 * Offset/Address/Size: 0xD5C | 0x8003BB6C | size: 0xB4
 */
void AILocToFieldLoc(nlVector3& result, const nlVector3& input, eTeamSide side)
{
    f32 maxZ;
    f32 minX = 0.0f;
    f32 normX;
    f32 maxX = -50.0f;
    f32 minZ = -35.0f;
    f32 xScale = 10.0f;
    f32 yScale = 8.0f;
    maxZ = 1.0f;

    if (side == AWAY)
    {
        minX = maxX;
        maxX = 0.0f;
        minZ = maxZ;
        maxZ = -35.0f;
    }

    normX = (input.f.x - minX) / (maxX - minX);
    if (normX > 1.0f)
        normX = 1.0f;
    if (normX < 0.0f)
        normX = 0.0f;

    result.f.x = normX * xScale + (-5.0f);
    f32 normZ = (input.f.y - minZ) / (maxZ - minZ);
    if (normZ > 1.0f)
        normZ = 1.0f;
    if (normZ < 0.0f)
        normZ = 0.0f;
    result.f.y = normZ * yScale + (-4.0f);
    result.f.z = 0.0f;
}

static inline void AILocToFieldLoc(nlVector2& dest, const nlVector2& ai_location, eTeamSide nTeamSide)
{
    float fMinFromX = -20.60211f;
    float fMaxFromX = 20.60211f;
    float fMinFromY = -12.0825f;
    float fMaxFromY = 12.0825f;

    if (nTeamSide == AWAY)
    {
        fMinFromX = -fMinFromX;
        fMaxFromX = -fMaxFromX;
        fMinFromY = -fMinFromY;
        fMaxFromY = -fMaxFromY;
    }

    dest.f.x = Remap(ai_location.f.x, 0.0f, 4.0f, fMinFromX, fMaxFromX);
    dest.f.y = Remap(ai_location.f.y, -1.0f, 1.0f, fMinFromY, fMaxFromY);
}

/**
 * Offset/Address/Size: 0xD24 | 0x8003BB34 | size: 0x38
 */
void FormationPos::GetLocationForTeam(nlVector2& dest, int teamId) const
{
    if (teamId == 0)
    {
        dest = m_Location;
        return;
    }
    nlVec2Set(dest, -m_Location.f.x, -m_Location.f.y);
}

/**
 * Offset/Address/Size: 0xCF0 | 0x8003BB00 | size: 0x34
 */
nlVector2& FormationSpec::GetKeyLocation() const
{
    if (m_iKeyIndex >= 0 && m_iKeyIndex < 4)
    {
        return const_cast<nlVector2&>(m_Positions[m_iKeyIndex].m_Location);
    }
    return *(nlVector2*)&const_cast<nlVector3&>(v3Zero);
}

/**
 * Offset/Address/Size: 0xC94 | 0x8003BAA4 | size: 0x5C
 */
void FormationSpec::CalculateExtents(nlVector2& minOut, nlVector2& maxOut, const nlVector2& input) const
{
    const float fieldHalfWidth = 18.541899f;
    const float fieldHalfHeight = 10.87425f;

    minOut.f.x = -fieldHalfWidth + (input.f.x - m_v2Min.f.x);
    maxOut.f.x = fieldHalfWidth + (input.f.x - m_v2Max.f.x);
    minOut.f.y = -fieldHalfHeight + (input.f.y - m_v2Min.f.y);
    maxOut.f.y = fieldHalfHeight + (input.f.y - m_v2Max.f.y);
}

/**
 * Offset/Address/Size: 0xC84 | 0x8003BA94 | size: 0x10
 */
FormationSpec* FormationSet::GetFormationSpec(int index) const
{
    return &m_FormationDefArray[index];
}

/**
 * Offset/Address/Size: 0xC08 | 0x8003BA18 | size: 0x7C
 */
FormationSpec* FormationSet::GetFormationSpecFromID(int formationID) const
{
    // Check for negative ID
    if (formationID < 0 || formationID >= m_NumFormationDefs)
    {
        // Fall through to linear search
    }
    else
    {
        // Try direct index access first
        FormationSpec* spec = &m_FormationDefArray[formationID];
        if (formationID == spec->m_ID)
        {
            return spec;
        }
    }

    // Linear search through all formation defs
    FormationSpec* array = m_FormationDefArray;
    int i = 0;
    int count = m_NumFormationDefs;
    for (; count > 0; count--)
    {
        if (formationID == array->m_ID)
        {
            return &m_FormationDefArray[i];
        }
        array++;
        i++;
    }

    return NULL;
}

/**
 * Offset/Address/Size: 0x0 | 0x8003AE10 | size: 0xC08
 * TODO: 97.8% match - r21/r22 register swap for formationList base and r4/r5 in copy loop
 */
FormationSet* FormationSet::LoadFormationSets(const char* filename, int& out_numsets)
{
    Config config(Config::ALLOCATE_HIGH);
    config.LoadFromFile(filename);

    out_numsets = GetConfigInt(config, "Number Of Formation Sets", 0);
    if (out_numsets == 0)
    {
        return NULL;
    }

    FormationSet* setList = new (8, false) FormationSet[out_numsets];
    char section_name[128];
    char var_name[128];
    FormationSpec formationList[42];
    int formation_id = 0;

    for (int i_set = 0; i_set < out_numsets; i_set++)
    {
        nlSNPrintf(section_name, 127, "FORMATION_SET%d", i_set);

        int i_formation;
        for (i_formation = 0;; i_formation++)
        {
            nlSNPrintf(var_name, 127, "%s/F%d_NAME", section_name, i_formation);
            if (!config.Exists(var_name))
            {
                break;
            }

            nlStrNCpy(formationList[i_formation].m_Name,
                config.Get<BasicString<char, Detail::TempStringAllocator> >(var_name,
                          BasicString<char, Detail::TempStringAllocator>("Unnamed"))
                    .c_str(),
                32);
            formationList[i_formation].m_Name[31] = 0;

            nlSNPrintf(var_name, 127, "%s/F%d_KEY_POS", section_name, i_formation);
            int keyIndex = GetConfigInt(config, var_name, -1);

            nlSNPrintf(var_name, 127, "%s/F%d_INRADIUS", section_name, i_formation);
            float inRadius = GetConfigFloat(config, var_name, 7.0f);

            nlSNPrintf(var_name, 127, "%s/F%d_OUTRADIUS", section_name, i_formation);
            float outRadius = GetConfigFloat(config, var_name, 11.0f);

            formationList[i_formation].m_InRadius = inRadius;
            formationList[i_formation].m_OutRadius = outRadius;

            for (int i_pos = 0; i_pos < 4; i_pos++)
            {
                FormationPos& position = formationList[i_formation].m_Positions[i_pos];

                nlSNPrintf(var_name, 127, "%s/F%d_P%d_X", section_name, i_formation, i_pos);
                float xVal = GetConfigFloat(config, var_name, -9999.9f);

                nlSNPrintf(var_name, 127, "%s/F%d_P%d_Y", section_name, i_formation, i_pos);
                float yVal = GetConfigFloat(config, var_name, -9999.9f);

                nlSNPrintf(var_name, 127, "%s/F%d_P%d_CAPTAINPREF", section_name, i_formation, i_pos);
                float captainPref = GetConfigFloat(config, var_name, 0.0f);

                nlVector2 ailocation = { 0.0f, 0.0f };
                nlVector2 fieldLocation;
                ailocation.f.x = xVal;
                ailocation.f.y = yVal;
                AILocToFieldLoc(fieldLocation, ailocation, HOME);

                position.m_Location = fieldLocation;
                position.m_CaptainPreference = captainPref;
            }

            formationList[i_formation].m_ID = formation_id;
            formationList[i_formation].m_iKeyIndex = keyIndex;
            formationList[i_formation].m_v2Min.f.x = 999999.9f;
            formationList[i_formation].m_v2Min.f.y = 999999.9f;
            formationList[i_formation].m_v2Max.f.x = -999999.9f;
            formationList[i_formation].m_v2Max.f.y = -999999.9f;
            formationList[i_formation].m_v2Center.f.x = 0.0f;
            formationList[i_formation].m_v2Center.f.y = 0.0f;

            FormationPos* pp = &formationList[i_formation].m_Positions[0];
            for (int n = 4; n != 0; n--)
            {
                formationList[i_formation].m_v2Min.f.x = (formationList[i_formation].m_v2Min.f.x <= pp->m_Location.f.x) ? formationList[i_formation].m_v2Min.f.x : pp->m_Location.f.x;
                formationList[i_formation].m_v2Min.f.y = (formationList[i_formation].m_v2Min.f.y <= pp->m_Location.f.y) ? formationList[i_formation].m_v2Min.f.y : pp->m_Location.f.y;
                formationList[i_formation].m_v2Max.f.x = (formationList[i_formation].m_v2Max.f.x >= pp->m_Location.f.x) ? formationList[i_formation].m_v2Max.f.x : pp->m_Location.f.x;
                formationList[i_formation].m_v2Max.f.y = (formationList[i_formation].m_v2Max.f.y >= pp->m_Location.f.y) ? formationList[i_formation].m_v2Max.f.y : pp->m_Location.f.y;
                {
                    float cx = formationList[i_formation].m_v2Center.f.x + pp->m_Location.f.x;
                    float cy = formationList[i_formation].m_v2Center.f.y + pp->m_Location.f.y;
                    formationList[i_formation].m_v2Center.f.x = cx;
                    formationList[i_formation].m_v2Center.f.y = cy;
                }
                pp++;
            }
            {
                float cx = formationList[i_formation].m_v2Center.f.x * 0.25f;
                float cy = formationList[i_formation].m_v2Center.f.y * 0.25f;
                formationList[i_formation].m_v2Center.f.x = cx;
                formationList[i_formation].m_v2Center.f.y = cy;
            }

            formation_id++;
        }

        setList[i_set].m_ID = i_set;
        setList[i_set].m_NumFormationDefs = i_formation;
        setList[i_set].m_AutoDelete = true;
        setList[i_set].m_FormationDefArray = new (8, false) FormationSpec[i_formation];

        for (int j = 0; j < i_formation; j++)
        {
            setList[i_set].m_FormationDefArray[j] = formationList[j];
        }
    }

    return setList;
}

void FormationDefines_stub()
{
    int i = 0;
    LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(i);
    float f = 0.0f;
    LexicalCast<BasicString<char, Detail::TempStringAllocator>, float>(f);
}
