#ifndef _SAVELOADCALLBACKS_INL_
#define _SAVELOADCALLBACKS_INL_

/**
 * Offset/Address/Size: 0x3254 | 0x8018CBB0 | size: 0x308
 * TODO: 93.27% match - icon header-size temporaries use different registers
 * and the read-call arguments are prepared earlier.
 */
inline unsigned long LoadCallbacks::ReadDoneCB(unsigned long Slot, long Result, void* pUserData)
{
    MemCardFunctor functor;
    typedef unsigned long (LoadCallbacks::*MemberCB)(unsigned long, long, void*);
    MemberCB cb;
    MemCard::ICON_CONFIG localCfg;
    MCFILE_HEADER* header;
    unsigned long calculatedcrc;
    unsigned long filesize;

    localCfg.BannerFormat = 0;
    localCfg.IconCount = 0;
    localCfg.IconFormat = 0;
    localCfg.IconAnimType = 0;
    memset(localCfg.IconSpeeds, 0, 8);
    memset(&localCfg, 0, sizeof(MemCard::ICON_CONFIG));

    localCfg.IconCount = 1;
    localCfg.IconFormat = 2;
    localCfg.IconSpeeds[0] = 3;
    localCfg.BannerFormat = 2;

    bool configValid;
    if (memcmp(&localCfg, &((MemCard::MC_FILE*)pUserData)->IconCfg, 1) != 0)
    {
        configValid = 0;
    }
    else
    {
        header = (MCFILE_HEADER*)m_pReadBuffer;
        configValid = (header->Size == (u32)nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize());
    }

    if (!configValid)
    {
        if (pUserData != NULL)
        {
            g_MemCards[Slot]->CloseFile((MemCard::MC_FILE*)pUserData);
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(-1000);
        m_MustFreeBuffers = true;
        return -1;
    }

    header = (MCFILE_HEADER*)m_pReadBuffer;
    filesize = nlSingleton<GameInfoManager>::s_pInstance->GetMemoryCardDataSize();
    calculatedcrc = nlChecksum32((MCFILE_HEADER*)m_pReadBuffer + 1, filesize);
    if (header->CRC != calculatedcrc)
    {
        if (pUserData != NULL)
        {
            g_MemCards[Slot]->CloseFile((MemCard::MC_FILE*)pUserData);
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(-1000);
        m_MustFreeBuffers = true;
        return -1;
    }

    m_pLoadFile = (MemCard::MC_FILE*)pUserData;

    cb = &LoadCallbacks::LoadIconDataDoneCB;
    new (functor.m_FunctorMem) MemCardFunctor::MCMemberFunctor<LoadCallbacks>(this, cb, pUserData);

    MemCard::MC_FILE* pLoadFile = m_pLoadFile;
    MemCard** memCards = g_MemCards;
    u8 bannerFmt = pLoadFile->IconCfg.BannerFormat;
    s8 iconFmt = pLoadFile->IconCfg.IconFormat;
    u8 iconCount = pLoadFile->IconCfg.IconCount;

    u32 totalHeader = 0;
    totalHeader += ((bannerFmt == 1) ? 0x200 : 0);
    totalHeader += bannerFmt * 0xC00;
    totalHeader += ((iconFmt == 1) ? 0x200 : 0);
    totalHeader += iconCount * (iconFmt << 10);

    u32 headerSize = totalHeader + 0x40;
    pLoadFile->IconCfg.HeaderSize = headerSize;

    u32 alignedSize = (headerSize + 0x1FF) & ~0x1FF;

    long result = memCards[Slot]->InternalReadFile(pLoadFile, m_pIconReadBuffer, alignedSize, 0, functor);

    if (result != 0)
    {
        if (pUserData != NULL)
        {
            g_MemCards[Slot]->CloseFile((MemCard::MC_FILE*)pUserData);
        }
        MemCard* card = g_MemCards[Slot];
        card->m_State = IS_IDLE;
        card->m_CardState = CS_IDLE;
        CARDUnmount(card->m_Slot);
        InOperation = false;
        g_Callback(result);
        m_MustFreeBuffers = true;
        return -1;
    }

    return result;
}

#endif // _SAVELOADCALLBACKS_INL_
