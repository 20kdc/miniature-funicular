#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "playcoff_fmt.h"
#include "playcoff_sys.h"

extern playcoff_sys_t playcoff_sys;

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
		if (ch->magic != PLAYCOFF_FMT_MAGIC_I386) {
			fprintf(stderr, "Bad magic.\n");
			return 1;
		}
		if (ch->sectionCount != 1) {
			fprintf(stderr, "Only one section is allowed.\n");
			return 1;
		}
		playcoff_fmt_section_t * se = ch->sections;
		if (ch->optSize != 0) {
			fprintf(stderr, "This is actually an executable.\n");
			return 1;
		}
		playcoff_fmt_rel_t * relocs = (playcoff_fmt_rel_t *) (data + se->relocsPtr);
		playcoff_fmt_symbol_t * syms = (playcoff_fmt_symbol_t *) (data + ch->symbolsPtr);
		fprintf(stderr, "OK: %s\n", se->sectionName);
		char * executableArea = playcoff_sys.allocateRWX(se->rawDataSize);
		if (!executableArea) {
			fprintf(stderr, "Failed executable area allocation.\n");
			return 1;
		}
		memcpy(executableArea, data + se->rawDataPtr, se->rawDataSize);
		for (uint16_t r = 0; r < se->relocsCount; r++) {
			fprintf(stderr, "Relocation type %i\n", relocs[r].type);
			playcoff_fmt_symbol_t * sym = syms + relocs[r].symbol;
			char * value = executableArea + sym->value;
			uintptr_t* rvaPtr = (uintptr_t*) (executableArea + relocs[r].rva);
			fprintf(stderr, " command target of EA at %p symbol at %p write at %p\n", executableArea, value, rvaPtr);
			switch (relocs[r].type) {
				case 6:
				{
					*rvaPtr += (uintptr_t) value;
					break;
				}
				case 20:
				{
					*rvaPtr += (uintptr_t) value;
					*rvaPtr -= ((uintptr_t) rvaPtr) + 4;
					fprintf(stderr, "R = %lx\n", (unsigned long) *rvaPtr);
					break;
				}
				default:
				{
					fprintf(stderr, "Unknown relocation type %i in %i.\n", relocs[r].type, r);
					return 1;
				}
			}
		}
		fprintf(stderr, "About to enter.\n");
		int (*execFn)() = (void *) executableArea;
		printf("%i\n", execFn());
	}
	return 0;
}

