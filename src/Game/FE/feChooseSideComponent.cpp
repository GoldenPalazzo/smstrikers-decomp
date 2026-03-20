#include "Game/FE/feChooseSideComponent.h"
#include "Game/FE/feTweener.h"
#include "Game/GameInfo.h"

#include "Game/FE/feTweenFuncs.h"

/**
 * Offset/Address/Size: 0x16B0 | 0x800C4AF4 | size: 0xCC
 */
IChooseSide::IChooseSide()
{
    mContext = CONTEXT_FE;

    for (int i = 0; i < 17; i++)
    {
        mInstanceTable[i] = NULL;
    }

    for (int i = 0; i < 4; i++)
    {
        mPlayingSides[i] = -1;
        mPlayerReady[i] = false;
    }
}

/**
 * Offset/Address/Size: 0x1658 | 0x800C4A9C | size: 0x58
 */
IChooseSide::~IChooseSide()
{
}

/**
 * Offset/Address/Size: 0x15D4 | 0x800C4A18 | size: 0x84
 */
UpdateResult IChooseSide::Update(float dt, eFEINPUT_PAD* pad, int param)
{
    UpdateResult result;
    if (mContext == CONTEXT_FE)
    {
        result = UpdateForFE(dt, pad);
    }
    else
    {
        result = UpdateForPause(dt, pad);
    }
    CheckControllers(param);
    mTweenManager.Update(dt);
    return result;
}

/**
 * Offset/Address/Size: 0xFD4 | 0x800C4418 | size: 0x600
 */
UpdateResult IChooseSide::UpdateForFE(float, eFEINPUT_PAD*)
{
    FORCE_DONT_INLINE;
    return UPDATE_OK;
}

/**
 * Offset/Address/Size: 0xCDC | 0x800C4120 | size: 0x2F8
 * TODO: 95.97% match - loop index/base pointer callee-save allocation (r29/r30/r31)
 * and residual branch/register drift in disconnected ready-indicator flow.
 */
UpdateResult IChooseSide::UpdateForPause(float, eFEINPUT_PAD* pad)
{
    for (int i = 0; i < 4; i++)
    {
        TLInstance* inst = mInstanceTable[i];
        if (g_pFEInput->IsConnected((eFEINPUT_PAD)i))
        {
            int side;
            int destPosIndex;
            feVector3 localPos;

            inst->m_bVisible = true;
            side = mPlayingSides[i];

            if (side == 0)
            {
                destPosIndex = 0;
            }
            else
            {
                int temp = 2;
                if (side == 1)
                {
                    temp = 1;
                }
                destPosIndex = temp;
            }

            localPos = inst->GetPosition();
            mTweenManager.clearTweensOnObj(inst);
            mInstanceTable[i]->SetAssetPosition(mControllerDestPos[destPosIndex], localPos.e[1], localPos.e[2]);

            mInstanceTable[i + 12]->m_bVisible = (side == -1);
            mInstanceTable[i + 8]->m_bVisible = (side != -1);
        }
        else
        {
            int side;
            int destPosIndex;
            feVector3 localPos;
            TLInstance* readyIndicator;
            int allReady = 0;

            inst->m_bVisible = false;
            mPlayingSides[i] = -1;

            if (mInstanceTable[i + 8] != NULL)
            {
                mInstanceTable[i + 8]->m_bVisible = true;
            }

            if (mInstanceTable[i + 12] != NULL)
            {
                mInstanceTable[i + 12]->m_bVisible = false;
            }

            side = mPlayingSides[i];
            if (side == 0)
            {
                destPosIndex = 0;
            }
            else
            {
                destPosIndex = 2;
                if (side == 1)
                {
                    destPosIndex = 1;
                }
            }

            inst = mInstanceTable[i];
            localPos = inst->GetPosition();
            mTweenManager.clearTweensOnObj(inst);
            mInstanceTable[i]->SetAssetPosition(mControllerDestPos[destPosIndex], localPos.e[1], localPos.e[2]);

            mPlayerReady[i] = (u8)allReady;
            mInstanceTable[i + 4]->m_bVisible = (u8)allReady;

            readyIndicator = mInstanceTable[16];
            if (readyIndicator != NULL)
            {
                if (mPlayerReady[0])
                {
                    allReady = 1;
                }
                else if (mPlayingSides[0] != -1)
                {
                    allReady = 0;
                    goto done_ready;
                }

                if (mPlayerReady[1])
                {
                    allReady = 1;
                }
                else if (mPlayingSides[1] != -1)
                {
                    allReady = 0;
                    goto done_ready;
                }

                if (mPlayerReady[2])
                {
                    allReady = 1;
                }
                else if (mPlayingSides[2] != -1)
                {
                    allReady = 0;
                    goto done_ready;
                }

                if (mPlayerReady[3])
                {
                    allReady = 1;
                }
                else if (mPlayingSides[3] != -1)
                {
                    allReady = 0;
                }

            done_ready:
                if ((u8)allReady == 1)
                    readyIndicator->m_bVisible = true;
                else
                    readyIndicator->m_bVisible = false;
            }
        }

        if (g_pFEInput->JustPressed((eFEINPUT_PAD)i, 0x200, false, NULL))
        {
            FEAudio::PlayAnimAudioEvent("sfx_back", false);
            if (pad != NULL)
            {
                *pad = (eFEINPUT_PAD)i;
            }

            for (int j = 0; j < 4; j++)
            {
                GameInfoManager::Instance()->SetPlayingSide((unsigned short)j, (short)mPlayingSides[j]);
            }

            return UPDATE_GO_BACK;
        }
    }

    return UPDATE_OK;
}

/**
 * Offset/Address/Size: 0x820 | 0x800C3C64 | size: 0x4BC
 */
void IChooseSide::CheckControllers(int)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x4C0 | 0x800C3904 | size: 0x360
 * TODO: 97.05% match - extra loop base pointer register (r31) for instance-table accesses;
 * -inline deferred merges mPlayingSides/mInstanceTable into single r30, decomp.me -inline auto does not.
 * Also extra `li r4, 0` for allReady loop init (same -inline deferred issue as SetReady).
 */
void IChooseSide::ResetAndPositionControllers(bool reset)
{
    int* playingSide = mPlayingSides;

    for (int i = 0; i < 4; i++, playingSide++)
    {
        int side;

        mPlayerReady[i] = false;
        mInstanceTable[i + 4]->m_bVisible = false;

        TLInstance* readyIndicator = mInstanceTable[16];
        if (readyIndicator != NULL)
        {
            int allReady = 0;
            for (int j = 0; j < 4; j++)
            {
                if (mPlayerReady[j])
                    allReady = 1;
                else if (mPlayingSides[j] != -1)
                {
                    allReady = 0;
                    break;
                }
            }

            if ((u8)allReady == 1)
                readyIndicator->m_bVisible = true;
            else
                readyIndicator->m_bVisible = false;
        }

        if (!g_pFEInput->IsConnected((eFEINPUT_PAD)i))
        {
            int destPosIndex;

            *playingSide = -1;
            side = *playingSide;
            if (side == 0)
            {
                destPosIndex = 0;
            }
            else
            {
                destPosIndex = 2;
                if (side == 1)
                {
                    destPosIndex = 1;
                }
            }

            TLInstance* inst = mInstanceTable[i];
            feVector3 localPos = inst->GetPosition();

            mTweenManager.clearTweensOnObj(inst);
            mInstanceTable[i]->SetAssetPosition(mControllerDestPos[destPosIndex], localPos.e[1], localPos.e[2]);

            mInstanceTable[i + 12]->m_bVisible = (side == -1);
            mInstanceTable[i + 8]->m_bVisible = (side != -1);

            if (mInstanceTable[i + 12] != NULL)
            {
                mInstanceTable[i + 12]->m_bVisible = false;
            }

            if (mInstanceTable[i] != NULL)
            {
                mInstanceTable[i]->m_bVisible = false;
            }
        }
        else if (reset)
        {
            int destPosIndex;

            *playingSide = -1;
            side = *playingSide;
            if (side == 0)
            {
                destPosIndex = 0;
            }
            else
            {
                destPosIndex = 2;
                if (side == 1)
                {
                    destPosIndex = 1;
                }
            }

            TLInstance* inst = mInstanceTable[i];
            feVector3 localPos = inst->GetPosition();

            mTweenManager.clearTweensOnObj(inst);
            mInstanceTable[i]->SetAssetPosition(mControllerDestPos[destPosIndex], localPos.e[1], localPos.e[2]);

            mInstanceTable[i + 12]->m_bVisible = (side == -1);
            mInstanceTable[i + 8]->m_bVisible = (side != -1);
        }
        else
        {
            int destPosIndex;

            *playingSide = (short)GameInfoManager::Instance()->GetPlayingSide((unsigned short)i);
            side = *playingSide;
            if (side == 0)
            {
                destPosIndex = 0;
            }
            else
            {
                destPosIndex = 2;
                if (side == 1)
                {
                    destPosIndex = 1;
                }
            }

            TLInstance* inst = mInstanceTable[i];
            feVector3 localPos = inst->GetPosition();

            mTweenManager.clearTweensOnObj(inst);
            mInstanceTable[i]->SetAssetPosition(mControllerDestPos[destPosIndex], localPos.e[1], localPos.e[2]);

            mInstanceTable[i + 12]->m_bVisible = (side == -1);
            mInstanceTable[i + 8]->m_bVisible = (side != -1);
        }
    }
}

/**
 * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0xE4
 * TODO: 98.25% match - extra `li r4, 0` dead store on first loop break path;
 * -inline deferred eliminates it (allReady already false from init), decomp.me -inline auto does not
 */
void IChooseSide::SetReady(int controllerIdx, bool ready)
{
    mPlayerReady[controllerIdx] = ready;
    mInstanceTable[controllerIdx + 4]->m_bVisible = ready;

    bool allReady;
    TLInstance* readyIndicator = mInstanceTable[16];
    if (readyIndicator == NULL)
    {
        return;
    }

    allReady = false;
    for (int i = 0; i < 4; i++)
    {
        if (mPlayerReady[i])
            allReady = true;
        else if (mPlayingSides[i] != -1)
        {
            allReady = false;
            break;
        }
    }

    if ((u8)allReady == 1)
        readyIndicator->m_bVisible = true;
    else
        readyIndicator->m_bVisible = false;
}

/**
 * Offset/Address/Size: 0x2A4 | 0x800C36E8 | size: 0x138
 * TODO: 99.7% match - r4/r5 swap in setvisibilities block when loading +0x44/+0x34
 */
void IChooseSide::PositionController(int padindex, bool usetween, bool setvisibilities)
{
    int side = mPlayingSides[padindex];
    int destPosIndex;

    if (side == 0)
    {
        destPosIndex = 0;
    }
    else
    {
        int temp = 2;
        if (side == 1)
        {
            temp = 1;
        }
        destPosIndex = temp;
    }

    TLInstance* inst = mInstanceTable[padindex];
    feVector3 localPos = inst->GetPosition();

    mTweenManager.clearTweensOnObj(inst);

    if (usetween)
    {
        FETweener* tweener = mTweenManager.createTween(
            localPos.e,
            &mControllerDestPos[destPosIndex],
            0.2f,
            0.0f,
            1,
            TweenFunctions::linear,
            inst,
            TweenSetPosCallback);
        mTweenManager.startTween(tweener);
    }
    else
    {
        mInstanceTable[padindex]->SetAssetPosition(mControllerDestPos[destPosIndex], localPos.e[1], localPos.e[2]);
    }

    if (setvisibilities)
    {
        mInstanceTable[padindex + 12]->m_bVisible = (side == -1);
        mInstanceTable[padindex + 8]->m_bVisible = (side != -1);
    }
}

/**
 * Offset/Address/Size: 0x1DC | 0x800C3620 | size: 0xC8
 * TODO: 92.6% match - MWCC strength-reduces ready pointer (r4=&mPlayerReady[1]) instead of
 * side pointer (r4=&mPlayingSides[1]). Exhaustively tested: pointer decl order, increment order,
 * single pointer, pure indexing, while/do-while, pragmas, types. Same MWCC quirk as AllControllersAreCentred.
 */
bool IChooseSide::AllPlayersReady() const
{
    const bool* ready = mPlayerReady;
    const int* side = mPlayingSides;
    u8 allReady = 0;

    for (int i = 0; i < 4; i++, ready++, side++)
    {
        if (*ready)
        {
            allReady = 1;
        }
        else if (*side != -1)
        {
            allReady = 0;
            break;
        }
    }

    if (allReady == 1)
    {
        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x16C | 0x800C35B0 | size: 0x70
 */
bool IChooseSide::AllPluggedInAreReady() const
{
    for (int i = 0; i < 4; i++)
    {
        if (g_pFEInput->IsConnected((eFEINPUT_PAD)i))
        {
            if (!mPlayerReady[i])
                return false;
        }
    }
    return true;
}

/**
 * Offset/Address/Size: 0x114 | 0x800C3558 | size: 0x58
 */
bool IChooseSide::AtLeastOnePlayerReady() const
{
    for (int i = 0; i < 4; i++)
    {
        if (mPlayerReady[i])
            return true;
    }
    return false;
}

/**
 * Offset/Address/Size: 0xBC | 0x800C3500 | size: 0x58
 * TODO: lwzu instruction mismatch - original uses pointer update pattern
 */
bool IChooseSide::AllControllersAreCentred() const
{
    for (int i = 0; i < 4; i++)
    {
        if (mPlayingSides[i] != -1)
            return false;
    }
    return true;
}

/**
 * Offset/Address/Size: 0x58 | 0x800C349C | size: 0x64
 */
void IChooseSide::TweenSetPosCallback(void* obj, const float* value)
{
    TLInstance* inst = (TLInstance*)obj;
    feVector3 pos = inst->GetPosition();
    inst->SetAssetPosition(*(const float*)value, pos.f.y, pos.f.z);
}

/**
 * Offset/Address/Size: 0x0 | 0x800C3444 | size: 0x58
 */
void IChooseSide::SaveChanges()
{
    for (int i = 0; i < 4; i++)
    {
        GameInfoManager::Instance()->SetPlayingSide(i, mPlayingSides[i]);
    }
}
