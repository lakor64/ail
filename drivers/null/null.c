/*
* IBM Audio Interface Library API NULL driver
*
* Version 1.00 of 11-Sept-24: Initial version
*
*/
/*
* This file can be used as a sample to implement a custom XMIDI/DIGITAL SOUND driver
* 
* NOTE: This code is untested under DOS or OS/2
*/

/* main ail32 inclusion for driver */
#include <AIL32DRIVER.H>

/*
* dll export for windows
*/
#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

/*
* Entrypoint structure of the driver
* the format is done this way for retrocompatibility with previous w32 drivers
*/
#pragma pack(1) /* make sure to pack this to 1 or the code might break!! */
struct my_entry
{
	/* must be the length of the copyright string PLUS the size of the length parameter itself (4 bytes) */
	uint32_t copyrightLen;
	/* copyright string, must start with "Copy" or AIL will not detect this as a valid driver */
	const char FAR copyright[28];
	/* all implemented functions by this driver */
	ail_driver_proc_table pp[6];
};

/* FORWARD DECLARATIONS OF THE DRIVER FUNCTIONS
* Refeer to the driver callbacks in AIL32DRIVER.H for how to write the functions
*/
static ail_drvr_desc* CDECL my_desc_driver(HDRIVER handle, AilIntRateProc proc);
static HBOOL CDECL my_detect(HDRIVER handle, uint32_t io_addr, uint32_t irq, uint32_t dma, uint32_t drq);
static HBOOL CDECL my_init(HDRIVER handle, uint32_t io_addr, uint32_t irq, uint32_t dma, uint32_t drq);
static void CDECL my_shutdown(HDRIVER handle, const char* signOff);
static HSEQUENCE CDECL my_seq_reg(HDRIVER driver, void* FORM_XMID, uint32_t sequence_num, void* state_table, void* controller_table);

/*
* Actual entrypoint definition
*/
static struct my_entry g_entry = {
	32, /* sizeof(copyrightLen) + sizeof(copyright) */
	"Copyright (C) 2024 lakor64",
	{
		/* function definitions */
		{ AIL_DESC_DRVR, my_desc_driver },
		{ AIL_DET_DEV, my_detect },
		{ AIL_INIT_DRVR, my_init },
		{ AIL_SHUTDOWN_DRVR, my_shutdown },
		{ AIL_REG_SEQ, my_seq_reg },
		/* MUST ADD THIS AT THE END!! OR AIL WILL CRASH */
		AIL_END_DRIVER_PROC_TABLE
	}
};

/* driver desc */
static ail_drvr_desc g_drv_desc = {
	/* must be the CURRENT_REV value or the driver will not be loaded */
	CURRENT_REV,
	/* type of driver!! */
	XMIDI_AND_DSP_DRVR,
	/* global timbre library suffix (we emulate OPL just to test) */
	{ 'O', 'P', 'L', '\x00' },
	/* pointer to a list of supported devices, NULL means any device */
	(char*)NULL,
	/* default I/O port, used for manual handling of the device */
	-1,
	/* default IRQ, used for manual handling of the device */
	-1,
	/* default DMA port, used for manual handling of the device */
	-1,
	/* default DRQ, used for manual handling of the device */
	-1,
	/* how many times a periodic call to service the audio is required, -1 means not required */
	-1,
	/* size of the display in the board, 0 if there is no display */
	0
};

/*
* Exported DLL function that will show the entrypoint
*/
#ifndef __DOS__
DLL_EXPORT void* CDECL AIL_ENTRYPOINT(void)
{
	return &g_entry;
}
#endif

/**
* This function gets the properties of the driver
* PARAM handle: Handle to an AIL driver pointer
* PARAM proc: Procedure provided by AIL to be called for getting the rate time
* RETURN: A driver descriptor
*/
ail_drvr_desc* CDECL my_desc_driver(HDRIVER handle, AilIntRateProc proc)
{
	return &g_drv_desc;
}

/**
* This function tries to detect if the specified driver is inserted
* PARAM handle: Handle to an AIL driver pointer
* PARAM io_addr: I/O port to search
* PARAM irq: IRQ interrupt
* PARAM dma: DMA port
* PARAM drq: DRQ
* RETURN: TRUE in case the driver is found, FALSE in case the driver is not found
*/
HBOOL CDECL my_detect(HDRIVER handle, uint32_t io_addr, uint32_t irq, uint32_t dma, uint32_t drq)
{
	/* always return true as it's a fake driver */
	return TRUE;
}

/**
* This function initializes the driver
* PARAM handle: Handle to an AIL driver pointer
* PARAM io_addr: I/O port
* PARAM irq: IRQ interrupt
* PARAM dma: DMA port
* PARAM drq: DRQ
*/
HBOOL CDECL my_init(HDRIVER handle, uint32_t io_addr, uint32_t irq, uint32_t dma, uint32_t drq)
{
	/* nothing to init */
	return TRUE;
}

/**
* This function shuts down the driver
* PARAM handle: Handle to an AIL driver pointer
* PARAM signOff: Shutdown message
*/
void CDECL my_shutdown(HDRIVER handle, const char* signOff)
{
	/* nothing to do */
}

/**
* This function registers a new XMIDI sequence to play
* PARAM driver: Handle to an AIL driver pointer
* PARAM FORM_XMID: XMIDI file
* PARAM sequence_num: Sequence index to register
* PARAM state_table: Allocated pointer to the state table
* PARAM controller_table: Allocated pointer to the controller table or NULL if there is no controller table
* RETURN: Handle to the sequence
*/
HSEQUENCE CDECL my_seq_reg(HDRIVER driver, void* FORM_XMID, uint32_t sequence_num, void* state_table, void* controller_table)
{
	/* always register as sequence 1 */
	return (HSEQUENCE)1;
}