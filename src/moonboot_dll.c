#include <windows.h>
#include <stdlib.h>

typedef void * (*newstate_t)();
typedef void (*openlibs_t)(void * L);
typedef int (*loadfile_t)(void * L, const char * filename);
typedef int (*pcall_t)(void * L, int nargs, int nresults, int errfunc);

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
	if (luaL_loadfile(L, "moonboot/bootloader.lua")) {
		MessageBoxA(NULL, "moonboot failed to load 'moonboot/bootloader.lua'", "moonboot diagnostic", 0);
		ExitProcess(1);
	}
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

