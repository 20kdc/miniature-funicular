extern "C" {
#include "playcoff_sys.h"
#include "playcoff_fmt.h"
}

class Example {
public:
	Example() {
		playcoff_sys.printf("Hello from portable code in OS %i\n", playcoff_sys.platformType);
	}
	~Example() {
		playcoff_sys.printf("Goodbye from portable code in OS %i\n", playcoff_sys.platformType);
	}
};

Example ex;

extern "C" int _main(int argc, char ** argv, char ** env) {
	return 0;
}

