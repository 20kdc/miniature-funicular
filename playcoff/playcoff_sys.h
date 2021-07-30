#pragma once

#include <stddef.h>

// System-dependent function module.

typedef struct {
	char * (*allocateRWX)(size_t size);
	void * (*dlopenNow)(const char * name);
	void * (*dlsym)(void * handle, const char * name);
	int (*dlclose)(void * handle);
	char * (*dlerror)();
	int (*printf)(const char * fmt, ...);
	int platformType;
} playcoff_sys_t;

#define PLAYCOFF_PLATFORM_UNIX 0
#define PLAYCOFF_PLATFORM_WINDOWS 1

extern const playcoff_sys_t playcoff_sys;

