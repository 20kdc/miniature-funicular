#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "playcoff_fmt.h"
#include "playcoff_sys.h"

static playcoff_fmt_head_t * playcoff_loader_main_module;

static int loader_resolver(void * data, const char * symbol, uint32_t * resolved) {
	if (!strcmp(symbol, "_playcoff_sys")) {
		*resolved = (uint32_t) &playcoff_sys;
		return 0;
	}
	if (!strcmp(symbol, "_playcoff_fmt")) {
		*resolved = (uint32_t) &playcoff_fmt;
		return 0;
	}
	if (!strcmp(symbol, "_playcoff_loader_main_module")) {
		*resolved = (uint32_t) playcoff_loader_main_module;
		return 0;
	}
	fprintf(stderr, "Failed to find symbol %s during bootstrap.\n", symbol);
	return 1;
}

int main(int argc, char ** argv, char ** env) {
	if (argc < 2) {
		fprintf(stderr, "Needs initial module.\n");
		return 1;
	}
	FILE * f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "Failed open.\n");
		return 1;
	}
	fseek(f, 0, SEEK_END);
	long res = ftell(f);
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
	playcoff_fmt_symbol_t * main = playcoff_fmt.symbolByName(ch, "__main");
	if (!main) {
		fprintf(stderr, "No __main symbol.\n");
		return 1;
	}
	if (main->sectionNumber != PLAYCOFF_FMT_SN_ABS) {
		fprintf(stderr, "__main symbol not absolute.\n");
		return 1;
	}
	int (*execFn)(int, char **, char **) = (void *) main->value;
	return execFn(argc, argv, env);
}

