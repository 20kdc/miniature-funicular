#include "playcoff_fmt.h"
#include <stdio.h>
#include <string.h>

#define PLAYCOFF_FMT_SYMS playcoff_fmt_symbol_t * syms = PLAYCOFF_FMT_PTR(playcoff_fmt_symbol_t, obj, obj->symbolsPtr)

// The most important symbol utility.

static const char * playcoff_fmt_getSymbolName(const playcoff_fmt_head_t * obj, const playcoff_fmt_symbol_t * sym, char shortname[9]) {
	if (!sym->symbolName[0]) {
		// Long name.
		PLAYCOFF_FMT_SYMS;
		const char * strtab = (const char *) (syms + obj->symbolsCount);
		return strtab + *((uint32_t *) (sym->symbolName + 4));
	} else {
		// Short name.
		memcpy(shortname, sym->symbolName, 8);
		shortname[8] = 0;
		return shortname;
	}
}

// Layout common

static const char * playcoff_fmt_basicChecks(const playcoff_fmt_head_t * obj) {
	if (obj->magic != PLAYCOFF_FMT_MAGIC_I386)
		return "Bad magic.";
	if (obj->optSize != 0)
		return "This is actually an executable.";
	return NULL;
}
static uint32_t playcoff_fmt_getPaddingAny(uint32_t align, uint32_t where) {
	uint32_t rem = where & (align - 1);
	if (rem != 0)
		return align - rem;
	return 0;
}
static uint32_t playcoff_fmt_getPadding(const playcoff_fmt_section_t * se, uint32_t where) {
	return playcoff_fmt_getPaddingAny(4, where);
}

// Returns 1 if the symbol should be placed at symbolValue.
// Alters symbolValue, totalSize.
static int playcoff_fmt_symbolLayout(const playcoff_fmt_head_t * obj, const playcoff_fmt_symbol_t * sym, uint32_t * symbolValue, uint32_t * totalSize) {
	switch (sym->sectionNumber) {
		case PLAYCOFF_FMT_SN_BSS_URS:
		{
			// BSS or unresolved.
			if (sym->value) {
				// BSS.
				*symbolValue = (*totalSize) + playcoff_fmt_getPaddingAny(4, *totalSize);
				*totalSize = (*symbolValue) + sym->value;
				return 1;
			}
			break;
		}
		case PLAYCOFF_FMT_SN_ABS:
		{
			// Nothing to do.
			break;
		}
		default:
		{
			if (sym->sectionNumber <= obj->sectionCount) {
				// Internal reference
				*symbolValue = obj->sections[sym->sectionNumber - 1].rva + sym->value;
				return 1;
			}
			// Out of range section, don't trust it's not metadata of some kind
			break;
		}
	}
	return 0;
}

// Layout handling

static size_t playcoff_fmt_getMinimumAllocation(const playcoff_fmt_head_t * obj) {
	// Run the sanity checks, but don't report errors.
	if (playcoff_fmt_basicChecks(obj))
		return 0;
	// Sections
	size_t totalSize = 0;
	for (uint16_t s = 0; s < obj->sectionCount; s++) {
		const playcoff_fmt_section_t * se = obj->sections + s;
		totalSize += playcoff_fmt_getPadding(se, totalSize);
		totalSize += se->rawDataSize;
	}
	// Symbols
	PLAYCOFF_FMT_SYMS;
	for (uint16_t s = 0; s < obj->symbolsCount; s++) {
		const playcoff_fmt_symbol_t * sym = syms + s;
		size_t scratch;
		playcoff_fmt_symbolLayout(obj, sym, &scratch, &totalSize);
		s += sym->numberOfAuxSymbols;
	}
	// Done!
	return totalSize;
}

static int playcoff_fmt_layout(playcoff_fmt_head_t * obj, uint32_t address) {
	const char * basicChecks = playcoff_fmt_basicChecks(obj);
	if (basicChecks) {
		fprintf(stderr, "%s\n", basicChecks);
		return 1;
	}
	uint32_t baseAddress = address;
	// Sections
	for (uint16_t s = 0; s < obj->sectionCount; s++) {
		playcoff_fmt_section_t * se = obj->sections + s;
		address += playcoff_fmt_getPadding(se, address);
		se->rva = address;
		address += se->rawDataSize;
	}
	// Symbols
	PLAYCOFF_FMT_SYMS;
	for (uint16_t s = 0; s < obj->symbolsCount; s++) {
		playcoff_fmt_symbol_t * sym = syms + s;
		uint32_t scratch;
		if (playcoff_fmt_symbolLayout(obj, sym, &scratch, &address)) {
			sym->sectionNumber = PLAYCOFF_FMT_SN_ABS;
			sym->value = scratch;
		}
		s += sym->numberOfAuxSymbols;
	}
	// Done!
	return 0;
}

// Resolver

static int playcoff_fmt_resolve(playcoff_fmt_head_t * obj, void * data, int (*resolver)(void * data, const char * symbol, uint32_t * resolved)) {
	PLAYCOFF_FMT_SYMS;
	char shortname[9];
	for (uint16_t s = 0; s < obj->symbolsCount; s++) {
		playcoff_fmt_symbol_t * sym = syms + s;
		if (sym->sectionNumber == PLAYCOFF_FMT_SN_BSS_URS) {
			const char * name = playcoff_fmt_getSymbolName(obj, sym, shortname);
			uint32_t val;
			if (resolver(data, name, &val))
				return 1;
			sym->sectionNumber = PLAYCOFF_FMT_SN_ABS;
			sym->value = val;
		}
		s += sym->numberOfAuxSymbols;
	}
	return 0;
}
static int playcoff_fmt_resolveAlwaysFail(void * data, const char * symbol, uint32_t * resolved) {
	return 1;
}

// Loading

static int playcoff_fmt_load(playcoff_fmt_head_t * obj) {
	char * data = (char *) obj;
	PLAYCOFF_FMT_SYMS;
	for (uint16_t s = 0; s < obj->sectionCount; s++) {
		playcoff_fmt_section_t * se = obj->sections + s;
		memcpy((void *) se->rva, ((char *) obj) + se->rawDataPtr, se->rawDataSize);
		playcoff_fmt_rel_t * relocs = (playcoff_fmt_rel_t *) (data + se->relocsPtr);
		for (uint16_t r = 0; r < se->relocsCount; r++) {
			// fprintf(stderr, "Relocation type %i\n", relocs[r].type);
			playcoff_fmt_symbol_t * sym = syms + relocs[r].symbol;
			if (sym->sectionNumber != PLAYCOFF_FMT_SN_ABS) {
				char dbg[9];
				const char * chr = playcoff_fmt_getSymbolName(obj, sym, dbg);
				fprintf(stderr, "Attempted to relocate unresolved symbol %s.\n", chr);
				return 1;
			}
			char * value = (char *) (uintptr_t) sym->value;
			uintptr_t* rvaPtr = (uintptr_t*) (se->rva + relocs[r].rva);
			// fprintf(stderr, " command target of EA at %p symbol at %p write at %p\n", target, value, rvaPtr);
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
					// fprintf(stderr, "R = %lx\n", (unsigned long) *rvaPtr);
					break;
				}
				default:
				{
					fprintf(stderr, "Unknown relocation type %i in %i.\n", relocs[r].type, r);
					return 1;
				}
			}
		}
	}
	return 0;
}

// Symbol Utilities

static playcoff_fmt_symbol_t * playcoff_fmt_symbolByName(playcoff_fmt_head_t * obj, const char * symbol) {
	PLAYCOFF_FMT_SYMS;
	char shortname[9];
	for (uint16_t s = 0; s < obj->symbolsCount; s++) {
		playcoff_fmt_symbol_t * sym = syms + s;
		if (sym->storageClass == PLAYCOFF_FMT_SC_EXTERNAL) {
			const char * name = playcoff_fmt_getSymbolName(obj, sym, shortname);
			if (!strcmp(name, symbol))
				return sym;
		}
		s += sym->numberOfAuxSymbols;
	}
	return NULL;
}

// Table

playcoff_fmt_t playcoff_fmt = {
	playcoff_fmt_getMinimumAllocation,
	playcoff_fmt_layout,
	playcoff_fmt_resolve,
	playcoff_fmt_resolveAlwaysFail,
	playcoff_fmt_load,
	playcoff_fmt_getSymbolName,
	playcoff_fmt_symbolByName
};


