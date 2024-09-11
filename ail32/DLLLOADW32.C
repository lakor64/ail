#define WIN32_LEAN_AND_MEAN 1
#include "AILDLL.H"
#include "AIL32DRIVER.H"
#undef NO_ERROR
#include <Windows.h>

typedef struct DLL_WIN32_internal_s
{
	HMODULE hLibrary;
	AilEntrypoint proc;
} DLL_WIN32_internal;

uint32_t CDECL DLL_size(const void* source, uint32_t flags)
{
	if (flags & DLLSRC_MEM)
		return 0; // NOT SUPPORTED ON WINDOWS

	return sizeof(DLL_WIN32_internal);
}

void* CDECL DLL_load(const void* source, uint32_t flags, void* dll)
{
	DLL_WIN32_internal* ptr;

	if (flags & DLLSRC_MEM)
		return NULL; // NOT SUPPORTED ON WINDOWS

	if (flags & DLLMEM_USER)
	{
		DLL_WIN32_internal* pp = (DLL_WIN32_internal*)dll;

		if (!pp)
			return NULL; // invalid args...

		pp->hLibrary = LoadLibraryA((const char*)source);
		if (!pp->hLibrary)
		{
			return NULL;
		}

		pp->proc = (AilEntrypoint)GetProcAddress(pp->hLibrary, AIL_ENTRYPOINT_STR);
		if (!pp->proc)
		{
			FreeLibrary(pp->hLibrary);
			return NULL;
		}

		return pp->proc();
	}

	ptr = (DLL_WIN32_internal*)HeapAlloc(GetProcessHeap(), 0, sizeof(DLL_WIN32_internal));

	if (!ptr)
		return NULL;

	ptr->hLibrary = LoadLibrary((const char*)source);

	if (!ptr->hLibrary)
	{
		HeapFree(GetProcessHeap(), 0, ptr);
		return NULL;
	}

	ptr->proc = (AilEntrypoint)GetProcAddress(ptr->hLibrary, AIL_ENTRYPOINT_STR);
	if (!ptr->proc)
	{
		FreeLibrary(ptr->hLibrary);
		HeapFree(GetProcessHeap(), 0, ptr);
		return NULL;
	}

	return ptr->proc();
}

void CDECL DLL_free(void* dll)
{
	if (dll)
	{
		DLL_WIN32_internal* dlli = (DLL_WIN32_internal*)((char*)dll - sizeof(void*));
		if (!dlli || !dlli->hLibrary)
			return; // invalid ptr!!

		FreeLibrary(dlli->hLibrary);
		HeapFree(GetProcessHeap(), 0, dlli);
	}
}

static HANDLE FILE_open(const char* filename, int rmode)
{
	// rmode = 1 (read-only)
	DWORD flg1 = FILE_GENERIC_READ;
	DWORD flg2 = FILE_SHARE_READ;
	DWORD flg3 = OPEN_EXISTING;

	// rmode = 0 (write)
	if (rmode == 0)
	{
		flg1 = FILE_GENERIC_WRITE;
		flg2 = FILE_SHARE_WRITE;
		flg3 = CREATE_ALWAYS;
	}
	// rmode = 2 (append)
	else if (rmode == 2)
	{
		flg1 = FILE_GENERIC_READ | FILE_APPEND_DATA;
		flg2 = FILE_SHARE_READ | FILE_SHARE_WRITE;
		//flg3 = OPEN_EXISTING;
	}

	return CreateFileA(filename, flg1, flg2, NULL, flg3, FILE_ATTRIBUTE_NORMAL, NULL);
}

static int32_t g_globalLastError = NO_ERROR;

int32_t CDECL FILE_error(void)
{
	return g_globalLastError;
}

int32_t CDECL FILE_size(const char* filename)
{
	HANDLE h;
	DWORD p;

	h = FILE_open(filename, 1);

	if (h == INVALID_HANDLE_VALUE)
	{
		g_globalLastError = FILE_NOT_FOUND;
		return 0;
	}

	p = GetFileSize(h, NULL);

	CloseHandle(h);

	return (int32_t)p;
}

void* CDECL FILE_read(const char* filename, void* dest)
{
	HANDLE h;
	DWORD rd = 0;
	DWORD totalSize;
	void* mydest = dest;

	if (!filename)
		return NULL;

	h = FILE_open(filename, 1);
	if (h == INVALID_HANDLE_VALUE)
	{
		g_globalLastError = FILE_NOT_FOUND;
		return NULL;
	}

	totalSize = GetFileSize(h, NULL);

	if (!mydest)
	{
		mydest = HeapAlloc(GetProcessHeap(), 0, totalSize);
		if (!mydest)
		{
			CloseHandle(h);
			return NULL;
		}
	}

	if (!ReadFile(h, mydest, totalSize, &rd, NULL))
	{
		g_globalLastError = CANT_READ_FILE;
		if (!dest)
		{
			HeapFree(GetProcessHeap(), 0, mydest);
		}
		CloseHandle(h);
		return NULL;
	}

	CloseHandle(h);

	if (rd != totalSize)
	{
		g_globalLastError = CANT_READ_FILE;
		if (!dest)
		{
			HeapFree(GetProcessHeap(), 0, mydest);
		}

		return NULL;
	}

	return mydest;
}

int32_t CDECL FILE_write(const char* filename, const void* buf, uint32_t len)
{
	HANDLE h;
	DWORD rd = 0;
	DWORD totalSize;
	BOOL ret;

	if (!buf || len < 1 || !filename)
		return -1;

	h = FILE_open(filename, 1);
	if (h == INVALID_HANDLE_VALUE)
	{
		g_globalLastError = FILE_NOT_FOUND;
		return -1;
	}

	totalSize = GetFileSize(h, NULL);

	ret = WriteFile(h, buf, len, &rd, NULL);
	CloseHandle(h);

	if (!ret)
	{
		g_globalLastError = CANT_WRITE_FILE;
		return -1;
	}

	return (int32_t)rd;
}

int32_t CDECL FILE_append(const char* filename, const void* buf, uint32_t len)
{
	HANDLE h;
	DWORD lp, rd = 0;
	BOOL r;

	h = FILE_open(filename, 2);
	if (h == INVALID_HANDLE_VALUE)
	{
		g_globalLastError = FILE_NOT_FOUND;
		return -1;
	}

	lp = SetFilePointer(h, 0, NULL, FILE_END);

	if (!LockFile(h, lp, 0, len, 0))
	{
		g_globalLastError = IO_ERROR;
		CloseHandle(h);
		return -1;
	}

	r = WriteFile(h, buf, len, &rd, NULL);

	UnlockFile(h, lp, 0, len, 0);
	CloseHandle(h);

	if (!r)
	{
		g_globalLastError = CANT_WRITE_FILE;
		return -1;
	}

	return (int32_t)rd;
}
