#include "ISD9xx.h"
#include "Lib\libSPIFlash.h"
#include "string.h"

#include "Conf.h"
#include "Audio.h"
#include "SpiFlash.h"
#include "SpeakerDrv.h"
#include "Reg.h"
#include "log.h"

static Audio_t audio;
__align(4) uint32_t SpiDataBuffer[20];
static int16_t *speakerBufferAdd;

static void Audio_LoadSoundData(ChSoundInfo_t *soundInfo)
{
	//load and decode sound data
    sflash_read(&g_SPIFLASH,
	            (unsigned long)soundInfo->audioDataAddr,
				(unsigned long*)SpiDataBuffer,
				(unsigned long)(COMPBUFSIZE<<1));

    LibS7Decode(&soundInfo->sEnDeCtlCh,
	            &soundInfo->sS7Dec_CtxCh,
				(signed short *)SpiDataBuffer,
				(signed short *)soundInfo->audioDataBuffer);

    soundInfo->audioDataAddr    = soundInfo->audioDataAddr + (COMPBUFSIZE << 1);
    soundInfo->audioSampleCount = soundInfo->audioSampleCount + AUDIOBUFFERSIZE;

	if(soundInfo->audioSampleCount >= soundInfo->totalG722Size)
    {
    	soundInfo->playingInfo.status.bits.state = CHANNEL_STATUS_COMPLETE;
    }else{
		if(soundInfo->playingInfo.status.bits.stopping == 1)
		{
			int i = 0;
			for(i = 0;i < AUDIOBUFFERSIZE;i++)
			{
			 	soundInfo->byte_cnt ++;

			 	if(soundInfo->byte_cnt % 20 == 0)
			 	{
				    if(soundInfo->factor > 1)
			 		{
			 			soundInfo->factor = soundInfo->factor - 1;
			 		}else
					{
					  soundInfo->factor = 0	;
					}
			 	}

	            if(soundInfo->factor > 15)
				{
					soundInfo->audioDataBuffer[i] = (soundInfo->audioDataBuffer[i] * soundInfo->factor) >> 7;
				}else if(soundInfo->factor > 10)
				{
					 soundInfo->audioDataBuffer[i] = soundInfo->audioDataBuffer[i]  >> 8;
				}else if(soundInfo->factor > 2)
				{
					 soundInfo->audioDataBuffer[i] = soundInfo->audioDataBuffer[i]  >> 12;
				}else
				{
					 soundInfo->audioDataBuffer[i] = soundInfo->audioDataBuffer[i]  >> 14;
				}
			}

			if(soundInfo->factor == 0)
			{
			   soundInfo->playingInfo.status.bits.state = CHANNEL_STATUS_COMPLETE;
			}
		}
	}
}

static int32_t Audio_VolumeControl(int32_t pi16Src)
{
    pi16Src = (pi16Src * (int32_t)audio.volume) >> 4;

    return pi16Src;
}

static void Audio_Mix(void)
{
	int32_t max = 0;
	int i = 0;

	speakerBufferAdd = (int16_t *)audio.speakerBufferAddr;

    for(i = 0;i < AUDIOBUFFERSIZE;i++)
    {
        max = audio.CH[0].on_PlaySoundInfo.audioDataBuffer[i] +
      		  audio.CH[1].on_PlaySoundInfo.audioDataBuffer[i] +
      		  audio.CH[2].on_PlaySoundInfo.audioDataBuffer[i];

    	//do vol control
    	max =  Audio_VolumeControl(max);

		//*(speakerBufferAdd+i) = (int16_t)(max / 4) ;
		if(max > 32760 )
		{
			max = 32760;
		}else if(max < -32760)
		{
			max = -32760;
		}

		*(speakerBufferAdd+i) = (int16_t)max;

		audio.CH[0].on_PlaySoundInfo.audioDataBuffer[i] = 0;
		audio.CH[1].on_PlaySoundInfo.audioDataBuffer[i] = 0;
		audio.CH[2].on_PlaySoundInfo.audioDataBuffer[i] = 0;
    }


}

static void Audio_LoadSoundInfo(ChSoundInfo_t *soundInfo)
{
    uint32_t  Temp0 = 0, Temp1 = 0, addr = 0;

	LibS7Init(&soundInfo->sEnDeCtlCh, BIT_RATE, BANDWIDTH);
    LibS7DeBufReset(soundInfo->sEnDeCtlCh.frame_size,&soundInfo->sS7Dec_CtxCh);

    addr = S7DATA_BASE_ADDR_ON_SPI + 8*(soundInfo->playingInfo.vpIdx+1);

    sflash_read(&g_SPIFLASH,
	            (unsigned long)addr,
				(unsigned long*)&Temp0,
				(unsigned long)4);

	addr = S7DATA_BASE_ADDR_ON_SPI + 8*(soundInfo->playingInfo.vpIdx+2);

    sflash_read(&g_SPIFLASH,
	            (unsigned long)addr,
				(unsigned long*)&Temp1,
				(unsigned long)4);

    soundInfo->startAddr = Temp0 + S7DATA_BASE_ADDR_ON_SPI;
    soundInfo->audioDataAddr = soundInfo->startAddr;
    soundInfo->totalG722Size = (Temp1 - Temp0)*4;
}

static void Audio_LoadNextSound(CH_t *chInfo)
{
	if(chInfo->next_PlaySoundInfo.status.bits.state == CHANNEL_STATUS_PLAY)
	{
		memset(&chInfo->on_PlaySoundInfo,0,sizeof(ChSoundInfo_t));
		memcpy(&chInfo->on_PlaySoundInfo.playingInfo,&chInfo->next_PlaySoundInfo,sizeof(RegChInfo_t));

		if(chInfo->next_PlaySoundInfo.status.bits.stopping == 1)
		{
			chInfo->on_PlaySoundInfo.factor = FACTOR_BASE;
			chInfo->on_PlaySoundInfo.zeroCrossFlag = 0;
		}

		memset(&chInfo->next_PlaySoundInfo,0,sizeof(RegChInfo_t));
        //load sound size info from flash
        Audio_LoadSoundInfo(&chInfo->on_PlaySoundInfo);
	}
}

static void Audio_StatusUpdate(CH_t *chInfo)
{
	if(chInfo->on_PlaySoundInfo.playingInfo.status.bits.state == CHANNEL_STATUS_PLAY)
	{
		return;
	}else{
		if((chInfo->on_PlaySoundInfo.playingInfo.status.bits.repeat == 1) &&
		   (chInfo->on_PlaySoundInfo.playingInfo.status.bits.state == CHANNEL_STATUS_COMPLETE))
		{
		    chInfo->on_PlaySoundInfo.playingInfo.status.bits.state = CHANNEL_STATUS_PLAY;
			chInfo->on_PlaySoundInfo.audioDataAddr  = chInfo->on_PlaySoundInfo.startAddr;
			chInfo->on_PlaySoundInfo.audioSampleCount = 0;
		}else
		{
			Audio_LoadNextSound(chInfo);
		}
	}
}

static void Audio_StopOnPlayingSound(ChSoundInfo_t *soundInfo)
{
    if(soundInfo->playingInfo.status.bits.stopping == 1) //on stopping
	{
		return;
	}

    if(audio.playingStatus == 0x07)
	{
		soundInfo->playingInfo.status.bits.state = CHANNEL_STATUS_COMPLETE;
	}else{
		soundInfo->playingInfo.status.bits.stopping = 1;
	}

	soundInfo->playingInfo.status.bits.repeat   = 0;
	soundInfo->factor = FACTOR_BASE;
}

void Audio_Process(void)   //Audio main process
{
	uint8_t i = 0;
    /* check system playing status */

	if(audio.playingStatus == 0)
	{
		//close speaker if opened
		if(audio.speakerStatus == SPEAKER_ON)
		{
			Speaker_Close();
			audio.speakerStatus = SPEAKER_OFF;
		}
		PM_DeepSleep();
	}else
	{
	    PM_Wakeup();
		if(audio.speakerStatus == SPEAKER_OFF)
	    {
	    	//Speaker_Open();
			audio.speakerStatus = SPEAKER_ON;
	    }

        /* get empty buffer */
		audio.speakerBufferAddr = Speaker_GetBufferStatus();

		if(audio.speakerBufferAddr != 0)
		{
			for (i = 0; i < CHANNEL_COUNT; ++i)
			{
				/* check chanel state */
				if(audio.CH[i].on_PlaySoundInfo.playingInfo.status.bits.state != CHANNEL_STATUS_PLAY)
				{
					continue;
				}

				/* load channel sound date */
		        Audio_LoadSoundData(&audio.CH[i].on_PlaySoundInfo);
		  	}
			/* sound mix */
		    Audio_Mix();
		}
	    /* start speaker if  closed */
	}

    for(i = 0; i < CHANNEL_COUNT; ++i)
    {
    	Audio_StatusUpdate(&audio.CH[i]);
    }

	audio.playingStatus = Audio_GetPlayStatus();
}

void Audio_PlayCh(uint8_t ch,RegChInfo_t chInfo)
{
	memcpy(&audio.CH[ch].next_PlaySoundInfo,&chInfo,sizeof(RegChInfo_t));

	if(audio.CH[ch].on_PlaySoundInfo.playingInfo.status.bits.state == CHANNEL_STATUS_PLAY)
	{
		Audio_StopOnPlayingSound(&audio.CH[ch].on_PlaySoundInfo);
	}
}

void Audio_StopCh(uint8_t ch) //set stop flag
{
	if(audio.CH[ch].on_PlaySoundInfo.playingInfo.status.bits.state == CHANNEL_STATUS_PLAY)
	{
		Audio_StopOnPlayingSound(&audio.CH[ch].on_PlaySoundInfo);
	}

    if(audio.CH[ch].next_PlaySoundInfo.status.bits.state == CHANNEL_STATUS_PLAY)
	{
		audio.CH[ch].next_PlaySoundInfo.status.bits.stopping = 1;
	}
}

void Audio_StopChImmediately(uint8_t ch) //stop all sound  no wait
{
	 audio.CH[ch].on_PlaySoundInfo.playingInfo.status.bits.state = 	CHANNEL_STATUS_IDLE;
	 audio.CH[ch].next_PlaySoundInfo.status.bits.state = CHANNEL_STATUS_IDLE;
}

uint8_t Audio_GetPlayStatus(void) //indify every channel playing status
{
	return ((audio.CH[2].on_PlaySoundInfo.playingInfo.status.bits.state == CHANNEL_STATUS_PLAY) << 2) |
	       ((audio.CH[1].on_PlaySoundInfo.playingInfo.status.bits.state == CHANNEL_STATUS_PLAY) << 1) |
	       ((audio.CH[0].on_PlaySoundInfo.playingInfo.status.bits.state == CHANNEL_STATUS_PLAY) << 0) ;
}

void Audio_PauseCh(uint8_t ch)
{

}

void Audio_PauseResume(uint8_t ch)
{

}

void Audio_ReplayCh(uint8_t ch)
{

}

uint8_t Audio_VolumeUp(void) //return Volume value
{
	if(audio.volume < VOLUME_MAX)
	{
		audio.volume ++;
	}

    return audio.volume;
}

uint8_t Audio_VolumeDown(void) //return Volume value
{
	if(audio.volume > VOLUME_MIN)
	{
		audio.volume --;
	}
    return audio.volume;
}

uint8_t Audio_SetVolume(uint8_t vol) //return Volume value
{
	audio.volume = vol;

	return audio.volume;
}

uint8_t Audio_GetVolume(void) //return Volume value
{
	return audio.volume;
}

void Audio_Init(void)
{
	//do mem mapping here
	audio.volume = VOLUME_DEFAULT;
}
