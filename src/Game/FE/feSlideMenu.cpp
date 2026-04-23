#include "Game/FE/feSlideMenu.h"

#include "NL/nlString.h"

/**
 * Offset/Address/Size: 0x0 | 0x80096C54 | size: 0x34
 */
void FESlideMenu::UpdatePresentation()
{
    m_pMenuComp->SetActiveSlide(m_menuItems[m_currentSlide].ItemSlide);
}

/**
 * Offset/Address/Size: 0x34 | 0x80096C88 | size: 0xE4
 */
bool FESlideMenu::PrevItem()
{
    if (m_lockInput)
    {
        return false;
    }

    u8 idx = m_currentSlide;
    bool changed = true;

    if (idx == 0)
    {
        if (m_doWrapAround)
        {
            m_currentSlide = m_size - 1;
        }
        else
        {
            changed = false;
        }
    }
    else
    {
        m_currentSlide = idx - 1;
    }

    if (changed)
    {
        m_pMenuComp->SetActiveSlide(m_menuItems[m_currentSlide].ItemSlide);

        MenuItem* item = &m_menuItems[m_currentSlide];
        s32 tag = item->ItemCBFuncs[1].mTag;
        if (tag != EMPTY)
        {
            if (tag == FREE_FUNCTION)
            {
                ((void (*)(MenuItem*))item->ItemCBFuncs[1].mFreeFunction)(item);
            }
            else
            {
                (*((FunctorBase*)item->ItemCBFuncs[1].mFunctor))();
            }
        }

        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x118 | 0x80096D6C | size: 0xE8
 */
bool FESlideMenu::NextItem()
{
    if (m_lockInput)
    {
        return false;
    }

    bool didChange = true;

    if (m_currentSlide == m_size - 1)
    {
        // At end of menu
        if (m_doWrapAround)
        {
            m_currentSlide = 0;
        }
        else
        {
            didChange = false;
        }
    }
    else
    {
        m_currentSlide++;
    }

    if (didChange)
    {
        m_pMenuComp->SetActiveSlide(m_menuItems[m_currentSlide].ItemSlide);

        MenuItem* item = &m_menuItems[m_currentSlide];
        int funcType = item->ItemCBFuncs[1].mTag;

        if (funcType == 0)
        {
            // No callback
        }
        else if (funcType == 1)
        {
            // Direct function call
            void (*fn)() = (void (*)())item->ItemCBFuncs[1].mFreeFunction;
            fn();
        }
        else
        {
            (*((FunctorBase*)item->ItemCBFuncs[1].mFunctor))();
        }

        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x200 | 0x80096E54 | size: 0xA4
 */
void FESlideMenu::SetSlideByIndex(unsigned char index)
{
    if (m_currentSlide == index || index >= m_size)
        return;

    m_currentSlide = index;

    MenuItem& item = m_menuItems[m_currentSlide];
    Tag tag = (Tag)item.ItemCBFuncs[ON_HIGHLIGHT].mTag;
    if (tag != EMPTY)
    {
        if (tag == FREE_FUNCTION)
        {
            ((void (*)())item.ItemCBFuncs[ON_HIGHLIGHT].mFreeFunction)();
        }
        else
        {
            (*((FunctorBase*)item.ItemCBFuncs[ON_HIGHLIGHT].mFunctor))();
        }
    }

    m_pMenuComp->SetActiveSlide(m_menuItems[m_currentSlide].ItemSlide);
}

/**
 * Offset/Address/Size: 0x2A4 | 0x80096EF8 | size: 0x84
 */
bool FESlideMenu::ApplyFunction()
{
    if (m_lockInput)
    {
        return false;
    }

    MenuItem* item = &m_menuItems[m_currentSlide];
    int tag = item->ItemCBFuncs[0].mTag;

    if (tag == EMPTY)
    {
        goto ret0;
    }

    if (tag == EMPTY)
    {
        goto ret1;
    }

    if (tag != FREE_FUNCTION)
    {
        goto functor;
    }

    ((void (*)(MenuItem*))item->ItemCBFuncs[0].mFreeFunction)(item);
    goto ret1;

functor:
    (*((FunctorBase*)item->ItemCBFuncs[0].mFunctor))();

ret1:
    return true;

ret0:
    return false;
}

/**
 * Offset/Address/Size: 0x328 | 0x80096F7C | size: 0xF8
 */
FESlideMenu::MenuItem* FESlideMenu::AddMenuItem(const char* name, const Function<FnVoidVoid>& func)
{
    MenuItem* item = &m_menuItems[m_size];
    m_menuItems[m_size].ItemSlide = nlStringLowerHash(name);
    m_menuItems[m_size].ItemCBFuncs[0] = func;
    m_size++;
    return item;
}

/**
 * Offset/Address/Size: 0x420 | 0x80097074 | size: 0x60
 */
FESlideMenu::MenuItem* FESlideMenu::AddMenuItem(const char* name)
{
    MenuItem* item = &m_menuItems[m_size];
    m_menuItems[m_size].ItemSlide = nlStringLowerHash(name);
    m_size++;
    return item;
}

/**
 * Offset/Address/Size: 0x480 | 0x800970D4 | size: 0x60
 */
FESlideMenu::~FESlideMenu()
{
}

/**
 * Offset/Address/Size: 0x4E0 | 0x80097134 | size: 0x64
 */
// void FESlideMenu::MenuItem::~MenuItem()
// {
// }

/**
 * Offset/Address/Size: 0x5C8 | 0x8009721C | size: 0x8C
 */
FESlideMenu::FESlideMenu(TLComponentInstance* pWorkPres)
{
    m_size = 0;
    m_currentSlide = 0;
    m_doWrapAround = 0;
    m_pMenuComp = pWorkPres;
    m_lockInput = 0;
    m_callbackParam = 0;
    mLastChosenSlide = -1;
    mLastRandomSlide = -1;
    mNumCyclesRemaining = 0;
    mRandDeltaTime = 0.0f;
}

/**
 * Offset/Address/Size: 0x654 | 0x800972A8 | size: 0x4C
 */
// void FESlideMenu::MenuItem::MenuItem()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800972F4 | size: 0xC
 */
// Force instantiation
Function<FnVoidVoid> __dummy_inst;
