#include "Game/FE/feNSNMessenger.h"
#include "Game/FE/feFinder.h"

#include "NL/gl/glStruct.h"
#include "NL/nlTask.h"

struct LOCHeader
{
    char Thumbprint[4];
    unsigned long Version;
    unsigned long Language;
    unsigned long StringCount;
    unsigned long Flags;
};

class nlLocalization
{
public:
    struct StringLookup
    {
        unsigned long hash;
        unsigned long StringOffset;
    };

    LOCHeader* m_pFile;
    StringLookup* m_LookupTable;
    unsigned short* m_FirstString;
};

extern void* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

template <typename T, typename U>
T* nlBSearch(const U&, T*, int);

static float MESSAGE_DISPLAY_TIME;
extern "C" FEScrollText* __ct__12FEScrollTextFP14TLTextInstanceii(FEScrollText*, TLTextInstance*, int, int);

/**
 * Offset/Address/Size: 0x0 | 0x800A131C | size: 0x1C0
 * TODO: 99.11% match - one redundant beq remains before callback teardown in delete path
 */
void NSNMessengerScene::EnableScrolling(bool state)
{
    if (state)
    {
        typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

        union
        {
            FindTextByValue byValue;
            FindTextByRef byRef;
        } findText;

        unsigned long hash;
        volatile InlineHasher hB, hA;
        volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

        findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;

        hash = nlStringLowerHash("msg");
        h6.m_Hash = hash;
        h7.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        h8.m_Hash = hash;
        h9.m_Hash = hash;

        hash = nlStringLowerHash("Text");
        hB.m_Hash = hash;
        hA.m_Hash = hash;

        TLTextInstance* textinstance = findText.byRef(
            m_pFEPresentation->m_currentSlide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        if (m_scrollText == NULL)
        {
            gl_ScreenInfo* screeninfo = glGetScreenInfo();
            FEScrollText* scrolltext;
            if ((scrolltext = (FEScrollText*)nlMalloc(0x22C, 8, false)) != NULL)
            {
                scrolltext = __ct__12FEScrollTextFP14TLTextInstanceii(scrolltext, textinstance, 0, screeninfo->ScreenWidth);
            }
            m_scrollText = scrolltext;
        }
        else
        {
            m_scrollText->ApplyNewTextInstancePointer(textinstance, 8000.0f, 100.0f);
        }
    }
    else
    {
        struct FEScrollTextDeleteType
        {
            TLTextInstance* m_controlText;
            BasicString<unsigned short, Detail::TempStringAllocator> m_message;
            int m_messageWidth;
            int m_pos;
            int m_width;
            float m_leftEdge;
            float m_msgTime;
            unsigned short m_textBuffer[256];
            Function<FnVoidVoid> m_messageFinishedCB;
            unsigned char m_visible;
            nlFont* m_textFont;
        };

        if (m_scrollText != NULL)
        {
            if (m_scrollText != NULL)
            {
                delete (FEScrollTextDeleteType*)m_scrollText;
            }
            m_scrollText = NULL;
        }
    }
}

/**
 * Offset/Address/Size: 0x1C0 | 0x800A14DC | size: 0x8
 */
bool NSNMessengerScene::IsMessengerOpen() const
{
    return m_bVisible;
}

/**
 * Offset/Address/Size: 0x1C8 | 0x800A14E4 | size: 0x48
 */
void NSNMessengerScene::CloseMessengerNow()
{
    SetVisible(false);
    m_curState = MS_CLOSED;
}

/**
 * Offset/Address/Size: 0x210 | 0x800A152C | size: 0x64
 */
void NSNMessengerScene::CloseMessenger()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    presentation->SetActiveSlide("Outro");
    presentation->m_currentSlide->m_time = 0.0f;
    presentation->m_currentSlide->Update(0.0f);

    m_curState = MS_CLOSING;
}

static inline const unsigned short* LookupLocText(const char* locMessage)
{
    unsigned long hash = nlStringLowerHash(locMessage);
    nlLocalization* loc = (nlLocalization*)g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* lookup = nlBSearch<nlLocalization::StringLookup, unsigned long>(hash, loc->m_LookupTable, loc->m_pFile->StringCount);
    if (lookup != 0)
    {
        return loc->m_FirstString + lookup->StringOffset;
    }
    return MissingLocString;
}

static inline void CopyWideString(BasicStringInternal* data, const unsigned short* text)
{
    data->mData = 0;
    data->mSize = 0;
    data->mCapacity = 0;

    const unsigned short* ptr = text;
    while (*ptr++ != 0)
    {
        data->mSize++;
    }

    data->mSize++;
    data->mData = (char*)nlMalloc((data->mSize + 1) * 2, 8, true);
    data->mCapacity = data->mSize;

    int j;
    int i = 0;
    j = i;
    while (i < data->mSize)
    {
        *(unsigned short*)(data->mData + j) = *text;
        i++;
        text++;
        j += 2;
    }

    data->mRefCount = 1;
}

/**
 * Offset/Address/Size: 0x274 | 0x800A1590 | size: 0x1A4
 * TODO: 98.71% match - post-call cleanup keeps data in r30 instead of reloading into r29 from stack
 */
void NSNMessengerScene::SetDisplayMessage(const char* locMessage)
{
    const unsigned short* text = LookupLocText(locMessage);

    BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        CopyWideString(data, text);
    }

    {
        BasicStringInternal* localMsgData = data;
        SetDisplayMessage(*(const BasicString<unsigned short, Detail::TempStringAllocator>*)&localMsgData);
    }

    BasicStringInternal* msgData = data;
    if (msgData != 0)
    {
        if (--msgData->mRefCount == 0)
        {
            if (msgData != 0)
            {
                if (msgData != 0)
                {
                    delete[] msgData->mData;
                }
                if (msgData != 0)
                {
                    nlFree(msgData);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x418 | 0x800A1734 | size: 0x220
 */
// void NSNMessengerScene::SetDisplayMessage(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

/**
 * Offset/Address/Size: 0x638 | 0x800A1954 | size: 0x70
 */
void NSNMessengerScene::ForceMessengerVisibleNow()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    SetVisible(true);
    presentation->SetActiveSlide("Visible");
    m_curState = MS_OPEN;
}

/**
 * Offset/Address/Size: 0x6A8 | 0x800A19C4 | size: 0x80
 */
void NSNMessengerScene::OpenMessengerNow()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    if (mVisibilityMask & nlTaskManager::m_pInstance->m_CurrState)
    {
        SetVisible(true);
    }

    presentation->SetActiveSlide("Visible");
    m_curState = MS_OPEN;
}

/**
 * Offset/Address/Size: 0x728 | 0x800A1A44 | size: 0x8C
 */
void NSNMessengerScene::OpenMessenger()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    if (mVisibilityMask & nlTaskManager::m_pInstance->m_CurrState)
    {
        SetVisible(true);
    }

    presentation->SetActiveSlide("Intro");
    presentation->m_currentSlide->Update(0.0f);
    m_curState = MS_OPENING;
}

/**
 * Offset/Address/Size: 0x7B4 | 0x800A1AD0 | size: 0x160
 * TODO: 99.94% match - scratch emits BaseSceneHandler inline virtual stubs
 * (InitializeSubHandlers/SetPresentation/SetVisible) before this symbol, so
 * Update starts at +0x14 and only branch displacements differ.
 */
void NSNMessengerScene::Update(float fDeltaT)
{
    BaseOverlayHandler::Update(fDeltaT);

    if (m_messageDisplaying)
    {
        m_messageDisplayTime += fDeltaT;
        if (m_messageDisplayTime > MESSAGE_DISPLAY_TIME && m_scrollText == NULL
            && m_messageFinishedCB.mTag != EMPTY)
        {
            m_messageDisplaying = false;
            if (m_messageFinishedCB.mTag == FREE_FUNCTION)
            {
                ((void (*)())m_messageFinishedCB.mFreeFunction)();
            }
            else
            {
                (*((FunctorBase*)m_messageFinishedCB.mFunctor))();
            }
        }
    }

    if (m_scrollText != NULL)
    {
        m_scrollText->Update(fDeltaT);
    }

    FEPresentation* pres = m_pFEScene->m_pFEPackage->GetPresentation();
    TLSlide* slide = pres->m_currentSlide;

    if (m_curState == MS_OPENING || m_curState == MS_CLOSING)
    {
        if (slide->m_time >= slide->m_start + slide->m_duration)
        {
            switch (m_curState)
            {
            case MS_OPENING:
                OpenMessengerNow();
                break;
            case MS_OPEN:
                break;
            case MS_CLOSING:
                CloseMessenger();
                break;
            default:
                break;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x914 | 0x800A1C30 | size: 0x2C
 * TODO: 99.55% match - vtable offset 0x3C instead of 0x1C (multiple inheritance layout difference)
 */
void NSNMessengerScene::SceneCreated()
{
    SceneCreated();
}

/**
 * Offset/Address/Size: 0x940 | 0x800A1C5C | size: 0x180
 * TODO: 96.72% match - one addic./beq pair remains in FEScrollText delete path and
 *       one addic./beq pair remains before m_messageFinishedCB teardown.
 */
NSNMessengerScene::~NSNMessengerScene()
{
    if (m_scrollText != NULL)
    {
        if (m_scrollText != NULL)
        {
            delete m_scrollText;
        }
    }
}

/**
 * Offset/Address/Size: 0xAC0 | 0x800A1DDC | size: 0x84
 */
NSNMessengerScene::NSNMessengerScene()
    : BaseOverlayHandler(3, POSITION_BOTTOM)
{
    m_messageFinishedCB.mTag = EMPTY;
    m_curState = MS_INVALID;
    m_messageDisplayTime = 0.0f;
    m_scrollText = NULL;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800A1E60 | size: 0xA8
//  */
// void 0x800A1F08..0x800A1F0C | size: 0x4
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A1F08 | size: 0x4
//  */
// void BaseSceneHandler::SceneCreated()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A1F0C | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800A1F44 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800A1FC8 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800A2124 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800A215C | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800A223C | size: 0x8
//  */
// void NSNMessengerScene::@4@SceneCreated()
// {
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x800A2244 | size: 0x8
//  */
// void NSNMessengerScene::@4@Update(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x10 | 0x800A224C | size: 0x8
//  */
// void 0x802A9938..0x802A99C0 | size: 0x88
// {
// }
