#include <windows.h>
#include <stdlib.h>

typedef void * (*newstate_t)();
typedef void (*openlibs_t)(void * L);
typedef int (*loadfile_t)(void * L, const char * filename);
typedef int (*pcall_t)(void * L, int nargs, int nresults, int errfunc);

// Yes, I *could* use UTF-16 here.
// But Lua *won't*. Hands are kinda tied.
// Anyway, nobody actually said how long a long path could be, so I'm hardcoding something "sufficiently big"...
char * findDir() {
	int pathBufferLen = 512;
	while (1) {
		char * buffer = malloc(pathBufferLen);
		int res = GetModuleFileNameA(NULL, buffer, pathBufferLen);
		if (res == pathBufferLen) {
			free(buffer);
			pathBufferLen *= 2;
		} else {
			buffer = realloc(buffer, res + 1);
			for (int i = res; i >= 0; i--) {
				char ch = buffer[i];
				if ((ch == '\\') || (ch == '/')) {
					buffer[i + 1] = 0;
					buffer = realloc(buffer, i + 2);
					return buffer;
				}
			}
			// no directory, realloc to empty string
			buffer = realloc(buffer, 1);
			buffer[0] = 0;
			return buffer;
		}
	}
}

char * concat(const char * a, const char * b) {
	size_t lenA = strlen(a);
	char * fin = malloc(lenA + strlen(b) + 1);
	strcpy(fin, a);
	strcpy(fin + lenA, b);
	return fin;
}

void luaboot() {
	void * dll = LoadLibraryA("lua51");
	if (!dll) {
		MessageBoxA(NULL, "moonboot failed to find LuaJIT (lua51.dll)", "moonboot diagnostic", 0);
		ExitProcess(1);
	}

	newstate_t luaL_newstate = GetProcAddress(dll, "luaL_newstate");
	openlibs_t luaL_openlibs = GetProcAddress(dll, "luaL_openlibs");
	loadfile_t luaL_loadfile = GetProcAddress(dll, "luaL_loadfile");
	pcall_t lua_pcall = GetProcAddress(dll, "lua_pcall");

	void * L = luaL_newstate();
	luaL_openlibs(L);
	char * dir = findDir();
	char * boot = concat(dir, "moonboot/bootloader.lua");
	free(dir);
	if (luaL_loadfile(L, boot)) {
		MessageBoxA(NULL, "moonboot failed to load 'moonboot/bootloader.lua'", "moonboot diagnostic", 0);
		ExitProcess(1);
	}
	free(boot);
	if (lua_pcall(L, 0, -1, 0)) {
		MessageBoxA(NULL, "moonboot failed to run bootloader", "moonboot diagnostic", 0);
		ExitProcess(1);
	}
}

BOOL WINAPI DllMain(HINSTANCE x, DWORD y, void * z) {
	switch (y)
	{
		case DLL_PROCESS_ATTACH:
			luaboot();
			break;
	}
	return TRUE;
}

