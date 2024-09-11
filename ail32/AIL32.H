/*���������������������������������������������������������������������������
//��                                                                       ��
//��   IBM Audio Interface Library                                         ��
//��                                                                       ��
//��   AIL32.H: C API function prototypes                                  ��
//��                                                                       ��
//��   Source compatible with 32-bit IBM ANSI C/C++                        ��
//��                                                                       ��
//��   AIL32.H release 1.00                                                ��
//��                                                                       ��
//���������������������������������������������������������������������������
//��                                                                       ��
//��   Copyright (C) 1991, 1992 Miles Design, Inc.                         ��
//��                                                                       ��
//��   Miles Design, Inc.                                                  ��
//��   10926 Jollyville #308                                               ��
//��   Austin, TX 78759                                                    ��
//��   (512) 345-2642 / FAX (512) 338-9630 / BBS (512) 454-9990            ��
//��                                                                       ��
//���������������������������������������������������������������������������
*/

#ifndef AIL32_H
#define AIL32_H

#include "AIL32DRIVER.H"

#ifdef __cplusplus
extern "C" {
#endif

/********************/
/*                  */
/* Process services */
/*                  */
/********************/

void CDECL AIL_startup(void);
void CDECL AIL_shutdown(const char *signoff_msg);
HTIMER CDECL AIL_register_timer(void *callback_fn);
void CDECL AIL_set_timer_period(HTIMER timer, uint32_t microseconds);
void CDECL AIL_set_timer_frequency(HTIMER timer, uint32_t hertz);
void CDECL AIL_set_timer_divisor(HTIMER timer, uint32_t PIT_divisor);
uint32_t CDECL AIL_interrupt_divisor(void);
void CDECL AIL_start_timer(HTIMER timer);
void CDECL AIL_start_all_timers(void);
void CDECL AIL_stop_timer(HTIMER timer);
void CDECL AIL_stop_all_timers(void);
void CDECL AIL_release_timer_handle(HTIMER timer);
void CDECL AIL_release_all_timers(void);

/*************************/
/*                       */
/* Installation services */
/*                       */
/*************************/

HDRIVER CDECL AIL_register_driver(void *driver_base_addr);
void CDECL AIL_release_driver_handle(HDRIVER driver);
ail_drvr_desc* CDECL AIL_describe_driver(HDRIVER driver);
HBOOL CDECL AIL_detect_device(HDRIVER driver, uint32_t IO_addr,
    uint32_t IRQ, uint32_t DMA, uint32_t DRQ);
void CDECL AIL_init_driver(HDRIVER driver, uint32_t IO_addr,
    uint32_t IRQ, uint32_t DMA, uint32_t DRQ);
void CDECL AIL_shutdown_driver(HDRIVER driver, const char *signoff_msg);
                    
/********************************/
/*                              */
/* Digital performance services */
/*                              */
/********************************/

uint32_t CDECL AIL_index_VOC_block(HDRIVER driver, void FAR *VOC_sel, 
   uint32_t VOC_seg, uint32_t block_marker, sound_buff *buff);
void CDECL AIL_format_VOC_file(HDRIVER driver, void FAR *VOC_sel, int
   block_marker);
void CDECL AIL_play_VOC_file(HDRIVER driver, void FAR *VOC_sel,
    uint32_t VOC_seg, int block_marker);

void CDECL AIL_register_sound_buffer(HDRIVER driver, uint32_t buffer_num,
   sound_buff *buff);
void CDECL AIL_format_sound_buffer(HDRIVER driver, sound_buff *buff);
uint32_t CDECL AIL_sound_buffer_status(HDRIVER driver, uint32_t buffer_num);
uint32_t CDECL AIL_VOC_playback_status(HDRIVER driver);

void CDECL AIL_start_digital_playback(HDRIVER driver);
void CDECL AIL_stop_digital_playback(HDRIVER driver);
void CDECL AIL_pause_digital_playback(HDRIVER driver);
void CDECL AIL_resume_digital_playback(HDRIVER driver);
void CDECL AIL_set_digital_playback_volume(HDRIVER driver, uint32_t volume);
uint32_t CDECL AIL_digital_playback_volume(HDRIVER driver);
void CDECL AIL_set_digital_playback_panpot(HDRIVER driver, uint32_t panpot);
uint32_t CDECL AIL_digital_playback_panpot(HDRIVER driver);

/******************************/
/*                            */
/* XMIDI performance services */
/*                            */
/******************************/

uint32_t CDECL AIL_state_table_size(HDRIVER driver);
HSEQUENCE CDECL AIL_register_sequence(HDRIVER driver, void *FORM_XMID,
   uint32_t sequence_num, void *state_table, void *controller_table);
void CDECL AIL_release_sequence_handle(HDRIVER driver, HSEQUENCE sequence);

uint32_t CDECL AIL_default_timbre_cache_size(HDRIVER driver);
void CDECL AIL_define_timbre_cache(HDRIVER driver, void *cache_addr,
   uint32_t cache_size);                     
uint32_t CDECL AIL_timbre_request(HDRIVER driver, HSEQUENCE sequence);
uint32_t CDECL AIL_timbre_status(HDRIVER driver, int bank, int patch);
void CDECL AIL_install_timbre(HDRIVER driver, int bank, int patch, 
   void *src_addr);
void CDECL AIL_protect_timbre(HDRIVER driver, int bank, int patch);
void CDECL AIL_unprotect_timbre(HDRIVER driver, int bank, int patch);

void CDECL AIL_start_sequence(HDRIVER driver, HSEQUENCE sequence);
void CDECL AIL_stop_sequence(HDRIVER driver, HSEQUENCE sequence);
void CDECL AIL_resume_sequence(HDRIVER driver, HSEQUENCE sequence);
uint32_t CDECL AIL_sequence_status(HDRIVER driver, HSEQUENCE sequence);
uint32_t CDECL AIL_relative_volume(HDRIVER driver, HSEQUENCE sequence);
uint32_t CDECL AIL_relative_tempo(HDRIVER driver, HSEQUENCE sequence);
void CDECL AIL_set_relative_volume(HDRIVER driver, HSEQUENCE sequence,
   uint32_t percent, uint32_t milliseconds);
void CDECL AIL_set_relative_tempo(HDRIVER driver, HSEQUENCE sequence,
   uint32_t percent, uint32_t milliseconds);
int CDECL AIL_controller_value(HDRIVER driver, HSEQUENCE sequence,
   uint32_t channel, uint32_t controller_num);
void CDECL AIL_set_controller_value(HDRIVER driver, HSEQUENCE sequence,
   uint32_t channel, uint32_t controller_num, uint32_t value);
uint32_t CDECL AIL_channel_notes(HDRIVER driver, HSEQUENCE sequence,
   uint32_t channel);
uint32_t CDECL AIL_beat_count(HDRIVER driver, HSEQUENCE sequence);
uint32_t CDECL AIL_measure_count(HDRIVER driver, HSEQUENCE sequence);
void CDECL AIL_branch_index(HDRIVER driver, HSEQUENCE sequence, 
   uint32_t marker_number);

void CDECL AIL_send_channel_voice_message(HDRIVER driver, uint32_t status, 
   uint32_t data_1, uint32_t data_2);
void CDECL AIL_send_sysex_message(HDRIVER driver, uint32_t addr_a,
   uint32_t addr_b, uint32_t addr_c, void *data, uint32_t size, 
   uint32_t delay);
void CDECL AIL_write_display(HDRIVER driver, const char *string);
void CDECL AIL_install_callback(HDRIVER driver, 
   void *callback_fn);
void CDECL AIL_cancel_callback(HDRIVER driver);

uint32_t CDECL AIL_lock_channel(HDRIVER driver);
void CDECL AIL_map_sequence_channel(HDRIVER driver, HSEQUENCE sequence,
   uint32_t sequence_channel, uint32_t physical_channel);
uint32_t CDECL AIL_true_sequence_channel(HDRIVER driver, HSEQUENCE sequence,
   uint32_t sequence_channel);
void CDECL AIL_release_channel(HDRIVER driver, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif
