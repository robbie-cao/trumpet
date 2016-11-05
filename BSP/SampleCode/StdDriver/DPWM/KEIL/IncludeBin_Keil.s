;/*---------------------------------------------------------------------------------------------------------*/
;/*                                                                                                         */
;/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
;/*                                                                                                         */
;/*---------------------------------------------------------------------------------------------------------*/


	AREA _audio, DATA, READONLY

	EXPORT  u32audioBegin
	EXPORT  u32audioEnd
    
u32audioBegin
    INCBIN ..\Audio.bin
u32audioEnd        
    
    
    END