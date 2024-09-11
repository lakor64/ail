/*
* IBM Audio Interface Library API module for 32-bit DPMI (AIL/32)
*
* Version 1.00 of 06-Sept-24: Initial version
*
*/
#include "AIL32.H"
#include "AIL32DRIVER.H"
#include "AIL32INTERNAL.H"

// Configuration equates

#define DRIVERS_MAX 16
#define EARLY_EOI -1 // -1 to send EOI at beginning of IRQ (for interrupt-critical environments)
#define NON_REENTRANT // TODO

// Macros, internal equates

#ifdef __DOS__
#define DEFAULT_PERIOD 54925
#define TIMERS_MAX (DRIVERS_MAX + 1) // 1 extra value is for the system timer
#define BIOS_TIMER DRIVERS_MAX // Handle to BIOS default timer
#define LAST_DRIVER_TIMER (HTIMER)BIOS_TIMER
#else
#define DEFAULT_PERIOD 0
#define TIMERS_MAX DRIVERS_MAX
#define LAST_DRIVER_TIMER (HTIMER)(DRIVERS_MAX - 1)
#endif

#define find_fun(opcode, driver) FUN_##opcode cb = (FUN_##opcode)find_proc(opcode, driver)
#define call_driver(opcode, driver, ...) FUN_##opcode cb = (FUN_##opcode)find_proc(opcode, driver); if (cb) cb(driver, ##__VA_ARGS__);


// Enum
enum ail_timer_status
{
    TIMER_STATUS_FREE,
    TIMER_STATUS_OFF,
    TIMER_STATUS_ON,
};

static void AIL_release_timer_handle(HTIMER Timer);
static void AIL_set_timer_period(HTIMER Timer, uint32_t uS);
static void AIL_start_timer(HTIMER Timer);

// Local data

static uint16_t active_timers = 0; // # of timers currently registered
static HBOOL timer_busy = FALSE; //Reentry flag for INT 8 handler

static void* timer_callback[TIMERS_MAX] = {0}; // Callback function addrs for timers
static enum ail_timer_status timer_status[TIMERS_MAX] = { TIMER_STATUS_FREE }; // Status of timers (0=free 1=off 2=on)
static uint32_t timer_elapsed[TIMERS_MAX] = {0}; // Modified DDA error counts for timers
static uint32_t timer_value[TIMERS_MAX] = {0}; // Modified DDA limit values for timers
static uint32_t timer_period = 0; //Modified DDA increment for timers

static uint32_t current_timer = 0;

static ail_driver_proc_table* index_base[DRIVERS_MAX] = { NULL }; // Driver table base addresses
static HTIMER assigned_timer[DRIVERS_MAX] = { TIMER_NULL }; // Timers assigned to drivers
static HBOOL driver_active[DRIVERS_MAX] = { FALSE };

/****************************************************************************
*                                                                           *
* Internal procedures                                                       *
*                                                                           *
*****************************************************************************/

// DOS only, this functions must be in assembler.
#ifdef __DOS__
// Take over default BIOS INT 8 handler
extern void CDECL hook_timer_process(void);
// Restore default BIOS INT 8 handler
extern void CDECL unhook_timer_process(void);
// Set 8253 Programmable Interval Timer to desired period in microseconds
extern void CDECL set_PIT_period(uint32_t Period);
// Set 8253 Programmable Interval Timer to desired IRQ 0 (INT 8) interval
extern void CDECL set_PIT_divisor(uint32_t Divisor);
// Get PIT divisor
extern void CDECL get_PIT_divisor(void);
#endif

static void* find_proc(uint32_t opcode, HDRIVER driver)
{
    if (driver >= DRIVERS_MAX || driver < 0)
        return NULL; // exit sanely if handle invalid
    
    if (!index_base[driver]) // driver procedure table
        return NULL; // unreg'd driver, exit
    
    uint32_t id = 0;
    ail_driver_proc_table* tbl = index_base[driver];

    for (; ; tbl++)
    {
        if (tbl->opcode == -1)
            break;
        else if (tbl->opcode == opcode)
            return tbl->function;
    }

    return NULL;
}

// Initialize timer DDA counters
static void init_DDA_arrays(void)
{
    timer_period = -1;

    for (int i = 0; i < TIMERS_MAX; i++)
    {
        timer_status[i] = TIMER_STATUS_FREE; // mark all timer handles "free"
        timer_elapsed[i] = 0;
        timer_value[i] = 0;
    }
}

// Establish timer interrupt rates based on fastest active timer
static void program_timers()
{
    NON_REENTRANT;
    
    uint32_t temp_period = -1;

    // find fastest active timer (include BIOS reserved timer)
    for (HTIMER timer = 0; timer < TIMERS_MAX; timer++)
    {
        if (timer_status[timer] == 0) // timer active (registred)?
            continue;

        if (timer_value[timer] < temp_period)
            temp_period = timer_value[timer];
    }

    if (temp_period == timer_period)
        return; // current rate hasn't changed, exit
    
    // else set new base timer rate (slowest possible base = 54 msec!)
    current_timer = -1;
    timer_period = temp_period;

#ifdef __DOS__
    set_PIT_period(temp_period);
#endif

    for (int i = 0; i < TIMERS_MAX; i++)
    {
        timer_elapsed[i] = 0; // reset ALL elapsed counters to 0 uS
    }
}

/*****************************************************************************
*                                                                           *
* Process services                                                          *
*                                                                           *
*****************************************************************************/

// Initialize AIL API
void CDECL AIL_startup(void)
{
    NON_REENTRANT;
    
    active_timers = 0; // # of registred timers

#ifdef __DOS__
    timer_busy = FALSE; // timer re-entrancy protection
#endif

    for (int i = 0; i < DRIVERS_MAX; i++)
    {
        index_base[i] = NULL;
        assigned_timer[i] = TIMER_NULL;
        driver_active[i] = FALSE;
    }

    // init timer countdown values
    init_DDA_arrays();
}

// Quick shutdown of all AIL resources
void CDECL AIL_shutdown(const char *signoff_msg)
{
    NON_REENTRANT;
    
    for (HDRIVER cur_drvr = 0; cur_drvr < DRIVERS_MAX; cur_drvr++)
    {
        if (index_base[cur_drvr] == 0)
            continue;

        if (assigned_timer[cur_drvr] != TIMER_NULL)
        {
            AIL_release_timer_handle(assigned_timer[cur_drvr]);
        }

        AIL_shutdown_driver(cur_drvr, signoff_msg);
    }
}

HTIMER CDECL AIL_register_timer(void *callback_fn)
{
    NON_REENTRANT;
    
    HTIMER free_id = 0;

    for (free_id = 0; free_id < TIMERS_MAX; free_id++)
    {
        if (timer_status[free_id] == 0)
            break;
    }

    if (free_id == TIMERS_MAX)
        return TIMER_NULL; // no free timers, return -1
    
    // yes, set up to return handle
    timer_status[free_id] = TIMER_STATUS_OFF; // turn the new timer "off" (stopped)
    timer_callback[free_id] = callback_fn;
    timer_value[free_id] = -1;
    active_timers++;
    if (active_timers != 1) // is this the first timer registred?
    {
        // no, just return
        return free_id;
    }

    // yes, set up our own INT 8 handler
    init_DDA_arrays(); // init timer countdown values

#ifdef __DOS__
    // TODO: needs adjustment
    timer_status[BIOS_TIMER] = TIMER_STATUS_ON;
    hook_timer_process(); // seize INT 8 and register BIOS handler
    AIL_set_timer_period(BIOS_TIMER, DEFAULT_PERIOD);
    AIL_start_timer(BIOS_TIMER);
#endif

    timer_status[free_id] = TIMER_STATUS_OFF; // (cleared by init_DDA_arrays)
    timer_value[free_id] = -1;

    return free_id;
}

void CDECL AIL_set_timer_period(HTIMER timer, uint32_t microseconds)
{
    NON_REENTRANT;
    
    enum ail_timer_status old_status = timer_status[timer]; // save timer's status
    timer_status[timer] = TIMER_STATUS_OFF; // stop timer while calculating...

    timer_value[timer] = microseconds;
    timer_elapsed[timer] = 0;
    program_timers(); // reset base interrupt rate if needed

    timer_status[timer] = old_status; // restore timer's former status
}

void CDECL AIL_set_timer_frequency(HTIMER timer, uint32_t hertz)
{
    AIL_set_timer_period(timer, 0x0f4240 / hertz);
}

void CDECL AIL_set_timer_divisor(HTIMER timer, uint32_t PIT_divisor)
{
    // TODO: this is probably x86 only...

    if (PIT_divisor == 0)
    {
        AIL_set_timer_period(timer, DEFAULT_PERIOD);
    }
    else
    {
        AIL_set_timer_period(timer, (int)((10000 * PIT_divisor) / 11932)); // convert to microseconds then divide accurate by 0.01%
    }
}

uint32_t CDECL AIL_interrupt_divisor(void)
{
#ifdef __DOS__
    return get_PIT_divisor();
#else
    return 0;
#endif
}

void CDECL AIL_start_timer(HTIMER timer)
{
    NON_REENTRANT;

    if (timer_status[timer] == TIMER_STATUS_OFF) // is the specified timer stopped?
        timer_status[timer] = TIMER_STATUS_ON; // yes, start it
}

void CDECL AIL_start_all_timers(void)
{
    NON_REENTRANT;

    for (HTIMER timer = LAST_DRIVER_TIMER; timer >= 0; timer--)
    {
        AIL_start_timer(timer);
    }
}

void CDECL AIL_stop_timer(HTIMER timer)
{
    NON_REENTRANT;

    if (timer_status[timer] == TIMER_STATUS_ON) // is the specified timer running?
    {
        timer_status[timer] = TIMER_STATUS_OFF; // yes, stop it
    }
}

void CDECL AIL_stop_all_timers(void)
{
    NON_REENTRANT;

    for (HTIMER timer = LAST_DRIVER_TIMER; timer >= 0; timer--)
    {
        AIL_stop_timer(timer);
    }
}

void CDECL AIL_release_timer_handle(HTIMER timer)
{
    NON_REENTRANT;

    if (timer == TIMER_NULL)
        return;
    
    if (timer_status[timer] == TIMER_STATUS_FREE) // is the specified timer active?
        return; // no, exit

    timer_status[timer] = TIMER_STATUS_FREE; // release the timer's handle
    active_timers--; // any active timers left?

#ifdef __DOS__
    if (active_timers == 0)
    {
        // if not, put the default handler back
        set_PIT_divisor(0);
        unhook_timer_process();
    }
#endif
}

void CDECL AIL_release_all_timers(void)
{
    NON_REENTRANT;

    for (HTIMER timer = LAST_DRIVER_TIMER; timer >= 0; timer--)
    {
        AIL_release_timer_handle(timer);
    }
}


/****************************************************************************
*                                                                           *
* Installation services                                                     *
*                                                                           *
*****************************************************************************/

HDRIVER CDECL AIL_register_driver(void *driver_base_addr)
{
    NON_REENTRANT;

    HDRIVER cur_drvr = 0;

    for (; cur_drvr < DRIVERS_MAX; cur_drvr++)
    {
        if (!index_base[cur_drvr])
            break;
    }

    if (cur_drvr == DRIVERS_MAX)
        return DRIVER_NULL; // return -1 if no free handles

    // else check for copyright string

    if (*((uint32_t*)((char*)driver_base_addr + sizeof(uint32_t))) != 0x79706f43) // Copy
        return (HDRIVER)-1; // avoid calling non-AIL drivers
    
    index_base[cur_drvr] = (ail_driver_proc_table*)((char*)driver_base_addr + *((uint32_t*)driver_base_addr));
    ail_drvr_desc* drvDesc = AIL_describe_driver(cur_drvr);

    if (!drvDesc)
    {
        return DRIVER_NULL; // return -1 if description call failed
    }

    if (drvDesc->min_API_version != CURRENT_REV)
    {
        return DRIVER_NULL; // return -1 if API out of date
    }

    return cur_drvr; // else return new driver handle
}

void CDECL AIL_release_driver_handle(HDRIVER driver)
{
    NON_REENTRANT;

    if (driver >= DRIVERS_MAX)
        return; // exit cleanly if invalid handle passed
    
    index_base[driver] = NULL;
}

ail_drvr_desc * CDECL AIL_describe_driver(HDRIVER driver)
{
    find_fun(AIL_DESC_DRVR, driver);
    if (!cb)
        return NULL;

    return cb(driver, AIL_interrupt_divisor);
}

HBOOL CDECL AIL_detect_device(HDRIVER driver, uint32_t IO_addr,
    uint32_t IRQ, uint32_t DMA, uint32_t DRQ)
{
    find_fun(AIL_DET_DEV, driver);
    if (!cb)
        return FALSE;

    return cb(driver, IO_addr, IRQ, DMA, DRQ);
}

void CDECL AIL_init_driver(HDRIVER driver, uint32_t IO_addr,
    uint32_t IRQ, uint32_t DMA, uint32_t DRQ)
{
    NON_REENTRANT;

    FUN_AIL_INIT_DRVR cb;
    ail_drvr_desc* dsc;
    HTIMER timer_handle = TIMER_NULL;

    if (driver >= DRIVERS_MAX)
        return; // exit cleanly if invalid handle passed

    
    dsc = AIL_describe_driver(driver);

    if (dsc->svc_rate != -1)
    {
        FUN_AIL_SERVE_DRVR cb2 = (FUN_AIL_SERVE_DRVR)find_proc(AIL_SERVE_DRVR, driver);
        if (cb2)
        {
            HTIMER tm = AIL_register_timer(cb2);
            assigned_timer[driver] = tm;
            timer_handle = tm;
            AIL_set_timer_frequency(timer_handle, dsc->svc_rate);
        }
    }

    cb = (FUN_AIL_INIT_DRVR)find_proc(AIL_INIT_DRVR, driver);
    if (cb)
    {
        if (cb(driver, IO_addr, IRQ, DMA, DRQ))
        {
            driver_active[driver] = TRUE;

            if (timer_handle != TIMER_NULL)
            {
                AIL_start_timer(timer_handle);
            }
        }
    }
    
    if (timer_handle != TIMER_NULL)
    {
        if (!driver_active[driver])
            AIL_release_timer_handle(timer_handle);
        else
            AIL_start_timer(timer_handle);
    }
}

void CDECL AIL_shutdown_driver(HDRIVER driver, const char *signoff_msg)
{
    FUN_AIL_SHUTDOWN_DRVR cb;

    if (driver >= DRIVERS_MAX)
        return; // exit cleanly if invalid handle passed
    
    if (driver_active[driver] == FALSE)
        return; // driver never initialized, exit
    
    if (assigned_timer[driver] != TIMER_NULL)
        AIL_release_timer_handle(assigned_timer[driver]);

    cb = (FUN_AIL_SHUTDOWN_DRVR)find_proc(AIL_SHUTDOWN_DRVR, driver);
    if (cb)
    {
        cb(driver, signoff_msg);
    }
}

/****************************************************************************
*                                                                           *
* Performance services                                                      *
*                                                                           *
*****************************************************************************/

uint32_t CDECL AIL_index_VOC_block(HDRIVER driver, void FAR* VOC_sel,
    uint32_t VOC_seg, uint32_t block_marker, sound_buff* buff)
{
    find_fun(AIL_INDEX_VOC_BLK, driver);
    if (!cb)
        return 0;

    return cb(driver, VOC_sel, VOC_seg, block_marker, buff);
}
void CDECL AIL_format_VOC_file(HDRIVER driver, void FAR* VOC_sel, int
    block_marker)
{
    call_driver(AIL_F_VOC_FILE, driver, VOC_sel, block_marker);
}

void CDECL AIL_play_VOC_file(HDRIVER driver, void FAR* VOC_sel,
    uint32_t VOC_seg, int block_marker)
{
    call_driver(AIL_P_VOC_FILE, driver, VOC_sel, VOC_seg, block_marker);
}

void CDECL AIL_register_sound_buffer(HDRIVER driver, uint32_t buffer_num,
    sound_buff* buff)
{
    call_driver(AIL_REG_SND_BUFF, driver, buffer_num, buff);
}

void CDECL AIL_format_sound_buffer(HDRIVER driver, sound_buff* buff)
{
    call_driver(AIL_F_SND_BUFF, driver, buff);
}

uint32_t CDECL AIL_sound_buffer_status(HDRIVER driver, uint32_t buffer_num)
{
    find_fun(AIL_SND_BUFF_STAT, driver);
    if (!cb)
        return DAC_DONE;

    return cb(driver, buffer_num);
}

uint32_t CDECL AIL_VOC_playback_status(HDRIVER driver)
{
    find_fun(AIL_VOC_PB_STAT, driver);
    if (!cb)
        return DAC_DONE;

    return cb(driver);
}

void CDECL AIL_start_digital_playback(HDRIVER driver)
{
    call_driver(AIL_START_D_PB, driver);
}

void CDECL AIL_stop_digital_playback(HDRIVER driver)
{
    call_driver(AIL_STOP_D_PB, driver);
}

void CDECL AIL_pause_digital_playback(HDRIVER driver)
{
    call_driver(AIL_PAUSE_D_PB, driver);
}

void CDECL AIL_resume_digital_playback(HDRIVER driver)
{
    call_driver(AIL_RESUME_D_PB, driver);
}

void CDECL AIL_set_digital_playback_volume(HDRIVER driver, uint32_t volume)
{
    call_driver(AIL_SET_D_PB_VOL, driver, volume);
}

uint32_t CDECL AIL_digital_playback_volume(HDRIVER driver)
{
    find_fun(AIL_D_PB_VOL, driver);
    if (!cb)
        return 0;

    return cb(driver);
}

void CDECL AIL_set_digital_playback_panpot(HDRIVER driver, uint32_t panpot)
{
    call_driver(AIL_SET_D_PB_PAN, driver, panpot);
}

uint32_t CDECL AIL_digital_playback_panpot(HDRIVER driver)
{
    find_fun(AIL_D_PB_PAN, driver);
    if (!cb)
        return 0;

    return cb(driver);
}

/******************************/
/*                            */
/* XMIDI performance services */
/*                            */
/******************************/

uint32_t CDECL AIL_state_table_size(HDRIVER driver)
{
    find_fun(AIL_STATE_TAB_SIZE, driver);
    if (!cb)
        return 0;

    return cb(driver);
}

HSEQUENCE CDECL AIL_register_sequence(HDRIVER driver, void* FORM_XMID,
    uint32_t sequence_num, void* state_table, void* controller_table)
{
    find_fun(AIL_REG_SEQ, driver);
    if (!cb)
        return SEQUENCE_NULL;

    return cb(driver, FORM_XMID, sequence_num, state_table, controller_table);
}

void CDECL AIL_release_sequence_handle(HDRIVER driver, HSEQUENCE sequence)
{
    call_driver(AIL_REL_SEQ_HND, driver, sequence);
}

uint32_t CDECL AIL_default_timbre_cache_size(HDRIVER driver)
{
    find_fun(AIL_T_CACHE_SIZE, driver);
    if (!cb)
        return 0;

    return cb(driver);
}

void CDECL AIL_define_timbre_cache(HDRIVER driver, void* cache_addr,
    uint32_t cache_size)
{
    call_driver(AIL_DEFINE_T_CACHE, driver, cache_addr, cache_size);
}

uint32_t CDECL AIL_timbre_request(HDRIVER driver, HSEQUENCE sequence)
{
    find_fun(AIL_T_REQ, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence);
}

uint32_t CDECL AIL_timbre_status(HDRIVER driver, int bank, int patch)
{
    find_fun(AIL_T_STATUS, driver);
    if (!cb)
        return -1;

    return cb(driver, bank, patch);
}

void CDECL AIL_install_timbre(HDRIVER driver, int bank, int patch,
    void* src_addr)
{
    call_driver(AIL_INSTALL_T, driver, bank, patch, src_addr);
}

void CDECL AIL_protect_timbre(HDRIVER driver, int bank, int patch)
{
    call_driver(AIL_PROTECT_T, driver, bank, patch);
}

void CDECL AIL_unprotect_timbre(HDRIVER driver, int bank, int patch)
{
    call_driver(AIL_UNPROTECT_T, driver, bank, patch);
}

void CDECL AIL_start_sequence(HDRIVER driver, HSEQUENCE sequence)
{
    call_driver(AIL_START_SEQ, driver, sequence);
}

void CDECL AIL_stop_sequence(HDRIVER driver, HSEQUENCE sequence)
{
    call_driver(AIL_STOP_SEQ, driver, sequence);
}

void CDECL AIL_resume_sequence(HDRIVER driver, HSEQUENCE sequence)
{
    call_driver(AIL_RESUME_SEQ, driver, sequence);
}

uint32_t CDECL AIL_sequence_status(HDRIVER driver, HSEQUENCE sequence)
{
    find_fun(AIL_SEQ_STAT, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence);
}

uint32_t CDECL AIL_relative_volume(HDRIVER driver, HSEQUENCE sequence)
{
    find_fun(AIL_REL_VOL, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence);
}

uint32_t CDECL AIL_relative_tempo(HDRIVER driver, HSEQUENCE sequence)
{
    find_fun(AIL_REL_TEMPO, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence);
}

void CDECL AIL_set_relative_volume(HDRIVER driver, HSEQUENCE sequence,
    uint32_t percent, uint32_t milliseconds)
{
    call_driver(AIL_SET_REL_VOL, driver, sequence, percent, milliseconds);
}

void CDECL AIL_set_relative_tempo(HDRIVER driver, HSEQUENCE sequence,
    uint32_t percent, uint32_t milliseconds)
{
    call_driver(AIL_SET_REL_TEMPO, driver, sequence, percent, milliseconds);
}

int CDECL AIL_controller_value(HDRIVER driver, HSEQUENCE sequence,
    uint32_t channel, uint32_t controller_num)
{
    find_fun(AIL_CON_VAL, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence, channel, controller_num);
}

void CDECL AIL_set_controller_value(HDRIVER driver, HSEQUENCE sequence,
    uint32_t channel, uint32_t controller_num, uint32_t value)
{
    call_driver(AIL_SET_CON_VAL, driver, sequence, channel, controller_num, value);
}

uint32_t CDECL AIL_channel_notes(HDRIVER driver, HSEQUENCE sequence,
    uint32_t channel)
{
    find_fun(AIL_CHAN_NOTES, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence, channel);
}

uint32_t CDECL AIL_beat_count(HDRIVER driver, HSEQUENCE sequence)
{
    find_fun(AIL_BEAT_CNT, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence);
}

uint32_t CDECL AIL_measure_count(HDRIVER driver, HSEQUENCE sequence)
{
    find_fun(AIL_BAR_CNT, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence);
}

void CDECL AIL_branch_index(HDRIVER driver, HSEQUENCE sequence,
    uint32_t marker_number)
{
    call_driver(AIL_BRA_INDEX, driver, sequence, marker_number);
}

void CDECL AIL_send_channel_voice_message(HDRIVER driver, uint32_t status,
    uint32_t data_1, uint32_t data_2)
{
    call_driver(AIL_SEND_CV_MSG, driver, status, data_1, data_2);
}

void CDECL AIL_send_sysex_message(HDRIVER driver, uint32_t addr_a,
    uint32_t addr_b, uint32_t addr_c, void* data, uint32_t size,
    uint32_t delay)
{
    call_driver(AIL_SEND_SYSEX_MSG, driver, );
}

void CDECL AIL_write_display(HDRIVER driver, const char* string)
{
    call_driver(AIL_WRITE_DISP, driver, string);
}

void CDECL AIL_install_callback(HDRIVER driver,
    void* callback_fn)
{
    call_driver(AIL_INSTALL_CB, driver, callback_fn);
}

void CDECL AIL_cancel_callback(HDRIVER driver)
{
    call_driver(AIL_CANCEL_CB, driver);
}

uint32_t CDECL AIL_lock_channel(HDRIVER driver)
{
    find_fun(AIL_LOCK_CHAN, driver);
    if (!cb)
        return -1;

    return cb(driver);
}

void CDECL AIL_map_sequence_channel(HDRIVER driver, HSEQUENCE sequence,
    uint32_t sequence_channel, uint32_t physical_channel)
{
    call_driver(AIL_MAP_SEQ_CHAN, driver, sequence, sequence_channel, physical_channel);
}

uint32_t CDECL AIL_true_sequence_channel(HDRIVER driver, HSEQUENCE sequence,
    uint32_t sequence_channel)
{
    find_fun(AIL_TRUE_SEQ_CHAN, driver);
    if (!cb)
        return -1;

    return cb(driver, sequence, sequence_channel);
}

void CDECL AIL_release_channel(HDRIVER driver, uint32_t channel)
{
    call_driver(AIL_RELEASE_CHAN, driver, channel)
}
