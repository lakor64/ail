/*
* IBM Audio Interface base defines
*
* Version 1.00 of 06-Sept-24: Initial version
*
*/

#ifndef _AILBASE_H_
#define _AILBASE_H_

#include <stdint.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#if defined(_MSC_VER)

#ifdef _WIN64
#define CDECL
#else
#define CDECL __cdecl
#endif

#ifndef FAR
#define FAR
#endif

#elif defined(__WATCOMC__)
#define FAR __far
#define CDECL __cdecl
#else
#error "Compiler not supported"
#endif

#define CURRENT_REV 215 /* current API revision level */

#define SEQ_STOPPED 0       /* equates for AIL_sequence_status()         */
#define SEQ_PLAYING 1
#define SEQ_DONE 2

#define DAC_STOPPED 0       /* equates for AIL_sound_buffer_status()     */
#define DAC_PAUSED 1        /*             AIL_VOC_playback_status()     */
#define DAC_PLAYING 2
#define DAC_DONE 3

#define MIDI_DRVR 1          /* Roland MT-32 MIDI synthesizer or compatible, deprecated in AIL2.xx */
#define DSP_DRVR 2           /* Digital Sound Processor used in Sound Blaster (VOC processor) */
#define XMIDI_DRVR 3         /* Roland MT-32 XMIDI synthesizer or compatible, use this and not MIDI_DRVR */
#define WIN_XMIDI_DRVR 4     /* XMIDI synthesizer for AIL/16 or Windows */
#define WIN_DSP_DRVR 5       /* DSP synthesizer for AIL/16 or Windows */
#define XMIDI_AND_DSP_DRVR 6 /* XMIDI and DSP synthesizer (AIL fork!!) */

typedef int32_t HTIMER;         /* handle to timer                           */
typedef int32_t HDRIVER;        /* handle to driver                          */
typedef int32_t HSEQUENCE;      /* handle to XMIDI sequence                  */
typedef uint8_t HBOOL;

#define DRIVER_NULL ((HDRIVER)-1)
#define TIMER_NULL ((HTIMER)-1)
#define SEQUENCE_NULL ((HSEQUENCE)-1)

typedef struct sound_buff_s
{
    uint32_t pack_type;
    uint32_t sample_rate;
    void FAR* sel_data;
    uint32_t seg_data;
    uint32_t len;
}
sound_buff;

#endif
