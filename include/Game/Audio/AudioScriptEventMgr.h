#ifndef _AUDIOSCRIPTEVENTMGR_H_
#define _AUDIOSCRIPTEVENTMGR_H_

// void AudioScriptEventHandler(Event*, void*);
// void RecordExcitingEvent();
// void Poll();
// void GetEventTeam<CollisionBallGoalpostData>(Event*, bool);
// void GetEventTeam<GoalScoredData>(Event*, bool);
// void nlBSearch<NIS_EVENT_LOOKUP, unsigned long>(const unsigned long&, NIS_EVENT_LOOKUP*, int);
// void nlQSort<NIS_EVENT_LOOKUP>(NIS_EVENT_LOOKUP*, int, int (*)(const NIS_EVENT_LOOKUP*, const NIS_EVENT_LOOKUP*));
// void nlDefaultQSortComparer<NIS_EVENT_LOOKUP>(const NIS_EVENT_LOOKUP*, const NIS_EVENT_LOOKUP*);
// void 0x8014B71C..0x8014B818 | size: 0xFC;
// void nlListAddEnd<ListEntry<AUDIO_EVENT_RECORD>>(ListEntry<AUDIO_EVENT_RECORD>**, ListEntry<AUDIO_EVENT_RECORD>**, ListEntry<AUDIO_EVENT_RECORD>*);

class AudioScriptEventMgr
{
public:
    enum AUDIO_EVENT
    {
        AE_Null = 0,
        AE_Goal = 1,
        AE_KickOff = 2,
        AE_Shot = 3,
        AE_HitPost = 4,
        AE_WinGame = 5,
        AE_SDWin = 6,
        AE_Attack = 7,
        AE_HitPlayer = 8,
        AE_Flyby = 9,
        AE_TeamIntro = 10,
        AE_PreKickOff = 11,
        AE_MissShot = 12,
        AE_GoalieSave = 13,
        AE_GoodPosition = 14,
        AE_WindUp = 15,
        AE_Halftime = 16,
        AE_LastPeriod = 17,
        AE_FinalSeconds = 18,
        AE_FinalSecondsTie = 19,
        AE_SuddenDeath = 20,
        AE_ShootToScore = 21,
        AE_CaptainS2S = 22,
        AE_S2SMiss = 23,
        AE_S2STackled = 24,
        AE_GenericS2SEnd = 25,
        AE_HyperStrike = 26,
        AE_SuperStrikeFloat = 27,
        AE_PowerUpActivate = 28,
        AE_PowerUpDisperse = 29,
        AE_PowerUpHit = 30,
        AE_BoredStart = 31,
        AE_BoredEnd = 32,
        AE_BoredPeriod = 33,
        AE_ChainChomp = 34,
        AE_ChainChompEnd = 35,
        AE_BowserAttackStart = 36,
        AE_BowserAttackEnd = 37,
        AE_BowserTilt = 38,
        AE_BowserLevel = 39,
        AE_BowserLandTilt = 40,
        AE_BowserLandReg = 41,
        AE_PerfectPass = 42,
        AE_PerfectPassEnd = 43,
        AE_LosingBadly = 44,
        AE_Comeback = 45,
        AE_GotPossession = 46,
        AE_COUNT = 47,
    };
    enum AUDIO_EVENT_TEAM
    {
        AET_Neutral = 0,
        AET_Home = 1,
        AET_Away = 2,
        AET_Special = 3,
    };

    static void Init();
    static void Purge();
    void Update();
    void FireEvent(AudioScriptEventMgr::AUDIO_EVENT, AudioScriptEventMgr::AUDIO_EVENT_TEAM);
};

// class _AudioEventRaiser
// {
// public:
//     void RaiseEvent(AUDIO_EVENT_RECORD*);
// };

// class WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>
// {
// public:
//     void Callback(ListEntry<AUDIO_EVENT_RECORD>*);
// };

// class ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD>>>
// {
// public:
//     void DeleteEntry(ListEntry<AUDIO_EVENT_RECORD>*);
// };

// class nlListSlotPool<AUDIO_EVENT_RECORD>
// {
// public:
//     void ~nlListSlotPool();
// };

// class nlWalkList<ListEntry<AUDIO_EVENT_RECORD>, WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>>(ListEntry<AUDIO_EVENT_RECORD>*, WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>*, void (WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>
// {
// public:
//     void *)(ListEntry<AUDIO_EVENT_RECORD>*));
// };

// class nlWalkList<ListEntry<AUDIO_EVENT_RECORD>, ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD>>>>(ListEntry<AUDIO_EVENT_RECORD>*, ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD>>>*, void (ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD>>>
// {
// public:
//     void *)(ListEntry<AUDIO_EVENT_RECORD>*));
// };

#endif // _AUDIOSCRIPTEVENTMGR_H_
