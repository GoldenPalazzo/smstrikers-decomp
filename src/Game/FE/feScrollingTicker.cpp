#include "Game/FE/feScrollingTicker.h"

/**
 * Offset/Address/Size: 0x0 | 0x8009FC58 | size: 0x8
 */
bool ScrollingTickerScene::IsMessengerOpen() const
{
    return m_bVisible;
}

/**
 * Offset/Address/Size: 0x8 | 0x8009FC60 | size: 0x15C
 */
void ScrollingTickerScene::CloseMessengerNow()
{
    m_pFETweenManager.clearTweensOnObj(this);

    f32 closedY = m_leftBallClosedPos.f.y;
    f32 open = m_leftBallOpenPos.f.x;
    f32 x;
    f32 val = 0.0f;

    x = val * (open - m_leftBallClosedPos.f.x) + m_leftBallClosedPos.f.x;
    m_leftBall->SetAssetPosition(x, closedY, val);

    open = m_rightBallOpenPos.f.x;
    x = open - m_rightBallClosedPos.f.x;
    x = val * x + m_rightBallClosedPos.f.x;
    m_rightBall->SetAssetPosition(x, closedY, val);

    open = m_grayOpenScale.f.x;
    x = open - m_grayClosedScale.f.x;
    x = val * x + m_grayClosedScale.f.x;
    m_backRectangle->SetAssetScale(x, m_grayOpenScale.f.y, 1.0f);

    f32 sz;
    f32 sy;
    f32 sx;
    sx = m_ballClosedScale.f.x * val;
    sy = m_ballClosedScale.f.y * val;
    sz = m_ballClosedScale.f.z * val;
    m_leftBall->SetAssetScale(sx, sy, sz);
    m_rightBall->SetAssetScale(sx, sy, sz);

    m_backRectangle->SetAssetScale(
        m_grayClosedScale.f.x * val,
        m_grayClosedScale.f.y * val,
        m_grayClosedScale.f.z * val);

    m_textBox->m_bVisible = false;
    m_active = 0;
    SetVisible(false);
}

/**
 * Offset/Address/Size: 0x164 | 0x8009FDBC | size: 0xF4
 */
void ScrollingTickerScene::CloseMessenger()
{
    m_pFETweenManager.clearTweensOnObj(this);

    f32 endScale = 0.0f;
    f32 startScale = 1.0f;

    FETweener* scaleTween = m_pFETweenManager.createTween(
        &endScale, &startScale, 0.3f, 0.1f, 1, TweenFunctions::easeinelastic, this, setScaleTweenCallback);

    FETweener* sizeTween = m_pFETweenManager.createTween(
        &endScale, &startScale, 0.15f, 0.0f, 1, TweenFunctions::linear, this, setSizeTweenCallback);

    sizeTween->setNextTween(scaleTween);
    scaleTween->setDoneCallFunc(tickerClosed, this);
    m_pFETweenManager.startTween(sizeTween);

    m_textBox->m_bVisible = false;
    m_active = 0;
}

/**
 * Offset/Address/Size: 0x258 | 0x8009FEB0 | size: 0x16C
 * TODO: 99.45% match - f0/f1 register swap in first SetAssetPosition block
 *       (fsubs f0 vs f1), and f29/f31 swap for sx/sz ball scale variables.
 *       Both are MWCC register allocator differences shared with CloseMessengerNow.
 */
void ScrollingTickerScene::OpenMessengerNow()
{
    m_pFETweenManager.clearTweens();

    f32 closedY = m_leftBallClosedPos.f.y;
    f32 open = m_leftBallOpenPos.f.x;
    f32 x;
    f32 val = 1.0f;
    f32 leftClosedX = m_leftBallClosedPos.f.x;

    x = open - leftClosedX;
    x = val * x + leftClosedX;
    m_leftBall->SetAssetPosition(x, closedY, 0.0f);

    open = m_rightBallOpenPos.f.x;
    x = open - m_rightBallClosedPos.f.x;
    x = val * x + m_rightBallClosedPos.f.x;
    m_rightBall->SetAssetPosition(x, closedY, 0.0f);

    open = m_grayOpenScale.f.x;
    x = open - m_grayClosedScale.f.x;
    x = val * x + m_grayClosedScale.f.x;
    m_backRectangle->SetAssetScale(x, m_grayOpenScale.f.y, 1.0f);

    f32 sz;
    f32 sy;
    f32 sx;
    sx = m_ballClosedScale.f.x * val;
    sy = m_ballClosedScale.f.y * val;
    sz = m_ballClosedScale.f.z * val;
    m_leftBall->SetAssetScale(sx, sy, sz);
    m_rightBall->SetAssetScale(sx, sy, sz);

    m_backRectangle->SetAssetScale(
        m_grayClosedScale.f.x * val,
        m_grayClosedScale.f.y * val,
        m_grayClosedScale.f.z * val);

    m_textBox->m_bVisible = true;
    SetVisible(true);
    m_active = 1;
    m_textScroller->m_msgTime = 0.0f;
}

/**
 * Offset/Address/Size: 0x3C4 | 0x800A001C | size: 0x20C
 * TODO: 99.47% match - remaining f28/f31 swap in scale values (z component path)
 *       and SDA vs stack loads for interpolation factor in right/gray interpolation.
 */
void ScrollingTickerScene::OpenMessenger()
{
    m_pFETweenManager.clearTweens();

    f32 from = 0.0f;
    f32 to = 1.0f;

    f32 closedY = m_leftBallClosedPos.f.y;
    f32 open = m_leftBallOpenPos.f.x;
    f32 x;

    x = from * (open - m_leftBallClosedPos.f.x) + m_leftBallClosedPos.f.x;
    m_leftBall->SetAssetPosition(x, closedY, from);

    open = m_rightBallOpenPos.f.x;
    x = open - m_rightBallClosedPos.f.x;
    x = from * x + m_rightBallClosedPos.f.x;
    m_rightBall->SetAssetPosition(x, closedY, from);

    open = m_grayOpenScale.f.x;
    x = open - m_grayClosedScale.f.x;
    x = from * x + m_grayClosedScale.f.x;
    m_backRectangle->SetAssetScale(x, m_grayOpenScale.f.y, 1.0f);

    f32 val = from;
    f32 sy;
    f32 sx;
    f32 sz;
    sx = m_ballClosedScale.f.x * val;
    sy = m_ballClosedScale.f.y * val;
    sz = m_ballClosedScale.f.z * val;
    m_leftBall->SetAssetScale(sx, sy, sz);
    m_rightBall->SetAssetScale(sx, sy, sz);

    m_backRectangle->SetAssetScale(
        m_grayClosedScale.f.x * val,
        m_grayClosedScale.f.y * val,
        m_grayClosedScale.f.z * val);

    m_textBox->m_bVisible = false;

    FETweener* tickerExpand = m_pFETweenManager.createTween(
        &from, &to, 1.0f, 0.0f, 1, TweenFunctions::easeoutelastic, this, setSizeTweenCallback);

    FETweener* tickerGrow = m_pFETweenManager.createTween(
        &from, &to, 0.15f, 0.0f, 1, TweenFunctions::linear, this, setScaleTweenCallback);

    tickerGrow->setNextTween(tickerExpand);
    tickerExpand->setDoneCallFunc(tickerOpened, this);
    m_pFETweenManager.startTween(tickerGrow);

    SetVisible(true);
}

/**
 * Offset/Address/Size: 0x5D0 | 0x800A0228 | size: 0x60
 */
void ScrollingTickerScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    m_pFETweenManager.Update(fDeltaT);
    if (m_active)
    {
        m_textScroller->Update(fDeltaT);
    }
}

/**
 * Offset/Address/Size: 0x630 | 0x800A0288 | size: 0x24
 */
// void ScrollingTickerScene::SetDisplayMessage(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

/**
 * Offset/Address/Size: 0x654 | 0x800A02AC | size: 0x634
 */
void ScrollingTickerScene::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0xC88 | 0x800A08E0 | size: 0x178
 * TODO: 92.82% match - FEIMessenger vtable layout in this TU still produces
 * `addi r0, r3, 0xC` + explicit FEIMessenger dtor call instead of target
 * `addi r0, r3, 0x24`; also one `addic.` null-check shape differs around
 * m_textScroller->m_messageFinishedCB teardown.
 */
ScrollingTickerScene::~ScrollingTickerScene()
{
    if (m_textScroller != NULL)
    {
        if (&m_textScroller->m_messageFinishedCB != NULL)
        {
            delete m_textScroller;
        }
    }
}

/**
 * Offset/Address/Size: 0xE00 | 0x800A0A58 | size: 0x88
 * TODO: 97% match - missing stw r5,0x88(r31) for m_cbFunc.mTag zeroing.
 *       Adding mTag=EMPTY to Function<FnVoidVoid> ctor fixes this to 100%
 *       but regresses 5 -inline deferred functions from 100% (dead store not eliminated).
 */
ScrollingTickerScene::ScrollingTickerScene()
    : BaseSceneHandler()
    , m_active(false)
    , m_cbFunc()
    , m_textScroller(NULL)
    , m_pFETweenManager()
{
}

/**
 * Offset/Address/Size: 0xE88 | 0x800A0AE0 | size: 0x30
 */
void ScrollingTickerScene::tickerClosed(void* scene)
{
    ((ScrollingTickerScene*)scene)->SetVisible(false);
}

/**
 * Offset/Address/Size: 0xEB8 | 0x800A0B10 | size: 0x18
 */
void ScrollingTickerScene::tickerOpened(void* pScene)
{
    ScrollingTickerScene* scene = (ScrollingTickerScene*)pScene;
    scene->m_active = 1;
    scene->m_textScroller->m_msgTime = 0.0f;
}

/**
 * Offset/Address/Size: 0xED0 | 0x800A0B28 | size: 0xCC
 */
void ScrollingTickerScene::setScaleTweenCallback(void* scene, const float* value)
{
    ScrollingTickerScene* tscene = (ScrollingTickerScene*)scene;
    f32 val = *value;
    f32 x = tscene->m_ballClosedScale.f.x * val;
    f32 y = tscene->m_ballClosedScale.f.y * val;
    f32 z = tscene->m_ballClosedScale.f.z * val;
    tscene->m_leftBall->SetAssetScale(x, y, z);
    tscene->m_rightBall->SetAssetScale(x, y, z);
    tscene->m_backRectangle->SetAssetScale(
        tscene->m_grayClosedScale.f.x * val,
        tscene->m_grayClosedScale.f.y * val,
        tscene->m_grayClosedScale.f.z * val);
}

/**
 * Offset/Address/Size: 0xF9C | 0x800A0BF4 | size: 0xB0
 */
void ScrollingTickerScene::setSizeTweenCallback(void* scene, const float* value)
{
    ScrollingTickerScene* tscene = (ScrollingTickerScene*)scene;
    f32 val = value[0];
    f32 closedY = tscene->m_leftBallClosedPos.f.y;
    f32 open = tscene->m_leftBallOpenPos.f.x;
    f32 x;

    x = val * (open - tscene->m_leftBallClosedPos.f.x) + tscene->m_leftBallClosedPos.f.x;
    tscene->m_leftBall->SetAssetPosition(x, closedY, 0.0f);

    open = tscene->m_rightBallOpenPos.f.x;
    x = open - tscene->m_rightBallClosedPos.f.x;
    x = val * x + tscene->m_rightBallClosedPos.f.x;
    tscene->m_rightBall->SetAssetPosition(x, closedY, 0.0f);

    open = tscene->m_grayOpenScale.f.x;
    x = open - tscene->m_grayClosedScale.f.x;
    x = val * x + tscene->m_grayClosedScale.f.x;
    tscene->m_backRectangle->SetAssetScale(x, tscene->m_grayOpenScale.f.y, 1.0f);
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800A0CA4 | size: 0x128
//  */
// void 0x800A0DCC..0x800A1304 | size: 0x538
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A0DCC | size: 0x38
//  */
// void FEFinder<TLInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800A0E04 | size: 0x84
//  */
// void FEFinder<TLInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800A0E88 | size: 0x84
//  */
// void FEFinder<TLInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x800A0F0C | size: 0x15C
//  */
// void FEFinder<TLInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x29C | 0x800A1068 | size: 0x38
//  */
// void FEFinder<TLInstance, 2>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800A10A0 | size: 0x84
//  */
// void FEFinder<TLInstance, 2>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x358 | 0x800A1124 | size: 0x84
//  */
// void FEFinder<TLInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x3DC | 0x800A11A8 | size: 0x15C
//  */
// void FEFinder<TLInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A1304 | size: 0x8
//  */
// void ScrollingTickerScene::@4@SceneCreated()
// {
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x800A130C | size: 0x8
//  */
// void ScrollingTickerScene::@4@Update(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x10 | 0x800A1314 | size: 0x8
//  */
// void 0x802A9880..0x802A9938 | size: 0xB8
// {
// }
