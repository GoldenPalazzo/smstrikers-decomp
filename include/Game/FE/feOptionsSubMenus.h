#ifndef _FEOPTIONSSUBMENUS_H_
#define _FEOPTIONSSUBMENUS_H_

#include "NL/nlMain.h"

#include "Game/DB/UserOptions.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feMenu.h"
#include "Game/FE/feSlideMenu.h"
#include "Game/FE/tlTextInstance.h"

class FEPresentation;
class TLComponentInstance;
class TLImageInstance;
class TLTextInstance;
class TLInstance;
class GameplaySettings;
class VisualSettings;
class AudioSettings;
class CheatSettings;

class OptionsSubMenu
{
public:
    OptionsSubMenu(FEPresentation* pres, ButtonComponent::ButtonState buttonstate)
        : m_pres(pres)
        , m_buttons(NULL)
        , m_currentButtonState(buttonstate)
        , mSettingsCRC(0)
    {
        mSlideMenuLists[0] = NULL;
        mSlideMenuLists[1] = NULL;
        mSlideMenuLists[2] = NULL;
        mSlideMenuLists[3] = NULL;
        mSlideMenuLists[4] = NULL;
        mSlideMenuLists[5] = NULL;
        mSlideMenuLists[6] = NULL;
        mSlideMenuLists[7] = NULL;
    }

    void ColourAllText(const nlColour& colour, int menuitem)
    {
        unsigned long hash;

        SlideMenuList* list = mSlideMenuLists[menuitem];
        if (list != NULL)
        {
            TLComponentInstance* component = list->GetComponentInstance();
            if (component != NULL && component->GetActiveSlide() != NULL)
            {
                TLSlide* firstSlide = component->GetActiveSlide();
                TLSlide* slide = firstSlide;
                do
                {
                    component->SetActiveSlide(slide);
                    TLInstance* firstChild = component->GetActiveSlide()->m_instances;
                    TLInstance* child = firstChild;
                    if (firstChild != NULL)
                    {
                        do
                        {
                            if (child->m_type == TLAT_TEXT)
                            {
                                child->SetAssetColour(colour);
                            }
                            else if (child->m_type == TLAT_IMAGE)
                            {
                                hash = child->m_hash;
                                if (hash != nlStringLowerHash("white_box"))
                                {
                                    child->SetAssetColour(colour);
                                }
                            }
                            child = child->m_next;
                        } while (child != firstChild);
                    }
                    slide = slide->m_next;
                } while (slide != firstSlide);
                component->SetActiveSlide(firstSlide);
            }
        }
    }

    void ColourAllText(TLComponentInstance& component, const nlColour& colour)
    {
        if (component.GetActiveSlide() != NULL)
        {
            TLSlide* firstSlide = component.GetActiveSlide();
            TLSlide* slide = firstSlide;
            do
            {
                component.SetActiveSlide(slide);
                TLInstance* firstChild = component.GetActiveSlide()->m_instances;
                TLInstance* child = firstChild;
                if (firstChild != NULL)
                {
                    do
                    {
                        if (child->m_type == TLAT_TEXT)
                        {
                            child->SetAssetColour(colour);
                        }
                        else if (child->m_type == TLAT_IMAGE)
                        {
                            unsigned long hash = child->m_hash;
                            if (hash != nlStringLowerHash("white_box"))
                            {
                                child->SetAssetColour(colour);
                            }
                        }
                        child = child->m_next;
                    } while (child != firstChild);
                }
                slide = slide->m_next;
            } while (slide != firstSlide);
            component.SetActiveSlide(firstSlide);
        }
    }

    virtual ~OptionsSubMenu();
    virtual void Update(float);
    virtual void Save() = 0;
    virtual void Revert() = 0;
    virtual bool ChangesMade() = 0;
    virtual void GoBack();

    void SetAButtonLOC(unsigned long);
    void SetButtonState(ButtonComponent::ButtonState);
    void BuildSubMenuList(int, TLComponentInstance*, bool, int);

    /* 0x004 */ FEPresentation* m_pres;                            // offset 0x4, size 0x4
    /* 0x008 */ TLComponentInstance* m_buttons;                    // offset 0x8, size 0x4
    /* 0x00C */ ButtonComponent::ButtonState m_currentButtonState; // offset 0xC, size 0x4
    /* 0x010 */ ButtonComponent mButtons;                          // offset 0x10, size 0x24
    /* 0x034 */ MenuList<TLComponentInstance> mMenuItems;          // offset 0x34, size 0x214
    /* 0x248 */ SlideMenuList* mSlideMenuLists[8];                 // offset 0x248, size 0x20
    /* 0x268 */ unsigned long mSettingsCRC;                        // offset 0x268, size 0x4
}; // total size: 0x26C

class OptionsSaveLoad : public OptionsSubMenu
{
public:
    OptionsSaveLoad(FEPresentation*, ButtonComponent::ButtonState);
    virtual ~OptionsSaveLoad();
    virtual void Update(float);
    virtual void Save();
    virtual void Revert();
    virtual bool ChangesMade()
    {
        return false;
    }
}; // total size: 0x26C

class OptionsGameplayMenuV2 : public OptionsSubMenu
{
public:
    OptionsGameplayMenuV2(FEPresentation*, ButtonComponent::ButtonState, GameplaySettings&, int);
    virtual ~OptionsGameplayMenuV2();
    virtual void Save();
    virtual void Revert();
    virtual bool ChangesMade()
    {
        u32 checksum = nlChecksum32(&mSettings, sizeof(GameplaySettings));
        return mSettingsCRC != checksum;
    }

    void BuildSkillLevelMenu(TLComponentInstance*, int, int);
    void CloseItem(TLComponentInstance*);

    /* 0x26C */ GameplaySettings& mSettings;
    /* 0x270 */ GameplaySettings mBackupSettings;
}; // total size: 0x27C

class OptionsVisualMenuV2 : public OptionsSubMenu
{
public:
    OptionsVisualMenuV2(FEPresentation*, ButtonComponent::ButtonState, VisualSettings&);
    virtual ~OptionsVisualMenuV2();
    virtual void Update(float);
    virtual void Save();
    virtual void Revert();
    virtual bool ChangesMade()
    {
        u32 checksum = nlChecksum32(&mSettings, sizeof(VisualSettings));
        return mSettingsCRC != checksum;
    }

    /* 0x26C */ VisualSettings& mSettings;
    /* 0x270 */ VisualSettings mBackupSettings;
}; // total size: 0x27C

class OptionsAudioMenuV2 : public OptionsSubMenu
{
public:
    OptionsAudioMenuV2(FEPresentation*, ButtonComponent::ButtonState, AudioSettings&);
    virtual ~OptionsAudioMenuV2();
    virtual void Update(float);
    virtual void Save();
    virtual void Revert();
    virtual bool ChangesMade()
    {
        u32 checksum = nlChecksum32(&mSettings, sizeof(AudioSettings));
        return mSettingsCRC != checksum;
    }

    /* 0x26C */ AudioSettings& mSettings;
    /* 0x270 */ AudioSettings mBackupSettings;
    /* 0x290 */ bool mbUpdateMode;
}; // total size: 0x294

class OptionsCheatsMenu : public OptionsSubMenu
{
public:
    OptionsCheatsMenu(FEPresentation*, ButtonComponent::ButtonState, CheatSettings&);
    virtual ~OptionsCheatsMenu();
    virtual void Save();
    virtual void Revert();
    virtual bool ChangesMade()
    {
        u32 checksum = nlChecksum32(&mSettings, sizeof(CheatSettings));
        return mSettingsCRC != checksum;
    }

    void BuildCustomPowerupsList(TLComponentInstance*, CustomPowerups, FEPresentation*);
    void BuildLockableSubMenuList(int, TLComponentInstance*, FEPresentation*, bool, int);

    /* 0x26C */ CheatSettings& mSettings;
    /* 0x270 */ CheatSettings mBackupSettings;
}; // total size: 0x278

// class Function1<void, SlideMenuItem*>
// {
// public:
//     void FunctorBase::~FunctorBase();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (SlideMenuList::*)()>, SlideMenuList*>>::Clone() const;
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (SlideMenuList::*)()>, SlideMenuList*>>::operator()(SlideMenuItem*);
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (SlideMenuList::*)()>, SlideMenuList*>>::~FunctorImpl();
// };

// class SlideMenuList
// {
// public:
//     ~SlideMenuList();
//     void Update(float);
// };

// class MenuItem<SlideMenuItem>
// {
// public:
//     void ~MenuItem();
//     void MenuItem();
// };

// class SlideMenuItem
// {
// public:
//     ~SlideMenuItem();
// };

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class FEFinder<TLInstance, 4>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
// };

// class MenuList<SlideMenuItem>
// {
// public:
//     void ~MenuList();
// };

// class MemFun<SlideMenuList, void>(void (SlideMenuList
// {
// public:
//     void *)());
// };

// class Bind < void, Detail
// {
// public:
//     void MemFunImpl<void, void (SlideMenuList::*)()>,
//         SlideMenuList * > (Detail::MemFunImpl<void, void (SlideMenuList::*)()>, SlideMenuList* const&);
// };

#endif // _FEOPTIONSSUBMENUS_H_
