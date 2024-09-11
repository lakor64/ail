//
// 386FX file & DLL loader functions/equates
//

#ifndef DLL_H
#define DLL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_ERRS
#define FILE_ERRS

#define NO_ERROR        0
#define IO_ERROR        1
#define OUT_OF_MEMORY   2
#define FILE_NOT_FOUND  3
#define CANT_WRITE_FILE 4
#define CANT_READ_FILE  5
#define DISK_FULL       6

#endif

#if defined(_MSC_VER)

#ifdef _WIN64
#define CDECL
#else
#define CDECL __cdecl
#endif

#elif defined(__WATCOMC__)
#define CDECL __cdecl
#else
#error "Compiler not supported"
#endif

//
// DLL loader flags & functions
//

#define DLLSRC_FILE  0     // *source is filename string
#define DLLSRC_MEM   1     // *source is pointer to DLL image in memory
#define DLLMEM_USER  2     // *dll -> output memory block alloc'd by user
#define DLLMEM_ALLOC 4     // *dll = don't care; allocate & return output mem

uint32_t CDECL DLL_size(const void *source, uint32_t flags);
void * CDECL DLL_load(const void *source, uint32_t flags, void *dll);
void CDECL DLL_free(void* dll);

//
// File functions
//

int32_t CDECL FILE_error(void);
int32_t CDECL FILE_size(const char *filename);
void * CDECL FILE_read(const char*filename, void *dest);
int32_t CDECL FILE_write(const char*filename, const void *buf, uint32_t len);
int32_t CDECL FILE_append(const char*filename, const void *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
