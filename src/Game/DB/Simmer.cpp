#include "Game/DB/Simmer.h"
#include "Game/GameInfo.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlString.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/stdio.h"

typedef struct _FILE FILE;
extern FILE* fopen(const char*, const char*);
extern int fclose(FILE*);
extern char* fgets(char*, int, FILE*);

static const char* SIM_FILE = "";

/**
 * Offset/Address/Size: 0x3D0 | 0x80191868 | size: 0x38
 */
Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator Tokenizer<BasicString<char, Detail::TempStringAllocator> >::begin() const
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x0 | 0x80191498 | size: 0x44
 */
Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator Tokenizer<BasicString<char, Detail::TempStringAllocator> >::end() const
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x398 | 0x80191830 | size: 0x38
 */
Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator& Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator::operator++()
{
    FORCE_DONT_INLINE;
}

// /**
//  * Offset/Address/Size: 0x88 | 0x80191520 | size: 0x310
//  */
// void Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator::FindNextToken()
// {
// }

// /**
//  * Offset/Address/Size: 0x44 | 0x801914DC | size: 0x44
//  */
// void Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator::iterator(const Tokenizer<BasicString<char, Detail::TempStringAllocator> >&, const char*)
// {
// }

/**
 * Offset/Address/Size: 0xC18 | 0x80191494 | size: 0x4
 */
Simulator::Simulator()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x0 | 0x8019087C | size: 0xC18
 * TODO: 72.88% match - register allocation differs (this=r22 vs r31, booleans shifted by 2)
 *       due to MWCC -O4 -inline deferred stack address hoisting optimization.
 */
void Simulator::InitializeStats()
{
    GameplaySettings::eSkillLevel skillLevel = GameInfoManager::s_pInstance->GetGameplayOptions().SkillLevel;
    int length = GameInfoManager::s_pInstance->GetGameplayOptions().GameTime;
    bool isMeanFound = false;
    bool isSDFound = false;
    bool doMean = true;
    int diff;

    if (skillLevel == GameplaySettings::ROOKIE)
    {
        diff = 1;
    }
    else if (skillLevel == GameplaySettings::PROFESSIONAL)
    {
        diff = 2;
    }
    else
    {
        diff = 3;
    }

    BasicString<char, Detail::TempStringAllocator> searchString = LexicalCast<BasicString<char, Detail::TempStringAllocator> >(diff);

    if (length <= 120)
    {
        searchString = searchString.Append(" 120");
    }
    else if (length <= 300)
    {
        searchString = searchString.Append(" 300");
    }
    else
    {
        searchString = searchString.Append(" 600");
    }

    BasicString<char, Detail::TempStringAllocator> meanString = searchString.Append(" Average");
    BasicString<char, Detail::TempStringAllocator> SDString = searchString.Append(" StdDev");
    BasicString<char, Detail::TempStringAllocator> statString;

    FILE* pFile = fopen(SIM_FILE, "r");
    if (pFile == 0)
    {
        return;
    }

    char line[0x100];
    while (fgets(line, 0x100, pFile) != 0)
    {
        bool isLineFound = false;
        unsigned long meanLen = meanString.m_data ? (unsigned long)(meanString.m_data->mSize - 1) : 0;
        const char* meanCstr = meanString.m_data ? meanString.m_data->mData : "";

        if (nlStrNCmp<char>(meanCstr, line, meanLen) == 0)
        {
            isMeanFound = true;
            doMean = true;
            statString = BasicString<char, Detail::TempStringAllocator>(line);
            isLineFound = true;
        }
        else
        {
            unsigned long sdLen = SDString.m_data ? (unsigned long)(SDString.m_data->mSize - 1) : 0;
            const char* sdCstr = SDString.m_data ? SDString.m_data->mData : "";

            if (nlStrNCmp<char>(sdCstr, line, sdLen) == 0)
            {
                isSDFound = true;
                doMean = false;
                statString = BasicString<char, Detail::TempStringAllocator>(line);
                isLineFound = true;
            }
        }

        if (isLineFound)
        {
            BasicString<char, Detail::TempStringAllocator> comma(",");
            Tokenizer<BasicString<char, Detail::TempStringAllocator> > tokenizer(statString, comma);
            int idx = 0;

            for (Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator it = tokenizer.begin(); it != tokenizer.end(); ++it)
            {
                if (idx != 2 && idx != 6 && idx != 7 && idx != 8 && idx != 9 && idx != 16 && idx != 18)
                {
                    float val = (float)atof(it.m_token.c_str());
                    if (doMean)
                    {
                        ((float*)this)[idx * 2] = val;
                    }
                    else
                    {
                        ((float*)this)[idx * 2 + 1] = val;
                    }
                }
                idx++;
            }

            if (isMeanFound && isSDFound)
            {
                break;
            }
        }
    }

    fclose(pFile);
}
