#pragma once

#include <stddef.h>
#include <stdint.h>

#define PLAYCOFF_PACKED __attribute__((packed))

typedef struct PLAYCOFF_PACKED playcoff_fmt_section {
	// section
	char sectionName[8];
	uint32_t virtSize; // UNUSED IN OBJECT FORMAT
	uint32_t rva; // UNUSED IN OBJECT FORMAT - PLAYCOFF uses this as RVA
	uint32_t rawDataSize;
	uint32_t rawDataPtr;
	uint32_t relocsPtr;
	uint32_t linesPtr;
	uint16_t relocsCount;
	uint16_t lineNumbersCount;
	uint32_t sectionChars;
} playcoff_fmt_section_t;

typedef struct PLAYCOFF_PACKED playcoff_fmt_head {
	// main
	uint16_t magic;
	uint16_t sectionCount;
	uint32_t tds;
	uint32_t symbolsPtr;
	uint32_t symbolsCount;
	uint16_t optSize;
	uint16_t chars;
	struct playcoff_fmt_section sections[0];
} playcoff_fmt_head_t;

typedef struct PLAYCOFF_PACKED playcoff_fmt_symbol {
	char symbolName[8];
	uint32_t value;
	// see PLAYCOFF_FMT_SN_*
	uint16_t sectionNumber;
	uint16_t type;
	uint8_t storageClass;
	uint8_t numberOfAuxSymbols;
} playcoff_fmt_symbol_t;

typedef struct PLAYCOFF_PACKED playcoff_fmt_rel {
	// despite the name, this might be relative to section
	uint32_t rva;
	uint32_t symbol;
	uint16_t type;
} playcoff_fmt_rel_t;

#undef PLAYCOFF_PACKED

#define PLAYCOFF_FMT_MAGIC_I386 0x014C

// to be more precise: size = 0 means it's unresolved, otherwise it's BSS.
#define PLAYCOFF_FMT_SN_BSS_URS 0
// generally: SN - 1 = index
#define PLAYCOFF_FMT_SN_ABS 0xFFFF

#define PLAYCOFF_FMT_SN 0xFFFF

// 32-bit word added to the target word.
#define PLAYCOFF_FMT_REL_ABSOLUTE 6
// 32-bit word added to the target word, but then the target position, plus 4, subtracted from the target word.
#define PLAYCOFF_FMT_REL_RELATIVE 20

// Actual module

typedef struct {
	// Returns the minimum allocation needed to hold the given object's contents
	// Returns 0 on either failure or empty object. Just assume it's good and move on.
	size_t (*getMinimumAllocation)(const playcoff_fmt_head_t * obj);
	// Fixes symbols inside the object's symbol table to match a given target address.
	// They are modified to the 'absolute' section.
	// Will use the section 'rva' field to store RVAs.
	// Returns non-zero on failure. Resolver also returns non-zero on failure.
	int (*layout)(playcoff_fmt_head_t * obj, uint32_t address);
	// Resolves unresolved symbols in the object.
	// Returns non-zero on failure. Resolver also returns non-zero on failure.
	int (*resolve)(playcoff_fmt_head_t * obj, void * data, int (*resolver)(void * data, const char * symbol, uint32_t * resolved));
	// Assuming the above two have been completed, loads the object at the given address.
	// Returns non-zero on failure.
	int (*load)(playcoff_fmt_head_t * obj, uint32_t address);
	// Gets an external-linkage symbol by name, if present.
	playcoff_fmt_symbol_t * (*symbolByName)(playcoff_fmt_head_t * obj, const char * symbol);
} playcoff_fmt_t;


