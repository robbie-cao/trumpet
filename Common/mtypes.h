#ifndef __MUA_TYPES_H__
#define __MUA_TYPES_H__

/* ------------------------------------------------------------------------------------------------
 *                                               Types
 * ------------------------------------------------------------------------------------------------
 */

#ifndef _HAL_TYPES_H
typedef signed   char   int8;     //!< Signed 8 bit integer
typedef unsigned char   uint8;    //!< Unsigned 8 bit integer
typedef signed   short  int16;    //!< Signed 16 bit integer
typedef unsigned short  uint16;   //!< Unsigned 16 bit integer
typedef signed   long   int32;    //!< Signed 32 bit integer
typedef unsigned long   uint32;   //!< Unsigned 32 bit integer
typedef unsigned char   bool;     //!< Boolean data type
#endif

#if /*!(defined __STDINT_H) &&*/ !(defined __stdint_h)
typedef signed   char   int8_t;   //!< Signed 8 bit integer
typedef unsigned char   uint8_t;  //!< Unsigned 8 bit integer
typedef signed   short  int16_t;  //!< Signed 16 bit integer
typedef unsigned short  uint16_t; //!< Unsigned 16 bit integer
typedef signed   long   int32_t;  //!< Signed 32 bit integer
typedef unsigned long   uint32_t; //!< Unsigned 32 bit integer
typedef unsigned char   bool_t;   //!< Boolean data type
#endif


/* ------------------------------------------------------------------------------------------------
 *                                        Standard Defines
 * ------------------------------------------------------------------------------------------------
 */
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL    0
#endif


/**************************************************************************************************
 */
#endif
