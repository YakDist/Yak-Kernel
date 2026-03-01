OUTPUT_FORMAT(elf64-x86-64)

ENTRY(exe_entry)

PHDRS
{
	limine_requests PT_LOAD;
	text PT_LOAD;
	rodata PT_LOAD;
	data PT_LOAD;
}

SECTIONS
{
	. = 0xffffffff80000000;

	.limine_requests : {
		KEEP(*(.limine_requests_start))
		KEEP(*(.limine_requests))
		KEEP(*(.limine_requests_end))
	} :limine_requests

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.text : {
		*(.text .text.*)
	} :text

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.rodata : {
		*(.rodata .rodata.*)
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN(__init_array_end = .);
	} :rodata

	.note.gnu.build-id : {
		*(.note.gnu.build-id)
	} :rodata

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.data : {
		*(.data .data.*)
	} :data

	/* Store the PERCPU area on a new page */
	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.percpu : {
		PROVIDE_HIDDEN(__percpu_start = .);
		KEEP(*(.percpu .percpu.*))
		PROVIDE_HIDDEN(__percpu_end = .);
	} :data

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.bss : {
		*(.bss .bss.*)
		*(COMMON)
	} :data

	/* Discard .note.* and .eh_frame* since they may cause issues on some hosts. */
	/DISCARD/ : {
		*(.eh_frame*)
		*(.note .note.*)
	}
}
