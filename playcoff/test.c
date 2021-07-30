#include "playcoff_sys.h"

int nya = 4321;
int bssExample;
char bssExampleTwo[0xA0];
int nameWof;
int zomf();

int meow() {
	return zomf();
}

int zomf() {
	playcoff_sys.printf("Hello from portable code, OS %i\n", playcoff_sys.platformType);
	return nya;
}

