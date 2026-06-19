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
 * TODO: 97.25% match - stmw r25 vs r24 (7 vs 8 callee-saved regs).
 * Target keeps an extra callee-saved register live (constant 0 reused across
 * the loop), shifting all register numbers off by one. The connected branch
 * also reuses r0=1 (mr) for destPosIndex=1 vs our li. Residual diffs are all
 * register 'r' diffs from the off-by-one numbering.
 */
UpdateResult IChooseSide::UpdateForFE(float, eFEINPUT_PAD* pad)
{
    int allReady = 0;
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

            mInstanceTable[i + 12]->m_bVisible = (side == -1);
            mInstanceTable[i + 8]->m_bVisible = (side != -1);
        }
        else
        {
            int side;
            int destPosIndex;
            feVector3 localPos;
            TLInstance* readyIndicator;
            allReady = 0;

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
                    goto done_ready1;
                }

                if (mPlayerReady[1])
                {
                    allReady = 1;
                }
                else if (mPlayingSides[1] != -1)
                {
                    allReady = 0;
                    goto done_ready1;
                }

                if (mPlayerReady[2])
                {
                    allReady = 1;
                }
                else if (mPlayingSides[2] != -1)
                {
                    allReady = 0;
                    goto done_ready1;
                }

                if (mPlayerReady[3])
                {
                    allReady = 1;
                }
                else if (mPlayingSides[3] != -1)
                {
                    allReady = 0;
                }

            done_ready1:
                if ((u8)allReady == 1)
                    readyIndicator->m_bVisible = true;
                else
                    readyIndicator->m_bVisible = false;
            }
        }

        if (g_pFEInput->JustPressed((eFEINPUT_PAD)i, 0x200, false, NULL))
        {
            if (mPlayerReady[i])
            {
                mPlayerReady[i] = false;
                mInstanceTable[i + 4]->m_bVisible = false;
                TLInstance* ri2 = mInstanceTable[16];
                if (ri2 != NULL)
                {
                    allReady = 0;
                    if (mPlayerReady[0])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[0] != -1)
                    {
                        allReady = 0;
                        goto done_ready2;
                    }

                    if (mPlayerReady[1])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[1] != -1)
                    {
                        allReady = 0;
                        goto done_ready2;
                    }

                    if (mPlayerReady[2])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[2] != -1)
                    {
                        allReady = 0;
                        goto done_ready2;
                    }

                    if (mPlayerReady[3])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[3] != -1)
                    {
                        allReady = 0;
                    }

                done_ready2:
                    if ((u8)allReady == 1)
                        ri2->m_bVisible = true;
                    else
                        ri2->m_bVisible = false;
                }
                FEAudio::PlayAnimAudioEvent("sfx_back", false);
                FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
            }
            else
            {
                if (pad != NULL)
                {
                    *pad = (eFEINPUT_PAD)i;
                }
                return UPDATE_GO_BACK;
            }
        }

        if (g_pFEInput->JustPressed((eFEINPUT_PAD)i, 0x100, false, NULL))
        {
            if (mPlayerReady[i])
            {
                if (pad != NULL)
                {
                    *pad = (eFEINPUT_PAD)i;
                }
                FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
                return UPDATE_GO_FORWARD;
            }
            if (mPlayingSides[i] != -1)
            {
                mPlayerReady[i] = true;
                mInstanceTable[i + 4]->m_bVisible = true;
                TLInstance* ri3 = mInstanceTable[16];
                if (ri3 != NULL)
                {
                    allReady = 0;
                    if (mPlayerReady[0])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[0] != -1)
                    {
                        allReady = 0;
                        goto done_ready3;
                    }

                    if (mPlayerReady[1])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[1] != -1)
                    {
                        allReady = 0;
                        goto done_ready3;
                    }

                    if (mPlayerReady[2])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[2] != -1)
                    {
                        allReady = 0;
                        goto done_ready3;
                    }

                    if (mPlayerReady[3])
                    {
                        allReady = 1;
                    }
                    else if (mPlayingSides[3] != -1)
                    {
                        allReady = 0;
                    }

                done_ready3:
                    if ((u8)allReady == 1)
                        ri3->m_bVisible = true;
                    else
                        ri3->m_bVisible = false;
                }
                if (pad != NULL)
                {
                    *pad = (eFEINPUT_PAD)i;
                }
                FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
                if (AllPluggedInAreReady())
                {
                    return UPDATE_GO_FORWARD;
                }
            }
            else
            {
                if (pad != NULL)
                {
                    *pad = (eFEINPUT_PAD)i;
                }
                if (AllControllersAreCentred())
                {
                    return UPDATE_GO_FORWARD;
                }
                FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
                return UPDATE_OK;
            }
        }
    }
    return UPDATE_OK;
}

/**
 * Offset/Address/Size: 0xCDC | 0x800C4120 | size: 0x2F8
 * TODO: 97.76% match - outer loop register allocation is shifted, and
 * the connected destination-index branch still emits an extra immediate load.
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

            inst = mInstanceTable[i];
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
                if (AllPlayersReady())
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

            for (int j = 0, *playingSide = mPlayingSides; j < 4; j++, playingSide++)
            {
                GameInfoManager::Instance()->SetPlayingSide((unsigned short)j, (short)*playingSide);
            }

            return UPDATE_GO_BACK;
        }
    }

    return UPDATE_OK;
}

/**
 * Offset/Address/Size: 0x820 | 0x800C3C64 | size: 0x4BC
 * TODO: 99.08% match - callee-saved register rotation (r29/r31 this, r28/r30 advPtr)
 * across entire function; remaining diffs are register-only.
 */
void IChooseSide::CheckControllers(int disabledSide)
{
    int newSide;
    int destIndex;
    TLInstance* inst;

    for (int i = 0; i < 4; i++)
    {
        if (mPlayerReady[i])
        {
            continue;
        }

        if (g_pFEInput->JustPressed((eFEINPUT_PAD)i, 11, true, NULL))
        {
            int side = mPlayingSides[i];

            if (side == -1 && disabledSide == 0)
            {
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
                continue;
            }

            switch (side)
            {
            case 1:
                mPlayingSides[i] = -1;
                break;
            case -1:
                mPlayingSides[i] = 0;
                break;
            }

            if (side != 0)
            {
                FEAudio::PlayAnimAudioEvent("sfx_side_select_left", false);
            }
            else
            {
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            }

            newSide = mPlayingSides[i];
            if (newSide == 0)
            {
                destIndex = 0;
            }
            else
            {
                destIndex = 2;
                if (newSide == 1)
                {
                    destIndex = 1;
                }
            }

            inst = mInstanceTable[i];
            feVector3 startPos = inst->GetPosition();

            mTweenManager.clearTweensOnObj(inst);
            mTweenManager.startTween(mTweenManager.createTween(
                (float*)&startPos,
                &mControllerDestPos[destIndex],
                0.075f,
                0.0f,
                1,
                TweenFunctions::linear,
                inst,
                TweenSetPosCallback));

            mInstanceTable[i + 12]->m_bVisible = (newSide == -1);
            mInstanceTable[i + 8]->m_bVisible = (newSide != -1);

            TLInstance* readyIndicator = mInstanceTable[16];
            if (readyIndicator != NULL)
            {
                if (AllPlayersReady())
                    readyIndicator->m_bVisible = true;
                else
                    readyIndicator->m_bVisible = false;
            }
        }
        else if (g_pFEInput->JustPressed((eFEINPUT_PAD)i, 12, true, NULL))
        {
            int side = mPlayingSides[i];

            if (side == -1 && disabledSide == 1)
            {
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
                continue;
            }

            switch (side)
            {
            case 0:
                mPlayingSides[i] = -1;
                break;
            case 1:
                break;
            case -1:
                mPlayingSides[i] = 1;
                break;
            }

            if (side != 1)
            {
                FEAudio::PlayAnimAudioEvent("sfx_side_select_right", false);
            }
            else
            {
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            }

            newSide = mPlayingSides[i];
            if (newSide == 0)
            {
                destIndex = 0;
            }
            else
            {
                destIndex = 2;
                if (newSide == 1)
                {
                    destIndex = 1;
                }
            }

            inst = mInstanceTable[i];
            feVector3 startPos = inst->GetPosition();

            mTweenManager.clearTweensOnObj(inst);
            mTweenManager.startTween(mTweenManager.createTween(
                (float*)&startPos,
                &mControllerDestPos[destIndex],
                0.075f,
                0.0f,
                1,
                TweenFunctions::linear,
                inst,
                TweenSetPosCallback));

            mInstanceTable[i + 12]->m_bVisible = (newSide == -1);
            mInstanceTable[i + 8]->m_bVisible = (newSide != -1);

            TLInstance* readyIndicator = mInstanceTable[16];
            if (readyIndicator != NULL)
            {
                if (AllPlayersReady())
                    readyIndicator->m_bVisible = true;
                else
                    readyIndicator->m_bVisible = false;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x4C0 | 0x800C3904 | size: 0x360
 */
void IChooseSide::ResetAndPositionControllers(bool reset)
{
    for (int i = 0; i < 4; i++)
    {
        mPlayerReady[i] = false;
        mInstanceTable[i + 4]->m_bVisible = false;

        TLInstance* readyIndicator = mInstanceTable[16];
        if (readyIndicator != NULL)
        {
            if (AllPlayersReady())
                readyIndicator->m_bVisible = true;
            else
                readyIndicator->m_bVisible = false;
        }

        if (!g_pFEInput->IsConnected((eFEINPUT_PAD)i))
        {
            mPlayingSides[i] = -1;
            PositionController(i, false, true);

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
            mPlayingSides[i] = -1;
            PositionController(i, false, true);
        }
        else
        {
            mPlayingSides[i] = (short)GameInfoManager::Instance()->GetPlayingSide((unsigned short)i);
            PositionController(i, false, true);
        }
    }
}

/**
 * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0xE4
 */
void IChooseSide::SetReady(int controllerIdx, bool ready)
{
    mPlayerReady[controllerIdx] = ready;
    mInstanceTable[controllerIdx + 4]->m_bVisible = ready;

    TLInstance* readyIndicator = mInstanceTable[16];
    if (readyIndicator == NULL)
    {
        return;
    }

    if (AllPlayersReady())
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
            0.075f,
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
 */
bool IChooseSide::AllPlayersReady() const
{
    u8 playerReady = 0;

    for (int i = 0; i < 4; i++)
    {
        if (mPlayerReady[i])
        {
            playerReady = 1;
        }
        else if (mPlayingSides[i] != -1)
        {
            playerReady = 0;
            break;
        }
    }

    if (playerReady == 1)
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
