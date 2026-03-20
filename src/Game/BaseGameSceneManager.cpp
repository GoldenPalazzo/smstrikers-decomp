#include "Game/BaseGameSceneManager.h"
#include "Game/main.h"
#include "Game/SH/SHBackground.h"
#include "Game/SH/SHTitleScreen.h"
#include "Game/SH/SHMainMenu.h"
#include "Game/SH/SHChooseSides.h"
#include "Game/SH/SHChooseCaptains.h"
#include "Game/SH/SHStadiumSelect.h"
#include "Game/SH/SHCupCheater.h"
#include "Game/SH/SHChooseCup.h"
#include "Game/SH/SHCupChooseCaptain.h"
#include "Game/SH/SHCupHub.h"
#include "Game/SH/SHSuperTeam.h"
#include "Game/SH/SHSpoils.h"
#include "Game/SH/SHSaveLoad.h"
#include "Game/SH/SHTournSetParams.h"
#include "Game/SH/SHTournTeamSetup.h"
#include "Game/SH/SHOptions.h"
#include "Game/SH/SHCrossFader.h"
#include "Game/SH/SHCupOptions.h"
#include "Game/SH/SHLoading.h"
#include "Game/SH/SHCupTrophy.h"
#include "Game/SH/SHMilestoneTrophy.h"
#include "Game/SH/SHMoviePlayer.h"
#include "Game/SH/SHQuickGameplayOptions.h"
#include "Game/SH/SHLoadingTransition.h"
#include "Game/SH/SHHealthWarning.h"
#include "Game/SH/SHCredits.h"
#include "Game/SH/SHProgressiveScan.h"
#include "Game/SH/SHPause.h"
#include "Game/SH/SHPauseOptions.h"
#include "Game/SH/SHPausePostGame.h"
#include "Game/SH/SHLesson.h"
#include "Game/SH/SHLessonSelect.h"
#include "Game/FE/feSceneManager.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/FE/feScrollingTicker.h"
#include "Game/FE/BraggingRights.h"
#include "Game/FE/feNSNMessenger.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/FE/Overlay/OverlayHandlerWinner.h"
#include "Game/OverlayHandlerHUD.h"
#include "Game/OverlayHandlerInGameText.h"
#include "Game/OverlayHandlerGoal.h"
#include "Game/OverlayHandlerDemo.h"
#include "Game/OverlayHandlerSTSX2.h"

#include "Game/BaseSceneHandler.h"

#include "NL/nlConfig.h"

// Define the scene entry struct
struct SceneEntry
{
    u32 sceneId;
    const char* sceneName;
};

// Declare the array of scene entries
struct SceneEntry SceneEntryTable[] = {
    { 0x00, "art/fe/background.fen" },
    { 0x01, "art/fe/mariobg.fen" },
    { 0x02, "art/fe/start_screen_v2.fen" },
    { 0x03, "art/fe/main_menuv2.fen" },
    { 0x04, "art/fe/choose_sides_v3.fen" },
    { 0x05, "art/fe/choose_sides_v3.fen" },
    { 0x06, "art/fe/choose_sides_v3.fen" },
    { 0x07, "art/fe/choose_sides_v3.fen" },
    { 0x08, "/art/fe/choose_captains_v3.fen" },
    { 0x09, "art/fe/choose_stadiums_v2.fen" },
    { 0x0A, "art/fe/cup_cheater.fen" },
    { 0x0B, "art/fe/cup_background.fen" },
    { 0x0C, "art/fe/super_cup_background.fen" },
    { 0x0D, "art/fe/choose_cup_v2.fen" },
    { 0x0E, "art/fe/choose_cup_v2.fen" },
    { 0x0F, "art/fe/cup_choose_captains_v3.fen" },
    { 0x10, "art/fe/cup_choose_captains_v3.fen" },
    { 0x11, "art/fe/cup_standings_v2.fen" },
    { 0x12, "art/fe/cup_standings_v2.fen" },
    { 0x13, "art/fe/cup_standings_v2.fen" },
    { 0x14, "art/fe/cup_standings_v2.fen" },
    { 0x15, "art/fe/cup_standings_v2.fen" },
    { 0x16, "art/fe/cup_standings_v2.fen" },
    { 0x17, "art/fe/cup_standings_v2.fen" },
    { 0x18, "art/fe/cup_standings_v2.fen" },
    { 0x19, "art/fe/cup_standings_v2.fen" },
    { 0x1A, "art/fe/super_team_reveal_v3.fen" },
    { 0x1B, "art/fe/popup_menu.fen" },
    { 0x1C, "art/fe/spoils_menu_v2.fen" },
    { 0x1D, "art/fe/ticker.fen" },
    { 0x1E, "art/fe/mainstart_background.fen" },
    { 0x1F, "art/fe/saving_loading.fen" },
    { 0x20, "art/fe/saving_loading.fen" },
    { 0x21, "art/fe/saving_loading.fen" },
    { 0x22, "art/fe/saving_loading.fen" },
    { 0x23, "art/fe/saving_loading.fen" },
    { 0x24, "art/fe/custom_tournament_options_v2.fen" },
    { 0x25, "art/fe/custom_tournament_capt_chooser.fen" },
    { 0x26, "art/fe/main_options_v2.fen" },
    { 0x27, "art/fe/englegal.fen" },
    { 0x28, "art/fe/main_options_v2.fen" },
    { 0x29, "art/fe/main_options_v2.fen" },
    { 0x2A, "art/fe/main_options_v2.fen" },
    { 0x2B, "art/fe/loadingtest.fen" },
    { 0x2C, "art/fe/spoils_battles_history_v2.fen" },
    { 0x2D, "art/fe/spoils_milestone_history_v2.fen" },
    { 0x2E, "art/fe/bragging_rights.fen" },
    { 0x2F, "art/fe/player_awards_v3.fen" },
    { 0x30, "art/fe/movieplayer.fen" },
    { 0x31, "art/fe/main_options_v2.fen" },
    { 0x32, "art/fe/vs_transition.fen" },
    { 0x33, "art/fe/health_and_safety.fen" },
    { 0x34, "art/fe/movieplayer.fen" },
    { 0x35, "art/fe/movieplayer.fen" },
    { 0x36, "art/fe/credits.fen" },
    { 0x37, "art/fe/progressive_scan.fen" },
    { 0x38, "art/fe/progressive_scan.fen" },
    { 0x39, "art/fe/pausemenu_v3.fen" },
    { 0x3A, "art/fe/choose_sides_v4.fen" },
    { 0x3B, "art/fe/pause_options_v2.fen" },
    { 0x3C, "art/fe/pause_options_v2.fen" },
    { 0x3D, "art/fe/post_game_menu_v2.fen" },
    { 0x3E, "art/fe/pausemenu101_v3.fen" },
    { 0x3F, "art/fe/lesson.fen" },
    { 0x40, "art/fe/strikers_101_lessons_v3.fen" },
    { 0x41, "art/fe/lessonmovieplayer.fen" },
    { 0x42, NULL }, // 0x00000000 indicates NULL pointer
    { 0x43, "art/fe/hud.fen" },
    { 0x44, "art/fe/ingame_text.fen" },
    { 0x45, "art/fe/popup_menu.fen" },
    { 0x46, "art/fe/summary.fen" },
    { 0x47, "art/fe/summary.fen" },
    { 0x48, "art/fe/goal_overlay.fen" },
    { 0x49, "art/fe/player_awards_ig_v3.fen" },
    { 0x4A, "art/fe/demo_overlay.fen" },
    { 0x4B, "art/fe/winner.fen" },
    { 0x4C, "art/fe/igticker.fen" },
    { 0x4D, "art/fe/x2_sts.fen" },
    { 0x4E, "art/fe/loading_screen.fen" },
    { 0x4F, NULL } // 0x00000000 indicates NULL pointer
};

/**
 * Offset/Address/Size: 0x142C | 0x800969E8 | size: 0xB4
 */
BaseGameSceneManager::BaseGameSceneManager()
{
    mCurrentStackDepth = 0;
    for (int i = 0; i < MAX_SCENE_COUNT; ++i)
    {
        m_sceneStack[i] = SCENE_INVALID;
        mBaseSceneHandlerStack[i] = 0;
    }
}

/**
 * Offset/Address/Size: 0x13B0 | 0x8009696C | size: 0x7C
 */
BaseGameSceneManager::~BaseGameSceneManager()
{
    while (mCurrentStackDepth != 0)
    {
        this->Pop();
    }
}

/**
 * Offset/Address/Size: 0x324 | 0x800958E0 | size: 0x108C
 */
BaseSceneHandler* BaseGameSceneManager::Push(SceneList newscene, ScreenMovement movement, bool popfirst)
{
    if (popfirst)
    {
        Pop();
    }

    if (newscene == SCENE_MAIN_BACKGROUND)
    {
        newscene = SCENE_MARIO_BACKGROUND;
    }

    GetConfigBool(Config::Global(), "fev2", false);

    if (newscene == SCENE_MAIN_MENU)
    {
        Push(SCENE_MARIO_BACKGROUND, SCREEN_NOTHING, false);
    }

    BaseSceneHandler* newHandler = NULL;
    const SceneEntry& entry = SceneEntryTable[newscene];
    const char* filename = entry.sceneName;

    GetConfigBool(Config::Global(), "fev2", false);

    if (newscene == SCENE_TITLE)
    {
        if (g_e3_Build || g_Europe)
        {
            filename = "art/fe/start_screen_v2_german.fen";
        }
        else if (g_Language == 5)
        {
            filename = "art/fe/start_screen_jpn.fen";
        }
    }

    if (newscene == SCENE_CHOOSE_CAPTAINS)
    {
        if (g_e3_Build)
        {
            filename = "art/fe/choose_captains_vLeeptsig.fen";
        }
    }

    switch (newscene)
    {
    case SCENE_FRIENDLY_BACKGROUND:
        newHandler = new (nlMalloc(0x24, 8, false)) BackgroundScene();
        break;
    case SCENE_MARIO_BACKGROUND:
        newHandler = new (nlMalloc(0x24, 8, false)) BackgroundScene();
        break;
    case SCENE_TITLE:
        newHandler = new (nlMalloc(0x28, 8, false)) TitleScene();
        break;
    case SCENE_MAIN_MENU:
        newHandler = new (nlMalloc(0x25C, 8, false)) SHMainMenu();
        break;
    case SCENE_CHOOSE_SIDES_FRIENDLY:
        newHandler = new (nlMalloc(0x110, 8, false)) SHChooseSides2((SHChooseSides2::eCSContext)0);
        break;
    case SCENE_CHOOSE_SIDES_CUP:
        newHandler = new (nlMalloc(0x110, 8, false)) SHChooseSides2((SHChooseSides2::eCSContext)1);
        break;
    case SCENE_CHOOSE_SIDES_SUPER_CUP:
        newHandler = new (nlMalloc(0x110, 8, false)) SHChooseSides2((SHChooseSides2::eCSContext)2);
        break;
    case SCENE_CHOOSE_SIDES_TOURNAMENT:
        newHandler = new (nlMalloc(0x110, 8, false)) SHChooseSides2((SHChooseSides2::eCSContext)3);
        break;
    case SCENE_CHOOSE_CAPTAINS:
        newHandler = new (nlMalloc(0x1C0, 8, false)) ChooseCaptainsSceneV2((ChooseCaptainsSceneV2::SceneType)0);
        break;
    case SCENE_STADIUM_SELECT:
        newHandler = new (nlMalloc(0x49C, 8, false)) StadiumSelectSceneV2();
        break;
    case SCENE_CUP_CHEATER:
        newHandler = new (nlMalloc(0x98, 8, false)) CupCheaterScene();
        break;
    case SCENE_CUP_BACKGROUND:
        newHandler = new (nlMalloc(0x24, 8, false)) BackgroundScene();
        break;
    case SCENE_SUPER_CUP_BACKGROUND:
        newHandler = new (nlMalloc(0x24, 8, false)) BackgroundScene();
        break;
    case SCENE_CUP_CHOOSE_CUP:
        newHandler = new (nlMalloc(0x484, 8, false)) ChooseCupSceneV2(false);
        break;
    case SCENE_SUPER_CUP_CHOOSE_CUP:
        newHandler = new (nlMalloc(0x484, 8, false)) ChooseCupSceneV2(true);
        break;
    case SCENE_CUP_CHOOSE_CAPTAIN:
        newHandler = new (nlMalloc(0xA0, 8, false)) CupChooseCaptainSceneV2(false);
        break;
    case SCENE_SUPER_CUP_CHOOSE_CAPTAIN:
        newHandler = new (nlMalloc(0xA0, 8, false)) CupChooseCaptainSceneV2(true);
        break;
    case SCENE_CUP_STANDINGS:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(false, false);
        break;
    case SCENE_CUP_STANDINGS_ANIM:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(true, false);
        break;
    case SCENE_CUP_STANDINGS_FINAL_ANIM:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(false, true);
        break;
    case SCENE_SUPER_CUP_STANDINGS:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(false, false);
        break;
    case SCENE_SUPER_CUP_STANDINGS_ANIM:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(true, false);
        break;
    case SCENE_SUPER_CUP_STANDINGS_FINAL_ANIM:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(false, true);
        break;
    case SCENE_TOURNAMENT_STANDINGS:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(false, false);
        break;
    case SCENE_TOURNAMENT_STANDINGS_ANIM:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(true, false);
        break;
    case SCENE_TOURNAMENT_STANDINGS_FINAL_ANIM:
        newHandler = new (nlMalloc(0x1260, 8, false)) CupHubScene(false, true);
        break;
    case SCENE_CUP_SUPER_TEAM:
        newHandler = new (nlMalloc(0x40, 8, false)) SuperTeamScene();
        break;
    case SCENE_POPUP_MENU:
        newHandler = new (nlMalloc(0xAA8, 8, false)) FEPopupMenu();
        break;
    case SCENE_TROPHY_ROOM:
        newHandler = new (nlMalloc(0x254, 8, false)) SpoilsScene();
        break;
    case SCENE_SCROLLING_TICKER:
    {
        ScrollingTickerScene* ticker = new (nlMalloc(0xCC, 8, false)) ScrollingTickerScene();
        newHandler = ticker;
        break;
    }
    case SCENE_MAIN_BACKGROUND:
        newHandler = new (nlMalloc(0x24, 8, false)) BackgroundScene();
        break;
    case SCENE_SAVE:
        newHandler = new (nlMalloc(0x34, 8, false)) SaveLoadScene((SaveLoadScene::eSaveLoadMode)1);
        break;
    case SCENE_LOAD:
        newHandler = new (nlMalloc(0x34, 8, false)) SaveLoadScene((SaveLoadScene::eSaveLoadMode)3);
        break;
    case SCENE_ASK_SAVE:
        newHandler = new (nlMalloc(0x34, 8, false)) SaveLoadScene((SaveLoadScene::eSaveLoadMode)2);
        break;
    case SCENE_ASK_LOAD:
        newHandler = new (nlMalloc(0x34, 8, false)) SaveLoadScene((SaveLoadScene::eSaveLoadMode)4);
        break;
    case SCENE_SHOULD_LOAD_OR_SAVE:
        newHandler = new (nlMalloc(0x34, 8, false)) SaveLoadScene((SaveLoadScene::eSaveLoadMode)0);
        break;
    case SCENE_TOURN_SETPARAMS:
        newHandler = new (nlMalloc(0x26C, 8, false)) TournSetParamsScene();
        break;
    case SCENE_TOURN_SETTEAMS:
        newHandler = new (nlMalloc(0x338, 8, false)) TournTeamSetupSceneV2();
        break;
    case SCENE_OPTIONS:
        newHandler = new (nlMalloc(0x260, 8, false)) OptionsScene();
        break;
    case SCENE_LEGAL:
        newHandler = new (nlMalloc(0x3C, 8, false)) CrossFaderScene();
        break;
    case SCENE_CUP_OPTIONS_INITIAL_CUP:
        newHandler = new (nlMalloc(0x28, 8, false)) CupOptionsScene(SCENE_CUP_CHOOSE_CAPTAIN, SCENE_CUP_CHOOSE_CUP);
        break;
    case SCENE_CUP_OPTIONS_INITIAL_SUPER:
        newHandler = new (nlMalloc(0x28, 8, false)) CupOptionsScene(SCENE_SUPER_CUP_CHOOSE_CAPTAIN, SCENE_SUPER_CUP_CHOOSE_CUP);
        break;
    case SCENE_CUP_OPTIONS_INITIAL_TOURN:
        newHandler = new (nlMalloc(0x28, 8, false)) CupOptionsScene(SCENE_TOURN_SETTEAMS, SCENE_TOURN_SETPARAMS);
        break;
    case SCENE_SUPER_LOADING:
        newHandler = new (nlMalloc(0x634, 8, false)) SuperLoadingScene();
        break;
    case SCENE_CUP_TROPHY:
        newHandler = new (nlMalloc(0x788, 8, false)) CupTrophyScene();
        break;
    case SCENE_MILESTONE_TROPHY:
        newHandler = new (nlMalloc(0x378, 8, false)) MilestoneTrophyScene();
        break;
    case SCENE_CUP_BRAG:
        newHandler = new (nlMalloc(0x1644, 8, false)) BraggingRightsScene();
        break;
    case SCENE_TOURNEY_BRAG:
        newHandler = new (nlMalloc(0x484, 8, false)) BraggingRightsOverlay();
        break;
    case SCENE_MOVIE_PLAYER:
        newHandler = new (nlMalloc(0xB0, 8, false)) MoviePlayerScene();
        break;
    case SCENE_QUICK_GAMEPLAY_OPTIONS:
        newHandler = new (nlMalloc(0x20, 8, false)) QuickGameplayOptionsScene();
        break;
    case SCENE_LOADING_TRANSITION:
        newHandler = new (nlMalloc(0x1C, 8, false)) LoadingTransitionScene();
        break;
    case SCENE_HEALTH_WARNING:
        newHandler = new (nlMalloc(0x2C, 8, false)) HealthWarningSceneV2();
        break;
    case SCENE_NLG_MOVIE:
        newHandler = new (nlMalloc(0xB0, 8, false)) NLGLogoMovieScene();
        break;
    case SCENE_INTRO_MOVIE:
        newHandler = new (nlMalloc(0xB0, 8, false)) IntroMovieScene();
        break;
    case SCENE_CREDITS:
        newHandler = new (nlMalloc(0xA78, 8, false)) CreditScene();
        break;
    case SCENE_PROGRESSIVE_SCAN:
        newHandler = new (nlMalloc(0x38, 8, false)) ProgressiveScanScene(false);
        break;
    case SCENE_EURO_RGB60:
        newHandler = new (nlMalloc(0x38, 8, false)) ProgressiveScanScene(true);
        break;
    case IGSCENE_PAUSE:
        newHandler = new (nlMalloc(0x290, 8, false)) PauseMenuScene((PauseMenuScene::ScreenContext)0);
        break;
    case IGSCENE_CHOOSE_SIDES:
        newHandler = new (nlMalloc(0x110, 8, false)) SHChooseSides2((SHChooseSides2::eCSContext)4);
        break;
    case IGSCENE_PAUSE_AUDIO:
        newHandler = new (nlMalloc(0x34, 8, false)) PauseOptionsScene((PauseOptionsScene::Mode)0);
        break;
    case IGSCENE_PAUSE_VISUAL:
        newHandler = new (nlMalloc(0x34, 8, false)) PauseOptionsScene((PauseOptionsScene::Mode)1);
        break;
    case IGSCENE_PAUSE_POST_GAME:
        newHandler = new (nlMalloc(0x374, 8, false)) PausePostGameScene();
        break;
    case IGSCENE_STRIKERS_101_PAUSE:
        newHandler = new (nlMalloc(0x290, 8, false)) PauseMenuScene((PauseMenuScene::ScreenContext)1);
        break;
    case IGSCENE_LESSON:
        newHandler = new (nlMalloc(0x48, 8, false)) LessonScene();
        break;
    case IGSCENE_LESSON_SELECT:
        newHandler = new (nlMalloc(0x2E0, 8, false)) LessonSelectScene();
        break;
    case IGSCENE_LESSON_MOVIE_PLAYER:
        newHandler = new (nlMalloc(0xD4, 8, false)) LessonMoviePlayerScene();
        break;
    case OVERLAY_START:
        break;
    case OVERLAY_HUD:
        newHandler = new (nlMalloc(0x1E0, 8, false)) HUDOverlay();
        break;
    case OVERLAY_TEXT:
        newHandler = new (nlMalloc(0xB0, 8, false)) InGameTextOverlay();
        break;
    case OVERLAY_POPUP:
        newHandler = new (nlMalloc(0xAA8, 8, false)) FEPopupMenu();
        break;
    case OVERLAY_SUMMARY:
        newHandler = new (nlMalloc(0xC64, 8, false)) SummaryOverlay((SummaryOverlay::eSummaryContext)0);
        break;
    case OVERLAY_SUMMARY_PAUSE:
        newHandler = new (nlMalloc(0xC64, 8, false)) SummaryOverlay((SummaryOverlay::eSummaryContext)1);
        break;
    case OVERLAY_GOAL:
        newHandler = new (nlMalloc(0x280, 8, false)) GoalOverlay();
        break;
    case OVERLAY_BRAG:
        newHandler = new (nlMalloc(0x484, 8, false)) BraggingRightsOverlay();
        break;
    case OVERLAY_DEMO:
        newHandler = new (nlMalloc(0x28, 8, false)) DemoOverlay();
        break;
    case OVERLAY_WINNER:
        newHandler = new (nlMalloc(0xC0, 8, false)) WinnerOverlay();
        break;
    case OVERLAY_LESSON_TICKER:
    {
        NSNMessengerScene* nsn = new (nlMalloc(0x240, 8, false)) NSNMessengerScene();
        newHandler = nsn;
        break;
    }
    case NUM_SCENES:
        newHandler = new (nlMalloc(0x2C, 8, false)) STSX2Overlay();
        break;
    case 78:
        newHandler = new (nlMalloc(0x1C, 8, false)) BaseSceneHandler();
        break;
    }

    FESceneManager::Instance()->QueueScenePush(newHandler, filename);

    m_sceneStack[mCurrentStackDepth] = newscene;
    mBaseSceneHandlerStack[mCurrentStackDepth] = newHandler;
    mCurrentStackDepth++;

    switch (movement)
    {
    case SCREEN_FORWARD:
        FEAudio::PlayAnimAudioEvent("sfx_screen_forward", false);
        break;
    case SCREEN_BACK:
        FEAudio::PlayAnimAudioEvent("sfx_screen_back", false);
        break;
    }

    return newHandler;
}

/**
 * Offset/Address/Size: 0x2D8 | 0x80095894 | size: 0x4C
 */
BaseSceneHandler* BaseGameSceneManager::GetScene(SceneList scene)
{
    BaseSceneHandler* handler = NULL;

    for (u32 i = 0; i < mCurrentStackDepth; ++i)
    {
        if (m_sceneStack[i] == scene)
        {
            handler = mBaseSceneHandlerStack[i];
            break;
        }
    }

    return handler;
}

/**
 * Offset/Address/Size: 0x288 | 0x80095844 | size: 0x50
 */
void BaseGameSceneManager::Pop()
{
    FESceneManager::Instance()->QueueScenePop();
    mBaseSceneHandlerStack[mCurrentStackDepth] = 0;
    mCurrentStackDepth = (mCurrentStackDepth - 1);
}

/**
 * Offset/Address/Size: 0x23C | 0x800957F8 | size: 0x4C
 */
void BaseGameSceneManager::PopEntireStack()
{
    while (mCurrentStackDepth != 0)
    {
        this->Pop();
    }
}

/**
 * Offset/Address/Size: 0x1F4 | 0x800957B0 | size: 0x48
 */
int BaseGameSceneManager::GetSceneType(BaseSceneHandler* handler)
{
    // Iterate rows; when mHandlers[i] == handler, return mTypes[i], else -2. :contentReference[oaicite:10]{index=10}
    for (u32 i = 0; i < mCurrentStackDepth; ++i)
    {
        if (mBaseSceneHandlerStack[i] == handler)
        {
            return m_sceneStack[i];
        }
    }
    return -2;
}

/**
 * Offset/Address/Size: 0x1C0 | 0x8009577C | size: 0x34
 */
bool BaseGameSceneManager::IsOnStack(SceneList scene)
{
    for (u32 i = 0; i < mCurrentStackDepth; ++i)
    {
        if (m_sceneStack[i] == scene)
            return true;
    }
    return false;
}

/**
 * Offset/Address/Size: 0x68 | 0x80095624 | size: 0x158
 */
BasicString<char, Detail::TempStringAllocator> BaseGameSceneManager::GetFileName(SceneList scene)
{
    BasicString<char, Detail::TempStringAllocator> result(
        SceneEntryTable[(u32)scene].sceneName);
    return result;
}

/**
 * Offset/Address/Size: 0x0 | 0x800955BC | size: 0x68
 */
void BaseGameSceneManager::PushLoadingScene(bool clearStack)
{
    if (clearStack)
    {
        this->Pop();
    }

    BackgroundScene* handler = (BackgroundScene*)Push((SceneList)0x2B, SCREEN_FORWARD, false);
    handler->mPlayMode = PM_STOPPED;
}
