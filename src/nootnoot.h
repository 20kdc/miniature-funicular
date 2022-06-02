#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

// logon experience customization assistant

FILE * com0;

int nootnoot_inited = 0;

HDESK nootnoot_desktop;

void say(const char * text) {
	if (!com0)
		com0 = fopen("COM1", "wb");
	OutputDebugStringA(text);
	if (com0) {
		fputs(text, com0);
		fputs("\r\n", com0);
		fflush(com0);
	}
}

void nootnoot() {
	if (!nootnoot_inited) {
		say("NOOTNOOT: Starting");
		// Either 0x02000000 is a secretive access right bit nobody knew about,
		//  or someone did a typo. Not going to bet either way.
		HWINSTA hi = OpenWindowStationA("WinSta0", TRUE, 0x02000000);
		if (hi) {
			SetProcessWindowStation(hi);
			say("NOOTNOOT: WINDOW STATION OPENED AND SET");
		} else {
			say("NOOTNOOT: WINDOW STATION OPEN FAILED");
		}
		say("NOOTNOOT: Setting desktop");
		nootnoot_desktop = OpenInputDesktop(0, TRUE, 0x02000000);
		if (!nootnoot_desktop)
			nootnoot_desktop = OpenDesktopA("Winlogon", 0, 0, 0x02000000);
		if (!nootnoot_desktop)
			say("NOOTNOOT: Or not, we don't have one");
		say("NOOTNOOT: We're done here");
		nootnoot_inited = 1;
	}
	if (nootnoot_desktop)
		SetThreadDesktop(nootnoot_desktop);
}

