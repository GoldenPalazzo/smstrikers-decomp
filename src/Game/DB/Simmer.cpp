#include "Game/DB/Simmer.h"
#include "Game/GameInfo.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlString.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/stdio.h"

extern "C" char* fgets(char*, int, FILE*);

static const char* SIM_FILE = "";

/**
 * Offset/Address/Size: 0x3D0 | 0x80191868 | size: 0x38
 */
Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator Tokenizer<BasicString<char, Detail::TempStringAllocator> >::begin() const
{
    return iterator(*this, m_source.m_data ? m_source.m_data->mData : (char*)0);
}

/**
 * Offset/Address/Size: 0x0 | 0x80191498 | size: 0x44
 */
Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator Tokenizer<BasicString<char, Detail::TempStringAllocator> >::end() const
{
    const char* endPtr;
    if (m_source.m_data)
    {
        endPtr = m_source.m_data->mData + m_source.m_data->mSize - 1;
    }
    else
    {
        endPtr = NULL;
    }
    return iterator(*this, endPtr);
}

/**
 * Offset/Address/Size: 0x398 | 0x80191830 | size: 0x38
 */
#pragma dont_inline on
Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator& Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator::operator++()
{
    mIter = mEnd;
    FindNextToken();
    return *this;
}
#pragma dont_inline off

/**
 * Offset/Address/Size: 0x88 | 0x80191520 | size: 0x310
 * TODO: 70.7% match - MWCC hoists tok->m_source.m_data and tok->m_delimiter.m_data outside loops,
 *       causing register shifts (r5/r6/r7 vs target r5/r6/r7 allocation)
 */
void Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator::FindNextToken()
{
    const char* iter = mIter;
    const Tokenizer<BasicString<char, Detail::TempStringAllocator> >* tok = mTokenizer;

    while (iter != (tok->m_source.m_data != 0 ? tok->m_source.m_data->mData + (tok->m_source.m_data->mSize - 1) : (char*)0))
    {
        int i = 0;
        while (i < (tok->m_delimiter.m_data != 0 ? tok->m_delimiter.m_data->mSize - 1 : 0))
        {
            signed char c = (signed char)*(unsigned char*)iter;
            signed char sep = (signed char)tok->m_delimiter.m_data->mData[i];
            if (c == sep)
            {
                break;
            }
            i++;
        }
        if (i == (tok->m_delimiter.m_data != 0 ? tok->m_delimiter.m_data->mSize - 1 : 0))
        {
            break;
        }
        iter++;
    }

    mIter = iter;
    iter = mIter;
    tok = mTokenizer;

    while (iter != (tok->m_source.m_data != 0 ? tok->m_source.m_data->mData + (tok->m_source.m_data->mSize - 1) : (char*)0))
    {
        int i = 0;
        while (i < (tok->m_delimiter.m_data != 0 ? tok->m_delimiter.m_data->mSize - 1 : 0))
        {
            signed char c = (signed char)*(unsigned char*)iter;
            signed char sep = (signed char)tok->m_delimiter.m_data->mData[i];
            if (c == sep)
            {
                break;
            }
            i++;
        }
        if (i != (tok->m_delimiter.m_data != 0 ? tok->m_delimiter.m_data->mSize - 1 : 0))
        {
            break;
        }
        iter++;
    }

    mEnd = iter;

    const char* end = mEnd;
    const char* start = mIter;
    BasicStringData<char>* data = (BasicStringData<char>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        int len = end - start + 1;
        data->mData = (char*)nlMalloc(len, 8, true);
        data->mSize = len;
        data->mCapacity = len;
        for (int j = 0; j < len; j++)
        {
            data->mData[j] = 0;
        }
        data->mRefCount = 1;
        for (int j = 0; j < data->mSize - 1; j++)
        {
            data->mData[j] = *start++;
        }
    }

    BasicString<char, Detail::TempStringAllocator> temp(data);
    mToken = temp;
}

#pragma dont_inline on
/**
 * Offset/Address/Size: 0x44 | 0x801914DC | size: 0x44
 */
Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator::iterator(
    const Tokenizer<BasicString<char, Detail::TempStringAllocator> >& tokenizer,
    const char* endPtr)
    : mTokenizer(&tokenizer)
    , mIter(endPtr)
    , mEnd(0)
{
    FindNextToken();
}
#pragma dont_inline off

/**
 * Offset/Address/Size: 0xC18 | 0x80191494 | size: 0x4
 */
Simulator::Simulator()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x0 | 0x8019087C | size: 0xC18
 * TODO: 97.83% match - this/flag registers still differ, with BasicString(line)
 *       temp pointer register drift.
 */
void Simulator::InitializeStats()
{
    eDifficultyID diff;
    GameplaySettings::eSkillLevel skillLevel = GameInfoManager::s_pInstance->GetGameplayOptions().SkillLevel;
    int length = GameInfoManager::s_pInstance->GetGameplayOptions().GameTime;
    FILE* pFile;
    unsigned char isMeanFound = 0;
    unsigned char isSDFound = 0;
    unsigned char doMean = 1;
    if (skillLevel == GameplaySettings::ROOKIE)
    {
        diff = (eDifficultyID)1;
    }
    else if (skillLevel == GameplaySettings::PROFESSIONAL)
    {
        diff = (eDifficultyID)2;
    }
    else
    {
        diff = (eDifficultyID)3;
    }
    BasicString<char, Detail::TempStringAllocator> searchString = LexicalCast<BasicString<char, Detail::TempStringAllocator> >((int)diff);
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
    char line[0x100];
    pFile = fopen(SIM_FILE, "r");
    if (pFile)
    {
        while (fgets(line, 0x100, pFile) != 0)
        {
            unsigned char isLineFound;
            if (nlStrNCmp<char>(meanString.c_str(), line, meanString.m_data ? (unsigned long)(meanString.m_data->mSize - 1) : 0) == 0)
            {
                statString = BasicString<char, Detail::TempStringAllocator>(line);
                isLineFound = 1;
                isMeanFound = 1;
                doMean = 1;
            }
            else if (nlStrNCmp<char>(SDString.c_str(), line, SDString.m_data ? (unsigned long)(SDString.m_data->mSize - 1) : 0) == 0)
            {
                statString = BasicString<char, Detail::TempStringAllocator>(line);
                isLineFound = 1;
                isSDFound = 1;
                doMean = 0;
            }
            else
            {
                isLineFound = 0;
            }
            if (isLineFound == 1)
            {
                Tokenizer<BasicString<char, Detail::TempStringAllocator> > tokenizer(statString, ",");
                int i = 0;
                Tokenizer<BasicString<char, Detail::TempStringAllocator> >::iterator iter = tokenizer.begin();
                ++iter;
                for (;
                    iter != tokenizer.end();)
                {
                    while (i == 2 || (unsigned)(i - 6) <= 3u || i == 16 || i == 18)
                    {
                        i++;
                    }
                    ePlayerStats stat = (ePlayerStats)i;
                    if (doMean == 1)
                    {
                        mStatistics[stat].mMean = (float)atof(iter.mToken.c_str());
                    }
                    else
                    {
                        mStatistics[stat].mStandardDeviation = (float)atof(iter.mToken.c_str());
                    }
                    ++iter;
                    i = (int)(stat + 1);
                }
                if (isMeanFound == 1 && isSDFound == 1)
                {
                    break;
                }
            }
        }
        fclose(pFile);
    }
}
