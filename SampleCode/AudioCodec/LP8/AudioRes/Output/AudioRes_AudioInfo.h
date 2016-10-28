#ifndef _AUDIOINFO_H_
#define _AUDIOINFO_H_

enum Audio_ID_Table
{
	SOUND_CANON_16K=0,	//LP8
	SOUND_MODLITWADZIEWICY_16K=1,	//LP8
	SPEECH_JAMES_12K=2,	//LP8
	SPEECH_LEFTRIGHT_12K=3,	//LP8
};
#define AUDIOSYN_SOUND_MAX_ID	3

// Define AudioRes_AudioInfoMerge.ROM size (without MIDI WavTable)
#define AUDIOINFO_ROM_NO_WTB_SIZE	944632
// Define MIDI WavTable size
#define MIDISYN_WTB_SIZE	0
// Define AudioRes_AudioInfoMerge.ROM size (with MIDI WavTable)
#define AUDIOINFO_ROM_SIZE	944632

#endif

