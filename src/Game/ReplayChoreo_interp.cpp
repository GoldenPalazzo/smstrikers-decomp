/**
 * Offset/Address/Size: 0x0 | 0x801287C0 | size: 0x3B4
 */
void ReplayChoreo::DoFunctionCall(unsigned int func)
{
    switch (func)
    {
    case 0:
    {
        m_SP--;
        ReplayCameraFocus arg0 = (ReplayCameraFocus)*m_SP;
        AddCameraFocus(arg0);
        break;
    }
    case 1:
    {
        m_SP--;
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        f32 duration = *(f32*)m_SP;
        Audio::FadeFilter(tweaks->unk214, tweaks->unk210, duration, 0.0f);
        break;
    }
    case 2:
    {
        m_SP--;
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        f32 duration = *(f32*)m_SP;
        Audio::FadeFilter(tweaks->unk210, tweaks->unk214, duration, 0.0f);
        break;
    }
    case 3:
    {
        mCamera.mFrozen = true;
        break;
    }
    case 4:
    {
        m_SP--;
        m_SP--;
        const char* sfxName = (const char*)*m_SP;
        Audio::PlayWorldSFXbyStr(sfxName, 100.0f, -1.0f, false, true, NULL, NULL, NULL);
        break;
    }
    case 5:
    {
        m_SP--;
        m_SP--;
        f32 vol = *(f32*)m_SP;
        m_SP--;
        const char* sfxName = (const char*)*m_SP;
        Audio::PlayWorldSFXbyStr(sfxName, vol, -1.0f, false, true, NULL, NULL, NULL);
        break;
    }
    case 6:
    {
        m_SP--;
        f32 timeOffset = *(f32*)m_SP;
        m_SP--;
        mReplayManager->SetCurrentTime(timeOffset + mReplay->TimeOfLastOccurence(*m_SP));
        f32 endTime = mReplayManager->mReplay->EndTime();
        if (endTime - mReplayManager->mTime > 5.0f)
        {
            mReplayManager->SetCurrentTime(mReplayManager->mReplay->EndTime() - 5.0f);
        }
        break;
    }
    case 7:
    {
        m_SP--;
        f32 speed = *(f32*)m_SP;
        if (mRunningFor)
        {
            if (0.0f >= mRunForTimeLeft)
            {
                mRunForTimeLeft = 0.0f;
                mRunningFor = false;
                break;
            }
        }
        if (!mRunningFor)
        {
            mRunForTimeLeft = speed;
            mRunningFor = true;
        }
        StopWithUndo();
        break;
    }
    case 8:
    {
        m_SP--;
        f32 timeOffset = *(f32*)m_SP;
        m_SP--;
        u32 eventId = *m_SP;
        if (!IsFinished())
        {
            f32 currentTime = mReplayManager->mTime;
            f32 lastOccurence = mReplay->TimeOfLastOccurence(eventId);
            if (currentTime < timeOffset + lastOccurence)
            {
                StopWithUndo();
            }
        }
        break;
    }
    case 9:
    {
        m_SP--;
        mCamera.CutTo((ReplayCameraPosition)*m_SP);
        break;
    }
    case 10:
    {
        m_SP--;
        mCamera.mFocus = *m_SP;
        break;
    }
    case 11:
    {
        m_SP--;
        f32 fov = *(f32*)m_SP;
        mCamera.mFov = fov;
        mCamera.mDeltaFov = 0.0f;
        break;
    }
    case 12:
    {
        m_SP--;
        f32 speed = *(f32*)m_SP;
        mReplayManager->mSpeed = speed;
        mReplayManager->mSpeedUp = 0.0f;
        break;
    }
    case 13:
    {
        m_SP--;
        f32 deltaFov = *(f32*)m_SP;
        mCamera.mDeltaFov = deltaFov;
        break;
    }
    case 14:
    {
        m_SP--;
        f32 speedUp = *(f32*)m_SP;
        mReplayManager->mSpeedUp = speedUp;
        break;
    }
    case 15:
    {
        m_SP--;
        const char* sfxName = (const char*)*m_SP;
        Audio::StopWorldSFXbyStr(sfxName);
        break;
    }
    default:
        nlBreak();
        break;
    }
}
