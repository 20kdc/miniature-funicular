#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "playcoff_fmt.h"
#include "playcoff_sys.h"

static int loader_resolver(void * data, const char * symbol, uint32_t * resolved) {
	if (!strcmp(symbol, "_playcoff_sys")) {
		*resolved = (uint32_t) &playcoff_sys;
		return 0;
	}
	if (!strcmp(symbol, "_playcoff_fmt")) {
		*resolved = (uint32_t) &playcoff_fmt;
		return 0;
	}
	return 1;
}

int main(int argc, char ** argv) {
	if (argc != 2) {
		fprintf(stderr, "Needs argument.\n");
		return 1;
	}
	FILE * f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "Failed open.\n");
		return 1;
	}
	fseek(f, 0, SEEK_END);
	long res = ftell(f);
	{
		char data[res];
		fseek(f, 0, SEEK_SET);
		if (fread(data, (size_t) res, 1, f) != 1) {
			fprintf(stderr, "Failed read.\n");
			return 1;
		}
		playcoff_fmt_head_t * ch = (playcoff_fmt_head_t *) data;
		fprintf(stderr, "MinAlloc.\n");
		size_t minAlloc = playcoff_fmt.getMinimumAllocation(ch);
		if (!minAlloc) {
			fprintf(stderr, "There's nothing there.\n");
			return 1;
		}
		fprintf(stderr, "Alloc.\n");
		char * executableArea = playcoff_sys.allocateRWX(minAlloc);
		if (!executableArea) {
			fprintf(stderr, "Failed executable area allocation.\n");
			return 1;
		}
		memset(executableArea, 0, minAlloc);
		fprintf(stderr, "Layout.\n");
		if (playcoff_fmt.layout(ch, (uint32_t) executableArea)) {
			fprintf(stderr, "Failed layout.\n");
			return 1;
		}
		fprintf(stderr, "Resolve.\n");
		if (playcoff_fmt.resolve(ch, NULL, loader_resolver)) {
			fprintf(stderr, "Failed resolve.\n");
			return 1;
		}
		fprintf(stderr, "Load.\n");
		if (playcoff_fmt.load(ch)) {
			fprintf(stderr, "Failed load.\n");
			return 1;
		}
		fprintf(stderr, "About to enter.\n");
		int (*execFn)() = (void *) executableArea;
		printf("%i\n", execFn());
	}
	return 0;
}

