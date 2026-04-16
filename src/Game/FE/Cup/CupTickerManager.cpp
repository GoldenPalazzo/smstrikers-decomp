#include "Game/FE/Cup/CupTickerManager.h"
#include "Game/FE/feScrollText.h"

namespace Detail
{
template <typename R, typename F>
struct MemFunImpl
{
    F mFuncPtr;
};
} // namespace Detail

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*)());

template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg);

typedef Detail::MemFunImpl<void, void (CupTickerManager::*)()> MemFunImpl_CupTickerManager_v;
typedef BindExp1<void, MemFunImpl_CupTickerManager_v, CupTickerManager*> BindExp1_vfmfcp;
typedef Function0<void>::FunctorImpl<BindExp1_vfmfcp> FunctorImpl_vfmfcp;

struct gl_ScreenInfo
{
    int ScreenWidth;
};

extern gl_ScreenInfo* glGetScreenInfo();

// /**
//  * Offset/Address/Size: 0x0 | 0x800F5EBC | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager*>(Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1C48 | 0x800F5D94 | size: 0x128
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, const unsigned short*>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0xF58 | 0x800F50A4 | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0xE30 | 0x800F4F7C | size: 0x128
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const BasicString<unsigned short, Detail::TempStringAllocator>&, const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x800F428C | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F414C | size: 0x140
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, const unsigned short*, unsigned short[16], unsigned short[16]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short* const&, const unsigned short(&)[16], const unsigned short(&)[16])
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F4130 | size: 0x1C
//  */
// void MemFun<CupTickerManager, void>(void (CupTickerManager::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F40D4 | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager*>>::~FunctorImpl()
// {
// }

/**
 * Offset/Address/Size: 0x78 | 0x800F40A4 | size: 0x30
 */
template <>
void Function0<void>::FunctorImpl<BindExp1_vfmfcp>::operator()()
{
    (mBind.mArg->*(mBind.mFuncPtr.mFuncPtr))();
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800F402C | size: 0x78
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager*>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800F3E24 | size: 0x208
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F3D68 | size: 0xBC
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<unsigned short, Detail::TempStringAllocator>&) const
// {
// }

/**
 * Offset/Address/Size: 0x1D90 | 0x800F3D58 | size: 0x10
 */
CupTickerManager::CupTickerManager()
{
}

/**
 * Offset/Address/Size: 0x1C90 | 0x800F3C58 | size: 0x100
 */
CupTickerManager::~CupTickerManager()
{
}

/**
 * Offset/Address/Size: 0x1968 | 0x800F3930 | size: 0x328
 * TODO: 95.99% match - mTicker load/store scheduling, string init ordering,
 * and cleanup r30/r31 register allocation differ due to -inline deferred vs -inline auto.
 */
void CupTickerManager::SetTickerTextInstance(TLTextInstance* tickerText)
{
    if (mTicker)
    {
        mTicker->ApplyNewTextInstancePointer(tickerText, 0.0f, 1.0f);
    }
    else
    {
        gl_ScreenInfo* screenInfo = glGetScreenInfo();
        mTicker = new ((FEScrollText*)nlMalloc(sizeof(FEScrollText), 0x20, true))
            FEScrollText(tickerText, 0, screenInfo->ScreenWidth + 0x32);

        {
            BindExp1_vfmfcp bind = Bind<void, MemFunImpl_CupTickerManager_v, CupTickerManager*>(
                MemFun<CupTickerManager, void>(&CupTickerManager::CreateNewMessage), this);

            Function<FnVoidVoid> callback;
            callback.mTag = FUNCTOR;

            FunctorImpl_vfmfcp* functor = new ((FunctorImpl_vfmfcp*)nlMalloc(sizeof(FunctorImpl_vfmfcp), 8, false))
                FunctorImpl_vfmfcp(bind);
            callback.mFunctor = functor;

            *(Function<FnVoidVoid>*)&mTicker->m_messageFinishedCB = callback;
        }

        this->CreateNewMessage();
    }

    BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
    if (data)
    {
        data->mData = 0;
        const unsigned short* src = mMessageBuffer;
        data->mSize = 0;
        const unsigned short* ptr = src;
        data->mCapacity = 0;

        while (*ptr++)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = i;
        while (i < data->mSize)
        {
            *(unsigned short*)(data->mData + j) = *src;
            i++;
            src++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    BasicStringInternal* msgData = data;
    mTicker->SetDisplayMessage(*(const BasicString<unsigned short, Detail::TempStringAllocator>*)&msgData);

    data = msgData;
    if (data)
    {
        if (--data->mRefCount == 0)
        {
            if (data)
            {
                if (data)
                {
                    delete[] data->mData;
                }
                if (data)
                {
                    nlFree(data);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x680 | 0x800F2648 | size: 0x12E8
 */
void CupTickerManager::CreateNewMessage()
{
}

/**
 * Offset/Address/Size: 0x654 | 0x800F261C | size: 0x2C
 */
void CupTickerManager::Update(float dt)
{
    if (mTicker != NULL)
    {
        mTicker->Update(dt);
    }
}

// /**
//  * Offset/Address/Size: 0x0 | 0x800F1FC8 | size: 0x654
//  */
// void CupTickerManager::BuildGoalTotalTickerMessage(BasicString<unsigned short, Detail::TempStringAllocator>&, bool)
// {
// }
