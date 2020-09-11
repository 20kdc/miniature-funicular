#include <windows.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
	MessageBoxA(NULL, "test.exe code has run.", "test.exe", 0);
	// generate the required import symbol for mod.exe to work
	LoadLibraryA("this_is_not_a_valid_library");
}

