#include "playcoff_sys.h"
#include "playcoff_fmt.h"

int nya = 4321;
int bssExample;
char bssExampleTwo[0xA0];
int nameWof;
int zomf();

int _main(int argc, char ** argv, char ** env) {
	return zomf(argv[1]);
}

int zomf(char * test) {
	playcoff_sys.printf("Hello from portable code in %s, OS %i\n", test, playcoff_sys.platformType);
	return nya;
}

