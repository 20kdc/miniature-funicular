#include <windows.h>
#include <stdlib.h>

BOOL WINAPI DllMain(HINSTANCE x, DWORD y, void * z) {
	switch (y)
	{
		case DLL_PROCESS_ATTACH:
			MessageBoxA(NULL, "test.dll was attached to the process successfully.", "test.dll", 0);
			break;
	}
	LoadLibraryA("this_library_also_does_not_exist");
	return TRUE;
}

