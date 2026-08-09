/**
 * Offset/Address/Size: 0x0 | 0x801267BC | size: 0xAE4
 */
void Presentation::DoFunctionCall(unsigned int func)
{
    switch (func)
    {
    case 0:
        BeginByPass();
        break;
    case 1:
        DisplayElectricFence();
        break;
    case 2:
        EndByPass();
        break;
    case 3:
        StopWithUndo();
        break;
    case 4:
        mInterruptWipe = (const char*)Pop();
        break;
    case 5:
        Jumbotron::instance.m_AnimationClass = (eJumboType)Pop();
        Jumbotron::instance.BeginLoad();
        break;
    case 6:
    {
        const char* name;
        NisWinnerType arg4 = (NisWinnerType)Pop();
        NisUseFilter arg3 = (NisUseFilter)Pop();
        NisUseStadiumOffset arg2 = (NisUseStadiumOffset)Pop();
        NisTarget arg1 = (NisTarget)Pop();
        name = (const char*)Pop();
        if (mByPassing)
        {
            break;
        }
        if (nlStrCmp<char>(name, "trophy") == 0 && !Config::Global().Exists("gimme_cup_trophy") && !nlSingleton<StatsTracker>::s_pInstance->mIsUserCupWinner)
        {
            break;
        }
        NisPlayer::Instance()->Load(name, arg1, arg2, arg3, arg4);
        break;
    }
    case 7:
        if (cupTrophyHash == 0)
        {
            break;
        }
        mTrophyTextureLoaded = false;
        trophyFileName[nlStrLen<char>(trophyFileName) - 1] = 't';
        glBeginLoadTextureBundle(trophyFileName, ReadTrophyTexture, g_TrophyTextureLocationInMemory);
        break;
    case 8:
        BeginFrameTask::s_FramerateLocked = 1;
        break;
    case 9:
    {
        ReplayType arg0 = (ReplayType)Pop();
        if (mByPassing)
        {
            break;
        }
        if (nlTaskManager::m_pInstance->m_CurrState != 0x10 && !IsDuringGamePauseState())
        {
            nlTaskManager::SetNextState(0x10);
        }
        ReplayChoreo::Instance().StartAutoReplay(arg0);
        if (arg0 == 7)
        {
            nlSingleton<OverlayManager>::s_pInstance->SetCurrentTextOverlaySlide((OverlaySlideName)7);
            nlSingleton<OverlayManager>::s_pInstance->SetVisible((SceneList)0x44, false, true);
            nlSingleton<OverlayManager>::s_pInstance->mIsInHighlights = true;
            if (mOverlayDisplayed)
            {
                nlSingleton<OverlayManager>::s_pInstance->SetVisible(mOverlayToDisplay, false, false);
            }
            mOverlayDisplayed = false;
            mOverlayToDisplay = SCENE_INVALID;
            mOverlayDisplayLength = 0.0f;
            mOverlayDelay = 0.0f;
            PlayOverlay("highlight", 0.5f, 30.0f);
        }
        else
        {
            nlSingleton<OverlayManager>::s_pInstance->SetCurrentTextOverlaySlide((OverlaySlideName)7);
            nlSingleton<OverlayManager>::s_pInstance->SetVisible((SceneList)0x44, true, true);
            nlSingleton<OverlayManager>::s_pInstance->mIsInHighlights = false;
        }
        break;
    }
    case 10:
        if (mByPassing)
        {
            break;
        }
        PlayCharacterDirection();
        break;
    case 11:
    {
        bool hasOverride = Config::Global().Exists("gimme_cup_trophy");
        if (!hasOverride && !nlSingleton<StatsTracker>::s_pInstance->mIsUserCupWinner)
        {
            break;
        }
        if (hasOverride)
        {
            break;
        }
        PlayCupOverlay();
        break;
    }
    case 12:
    {
        nlSingleton<ScreenTransitionManager>::s_pInstance->m_SelectedTransition = NULL;
        if (ReplayChoreo::Instance().NumHighlights() <= 0)
        {
            break;
        }
        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);
        if (nlStrCmp<char>(idleFun, mCurrentFunction) != 0 && nlStrCmp<char>(idleFun, "PlayHighlight") != 0)
        {
            mQueuedFunction = "PlayHighlight";
            break;
        }
        nlStrNCpy<char>(mCurrentFunction, "PlayHighlight", 64);
        mSkipPressed = false;
        mInsideByPass = false;
        mByPassing = false;
        mInterruptWipe = 0;
        mUseInterruptWipe = 0;
        mTimeInFunction = 0.0f;
        NisPlayer::Instance()->SetExtraNameFilter("");
        CallFunction(nlStringHash("PlayHighlight"));
        break;
    }
    case 13:
        PlayJumbotron();
        break;
    case 14:
        if (mByPassing)
        {
            break;
        }
        PlayNis();
        break;
    case 15:
    {
        float length, delay;
        const char* name;
        m_SP--;
        length = *(float*)m_SP;
        m_SP--;
        delay = *(float*)m_SP;
        name = (const char*)Pop();
        PlayOverlay(name, delay, length);
        break;
    }
    case 16:
    {
        const char* name = (const char*)Pop();
        if (mByPassing)
        {
            break;
        }
        PlaySfx(name);
        break;
    }
    case 17:
    {
        float vol;
        const char* name;
        m_SP--;
        vol = *(float*)m_SP;
        name = (const char*)Pop();
        if (mByPassing)
        {
            break;
        }
        PlaySfxWithVol(name, vol);
        break;
    }
    case 18:
        Pop();
        Pop();
        Pop();
        Pop();
        break;
    case 19:
        RaiseEvent((const char*)Pop(), (const char*)Pop());
        break;
    case 20:
        ResetNisPlayer();
        break;
    case 21:
        if (mByPassing)
        {
            break;
        }
        SaveGoalAsHighlight();
        break;
    case 22:
        CrowdManager::instance.SetState((eCrowdState)Pop(), false);
        break;
    case 23:
    {
        bool arg0;
        arg0 = Pop() != 0;
        SetTrophyVisible(arg0);
        break;
    }
    case 24:
        StopAllStreams();
        break;
    case 25:
        StopJumbotron();
        break;
    case 26:
        StopOverlay();
        break;
    case 27:
        Pop();
        break;
    case 28:
        UnloadJumbotron();
        break;
    case 29:
        BeginFrameTask::s_FramerateLocked = 0;
        break;
    case 30:
    {
        const char* filter = (const char*)Pop();
        if (mByPassing)
        {
            break;
        }
        WaitForAutoReplayCompletion(filter);
        break;
    }
    case 31:
        if (mByPassing)
        {
            break;
        }
        WaitForCharacterDirection();
        break;
    case 32:
    {
        const char* filter = (const char*)Pop();
        if (mByPassing)
        {
            break;
        }
        WaitForNisCompletion(filter);
        break;
    }
    case 33:
        if (cupTrophyHash == 0)
        {
            break;
        }
        if (mTrophyTextureLoaded)
        {
            break;
        }
        StopWithUndo();
        break;
    case 34:
        Wipe((const char*)Pop());
        break;
    default:
        nlBreak();
        break;
    }
}
