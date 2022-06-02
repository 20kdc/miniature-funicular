#include "nootnoot.h"

int main(int argc, char ** argv) {
	nootnoot();
	MessageBoxA(NULL, "first impact.", "ceboot.exe", 0);
	say("CEBOOT: Performing ShellExecute");
	ShellExecuteA(NULL, "open", "C:\\CE\\cheatengine-i386.exe", "", NULL, 0);
	say("CEBOOT: Finishing up");
	MessageBoxA(NULL, "second impact.", "ceboot.exe", 0);
	return TRUE;
}

