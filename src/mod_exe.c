// NOTE: Ideally this will run natively on Linux as well.
// That said don't expect it to be *endian-independent*.

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// -- global file --

FILE * file;

void closeFile() {
	fclose(file);
}

// -- file get/put assist --

#define GET_PUT_PAIR(type, get, put) \
type get(long at) { \
	type val; \
	fseek(file, at, SEEK_SET); \
	fread(&val, sizeof(type), 1, file); \
	return val; \
} \
void put(long at, type val) { \
	fseek(file, at, SEEK_SET); \
	fwrite(&val, sizeof(type), 1, file); \
}

GET_PUT_PAIR(uint32_t, getU32, setU32)
GET_PUT_PAIR(uint16_t, getU16, setU16)
GET_PUT_PAIR(uint8_t, getU8, setU8)

int fsMatch(long at, const char * text) {
	while (1) {
		if (getU8(at) != (uint8_t) *text)
			return 0;
		if (!*text)
			break;
		at++;
		text++;
	}
	return 1;
}

void padTo(long at) {
	fseek(file, 0, SEEK_END);
	long where = ftell(file);
	while (where < at) {
		fputc(0, file);
		where++;
	}
	fseek(file, at, SEEK_SET);
}

// --

int main(int argc, char ** argv) {
	if (argc != 3) {
		puts("mod LIBNAME EXE");
		puts("Modifies a 32-bit EXE to cause a DLL to be loaded immediately before the entrypoint.");
		puts("This allows injecting a DLL while reducing the chance of conflicts with any other injectors so long as they don't check EXE hashes.");
		puts("This is primarily meant as a universal way to inject code into RPG Maker 2000/2003 games (which tend to have many injectors of their own)");
		return 1;
	}
	file = fopen(argv[2], "r+b");
	if (!file) {
		puts("Unable to open file");
		return 1;
	}
	atexit(closeFile);

	// IMAGE_FILE_HEADER
	long peHeader = getU32(0x3C) + 4;
	if (getU32(peHeader - 4) != 0x4550) {
		puts("not a valid target");
		return 1;
	}
	int sectionCount = getU16(peHeader + 0x02);
	int optHeaderSize = getU16(peHeader + 0x10);
	// IMAGE_OPTIONAL_HEADER
	long optHeader = peHeader + 0x14;
	// .DataDirectory
	long optHeaderDD = optHeader + 0x60;
	long importRVA = getU32(optHeaderDD + 8);
	long sectionHeader = optHeader + optHeaderSize;

	// Ok, so now we've roughly confirmed it's correct,
	//  we need to:
	// 1. Confirm the location of LoadLibraryA in the import table (RPG_RT always uses it)
	// 2. Create the Injected Section (.vahlen) with the bootstrap code
	// 3. Laugh evilly as basically no sane injector or code patch will defeat this bootstrap system
	//  (unless they checksum the EXE, in which case, patch that out!)

	// Part 2: Create the Injected Section
	// Calculate injection details
	uint32_t newSectionRVA = 0;
	uint32_t newSectionFA = 0;
	for (int i = 0; i < sectionCount; i++) {
		long target = sectionHeader + (i * 40);
		uint32_t virtualExt = getU32(target + 0x08) + getU32(target + 0x0C);
		if (virtualExt & 0xFFF)
			virtualExt = (virtualExt & ~0xFFF) + 0x1000;
		if (newSectionRVA < virtualExt)
			newSectionRVA = virtualExt;

		uint32_t rawExt = getU32(target + 0x10) + getU32(target + 0x14);
		if (rawExt & 0xFFF)
			rawExt = (rawExt & ~0xFFF) + 0x1000;
		if (newSectionFA < rawExt)
			newSectionFA = rawExt;
	}
	// Change entry point and image size
	uint32_t entryPoint = getU32(optHeader + 0x10);
	setU32(optHeader + 0x10, newSectionRVA);
	setU32(optHeader + 0x38, newSectionRVA + 0x1000);
	// Write section header
	setU16(peHeader + 0x02, sectionCount + 1);
	long newSection = sectionHeader + (sectionCount * 40);
	setU32(newSection + 0x00, 'hav.');
	setU32(newSection + 0x04, '\x00nel');
	setU32(newSection + 0x08, 0x1000); // virtual size
	setU32(newSection + 0x0C, newSectionRVA);
	setU32(newSection + 0x10, 0x1000); // raw data size
	setU32(newSection + 0x14, newSectionFA);
	setU32(newSection + 0x18, 0);
	setU32(newSection + 0x1C, 0);
	setU32(newSection + 0x20, 0);
	setU32(newSection + 0x24, 0x60000020);
	// Prepare to write section contents
	padTo(newSectionFA);
	// Write section contents
	fputc(0xC3, file);
	// Finish
	padTo(newSectionFA + 0x1000);
	puts("ACTUAL TASK NOT YET IMPLEMENTED: the program will instead immediately exit");
	return 0;
}

