/*
* IBM Audio Interface Driver API
*
* Version 1.00 of 06-Sept-24: Initial version
*
*/

#ifndef _AIL32DRIVER_H_
#define _AIL32DRIVER_H_

#include "AILBASE.H"

#define SEQ_STOPPED 0
#define SEQ_PLAYING 1
#define SEQ_DONE 2

#define DAC_STOPPED 0
#define DAC_PAUSED 1
#define DAC_PLAYING 2
#define DAC_DONE 3

//
//MIDI and XMIDI controllers recognized by AIL drivers
//

#define START_MSB_1     32                  //System Exclusive XMIDI controllers
#define START_KSB_1     33                  //(Roland DT1 format)
#define START_LSB_1     34
#define DATA_BYTE_1     35
#define FINAL_BYTE_1    36
#define START_MSB_2     37       
#define START_KSB_2     38
#define START_LSB_2     39
#define DATA_BYTE_2     40
#define FINAL_BYTE_2    41
#define START_MSB_3     42       
#define START_KSB_3     43
#define START_LSB_3     44
#define DATA_BYTE_3     45
#define FINAL_BYTE_3    46

#define RHYTHM_KEY_TIMB 58                  //Roland-exclusive XMIDI controllers
#define PATCH_REVERB    59                  
#define PATCH_BENDER    60
#define REVERB_MODE     61
#define REVERB_TIME     62
#define REVERB_LEVEL    63

#define CHAN_LOCK       110                 //General XMIDI controllers
#define CHAN_PROTECT    111
#define VOICE_PROTECT   112
#define TIMBRE_PROTECT  113
#define PATCH_BANK_SEL  114
#define INDIRECT_C_PFX  115
#define FOR_LOOP        116
#define NEXT_LOOP       117
#define CLEAR_BEAT_BAR  118
#define CALLBACK_TRIG   119
#define SEQ_INDEX       120
 
#define MODULATION      1                   //MIDI controllers and Channel Mode
#define PART_VOLUME     7                   //messages recognized by AIL drivers
#define PANPOT          10
#define EXPRESSION      11
#define SUSTAIN         64
#define RESET_ALL_CTRLS 121
#define ALL_NOTES_OFF   123

// Dynamically linked driver procedure numbers

#define AIL_DESC_DRVR           100
#define AIL_DET_DEV             101
#define AIL_INIT_DRVR           102
#define AIL_SERVE_DRVR          103
#define AIL_SHUTDOWN_DRVR       104

#define AIL_INDEX_VOC_BLK       120
#define AIL_REG_SND_BUFF        121
#define AIL_SND_BUFF_STAT       122
#define AIL_P_VOC_FILE          123
#define AIL_VOC_PB_STAT         124
#define AIL_START_D_PB          125
#define AIL_STOP_D_PB           126
#define AIL_PAUSE_D_PB          127
#define AIL_RESUME_D_PB         128
#define AIL_SET_D_PB_VOL        129
#define AIL_D_PB_VOL            130
#define AIL_SET_D_PB_PAN        131
#define AIL_D_PB_PAN            132
#define AIL_F_VOC_FILE          133
#define AIL_F_SND_BUFF          134
            
#define AIL_STATE_TAB_SIZE      150
#define AIL_REG_SEQ             151
#define AIL_REL_SEQ_HND         152
#define AIL_T_CACHE_SIZE        153
#define AIL_DEFINE_T_CACHE      154
#define AIL_T_REQ               155
#define AIL_INSTALL_T           156
#define AIL_PROTECT_T           157
#define AIL_UNPROTECT_T         158
#define AIL_T_STATUS            159

#define AIL_START_SEQ           170
#define AIL_STOP_SEQ            171
#define AIL_RESUME_SEQ          173
#define AIL_SEQ_STAT            174
#define AIL_REL_VOL             175
#define AIL_REL_TEMPO           176
#define AIL_SET_REL_VOL         177
#define AIL_SET_REL_TEMPO       178
#define AIL_BEAT_CNT            179
#define AIL_BAR_CNT             180
#define AIL_BRA_INDEX           181

#define AIL_CON_VAL             182
#define AIL_SET_CON_VAL         183
#define AIL_CHAN_NOTES          185
#define AIL_SEND_CV_MSG         186
#define AIL_SEND_SYSEX_MSG      187
#define AIL_WRITE_DISP          188
#define AIL_INSTALL_CB          189
#define AIL_CANCEL_CB           190

#define AIL_LOCK_CHAN           191
#define AIL_MAP_SEQ_CHAN        192
#define AIL_RELEASE_CHAN        193
#define AIL_TRUE_SEQ_CHAN       194

typedef struct ail_driver_proc_table_s
{
    uint32_t opcode;
    void FAR* function;
} ail_driver_proc_table;

#define AIL_END_DRIVER_PROC_TABLE { -1, (void FAR*)0x00 }

#ifndef __DOS__
#define AIL_ENTRYPOINT AIL_Driver
#define MACRO_TO_STR(name) #name
#define MACRO_TO_STR2(name) MACRO_TO_STR(name)
#define AIL_ENTRYPOINT_STR MACRO_TO_STR2(AIL_ENTRYPOINT)
typedef void* (CDECL* AilEntrypoint)(void);
#endif

typedef struct ail_drvr_desc
{
    uint32_t min_API_version;
    uint32_t drvr_type;
    char data_suffix[4];
    void * dev_name_table;
    uint32_t def_IO;
    uint32_t def_IRQ;
    uint32_t def_DMA;
    uint32_t def_DRQ;
    uint32_t svc_rate;
    uint32_t dsp_size;
} ail_drvr_desc;

// Driver callbacks

typedef uint32_t(CDECL* AilIntRateProc)(void);

typedef ail_drvr_desc* (CDECL* FUN_AIL_DESC_DRVR)(HDRIVER handle, AilIntRateProc ptr);
typedef HBOOL(CDECL* FUN_AIL_DET_DEV)(HDRIVER handle, uint32_t io_addr, uint32_t irq, uint32_t dma, uint32_t drq);
typedef HBOOL(CDECL* FUN_AIL_INIT_DRVR)(HDRIVER handle, uint32_t io_addr, uint32_t irq, uint32_t dma, uint32_t drq);
typedef void(CDECL* FUN_AIL_SERVE_DRVR)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_SHUTDOWN_DRVR)(HDRIVER handle, const char* signOff);

typedef uint32_t(CDECL* FUN_AIL_INDEX_VOC_BLK)(HDRIVER driver, void FAR* VOC_sel, uint32_t VOC_seg, uint32_t block_marker, sound_buff* buff);
typedef void(CDECL* FUN_AIL_REG_SND_BUFF)(HDRIVER driver, uint32_t buffer_num, sound_buff* buff);
typedef uint32_t(CDECL* FUN_AIL_SND_BUFF_STAT)(HDRIVER driver, uint32_t buffer_num);
typedef void(CDECL* FUN_AIL_P_VOC_FILE)(HDRIVER driver, void FAR* VOC_sel, uint32_t VOC_seg, int block_marker);
typedef uint32_t(CDECL* FUN_AIL_VOC_PB_STAT)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_START_D_PB)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_STOP_D_PB)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_PAUSE_D_PB)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_RESUME_D_PB)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_SET_D_PB_VOL)(HDRIVER driver, uint32_t volume);
typedef uint32_t(CDECL* FUN_AIL_D_PB_VOL)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_SET_D_PB_PAN)(HDRIVER driver, uint32_t panpot);
typedef uint32_t(CDECL* FUN_AIL_D_PB_PAN)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_F_VOC_FILE)(HDRIVER driver, void FAR* VOC_sel, int block_marker);
typedef void(CDECL* FUN_AIL_F_SND_BUFF)(HDRIVER driver, sound_buff* buff);

typedef uint32_t(CDECL* FUN_AIL_STATE_TAB_SIZE)(HDRIVER driver);
typedef HSEQUENCE(CDECL* FUN_AIL_REG_SEQ)(HDRIVER driver, void* FORM_XMID, uint32_t sequence_num, void* state_table, void* controller_table);
typedef void(CDECL* FUN_AIL_REL_SEQ_HND)(HDRIVER driver, HSEQUENCE sequence);
typedef uint32_t(CDECL* FUN_AIL_T_CACHE_SIZE)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_DEFINE_T_CACHE)(HDRIVER driver, void* cache_addr, uint32_t cache_size);
typedef uint32_t(CDECL* FUN_AIL_T_REQ)(HDRIVER driver, HSEQUENCE sequence);
typedef void(CDECL* FUN_AIL_INSTALL_T)(HDRIVER driver, int bank, int patch, void* src_addr);
typedef void(CDECL* FUN_AIL_PROTECT_T)(HDRIVER driver, int bank, int patch);
typedef void(CDECL* FUN_AIL_UNPROTECT_T)(HDRIVER driver, int bank, int patch);
typedef uint32_t(CDECL* FUN_AIL_T_STATUS)(HDRIVER driver, int bank, int patch);

typedef void(CDECL* FUN_AIL_START_SEQ)(HDRIVER driver, HSEQUENCE sequence);
typedef void(CDECL* FUN_AIL_STOP_SEQ)(HDRIVER driver, HSEQUENCE sequence);
typedef void(CDECL* FUN_AIL_RESUME_SEQ)(HDRIVER driver, HSEQUENCE sequence);
typedef uint32_t(CDECL* FUN_AIL_SEQ_STAT)(HDRIVER driver, HSEQUENCE sequence);
typedef uint32_t(CDECL* FUN_AIL_REL_VOL)(HDRIVER driver, HSEQUENCE sequence);
typedef uint32_t(CDECL* FUN_AIL_REL_TEMPO)(HDRIVER driver, HSEQUENCE sequence);
typedef void(CDECL* FUN_AIL_SET_REL_VOL)(HDRIVER driver, HSEQUENCE sequence, uint32_t percent, uint32_t millis);
typedef void(CDECL* FUN_AIL_SET_REL_TEMPO)(HDRIVER driver, HSEQUENCE sequence, uint32_t percent, uint32_t millis);
typedef uint32_t(CDECL* FUN_AIL_BEAT_CNT)(HDRIVER driver, HSEQUENCE sequence);
typedef uint32_t(CDECL* FUN_AIL_BAR_CNT)(HDRIVER driver, HSEQUENCE sequence);
typedef void(CDECL* FUN_AIL_BRA_INDEX)(HDRIVER driver, HSEQUENCE sequence, uint32_t marker_number);

typedef int(CDECL* FUN_AIL_CON_VAL)(HDRIVER driver, HSEQUENCE sequence, uint32_t channel, uint32_t channel_num);
typedef void(CDECL* FUN_AIL_SET_CON_VAL)(HDRIVER driver, HSEQUENCE sequence, uint32_t channel, uint32_t controller_num, uint32_t value);
typedef uint32_t(CDECL* FUN_AIL_CHAN_NOTES)(HDRIVER driver, HSEQUENCE sequence, uint32_t channel);
typedef void(CDECL* FUN_AIL_SEND_CV_MSG)(HDRIVER driver, uint32_t status, uint32_t data_1, uint32_t data_2);
typedef void(CDECL* FUN_AIL_SEND_SYSEX_MSG)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_WRITE_DISP)(HDRIVER driver, const char* string);
typedef void(CDECL* FUN_AIL_INSTALL_CB)(HDRIVER driver, void* callback_f);
typedef void(CDECL* FUN_AIL_CANCEL_CB)(HDRIVER driver);

typedef uint32_t(CDECL* FUN_AIL_LOCK_CHAN)(HDRIVER driver);
typedef void(CDECL* FUN_AIL_MAP_SEQ_CHAN)(HDRIVER driver, HSEQUENCE sequence, uint32_t sequence_channel, uint32_t physical_channel);
typedef void(CDECL* FUN_AIL_RELEASE_CHAN)(HDRIVER driver, uint32_t channel);
typedef uint32_t(CDECL* FUN_AIL_TRUE_SEQ_CHAN)(HDRIVER driver, HSEQUENCE sequence, uint32_t sequence_channel);

#endif
