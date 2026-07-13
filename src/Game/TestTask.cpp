#define BASICSTRING_DELEGATING_CTOR
#define BASICSTRING_INLINE_ERASE
#pragma pool_data off

#include "Game/TestTask.h"

#include "NL/nlDebugFile.h"
#include "NL/nlMemory.h"
#include "NL/nlConfig.h"
#include "types.h"

#include "NL/nlBasicString.h"
#include "NL/nlFormat.h"

typedef BasicString<char, Detail::TempStringAllocator> NLString;
typedef NLString (*Format2FFn)(const NLString&, const float&, const float&);

void TestTask_stub()
{
    NLString format;
    float value1 = 0.0f;
    float value2 = 0.0f;
    volatile Format2FFn fn2 = Format<NLString, float, float>;
    fn2(format, value1, value2);
}

namespace
{
const char* testLog = "test.log";
static const char* smokeTestSuccessOutput = "smoke_test.success";
static const char* frameRateTestSuccessOutput = "frame_rate.success";
} // namespace

/**
 * Offset/Address/Size: 0xB4C | 0x8016D448 | size: 0x44
 */
TestTask::TestTask()
{
    mTestLog = 0;
    mTestTimeOut = 0.0f;
    mRunSmokeTest = false;
    mRunUnitTests = false;
    mFrameRateTestFailure = false;
    mRunFrameRateTest = false;
    mMinimumFrameRate = 10.0f;
}

/**
 * Offset/Address/Size: 0x774 | 0x8016D070 | size: 0x3D8
 */
void TestTask::Initialize()
{
    mTestTimeOut = GetConfigFloat(Config::Global(), "test/time_out_sec ", 10.0f);
    mRunUnitTests = GetConfigBool(Config::Global(), "test/unit_tests", false);
    mRunSmokeTest = GetConfigBool(Config::Global(), "test/smoke", false);
    mRunFrameRateTest = GetConfigBool(Config::Global(), "test/frame_rate", false);
    mMinimumFrameRate = GetConfigFloat(Config::Global(), "test/frame_rate_min", 20.0f);

    if (GetConfigBool(Config::Global(), "test/enable", false))
    {
        mTestLog = nlOpenFileDebug(testLog, false, false);
    }
}

/**
 * Offset/Address/Size: 0x6F0 | 0x8016CFEC | size: 0x84
 */
void TestTask::Run(float dt)
{
    if (!mTestLog)
    {
        return;
    }

    mTestTimeOut = mTestTimeOut - dt;

    RunSmokeTest(dt);
    RunFrameRateTest(dt);

    if (mTestTimeOut < 0.0f)
    {
        nlCloseFileDebug(mTestLog);
        mTestLog = 0;
    }
}

static inline void CreateSuccessFile(const char* filename)
{
    void* file = nlOpenFileDebug(filename, false, false);
    if (file)
    {
        nlWriteLineDebug(file, filename, false);
        nlFlushFileDebug(file);
        nlCloseFileDebug(file);
    }
}

static inline void WriteToTestLog(TestTask* self, const char* msg)
{
    if (self->mTestLog)
    {
        nlWriteLineDebug(self->mTestLog, msg, false);
        nlWriteLineDebug(self->mTestLog, "\n", false);
        nlFlushFileDebug(self->mTestLog);
    }
}

/**
 * Offset/Address/Size: 0x424 | 0x8016CD20 | size: 0x2CC
 * TODO: 98.49% match - this-pointer vs string-cursor callee-saved swap
 * (this held in r31 vs target r29; text/filename cursors in r29 vs target r31).
 */
void TestTask::RunSmokeTest(float)
{
    if (mRunSmokeTest)
    {
        if (mTestTimeOut <= 0.0f)
        {
            CreateSuccessFile(smokeTestSuccessOutput);
            NLString fmt("SUCCES: smoke test successful, didn't crash for {0} seconds");
            WriteToTestLog(this, Format(fmt, GetConfigFloat(Config::Global(), "test/time_out_sec ", 10.0f)).c_str());
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8016C8FC | size: 0x424
 * TODO: 99.60% match - the failure string's allocation pointer and scan cursor
 * use r31/r30 instead of r30/r31.
 */
void TestTask::RunFrameRateTest(float dt)
{
    if (mRunFrameRateTest)
    {
        float frameRateThreshold = 1.0f / mMinimumFrameRate;

        if (dt > frameRateThreshold)
        {
            mFrameRateTestFailure = true;

            NLString fmt("FAILURE: frame rate dropped below {0}, namely {1}");
            float actualFrameRate = 1.0f / dt;
            WriteToTestLog(this, Format(fmt, mMinimumFrameRate, actualFrameRate).c_str());
        }

        if (mTestTimeOut <= 0.0f && !mFrameRateTestFailure)
        {
            CreateSuccessFile(frameRateTestSuccessOutput);
            WriteToTestLog(this, Format(NLString("SUCCES: frame rate test successful, never dropped below {0}"), mMinimumFrameRate).c_str());
        }
    }
}

typedef FormatImpl<NLString> NLFormatImpl;

static inline void EraseRange(NLString& s, const char* begin, const char* end)
{
    s[0];
    BasicStringData<char>* data = s.m_data;
    int size = end - begin;
    int offset = begin - data->mData;
    char* at = data->mData + offset;
    while (end != data->mData + data->mSize)
    {
        *at = *end;
        end++;
        at++;
    }
    data->mSize -= size;
}

/**
 * Offset/Address/Size: 0xCC0 | 0x8016D5BC | size: 0xD74
 * TODO: 99.59% match - copy-on-write temp register swap (r26 vs r27) in the
 * insert path and marker add operand order.
 */
template <>
NLFormatImpl& NLFormatImpl::operator% <float>(const float& t)
{
    NLString insert = LexicalCast<NLString, float>(t);

    for (int i = 0; i < (mString.m_data ? mString.m_data->mSize - 1 : 0); i++)
    {
        if (mString[i] != (char)'{')
            continue;

        if (i + 1 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* marker = &mString[i];
        if (mCurrentPos != marker[1] - '0')
            continue;

        if (i + 2 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* markerEnd = &mString[i];
        if (markerEnd[2] != (char)'}')
            continue;

        mString[0];
        EraseRange(mString, ((void)mString[0], (mString.m_data ? mString.m_data->mData : (char*)0) + i), (mString.m_data ? mString.m_data->mData : (char*)0) + i + 3);
        mString[i];
        char* mStringData = mString.m_data ? mString.m_data->mData : 0;
        char* insertBegin = ((void)insert[0], insert.m_data ? insert.m_data->mData : 0);
        insert[(int)(insert.m_data ? insert.m_data->mSize - 1 : 0)];
        mString.insert(mStringData + i, insertBegin, insert.m_data ? insert.m_data->mData + insert.m_data->mSize - 1 : (char*)0);
    }

    mCurrentPos++;
    return *this;
}
