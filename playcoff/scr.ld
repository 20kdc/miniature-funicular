OUTPUT_FORMAT (pe-i386)

SECTIONS {
	.all : {
		*(.text)
		*(.data)
		*(.rodata)
		*(.bss)
	}
	/DISCARD/ : {
		*(.llvm_addrsig)
	}
}

