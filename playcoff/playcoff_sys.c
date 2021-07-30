#include "playcoff_sys.h"

#ifndef WINDOWS

#include <sys/mman.h>
#include <stdio.h>
#include <dlfcn.h>

static char * playcoff_allocate_rwx(size_t size) {
	char * executableArea = mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	// Translate to null-return.
	if (executableArea == MAP_FAILED)
		return NULL;
	return executableArea;
}

static void * playcoff_dlopen_now(const char * name) {
	return dlopen(name, RTLD_NOW);
}

const playcoff_sys_t playcoff_sys = {
	playcoff_allocate_rwx,
	playcoff_dlopen_now,
	dlsym,
	dlclose,
	dlerror,
	printf,
	PLAYCOFF_PLATFORM_UNIX
};

#else

#include <windows.h>
#include <stdio.h>

static char * playcoff_allocate_rwx(size_t size) {
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

static void * playcoff_dlopen_now(const char * name) {
	return LoadLibraryA(name);
}

static void * playcoff_dlsym(void * handle, const char * name) {
	return GetProcAddress(handle, name);
}

static int playcoff_dlclose(void * handle) {
	FreeLibrary(handle);
	return 0;
}

static char error[13];

static char * playcoff_dlerror() {
	sprintf(error, "ERR %x", GetLastError());
	return error;
}

const playcoff_sys_t playcoff_sys = {
	playcoff_allocate_rwx,
	playcoff_dlopen_now,
	playcoff_dlsym,
	playcoff_dlclose,
	playcoff_dlerror,
	printf,
	PLAYCOFF_PLATFORM_WINDOWS
};

#endif

