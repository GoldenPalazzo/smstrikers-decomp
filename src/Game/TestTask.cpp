#pragma pool_data off

#include "Game/TestTask.h"

#include "NL/nlDebugFile.h"
#include "NL/nlMemory.h"
#include "NL/nlConfig.h"
#include "types.h"

#include "NL/nlBasicString.h"

namespace
{
const char* testLog = "test.log";
}

static const char* smokeTestSuccessOutput = "smoke_test.success";
static const char* frameRateTestSuccessOutput = "frame_rate.success";

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

/**
 * Offset/Address/Size: 0x424 | 0x8016CD20 | size: 0x2CC
 * TODO: 97.7% match - remaining diffs are register-allocation around `this` and
 * string temporaries (r29/r31), plus cleanup-path branch layout.
 */
void TestTask::RunSmokeTest(float)
{
    if (mRunSmokeTest)
    {
        if (mTestTimeOut <= 0.0f)
        {
            const char* text = smokeTestSuccessOutput;
            void* debugFile = nlOpenFileDebug(text, false, false);
            if (debugFile)
            {
                nlWriteLineDebug(debugFile, text, false);
                nlFlushFileDebug(debugFile);
                nlCloseFileDebug(debugFile);
            }

            BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
            if (data)
            {
                text = "SUCCESS: smoke test successful, didn't crash for {0} seconds";
                data->mData = 0;
                data->mSize = 0;
                data->mCapacity = 0;

                const char* p = text;
                while (*p++ != '\0')
                {
                    data->mSize++;
                }
                data->mSize++;

                data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
                data->mCapacity = data->mSize;

                for (int i = 0; i < data->mSize; i++)
                {
                    data->mData[i] = *text++;
                }
                data->mRefCount = 1;
            }

            BasicStringInternal* formatData = data;
            float configValue = GetConfigFloat(Config::Global(), "test/time_out_sec ", 10.0f);
            BasicStringInternal* formattedData;

            Format(*(BasicString<char, Detail::TempStringAllocator>*)&formattedData,
                *(BasicString<char, Detail::TempStringAllocator>*)&formatData,
                configValue);

            const char* logText = ((BasicString<char, Detail::TempStringAllocator>*)&formattedData)->c_str();
            if (mTestLog)
            {
                nlWriteLineDebug(mTestLog, logText, false);
                nlWriteLineDebug(mTestLog, "\n", false);
                nlFlushFileDebug(mTestLog);
            }

            BasicStringInternal* toFree = formattedData;
            if (toFree)
            {
                if (--toFree->mRefCount == 0)
                {
                    if (toFree)
                    {
                        if (toFree)
                        {
                            delete[] toFree->mData;
                        }
                        if (toFree)
                        {
                            nlFree(toFree);
                        }
                    }
                }
            }

            toFree = formatData;
            if (toFree)
            {
                if (--toFree->mRefCount == 0)
                {
                    if (toFree)
                    {
                        if (toFree)
                        {
                            delete[] toFree->mData;
                        }
                        if (toFree)
                        {
                            nlFree(toFree);
                        }
                    }
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8016C8FC | size: 0x424
 * TODO: 88.2% scratch match; remaining diffs are mostly r29/r30/r31 allocation.
 */
void TestTask::RunFrameRateTest(float dt)
{
    if (!mRunFrameRateTest)
    {
        return;
    }

    float frameRateThreshold = 1.0f / mMinimumFrameRate;

    if (dt > frameRateThreshold)
    {
        mFrameRateTestFailure = true;

        BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (data)
        {
            const char* formatTemplate = "FAILURE: frame rate dropped below {0}, namely {1}";
            data->mData = 0;
            data->mSize = 0;
            data->mCapacity = 0;

            const char* p = formatTemplate;
            while (*p != '\0')
            {
                data->mSize++;
                p++;
            }
            data->mSize++;

            data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
            data->mCapacity = data->mSize;

            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = formatTemplate[i];
            }
            data->mRefCount = 1;
        }

        BasicStringInternal* formatData = data;
        float actualFrameRate = 1.0f / dt;
        BasicStringInternal* formattedData;

        Format(*(BasicString<char, Detail::TempStringAllocator>*)&formattedData,
            *(BasicString<char, Detail::TempStringAllocator>*)&formatData,
            mMinimumFrameRate,
            actualFrameRate);

        if (mTestLog)
        {
            nlWriteLineDebug(mTestLog,
                ((BasicString<char, Detail::TempStringAllocator>*)&formattedData)->c_str(),
                false);
            nlWriteLineDebug(mTestLog, "\n", false);
            nlFlushFileDebug(mTestLog);
        }

        if (formattedData)
        {
            if (--formattedData->mRefCount == 0)
            {
                if (formattedData)
                {
                    if (formattedData)
                    {
                        delete[] formattedData->mData;
                    }
                    if (formattedData)
                    {
                        nlFree(formattedData);
                    }
                }
            }
        }

        if (formatData)
        {
            if (--formatData->mRefCount == 0)
            {
                if (formatData)
                {
                    if (formatData)
                    {
                        delete[] formatData->mData;
                    }
                    if (formatData)
                    {
                        nlFree(formatData);
                    }
                }
            }
        }
    }

    if (mTestTimeOut <= 0.0f && !mFrameRateTestFailure)
    {
        const char* output = frameRateTestSuccessOutput;
        void* debugFile = nlOpenFileDebug(output, false, false);
        if (debugFile)
        {
            nlWriteLineDebug(debugFile, output, false);
            nlFlushFileDebug(debugFile);
            nlCloseFileDebug(debugFile);
        }

        BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (data)
        {
            const char* formatTemplate = "SUCCES: frame rate test successful, never dropped below {0}";
            data->mData = 0;
            data->mSize = 0;
            data->mCapacity = 0;

            const char* p = formatTemplate;
            while (*p != '\0')
            {
                data->mSize++;
                p++;
            }
            data->mSize++;

            data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
            data->mCapacity = data->mSize;

            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = formatTemplate[i];
            }
            data->mRefCount = 1;
        }

        BasicStringInternal* formatData = data;
        BasicStringInternal* formattedData;

        Format(*(BasicString<char, Detail::TempStringAllocator>*)&formattedData,
            *(BasicString<char, Detail::TempStringAllocator>*)&formatData,
            mMinimumFrameRate);

        if (mTestLog)
        {
            nlWriteLineDebug(mTestLog,
                ((BasicString<char, Detail::TempStringAllocator>*)&formattedData)->c_str(),
                false);
            nlWriteLineDebug(mTestLog, "\n", false);
            nlFlushFileDebug(mTestLog);
        }

        if (formattedData)
        {
            if (--formattedData->mRefCount == 0)
            {
                if (formattedData)
                {
                    if (formattedData)
                    {
                        delete[] formattedData->mData;
                    }
                    if (formattedData)
                    {
                        nlFree(formattedData);
                    }
                }
            }
        }

        if (formatData)
        {
            if (--formatData->mRefCount == 0)
            {
                if (formatData)
                {
                    if (formatData)
                    {
                        delete[] formatData->mData;
                    }
                    if (formatData)
                    {
                        nlFree(formatData);
                    }
                }
            }
        }
    }
}
