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

	PROVIDE_HIDDEN(__kernel_limine_start = .);

	.limine_requests : {
		KEEP(*(.limine_requests_start))
		KEEP(*(.limine_requests))
		KEEP(*(.limine_requests_end))
	} :limine_requests

	PROVIDE_HIDDEN(__kernel_limine_end = .);

	. = ALIGN(4K);

	PROVIDE_HIDDEN(__kernel_text_start = .);

	.text : {
		*(.text .text.*)
	} :text

	PROVIDE_HIDDEN(__kernel_text_end = .);

	. = ALIGN(4K);

	PROVIDE_HIDDEN(__kernel_rodata_start = .);

	.rodata : {
		*(.rodata .rodata.*)
	} :rodata

	.init_array : ALIGN(16) {
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN(__init_array_end = .);
	} :rodata

	.note.gnu.build-id : {
		*(.note.gnu.build-id)
	} :rodata

	.note : {
		*(.note .note.*)
	} :rodata

	.eh_frame_hdr : {
		*(.eh_frame_hdr)
	} :rodata

	.eh_frame : {
		*(.eh_frame)
	} :rodata

	PROVIDE_HIDDEN(__kernel_rodata_end = .);

	. = ALIGN(4K);

	PROVIDE_HIDDEN(__kernel_data_start = .);

	.data : {
		*(.data .data.*)
	} :data

	/* Store the PERCPU area on a new page */
	. = ALIGN(4K);

	.percpu : {
		PROVIDE_HIDDEN(__percpu_start = .);
		KEEP(*(.percpu.cpudata))
		KEEP(*(.percpu .percpu.*))
		PROVIDE_HIDDEN(__percpu_end = .);
	} :data

	. = ALIGN(4K);

	.bss (NOLOAD) : {
		*(.bss .bss.*)
		*(COMMON)
	} :data

	PROVIDE_HIDDEN(__kernel_data_end = .);

	. = ALIGN(4K);

	.stack (NOLOAD) : ALIGN(4K) {
		PROVIDE_HIDDEN(__kernel_entry_stack_guard_bottom = .);
		. += 4K;
		PROVIDE_HIDDEN(__kernel_entry_stack_bottom = .);
		. += 16K;
		PROVIDE_HIDDEN(__kernel_entry_stack_top = .);
	} :data

	/DISCARD/ : {
		*(.comment)
	}
}
