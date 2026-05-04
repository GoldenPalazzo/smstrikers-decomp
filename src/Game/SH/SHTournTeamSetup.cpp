#include "Game/SH/SHTournTeamSetup.h"
#include "types.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feTemplates.h"
#include "Game/GameSceneManager.h"
#include "Game/SH/SHCupHub.h"

// Temporary dummy object for reference member initialization
static CustomTournament s_dummyTourn;

static unsigned long TOURN_CAPTAIN_DESCRIPTIONS[] = {
    0xFF68ABBA, 0xE2D37C19, 0x000465BA, 0x000BAD38, 0x0043DF21, 0x330C3072, 0x00C0A242, 0x00EC84AC, 0x69BFAF9D
};

// /**
//  * Offset/Address/Size: 0x0 | 0x800E7710 | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>, TournTeamSetupSceneV2*, int>(Detail::MemFunImpl<void,
// void (TournTeamSetupSceneV2::*)(int)>, TournTeamSetupSceneV2* const&, const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E76F4 | size: 0x1C
//  */
// void MemFun<TournTeamSetupSceneV2, void, int>(void (TournTeamSetupSceneV2::*)(int))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E7698 | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>,
// TournTeamSetupSceneV2*, int>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x800E7664 | size: 0x34
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>,
// TournTeamSetupSceneV2*, int>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E75E4 | size: 0x80
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>,
// TournTeamSetupSceneV2*, int>>::Clone() const
// {
// }

/**
 * Offset/Address/Size: 0x704 | 0x800E7488 | size: 0x15C
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 4>::_Find<TLInstance>(
    TLInstance* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->pChildren, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->pChildren, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x680 | 0x800E7404 | size: 0x84
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 4>::_Find<TLSlide>(
    TLSlide* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->m_instances, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->m_instances, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

// /**
//  * Offset/Address/Size: 0x648 | 0x800E73CC | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800E7270 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800E71EC | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800E71B4 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800E7058 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800E6FD4 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800E6F9C | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800E6E40 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800E6DBC | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E6D84 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x4DBC | 0x800E6C60 | size: 0x124
 */
TournTeamSetupSceneV2::TournTeamSetupSceneV2()
    : BaseSceneHandler()
    , mMenuItems()
    , mRowOffset(0)
    , mCurrentRow(0)
    , mTournInfo(nlSingleton<GameInfoManager>::s_pInstance->mCustomTournamentInfo)
    , mCurrentState(STATE_IN)
    , mCurrentCaptain(TEAM_MARIO)
    , mCurrentSK(SK_TOAD)
    , mButtons1()
    , mButtons2()
{
    int i = 0;

    *(u8*)((u8*)this + 0x334) = false;

    for (; i < mTournInfo.m_numTeams; i++)
    {
        mTeamData[i].isEmpty = true;
        mTeamData[i].isHumanPlayer = false;
        mTeamData[i].captain = TEAM_MARIO;
        mTeamData[i].sidekick = SK_TOAD;
    }
}

/**
 * Offset/Address/Size: 0x4C18 | 0x800E6ABC | size: 0x1A4
 */
TournTeamSetupSceneV2::~TournTeamSetupSceneV2()
{
    delete mCaptainGrid;
    delete mSKGrid;

    FEScrollText* ticker = mTicker;

    if (ticker != NULL)
    {
        if (ticker != NULL)
        {
            if ((char*)ticker + 0x21C)
            {
                volatile FEScrollText* vticker = ticker;
                if ((char*)vticker + 0x21C)
                {
                    if (ticker->m_messageFinishedCB.mTag == FUNCTOR)
                    {
                        delete ticker->m_messageFinishedCB.mFunctor;
                    }
                    ticker->m_messageFinishedCB.mTag = EMPTY;
                }
            }

            if ((char*)ticker + 4)
            {
                BasicStringData<unsigned short>* data = ticker->m_message.m_data;
                if (data != NULL)
                {
                    if (--data->mRefCount == 0)
                    {
                        if (data != NULL)
                        {
                            if (data != NULL)
                            {
                                delete[] data->mData;
                            }
                            if (data != NULL)
                            {
                                nlFree(data);
                            }
                        }
                    }
                }
            }

            ::operator delete(ticker);
        }
    }
}

/**
 * Offset/Address/Size: 0x3ED4 | 0x800E5D78 | size: 0xD44
 */
void TournTeamSetupSceneV2::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x32F4 | 0x800E5198 | size: 0xBE0
 */
void TournTeamSetupSceneV2::Update(float)
{
}

/**
 * Offset/Address/Size: 0x2FB4 | 0x800E4E58 | size: 0x340
 */
void TournTeamSetupSceneV2::UpdateControllerIcon(int arg)
{
    typedef TLComponentInstance* (*FindCompByValue)(
        TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(
        TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    int nlSNPrintf(char*, unsigned long, const char*, ...);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    int row = arg + mRowOffset;

    volatile InlineHasher hA, hB;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher innerHash;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    volatile InlineHasher eB, eA;
    volatile InlineHasher e4, e3, e2, e1, e0;

    volatile InlineHasher dB, dA;
    volatile InlineHasher d4, d3, d2, d1, d0;

    char menuName[68];

    nlSNPrintf(menuName, 64, "MENU ITEM%d", arg + 1);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    unsigned long hash = nlStringLowerHash(menuName);
    h8.m_Hash = hash;
    h9.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* item = findComp.byRef(
        presentation->m_currentSlide,
        (InlineHasher&)hA,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    item->SetActiveSlide("IN");
    TLSlide* active = item->GetActiveSlide();
    item->Update(active->m_start + active->m_duration);

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("CONTROLLER");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* controllerA = findComp.byRef(
        item->GetActiveSlide(),
        (InlineHasher&)gB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    item->SetActiveSlide("OUT");
    active = item->GetActiveSlide();
    item->Update(active->m_start + active->m_duration);

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("CONTROLLER");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* controllerB = findComp.byRef(
        item->GetActiveSlide(),
        (InlineHasher&)fB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    item->SetActiveSlide("IN");
    active = item->GetActiveSlide();
    item->Update(active->m_start + active->m_duration);

    if (mTeamData[row].isHumanPlayer)
    {
        controllerA->SetActiveSlide("CONTROLLER");
        controllerB->SetActiveSlide("CONTROLLER");
    }
    else
    {
        controllerA->SetActiveSlide("CPU");
        controllerB->SetActiveSlide("CPU");
    }

    e0.m_Hash = 0;
    h1.m_Hash = 0;
    e1.m_Hash = 0;
    h3.m_Hash = 0;
    e2.m_Hash = 0;
    h5.m_Hash = 0;
    e3.m_Hash = 0;
    h7.m_Hash = 0;
    e4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("arrows");
    eA.m_Hash = hash;
    eB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* arrowsA = findComp.byRef(
        controllerA->GetActiveSlide(),
        (InlineHasher&)eB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    d0.m_Hash = 0;
    h1.m_Hash = 0;
    d1.m_Hash = 0;
    h3.m_Hash = 0;
    d2.m_Hash = 0;
    h5.m_Hash = 0;
    d3.m_Hash = 0;
    h7.m_Hash = 0;
    d4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("arrows");
    dA.m_Hash = hash;
    dB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* arrowsB = findComp.byRef(
        controllerB->GetActiveSlide(),
        (InlineHasher&)dB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    arrowsA->m_bVisible = true;
    arrowsB->m_bVisible = true;
}

/**
 * Offset/Address/Size: 0x2500 | 0x800E43A4 | size: 0xAB4
 */
void TournTeamSetupSceneV2::UpdateRow(int)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x1F30 | 0x800E3DD4 | size: 0x5D0
 */
void TournTeamSetupSceneV2::ChangeState(TournTeamSetupSceneV2::eTeamChooserState, TournTeamSetupSceneV2::eTeamChooserState)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x1EC4 | 0x800E3D68 | size: 0x6C
 */
void TournTeamSetupSceneV2::StartChooseCaptain(int arg)
{
    mCurrentRow = arg + mRowOffset;

    if (!mTeamData[mCurrentRow].isEmpty)
    {
        mCaptainGrid->SetValid(mTeamData[mCurrentRow].captain, true);
    }

    ChangeState(STATE_SCROLLING, STATE_CAPTAIN);
}

/**
 * Offset/Address/Size: 0x1CC4 | 0x800E3B68 | size: 0x200
 * TODO: 87.3% match - stack frame size 0xB0 vs target 0x80, all diffs are stack
 * offset (s/i) only. -inline deferred file causes different InlineHasher temporary
 * stack allocation on decomp.me vs original build.
 */
void TournTeamSetupSceneV2::UpdateCaptainName()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    TLSlide* slide = mComponents[0]->GetActiveSlide();

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

    unsigned long hash = nlStringLowerHash("CAPTAIN_NAME");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLTextInstance* captainNameText = findText.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    captainNameText->m_LocStrId = GetLOCCharacterName(mCurrentCaptain, false, true);
    captainNameText->m_OverloadFlags |= 0x8;

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

    hash = nlStringLowerHash("CAPTAIN_NAME2");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    TLTextInstance* captainDesc = findText.byRef(
        slide,
        (InlineHasher&)gB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mCurrentCaptain == TEAM_MYSTERY && !nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
    {
        captainDesc->SetStringId("CUP_ATTR_MYSTERY_LOCKED");
    }
    else
    {
        captainDesc->m_LocStrId = TOURN_CAPTAIN_DESCRIPTIONS[mCurrentCaptain];
        captainDesc->m_OverloadFlags |= 0x8;
    }

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("COMPONENT");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    TLComponentInstance* comp = findComp.byRef(
        slide,
        (InlineHasher&)fB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    comp->SetActiveSlide(GetTeamName(mCurrentCaptain));
}

/**
 * Offset/Address/Size: 0x1A48 | 0x800E38EC | size: 0x27C
 */
void TournTeamSetupSceneV2::UpdateSKName()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher iB, iA;
    volatile InlineHasher i4, i3, i2, i1, i0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    TLSlide* slide = mComponents[0]->GetActiveSlide();

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

    unsigned long hash = nlStringLowerHash("CAPTAIN_NAME");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLTextInstance* captainNameText = findText.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    captainNameText->m_LocStrId = GetLOCCharacterName(mCurrentCaptain, false, false);
    captainNameText->m_OverloadFlags |= 0x8;

    i0.m_Hash = 0;
    h1.m_Hash = 0;
    i1.m_Hash = 0;
    h3.m_Hash = 0;
    i2.m_Hash = 0;
    h5.m_Hash = 0;
    i3.m_Hash = 0;
    h7.m_Hash = 0;
    i4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("SIDEKICK_NAME");
    iA.m_Hash = hash;
    iB.m_Hash = hash;

    TLTextInstance* sidekickNameText = findText.byRef(
        slide,
        (InlineHasher&)iB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    sidekickNameText->m_LocStrId = GetLOCSidekickName(mCurrentSK);
    sidekickNameText->m_OverloadFlags |= 0x8;

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

    hash = nlStringLowerHash("CAPTAIN_NAME2");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    TLTextInstance* captainDescText = findText.byRef(
        slide,
        (InlineHasher&)gB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mCurrentCaptain == TEAM_MYSTERY && !nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
    {
        captainDescText->SetStringId("CUP_ATTR_MYSTERY_LOCKED");
    }
    else
    {
        captainDescText->m_LocStrId = TOURN_CAPTAIN_DESCRIPTIONS[mCurrentCaptain];
        captainDescText->m_OverloadFlags |= 0x8;
    }

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("COMPONENT");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    TLComponentInstance* teamComp = findComp.byRef(
        slide,
        (InlineHasher&)fB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    teamComp->SetActiveSlide(GetTeamName(mCurrentCaptain));
}

/**
 * Offset/Address/Size: 0x15F4 | 0x800E3498 | size: 0x454
 */
void TournTeamSetupSceneV2::Proceed()
{
    GameInfoManager* pGameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    int i = 0;

    mTournInfo.m_cup->mHumanTeams = 0;

    for (; i < mTournInfo.m_numTeams; i++)
    {
        if (mTeamData[i].isHumanPlayer)
        {
            mTournInfo.m_cup->mHumanTeams = (u16)(mTournInfo.m_cup->mHumanTeams | (1 << mTeamData[i].captain));
        }
    }

    pGameInfo->SetMode(GameInfoManager::GM_TOURNAMENT);
    nlSingleton<GameInfoManager>::s_pInstance->GetCurrentRoundNumber();

    if (mTournInfo.m_tournMode == TM_LEAGUE)
    {
        GameInfoManager* const pTournamentInfo = nlSingleton<GameInfoManager>::s_pInstance;
        u16 numPlayingTeams = pTournamentInfo->GetNumPlayingTeams();

        eSidekickID sklineup[8];
        eTeamID lineup[8];

        int idx = 0;
        for (; idx < numPlayingTeams; idx++)
        {
            lineup[idx] = mTeamData[idx].captain;
            sklineup[idx] = mTeamData[idx].sidekick;
        }

        pTournamentInfo->SetupRoundRobinSchedule(lineup, sklineup);
        pGameInfo->SetPreviousTeamStats();
        pGameInfo->IncreaseRoundNumber();
    }
    else
    {
        BaseCup* pCup = pGameInfo->mCurrentCup;

        pCup->mRoundNumber = -4;

        if (pCup->GetNumRounds() == 2)
        {
            pCup->mRoundNumber = -3;
        }

        GameInfoManager* const pTournamentInfo = nlSingleton<GameInfoManager>::s_pInstance;
        u16 numPlayingTeams = pTournamentInfo->GetNumPlayingTeams();

        eSidekickID sklineup[8];
        eTeamID lineup[8];

        int idx = 0;
        for (; idx < numPlayingTeams; idx++)
        {
            lineup[idx] = mTeamData[idx].captain;
            sklineup[idx] = mTeamData[idx].sidekick;
        }

        pTournamentInfo->SetupTournamentKnockout(lineup, sklineup);
    }

    while (pGameInfo->GetCurrentRoundNumber() != -5)
    {
        if (pGameInfo->DetermineNextMatchups(0x1B))
        {
            break;
        }

        pGameInfo->IncreaseRoundNumber();
    }

    pGameInfo->SetResultsOfLastUserGame(RESULT_CUP_START);

    nlSingleton<GameSceneManager>::s_pInstance->Pop();

    if (pGameInfo->GetNumHumanTeams() == 1)
    {
        int j = 0;

        for (; j < mTournInfo.m_numTeams; j++)
        {
            if (mTeamData[j].isHumanPlayer)
            {
                pGameInfo->SetUserSelectedCupTeam(mTeamData[j].captain);
                break;
            }
        }
    }

    GameSceneManager* pSceneMgr = nlSingleton<GameSceneManager>::s_pInstance;
    SceneList nextScene = SCENE_TOURNAMENT_STANDINGS;

    if (mTournInfo.m_tournMode == TM_LEAGUE)
    {
        nextScene = SCENE_TOURNAMENT_STANDINGS_ANIM;
    }

    CupHubScene* pHubScene = (CupHubScene*)pSceneMgr->Push(nextScene, SCREEN_FORWARD, false);
    pHubScene->mDoAutoSave = true;
}

/**
 * Offset/Address/Size: 0xC34 | 0x800E2AD8 | size: 0x9C0
 */
BasicString<char, Detail::TempStringAllocator> TournTeamSetupSceneV2::FindCaptainSlideName(eTeamID)
{
}

/**
 * Offset/Address/Size: 0x76C | 0x800E2610 | size: 0x4C8
 * TODO: 93.1% match - r29/r30 register swap: MWCC hoists string literal address
 * before nlMalloc in inlined BasicString constructor, swapping data/str registers
 */
BasicString<char, Detail::TempStringAllocator> TournTeamSetupSceneV2::FindSidekickSlideName(eSidekickID sidekick)
{
    BasicString<char, Detail::TempStringAllocator> returnValue;

    switch (sidekick)
    {
    case SK_TOAD:
        returnValue = BasicString<char, Detail::TempStringAllocator>("toad");
        break;
    case SK_KOOPA:
        returnValue = BasicString<char, Detail::TempStringAllocator>("koopa");
        break;
    case SK_HAMMERBROS:
        returnValue = BasicString<char, Detail::TempStringAllocator>("hammer");
        break;
    case SK_BIRDO:
        returnValue = BasicString<char, Detail::TempStringAllocator>("birdo");
        break;
    }

    return returnValue;
}

/**
 * Offset/Address/Size: 0x4B0 | 0x800E2354 | size: 0x2BC
 */
void TournTeamSetupSceneV2::AutoFill()
{
    bool changed = false;
    int i = 0;

    for (; i < mTournInfo.m_numTeams; i++)
    {
        if (mTeamData[i].isEmpty)
        {
            eTeamID randCapt;
            u8 alreadySelected;

            do
            {
                if (nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
                {
                    randCapt = (eTeamID)nlRandom(9, &nlDefaultSeed);
                }
                else
                {
                    randCapt = (eTeamID)nlRandom(8, &nlDefaultSeed);
                }

                for (int k = 0; k < mTournInfo.m_numTeams; k++)
                {
                    if (!mTeamData[k].isEmpty && mTeamData[k].captain == randCapt)
                    {
                        alreadySelected = 1;
                        goto done_select_check;
                    }
                }

                alreadySelected = 0;
            done_select_check:;
            } while (alreadySelected);

            if (!(*(u8*)((u8*)this + 0x334)))
            {
                mTeamData[mCurrentRow].isHumanPlayer = true;
                *(u8*)((u8*)this + 0x334) = true;
            }

            mTeamData[i].isEmpty = false;
            mTeamData[i].sidekick = (eSidekickID)nlRandom(4, &nlDefaultSeed);
            mTeamData[i].captain = randCapt;
            mCaptainGrid->SetValid(randCapt, false);
            changed = true;
        }
    }

    if (changed)
    {
        int numRows = mTournInfo.m_numTeams == 3 ? 3 : 4;
        mRowOffset = mTournInfo.m_numTeams - numRows;

        {
            int currentIndex = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) != 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
                }
            }
        }

        mMenuItems.mCurrentIndex = numRows - 1;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) != 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
                }
            }
        }

        mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
        mCurrentCaptain = mTeamData[mCurrentRow].captain;
        mCurrentSK = mTeamData[mCurrentRow].sidekick;

        mCaptainGrid->MoveHighlightToTarget(mCurrentCaptain);
        ((IGridComponent<eTeamID>*)mSKGrid)->MoveHighlightToTarget((eTeamID)mCurrentSK);
        UpdateCaptainName();

        int j = 0;
        int numRows2 = ((u32)(3 - (u32)mTournInfo.m_numTeams) >> 31) + 3;
        for (; j < numRows2; j++)
        {
            UpdateRow(j);
        }
    }
}

/**
 * Offset/Address/Size: 0x3E4 | 0x800E2288 | size: 0xCC
 */
void TournTeamSetupSceneV2::UpdateForCurrentRow()
{
    FORCE_DONT_INLINE;
    int numRows;
    int i = 0;
    numRows = ((u32)(3 - (u32)mTournInfo.m_numTeams) >> 31) + 3;

    for (; i < numRows; i++)
    {
        UpdateRow(i);
    }

    mCurrentCaptain = mTeamData[mCurrentRow].captain;
    mCurrentSK = mTeamData[mCurrentRow].sidekick;

    mCaptainGrid->MoveHighlightToTarget(mCurrentCaptain);
    ((IGridComponent<eTeamID>*)mSKGrid)->MoveHighlightToTarget((eTeamID)mCurrentSK);
    UpdateCaptainName();
}

/**
 * Offset/Address/Size: 0x1FC | 0x800E20A0 | size: 0x1E8
 */
void TournTeamSetupSceneV2::ScrollUp(bool bPlaySound)
{
    int newIndex;
    bool doUpdate;

    if (!bPlaySound)
    {
        FEAudio::EnableSounds(false);
    }

    int flags = mMenuItems.mFlags;
    int skipFlag;
    int wrapFlag;
    int currentIndex;
    wrapFlag = flags & 1;
    skipFlag = flags & 2;
    currentIndex = mMenuItems.mCurrentIndex;
    newIndex = currentIndex - 1;

loop:
    if (wrapFlag)
    {
        if (newIndex < 0)
        {
            newIndex = mMenuItems.mNumItemsAdded - 1;
        }
    }
    else
    {
        if (newIndex < 0)
        {
            newIndex = 2;
            goto end_section;
        }
    }

    if (skipFlag)
    {
        if (mMenuItems.mMenuItems[newIndex].mDisabled)
        {
            newIndex--;
            goto loop;
        }
    }

    // Deselect old item (callback 2)
    {
        int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
        if (((u32)((-tag) | tag) >> 31) != 0)
        {
            TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
            if (tag == FREE_FUNCTION)
            {
                mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
            }
            else
            {
                (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
            }
        }
    }

    mMenuItems.mCurrentIndex = newIndex;

    int selIdx = mMenuItems.mCurrentIndex;
    int tag2 = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
    if (((u32)((-tag2) | tag2) >> 31) != 0)
    {
        TLComponentInstance* type2 = mMenuItems.mMenuItems[selIdx].mType;
        if (tag2 == FREE_FUNCTION)
        {
            mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type2);
        }
        else
        {
            (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type2);
        }
    }

    newIndex = 1;

end_section:
    mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
    FEAudio::EnableSounds(false);

    doUpdate = true;
    if (newIndex == 2)
    {
        if (mRowOffset > 0)
        {
            mRowOffset--;
            mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
            goto check_update;
        }
    }
    if (newIndex == 2)
    {
        doUpdate = false;
        FEAudio::PlayAnimAudioEvent("sfx_deny", false);
    }

check_update:
    if (doUpdate)
    {
        if (newIndex == 2)
        {
            FEAudio::EnableSounds(true);
        }
        UpdateForCurrentRow();
    }

    FEAudio::EnableSounds(true);
}

/**
 * Offset/Address/Size: 0x0 | 0x800E1EA4 | size: 0x1FC
 */
void TournTeamSetupSceneV2::ScrollDown(bool bPlaySound)
{
    int newIndex;
    bool doUpdate;

    if (!bPlaySound)
    {
        FEAudio::EnableSounds(false);
    }

    int flags = mMenuItems.mFlags;
    int skipFlag;
    int wrapFlag;
    int currentIndex;
    wrapFlag = flags & 1;
    skipFlag = flags & 2;
    currentIndex = mMenuItems.mCurrentIndex;
    newIndex = currentIndex + 1;

loop:
    if (wrapFlag)
    {
        newIndex = newIndex % mMenuItems.mNumItemsAdded;
    }
    else
    {
        if (newIndex >= mMenuItems.mNumItemsAdded)
        {
            newIndex = 2;
            goto end_section;
        }
    }

    if (skipFlag)
    {
        if (mMenuItems.mMenuItems[newIndex].mDisabled)
        {
            newIndex++;
            goto loop;
        }
    }

    // Deselect old item (callback 2)
    {
        int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
        if (((u32)((-tag) | tag) >> 31) != 0)
        {
            TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
            if (tag == FREE_FUNCTION)
            {
                mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
            }
            else
            {
                (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
            }
        }
    }

    mMenuItems.mCurrentIndex = newIndex;

    // Select new item (callback 1)
    {
        int selIdx = mMenuItems.mCurrentIndex;
        int tag2 = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
        if (((u32)((-tag2) | tag2) >> 31) != 0)
        {
            TLComponentInstance* type2 = mMenuItems.mMenuItems[selIdx].mType;
            if (tag2 == FREE_FUNCTION)
            {
                mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type2);
            }
            else
            {
                (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type2);
            }
        }
    }

    newIndex = 1;

end_section:
    mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
    FEAudio::EnableSounds(false);

    doUpdate = true;
    if (newIndex == 2)
    {
        if (mRowOffset + 3 < (int)mTournInfo.m_numTeams - 1)
        {
            mRowOffset++;
            mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
            goto check_update;
        }
    }
    if (newIndex == 2)
    {
        doUpdate = false;
        FEAudio::PlayAnimAudioEvent("sfx_deny", false);
    }

check_update:
    if (doUpdate)
    {
        if (newIndex == 2)
        {
            FEAudio::EnableSounds(true);
        }
        UpdateForCurrentRow();
    }

    FEAudio::EnableSounds(true);
}

void SHTournTeamSetup_stub()
{
    TLInstance* (*volatile forceFind)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher) = &FEFinder<TLInstance, 4>::Find<TLSlide>;
    (void)forceFind;
}
