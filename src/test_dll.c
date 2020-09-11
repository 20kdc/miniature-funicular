#include <windows.h>
#include <stdlib.h>

BOOL WINAPI DllMain(HINSTANCE x, DWORD y, void * z) {
	switch (y)
	{
		case DLL_PROCESS_ATTACH:
			MessageBoxA(NULL, "test.dll was attached to the process successfully.", "test.dll", 0);
			break;
	}
	return TRUE;
}

