#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef WINDOWS
#include <sys/mman.h>

char * mapRWX(size_t size) {
	char * executableArea = mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	// Translate to null-return.
	if (executableArea == MAP_FAILED)
		return NULL;
	return executableArea;
}

#else
#include <windows.h>

char * mapRWX(size_t size) {
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

#endif

#define PACKED __attribute__((packed))

typedef struct PACKED {
	// main
	uint16_t magic;
	uint16_t sections;
	uint32_t tds;
	uint32_t symbolsPtr;
	uint32_t nos;
	uint16_t optSize;
	uint16_t chars;
	// section
	char sectionName[8];
	uint32_t virtSize;
	uint32_t rva;
	uint32_t rawDataSize;
	uint32_t rawDataPtr;
	uint32_t relocsPtr;
	uint32_t linesPtr;
	uint16_t relocsCount;
	uint16_t lineNumbersCount;
	uint32_t sectionChars;
} c1_head_t;

typedef struct PACKED {
	char symbolName[8];
	uint32_t value;
	uint16_t sectionNumber;
	uint16_t type;
	uint8_t storageClass;
	uint8_t numberOfAuxSymbols;
} symbol_t;

typedef struct PACKED {
	uint32_t rva;
	uint32_t symbol;
	uint16_t type;
} rel_t;

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
		c1_head_t * ch = (c1_head_t *) data;
		if (ch->magic != 0x014C) {
			fprintf(stderr, "Bad magic.\n");
			return 1;
		}
		if (ch->sections != 1) {
			fprintf(stderr, "Only one section is allowed.\n");
			return 1;
		}
		if (ch->optSize != 0) {
			fprintf(stderr, "This is actually an executable.\n");
			return 1;
		}
		rel_t * relocs = (rel_t *) (data + ch->relocsPtr);
		symbol_t * syms = (symbol_t *) (data + ch->symbolsPtr);
		fprintf(stderr, "OK: %s\n", ch->sectionName);
		char * executableArea = mapRWX(ch->rawDataSize);
		if (!executableArea) {
			fprintf(stderr, "Failed executable area allocation.\n");
			return 1;
		}
		memcpy(executableArea, data + ch->rawDataPtr, ch->rawDataSize);
		for (uint16_t r = 0; r < ch->relocsCount; r++) {
			fprintf(stderr, "Relocation type %i\n", relocs[r].type);
			symbol_t * sym = syms + relocs[r].symbol;
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

