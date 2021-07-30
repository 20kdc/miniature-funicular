OUTPUT_FORMAT (pe-i386)

SECTIONS {
	.all : {
		*(.text*)
		*(.data*)
		*(.rodata*)
		*(.rdata*)
		*(.bss*)
	}
	/DISCARD/ : {
		*(.llvm_addrsig)
	}
}

