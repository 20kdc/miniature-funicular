// NOTE: Ideally this will run natively on Linux as well.
// That said don't expect it to be *endian-independent*.

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// -- global file --

FILE * file;

void closeFile() {
	fclose(file);
}

// -- structures --

typedef struct {
	char name[8];
	uint32_t virtualSize;
	uint32_t rva;
	uint32_t rawDataSize;
	uint32_t fileAddress;
	uint32_t ignoreMe1;
	uint32_t ignoreMe2;
	uint32_t ignoreMe3;
	uint32_t characteristics;
} PE_SH_t;

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
GET_PUT_PAIR(PE_SH_t, getPESH, setPESH)

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

void fsPut(long at) {
	int moo = 0;
	while (1) {
		char c = getU8(at);
		if (!c)
			break;
		moo++;
		putchar(c);
		at++;
		// stupidity insurance
		if (moo == 0x40)
			break;
	}
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

void w(char c) {
	fputc(c, file);
}

void w32(uint32_t x) {
	fwrite(&x, sizeof(uint32_t), 1, file);
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
	// 1. Calculate injection details
	//  a. RVA/FA to create section at
	//  b. the location of LoadLibraryA in the import table (RPG_RT always uses it)
	// 2. Create the Injected Section (.vahlen) with the bootstrap code
	// 3. Laugh evilly as basically no sane injector or code patch will defeat this bootstrap system
	//  (unless they checksum the EXE, in which case, patch that out!)

	// Part 1: Calculate injection details
	// Scan section headers
	uint32_t newSectionRVA = 0;
	uint32_t newSectionFA = 0;
	uint32_t importSectionRVA = 0;
	uint32_t importSectionFA = 0;
	for (int i = 0; i < sectionCount; i++) {
		long target = sectionHeader + (i * sizeof(PE_SH_t));
		PE_SH_t targetHdr = getPESH(target);

		if ((importRVA >= targetHdr.rva) && (importRVA < (targetHdr.rva + targetHdr.virtualSize))) {
			importSectionRVA = targetHdr.rva;
			importSectionFA = targetHdr.fileAddress;
		}

		uint32_t virtualExt = targetHdr.rva + targetHdr.virtualSize;
		if (virtualExt & 0xFFF)
			virtualExt = (virtualExt & ~0xFFF) + 0x1000;
		if (newSectionRVA < virtualExt)
			newSectionRVA = virtualExt;

		uint32_t rawExt = targetHdr.fileAddress + targetHdr.rawDataSize;
		if (rawExt & 0xFFF)
			rawExt = (rawExt & ~0xFFF) + 0x1000;
		if (newSectionFA < rawExt)
			newSectionFA = rawExt;
	}
	// Finish up transform
	if (!importSectionRVA) {
		puts("Unable to continue: unable to find import table");
		return 1;
	}
	printf("Import RVA: %x\n", importRVA);
	printf("Import Section RVA: %x\n", importSectionRVA);
	printf("\nFinding LoadLibraryA...\n");
	uint32_t importTransform = importSectionFA - importSectionRVA;
	// Find LoadLibraryA import
	uint32_t importDirEntry = importRVA + importTransform;
	uint32_t loadLibraryARVA = 0;
	while (1) {
		uint32_t addrTableRVA = getU32(importDirEntry + 0x10);
		if (!addrTableRVA)
			break;
		printf(" ");
		int isDefinitelyKernel32 = 0;
		long libName = getU32(importDirEntry + 0x0C) + importTransform;
		fsPut(libName);
		if (fsMatch(libName, "KERNEL32.dll")) {
			isDefinitelyKernel32 = 1;
			printf(" (definitely kernel32)");
		}
		printf("\n");
		while (1) {
			long name = getU32(addrTableRVA + importTransform);
			if (!name)
				break;
			printf("  %lx ", name);
			int isLLA = 0;
			if ((name == 0x7C801D77) && isDefinitelyKernel32) {
				isLLA = 1;
				printf("[LoadLibraryA ordinal]");
			} else {
				name += 2 + importTransform;
				fsPut(name);
				if (fsMatch(name, "LoadLibraryA")) {
					isLLA = 1;
				}
			}
			if (isLLA) {
				loadLibraryARVA = addrTableRVA;
				printf(" <- (RVA %x)\n", addrTableRVA);
			} else {
				printf("\n");
			}
			addrTableRVA += 4;
		}
		importDirEntry += 0x14;
	}
	if (!loadLibraryARVA) {
		puts("Unable to continue: unable to find LoadLibraryA import");
		return 1;
	}
	printf("\n");
	// Part 2: Create the Injected Section
	// Change entry point and image size
	uint32_t entryPointRVA = getU32(optHeader + 0x10);
	setU32(optHeader + 0x10, newSectionRVA);
	setU32(optHeader + 0x38, newSectionRVA + 0x1000);
	// Write section header
	setU16(peHeader + 0x02, sectionCount + 1);
	long newSection = sectionHeader + (sectionCount * sizeof(PE_SH_t));
	PE_SH_t newSectionHdr;
	strcpy(newSectionHdr.name, ".vahlen");
	newSectionHdr.virtualSize = 0x1000;
	newSectionHdr.rva = newSectionRVA;
	newSectionHdr.rawDataSize = 0x1000;
	newSectionHdr.fileAddress = newSectionFA;
	newSectionHdr.ignoreMe1 = 0;
	newSectionHdr.ignoreMe2 = 0;
	newSectionHdr.ignoreMe3 = 0;
	newSectionHdr.characteristics = 0x60000020;
	setPESH(newSection, newSectionHdr);
	// Prepare to write section contents
	padTo(newSectionFA);
	// Write section contents (KEEP IN SYNC WITH LISTING)
	w(0xE8); w32(0);
	w(0x81); w(0x2C); w(0x24); w32(0x05);
	w(0x8B); w(0x04); w(0x24);
	w(0x05); w32(0x2A);
	w(0x50);
	w(0x8B); w(0x44); w(0x24); w(0x04);
	w(0x05); w32(loadLibraryARVA - newSectionRVA);
	w(0x8B); w(0x00);
	w(0xFF); w(0xD0);
	w(0x58);
	w(0x05); w32(entryPointRVA - newSectionRVA);
	w(0xFF); w(0xE0);
	// Add text
	fputs(argv[1], file);
	w(0);
	// Finish
	padTo(newSectionFA + 0x1000);
	return 0;
}

