#include "Game/FE/feOptionsSubMenus.h"

#include "NL/platpad.h"
#include "NL/nlTask.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/FE/feSlideMenu.h"
#include "Game/FE/fePresentation.h"
#include "Game/FE/feInput.h"
#include "Game/FE/FEAudio.h"
#include "Game/SH/SHSaveLoad.h"
#include "Game/GameSceneManager.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/FE/feFinder.h"
#include "types.h"

typedef void FnTLComponentInstanceCb(TLComponentInstance*);
typedef void FnSlideMenuItemCb(SlideMenuItem*);

extern nlColour MenuHighliteColour;

static const char* MAIN_MENU_SLIDE = "Slide1";
static const char* VISUAL_MENU_SLIDE = "Slide6";
static const char* SAVE_LOAD_SLIDE = "Slide_SaveLoad";
static char* MENU_ITEMS_OSL[] = { "Item_Save", "Item_Load" };

/**
 * Offset/Address/Size: 0x0 | 0x800B5044 | size: 0x4
 */
void OptionsSaveLoad::Revert()
{
    // Empty
}

/**
 * Offset/Address/Size: 0x4 | 0x800B5048 | size: 0x4
 */
void OptionsSaveLoad::Save()
{
    // Empty
}

/**
 * Offset/Address/Size: 0x8 | 0x800B504C | size: 0x11C
 */
void OptionsSaveLoad::Update(float dt)
{
    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        if (mMenuItems.mCurrentIndex == 0)
        {
            ResetEnableSaveLoadFlag();
            if (SaveLoadScene::IsIOEnabled())
            {
                nlSingleton<GameSceneManager>::s_pInstance->Pop();
                SaveLoadScene* handler = (SaveLoadScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_ASK_SAVE, SCREEN_FORWARD, false);
                handler->mNextScene = SCENE_OPTIONS;
            }
        }
        else
        {
            ResetEnableSaveLoadFlag();
            if (SaveLoadScene::IsIOEnabled())
            {
                nlSingleton<GameSceneManager>::s_pInstance->Pop();
                SaveLoadScene* handler = (SaveLoadScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_ASK_LOAD, SCREEN_FORWARD, false);
                handler->mNextScene = SCENE_OPTIONS;
            }
        }
        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
    }
    else
    {
        OptionsSubMenu::Update(dt);
    }
}

/**
 * Offset/Address/Size: 0x5FD4 | 0x800BB018 | size: 0xDC
 */
OptionsSubMenu::~OptionsSubMenu()
{
    for (int i = 0; i < 8; i++)
    {
        if (mSlideMenuLists[i] != NULL)
        {
            delete mSlideMenuLists[i];
        }
    }
}

/**
 * Offset/Address/Size: 0x124 | 0x800B5168 | size: 0xEC
 */
OptionsSaveLoad::~OptionsSaveLoad()
{
}

namespace DoubleHighlite
{
extern const char* SLIDE_IN;
extern const char* SLIDE_OUT;
void OpenItem(TLComponentInstance*);
void CloseItem(TLComponentInstance*);
void TempDisableSound();
} // namespace DoubleHighlite

/**
 * Offset/Address/Size: 0x210 | 0x800B5254 | size: 0x410
 * TODO: 78.83% match - stmw r24 vs r25 register allocation (extra callee-saved reg for
 * buttonstate), missing li r0,0 before stmw, prologue store reordering (m_pres/m_buttons/
 * m_currentButtonState not hoisted before ButtonComponent ctor by MWCC -O4)
 */
OptionsSaveLoad::OptionsSaveLoad(FEPresentation* presentation, ButtonComponent::ButtonState buttonstate)
{
    m_pres = presentation;
    m_buttons = NULL;
    m_currentButtonState = buttonstate;
    mSettingsCRC = 0;
    mSlideMenuLists[0] = NULL;
    mSlideMenuLists[1] = NULL;
    mSlideMenuLists[2] = NULL;
    mSlideMenuLists[3] = NULL;
    mSlideMenuLists[4] = NULL;
    mSlideMenuLists[5] = NULL;
    mSlideMenuLists[6] = NULL;
    mSlideMenuLists[7] = NULL;

    presentation->SetActiveSlide(SAVE_LOAD_SLIDE);
    presentation->Update(0.0f);

    SetButtonState(buttonstate);
    mButtons.CentreButtons();

    TLSlide* slide = presentation->m_currentSlide;

    void (*openItem)(TLComponentInstance*) = DoubleHighlite::OpenItem;
    void (*closeItem)(TLComponentInstance*) = DoubleHighlite::CloseItem;

    for (int i = 0; i < 2; i++)
    {
        TLComponentInstance* instance = FEFinder<TLComponentInstance, 4>::Find(
            slide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(MENU_ITEMS_OSL[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        instance->SetActiveSlide((i == 0) ? DoubleHighlite::SLIDE_IN : DoubleHighlite::SLIDE_OUT);
        instance->Update(0.0f);

        MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
        menuItem->mType = instance;
        mMenuItems.mNumItemsAdded++;

        {
            Function<FnTLComponentInstanceCb> openFunc;
            openFunc.mTag = FREE_FUNCTION;
            openFunc.mFreeFunction = openItem;
            menuItem->mCallbacks[1] = openFunc;
        }

        {
            Function<FnTLComponentInstanceCb> closeFunc;
            closeFunc.mTag = FREE_FUNCTION;
            closeFunc.mFreeFunction = closeItem;
            menuItem->mCallbacks[2] = closeFunc;
        }

        if (i == 0)
            DoubleHighlite::TempDisableSound();

        menuItem->mCallbacks[(i == 0) ? 1 : 2](menuItem->mType);
    }

    mMenuItems.mFlags = 1;
}

/**
 * Offset/Address/Size: 0x620 | 0x800B5664 | size: 0x124
 */
void OptionsGameplayMenuV2::CloseItem(TLComponentInstance* compinstance)
{
    typedef TLComponentInstance* (*FindComponentByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindComponentByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLInstance* (*FindImageAsInstance)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindComponentByValue byValue;
        FindComponentByRef byRef;
    } findComponent;

    union
    {
        FindImageByValue typed;
        FindImageAsInstance asInstance;
    } findImage;

    unsigned long hash;

    findComponent.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    findImage.typed = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

    compinstance->SetActiveSlide("out");
    compinstance->Update(0.0f);

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("high");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    compinstance = findComponent.byRef(
        compinstance->GetActiveSlide(),
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    compinstance->SetActiveSlide("out");
    compinstance->Update(0.0f);

    volatile InlineHasher g7, g6;
    volatile InlineHasher g5, g4, g3, g2, g1, g0;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("may_highlite");
    g6.m_Hash = hash;
    g7.m_Hash = hash;
    findImage.asInstance(
                 compinstance->GetActiveSlide(),
                 (InlineHasher&)g7,
                 (InlineHasher&)h9,
                 (InlineHasher&)h7,
                 (InlineHasher&)h5,
                 (InlineHasher&)h3,
                 (InlineHasher&)h1)
        ->SetAssetColour(MenuHighliteColour);
}

/**
 * Offset/Address/Size: 0x744 | 0x800B5788 | size: 0x54
 */
void OptionsGameplayMenuV2::Revert()
{
    memcpy(&mSettings, &mBackupSettings, sizeof(GameplaySettings));
    mSettings.OnSettingsUpdated();
    cPlatPad::m_bDisableRumble = !mSettings.RumbleEnabled;
}

/**
 * Offset/Address/Size: 0x798 | 0x800B57DC | size: 0x224
 */
void OptionsGameplayMenuV2::Save()
{
    GameplaySettings localSettings;
    SlideMenuList* list;
    int val;
    SlideMenuItem** pType;

    // Skill Level
    list = (SlideMenuList*)mSlideMenuLists[0];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.SkillLevel = (GameplaySettings::eSkillLevel)val;

    // Game Time - convert from menu index to seconds
    list = (SlideMenuList*)mSlideMenuLists[1];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    switch (val)
    {
    case 0:
        localSettings.GameTime = 120;
        break;
    case 1:
        localSettings.GameTime = 180;
        break;
    case 2:
        localSettings.GameTime = 240;
        break;
    case 3:
        localSettings.GameTime = 300;
        break;
    case 4:
        localSettings.GameTime = 600;
        break;
    case 5:
        localSettings.GameTime = 900;
        break;
    default:
        localSettings.GameTime = 120;
        break;
    }

    // Power Ups
    list = (SlideMenuList*)mSlideMenuLists[2];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.PowerUps = (val == 0);

    // Shoot2Score
    list = (SlideMenuList*)mSlideMenuLists[3];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.Shoot2Score = (val == 0);

    // RumbleEnabled (index 4 stores to offset 0xB)
    list = (SlideMenuList*)mSlideMenuLists[4];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.RumbleEnabled = (val == 0);

    // BowserAttackEnabled (index 5 stores to offset 0xA)
    list = (SlideMenuList*)mSlideMenuLists[5];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.BowserAttackEnabled = (val == 0);

    mSettings = localSettings;
    mSettings.OnSettingsUpdated();

    cPlatPad::m_bDisableRumble = !localSettings.RumbleEnabled;
}

/**
 * Offset/Address/Size: 0x9BC | 0x800B5A00 | size: 0xEC
 */
OptionsGameplayMenuV2::~OptionsGameplayMenuV2()
{
}

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*)());

extern char __vt__13SlideMenuItem[];

/**
 * Offset/Address/Size: 0xAA8 | 0x800B5AEC | size: 0x438
 * TODO: 88.84% match - one duplicate beq from SlideMenuList placement new (known MWCC quirk)
 */
void OptionsGameplayMenuV2::BuildSkillLevelMenu(TLComponentInstance* compinstance, int startindex, int skilltoskip)
{
    extern int nlSNPrintf(char*, unsigned long, const char*, ...);
    typedef Detail::MemFunImpl<void, void (SlideMenuList::*)()> MemFunImpl_SML;
    typedef BindExp1<void, MemFunImpl_SML, SlideMenuList*> BindExp1_SML;

    SlideMenuList* list = (SlideMenuList*)nlMalloc(sizeof(SlideMenuList), 8, false);
    if (list != NULL)
    {
        new (list) SlideMenuList();
        list->mComponentInstance = compinstance;
    }
    mSlideMenuLists[0] = (MenuList<SlideMenuList>*)list;

    char slidename[64] = { 0 };
    MenuItem<SlideMenuItem>* menuItem;
    int slidenum = 1;
    while (true)
    {
        nlSNPrintf(slidename, 64, "Slide%d", slidenum);
        compinstance->SetActiveSlide(slidename);
        if (compinstance->GetActiveSlide() == NULL)
        {
            break;
        }
        unsigned long slideHash = compinstance->GetActiveSlide()->m_hash;

        SlideMenuList* sml = (SlideMenuList*)mSlideMenuLists[0];

        SlideMenuItem* item = (SlideMenuItem*)nlMalloc(sizeof(SlideMenuItem), 8, true);
        if (item != NULL)
        {
            TLComponentInstance* comp = sml->mComponentInstance;
            *(char**)item = __vt__13SlideMenuItem;
            item->mSlideMenuHash = (unsigned long)-1;
            item->mComponentInstance = comp;
            item->mUserEnumType = slidenum;
        }
        item->mSlideMenuHash = slideHash;

        menuItem = &sml->mMenuItems[sml->mNumItemsAdded];
        menuItem->mType = item;
        sml->mNumItemsAdded++;

        {
            BindExp1_SML bind = Bind<void, MemFunImpl_SML, SlideMenuList*>(
                MemFun<SlideMenuList, void>(&SlideMenuList::SetSlide), sml);
            Function<FnSlideMenuItemCb> callback(bind);
            menuItem->mCallbacks[1] = callback;
        }

        if (skilltoskip == slidenum)
            menuItem->mDisabled = true;
        else
            menuItem->mDisabled = false;

        slidenum++;
    }

    SlideMenuList* sml = (SlideMenuList*)mSlideMenuLists[0];
    menuItem = &sml->mMenuItems[sml->mCurrentIndex];
    menuItem->mCallbacks[2](menuItem->mType);
    sml->mCurrentIndex = startindex - 1;
    menuItem = &sml->mMenuItems[sml->mCurrentIndex];
    menuItem->mCallbacks[1](menuItem->mType);
    sml->mFlags = 3;
}

/**
 * Offset/Address/Size: 0xEE0 | 0x800B5F24 | size: 0x850
 */
OptionsGameplayMenuV2::OptionsGameplayMenuV2(FEPresentation*, ButtonComponent::ButtonState, GameplaySettings& settings, int)
    : mSettings(settings)
{
}

/**
 * Offset/Address/Size: 0x1730 | 0x800B6774 | size: 0x30
 */
void OptionsVisualMenuV2::Revert()
{
    memcpy(&mSettings, &mBackupSettings, sizeof(VisualSettings));
}

/**
 * Offset/Address/Size: 0x1760 | 0x800B67A4 | size: 0xD4
 */
void OptionsVisualMenuV2::Save()
{
    SlideMenuList* list;
    int val;
    SlideMenuItem** pType;

    // Auto zoom camera
    list = (SlideMenuList*)mSlideMenuLists[0];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    mSettings.mIsAutoZoomCamera = (val == 0);

    // Camera zoom level
    list = (SlideMenuList*)mSlideMenuLists[1];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    mSettings.mCameraZoomLevel = (float)val / 10.0f;

    // Widescreen
    list = (SlideMenuList*)mSlideMenuLists[2];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    mSettings.mIsWidescreen = (val != 0);
}

/**
 * Offset/Address/Size: 0x1834 | 0x800B6878 | size: 0xC0
 */
void OptionsVisualMenuV2::Update(float dt)
{
    OptionsSubMenu::Update(dt);

    MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[1];
    if (menuItem == NULL)
        return;

    int userEnumType;
    SlideMenuList* list = (SlideMenuList*)mSlideMenuLists[0];
    if (list != NULL)
    {
        int index = list->mCurrentIndex;
        SlideMenuItem** pType = &list->mMenuItems[index].mType;
        userEnumType = (*pType)->mUserEnumType;
    }
    else
    {
        userEnumType = -1;
    }

    if (userEnumType == 0)
    {
        TLComponentInstance* compInstance = (TLComponentInstance*)menuItem->mType;
        if (compInstance->m_bVisible != false)
        {
            compInstance->m_bVisible = false;
            menuItem->mDisabled = true;

            SlideMenuList* list2 = (SlideMenuList*)mSlideMenuLists[1];
            TLComponentInstance* compInstance2 = list2->mComponentInstance;
            compInstance2->m_bVisible = false;
        }
    }
    else
    {
        TLComponentInstance* compInstance = (TLComponentInstance*)menuItem->mType;
        if (compInstance->m_bVisible == false)
        {
            compInstance->m_bVisible = true;
            menuItem->mDisabled = false;

            SlideMenuList* list2 = (SlideMenuList*)mSlideMenuLists[1];
            TLComponentInstance* compInstance2 = list2->mComponentInstance;
            compInstance2->m_bVisible = true;
        }
    }
}

/**
 * Offset/Address/Size: 0x18F4 | 0x800B6938 | size: 0xEC
 */
OptionsVisualMenuV2::~OptionsVisualMenuV2()
{
}

// Forward declarations for SingleHighlite
namespace SingleHighlite
{
void OpenItem(TLComponentInstance*);
void CloseItem(TLComponentInstance*);
void TempDisableSound();
} // namespace SingleHighlite

extern nlColour SubMenuHighliteColour;
extern nlColour SubMenuUnhighliteColour;

static const char* MENU_ITEMS_VISUAL[] = {
    "CameraZoom",
    "CameraLevel",
    "Widescreen",
};

/**
 * Offset/Address/Size: 0x19E0 | 0x800B6A24 | size: 0x6E8
 */
OptionsVisualMenuV2::OptionsVisualMenuV2(FEPresentation* pres, ButtonComponent::ButtonState btnState, VisualSettings& settings)
    : mSettings(settings)
{
}

/**
 * Offset/Address/Size: 0x20C8 | 0x800B710C | size: 0xA3C
 */
void OptionsAudioMenuV2::Update(float)
{
}

/**
 * Offset/Address/Size: 0x2B04 | 0x800B7B48 | size: 0x44
 */
void OptionsAudioMenuV2::Revert()
{
    memcpy(&mSettings, &mBackupSettings, sizeof(AudioSettings));
    mSettings.ForceApplySettings(false);
}

/**
 * Offset/Address/Size: 0x2B48 | 0x800B7B8C | size: 0x110
 */
void OptionsAudioMenuV2::Save()
{
    SlideMenuList* list;
    int val;
    SlideMenuItem** pType;

    // Music Volume
    list = (SlideMenuList*)mSlideMenuLists[0];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    mSettings.MusicVolume = val;

    // SFX Volume
    list = (SlideMenuList*)mSlideMenuLists[1];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    mSettings.SFXVolume = val;

    // Voice Volume
    list = (SlideMenuList*)mSlideMenuLists[2];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    mSettings.VoiceVolume = val;

    // Audio Mode
    list = (SlideMenuList*)mSlideMenuLists[3];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    mSettings.Mode = (eAudioMode)val;

    mSettings.ApplySettings(mbUpdateMode, false);
    if (mbUpdateMode)
    {
        AudioLoader::PlayFEMenuMusic();
        mbUpdateMode = false;
    }
}

/**
 * Offset/Address/Size: 0x2C58 | 0x800B7C9C | size: 0xEC
 */
OptionsAudioMenuV2::~OptionsAudioMenuV2()
{
}

/**
 * Offset/Address/Size: 0x2D44 | 0x800B7D88 | size: 0x76C
 */
OptionsAudioMenuV2::OptionsAudioMenuV2(FEPresentation*, ButtonComponent::ButtonState, AudioSettings& settings)
    : mSettings(settings)

{
}

/**
 * Offset/Address/Size: 0x34B0 | 0x800B84F4 | size: 0x88C
 */
void OptionsCheatsMenu::BuildCustomPowerupsList(TLComponentInstance*, CustomPowerups, FEPresentation*)
{
}

/**
 * Offset/Address/Size: 0x3D3C | 0x800B8D80 | size: 0x7D4
 */
void OptionsCheatsMenu::BuildLockableSubMenuList(int, TLComponentInstance*, FEPresentation*, bool, int)
{
}

/**
 * Offset/Address/Size: 0x4510 | 0x800B9554 | size: 0x40
 */
void OptionsCheatsMenu::Revert()
{
    memcpy(&mSettings, &mBackupSettings, sizeof(CheatSettings));
    mSettings.OnSettingsUpdated();
}

/**
 * Offset/Address/Size: 0x4550 | 0x800B9594 | size: 0x170
 */
void OptionsCheatsMenu::Save()
{
    CheatSettings localSettings;
    SlideMenuList* list;
    int val;
    SlideMenuItem** pType;

    // Custom Powerups
    list = (SlideMenuList*)mSlideMenuLists[0];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.mCustomPowerups = (CustomPowerups)val;

    // Stunned Goalies
    list = (SlideMenuList*)mSlideMenuLists[1];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.mStunnedGoalies = (val != 0);

    // Infinite Powerups
    list = (SlideMenuList*)mSlideMenuLists[2];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.mInfinitePowerups = (val != 0);

    // Cheat TBD1
    list = (SlideMenuList*)mSlideMenuLists[3];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.mCheatTBD1Enabled = (val != 0);

    // Cheat TBD2
    list = (SlideMenuList*)mSlideMenuLists[4];
    if (list != NULL)
    {
        pType = &list->mMenuItems[list->mCurrentIndex].mType;
        val = (*pType)->mUserEnumType;
    }
    else
    {
        val = -1;
    }
    localSettings.mCheatTBD2Enabled = (val != 0);

    mSettings = localSettings;
    mSettings.OnSettingsUpdated();
}

/**
 * Offset/Address/Size: 0x46C0 | 0x800B9704 | size: 0xEC
 */
OptionsCheatsMenu::~OptionsCheatsMenu()
{
}

/**
 * Offset/Address/Size: 0x47AC | 0x800B97F0 | size: 0x720
 */
OptionsCheatsMenu::OptionsCheatsMenu(FEPresentation*, ButtonComponent::ButtonState, CheatSettings& settings)
    : mSettings(settings)
{
}

/**
 * Offset/Address/Size: 0x4ECC | 0x800B9F10 | size: 0xB8
 */
void OptionsSubMenu::SetAButtonLOC(unsigned long locStrId)
{
    FORCE_DONT_INLINE;

    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    if (m_buttons == NULL)
    {
        return;
    }

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash("accept");
    hB.m_Hash = hash;
    hA.m_Hash = hash;

    TLTextInstance* textInstance = findText.byRef(
        m_buttons->GetActiveSlide(),
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (textInstance != NULL)
    {
        textInstance->m_LocStrId = locStrId;
        textInstance->m_OverloadFlags |= 0x8u;
    }
}

/**
 * Offset/Address/Size: 0x4F84 | 0x800B9FC8 | size: 0xC8
 */
void OptionsSubMenu::SetButtonState(ButtonComponent::ButtonState buttonState)
{
    FORCE_DONT_INLINE;

    typedef TLComponentInstance* (*FindComponentByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindComponentByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindComponentByValue byValue;
        FindComponentByRef byRef;
    } findComponent;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    h9.m_Hash = h8.m_Hash = nlStringLowerHash("buttons");
    hA.m_Hash = hB.m_Hash = nlStringLowerHash("Layer");

    findComponent.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    m_buttons = findComponent.byRef(
        m_pres->m_currentSlide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    mButtons.mButtonInstance = m_buttons;
    mButtons.SetState(buttonState);
    m_currentButtonState = buttonState;

    SetAButtonLOC(0x527F4084);
}

/**
 * Offset/Address/Size: 0x504C | 0x800BA090 | size: 0x434
 * TODO: 88.99% match - r28/r29 register swap for this+listIndex offset and bulk copy temp
 */
void OptionsSubMenu::BuildSubMenuList(int listIndex, TLComponentInstance* compInstance, bool setFlag, int currentIndex)
{
    extern int nlSNPrintf(char*, unsigned long, const char*, ...);
    typedef Detail::MemFunImpl<void, void (SlideMenuList::*)()> MemFunImpl_SML;
    typedef BindExp1<void, MemFunImpl_SML, SlideMenuList*> BindExp1_SML;

    MenuItem<SlideMenuItem>* menuItem;
    SlideMenuList* list = new ((SlideMenuList*)nlMalloc(sizeof(SlideMenuList), 8, false)) SlideMenuList(compInstance);
    mSlideMenuLists[listIndex] = (MenuList<SlideMenuList>*)list;

    char slidename[64] = { 0 };

    int slidenum = 0;
    while (true)
    {
        nlSNPrintf(slidename, 64, "Slide%d", slidenum + 1);
        compInstance->SetActiveSlide(slidename);

        if (compInstance->GetActiveSlide() == NULL)
        {
            break;
        }

        unsigned long slideHash = compInstance->GetActiveSlide()->m_hash;

        SlideMenuList* sml = (SlideMenuList*)mSlideMenuLists[listIndex];

        SlideMenuItem* item = (SlideMenuItem*)nlMalloc(sizeof(SlideMenuItem), 8, true);
        if (item != NULL)
        {
            TLComponentInstance* comp = sml->mComponentInstance;
            *(char**)item = __vt__13SlideMenuItem;
            item->mSlideMenuHash = (unsigned long)-1;
            item->mComponentInstance = comp;
            item->mUserEnumType = slidenum;
        }
        item->mSlideMenuHash = slideHash;

        menuItem = &sml->mMenuItems[sml->mNumItemsAdded];
        menuItem->mType = item;
        sml->mNumItemsAdded++;

        {
            BindExp1_SML bind = Bind<void, MemFunImpl_SML, SlideMenuList*>(
                MemFun<SlideMenuList, void>(&SlideMenuList::SetSlide), sml);
            Function<FnSlideMenuItemCb> callback(bind);
            menuItem->mCallbacks[1] = callback;
        }

        slidenum++;
    }

    SlideMenuList* sml = (SlideMenuList*)mSlideMenuLists[listIndex];
    menuItem = &sml->mMenuItems[sml->mCurrentIndex];
    menuItem->mCallbacks[2](menuItem->mType);

    sml->mCurrentIndex = currentIndex;

    menuItem = &sml->mMenuItems[sml->mCurrentIndex];
    menuItem->mCallbacks[1](menuItem->mType);

    if (setFlag)
    {
        sml->mFlags = 1;
    }
}

/**
 * Offset/Address/Size: 0x5480 | 0x800BA4C4 | size: 0x124
 */
void OptionsSubMenu::GoBack()
{
    TLInstance* inst;
    TLInstance* firstChild;
    TLSlide* currentSlide;
    TLSlide* startSlide;
    TLComponentInstance* compInstance;
    u32 hash;

    SlideMenuList* slideMenuList = (SlideMenuList*)mSlideMenuLists[mMenuItems.mCurrentIndex];
    if (slideMenuList != NULL)
    {
        compInstance = slideMenuList->mComponentInstance;
        if (compInstance != NULL && compInstance->GetActiveSlide() != NULL)
        {
            startSlide = compInstance->GetActiveSlide();
            currentSlide = startSlide;

            do
            {
                compInstance->SetActiveSlide(currentSlide);
                firstChild = compInstance->GetActiveSlide()->m_instances;
                inst = firstChild;
                if (firstChild != NULL)
                {
                    do
                    {
                        if (inst->m_type == (eTimeLineAssetType)3)
                        {
                            inst->SetAssetColour(SubMenuUnhighliteColour);
                        }
                        else if (inst->m_type == (eTimeLineAssetType)2)
                        {
                            hash = inst->m_hashID;
                            if (hash != nlStringLowerHash("white_box"))
                            {
                                inst->SetAssetColour(SubMenuUnhighliteColour);
                            }
                        }
                        inst = inst->m_next;
                    } while (inst != firstChild);
                }
                currentSlide = currentSlide->m_next;
            } while (currentSlide != startSlide);

            compInstance->SetActiveSlide(startSlide);
        }
    }

    m_pres->SetActiveSlide(MAIN_MENU_SLIDE);
    m_pres->Update(0.0f);
    SetButtonState(ButtonComponent::BS_A_AND_B);
}

/**
 * Offset/Address/Size: 0x55A4 | 0x800BA5E8 | size: 0xA30
 */
void OptionsSubMenu::Update(float)
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800BB21C | size: 0x48
//  */
// void Function1<void, SlideMenuItem*>::FunctorBase::~FunctorBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x54 | 0x800BB270 | size: 0x78
//  */
// void Function1<void, SlideMenuItem*>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (SlideMenuList::*)()>,
// SlideMenuList*>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0xCC | 0x800BB2E8 | size: 0x30
//  */
// void Function1<void, SlideMenuItem*>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (SlideMenuList::*)()>,
// SlideMenuList*>>::operator()(SlideMenuItem*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800BB318 | size: 0xE4
 * TODO: 99.39% match - r30/r31 register swap for numItems and pointer walker
 */
SlideMenuList::~SlideMenuList()
{
    int numItems = mNumItemsAdded;
    if (numItems > 0)
    {
        for (int i = 0; i < numItems; i++)
        {
            MenuItem<SlideMenuItem>* item;
            if (i == -1)
            {
                item = &mMenuItems[mCurrentIndex];
            }
            else
            {
                item = &mMenuItems[i];
            }
            delete item->mType;
        }
    }
}

// /**
//  * Offset/Address/Size: 0xE4 | 0x800BB3FC | size: 0x60
//  */
// void MenuItem<SlideMenuItem>::~MenuItem()
// {
// }

/**
 * Offset/Address/Size: 0x1C8 | 0x800BB4E0 | size: 0x48
 */
SlideMenuItem::~SlideMenuItem()
{
}

// /**
//  * Offset/Address/Size: 0x210 | 0x800BB528 | size: 0x5C
//  */
// void Function1<void, SlideMenuItem*>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (SlideMenuList::*)()>,
// SlideMenuList*>>::~FunctorImpl()
// {
// }

/**
 * Offset/Address/Size: 0x26C | 0x800BB584 | size: 0x4
 */
void SlideMenuList::Update(float dt)
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800BB588 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800BB5C0 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800BB644 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800BB7A0 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800BB7D8 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800BB85C | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800BB9B8 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800BB9F0 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800BBA74 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x800BBBD0 | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x680 | 0x800BBC08 | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x704 | 0x800BBC8C | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800BBDE8 | size: 0x70
//  */
// void MenuList<SlideMenuItem>::~MenuList()
// {
// }

// /**
//  * Offset/Address/Size: 0x70 | 0x800BBE58 | size: 0x58
//  */
// void MenuItem<SlideMenuItem>::MenuItem()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800BBEB0 | size: 0x1C
//  */
// void MemFun<SlideMenuList, void>(void (SlideMenuList::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800BBECC | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (SlideMenuList::*)()>, SlideMenuList*>(Detail::MemFunImpl<void, void (SlideMenuList::*)()>,
// SlideMenuList* const&)
// {
// }
