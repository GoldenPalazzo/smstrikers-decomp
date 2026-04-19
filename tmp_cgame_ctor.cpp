cGame::cGame()
{
    m_bBallInNet = false;
    m_eGameState = GS_NONE;
    m_pScorer = NULL;
    m_pAssister = NULL;
    mbCaptainShotToScoreOn = false;
    mIsPure = false;
    mInSuddenDeath = false;
    mThoughtsQueue.m_Head = NULL;
    mThoughtsQueue.m_Tail = NULL;

    void* mem = nlMalloc(sizeof(Clock), 8, false);
    m_pPostResetClock = mem ? new (mem) Clock(0.0f, 2.0f, 1.0f, 2, cGame::PostResetCallback) : NULL;
    if (m_pPostResetClock != NULL) {
        m_pPostResetClock->m_uParam1 = reinterpret_cast<unsigned long>(this);
    }

    mem = nlMalloc(sizeof(GameTweaks), 8, false);
    m_pGameTweaks = mem ? new (mem) GameTweaks("GameTweaks.ini") : NULL;

    mem = nlMalloc(sizeof(FuzzyTweaks), 8, false);
    m_pFuzzyTweaks = mem ? new (mem) FuzzyTweaks("FuzzyTweaks.ini") : NULL;

    m_fGameDuration = m_pGameTweaks->fGameDuration;

    mem = nlMalloc(sizeof(Clock), 8, false);
    m_pGameClock = mem ? new (mem) Clock(0.0f, 60000.0f, 1.0f, 2, NULL) : NULL;

    mem = nlMalloc(sizeof(Clock), 8, false);
    m_pPostGameDoneClock = mem ? new (mem) Clock(0.0f, 1.4f, 1.0f, 2, NULL) : NULL;

    m_pTarget = NULL;
    m_pTeamTouch[1] = NULL;
    m_pTeamTouch[0] = NULL;
    m_pRandomPlayersArray = (cPlayer**)nlMalloc(sizeof(cPlayer*) * 10, 8, false);

    Config& cfg = Config::Global();
    mIsPure = GetConfigBool(cfg, "pure_game", false);
    if (GetConfigBool(Config::Global(), "save_stats", false) != false) {
        nlSingleton<StatsTracker>::s_pInstance->WriteCurrentlyPlaying();
    }
}
