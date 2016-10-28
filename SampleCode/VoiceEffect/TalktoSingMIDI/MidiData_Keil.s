;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/


    AREA _MidiData, DATA, READONLY

    EXPORT  u32MidiDataBegin
    EXPORT  u32MidiDataEnd
    
u32MidiDataBegin
	INCBIN AudioRes\Output\AudioRes_AudioInfoMerge.ROM
u32MidiDataEnd        
    
    
    END