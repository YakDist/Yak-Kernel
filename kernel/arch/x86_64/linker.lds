OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

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

    /* Define a section to contain the Limine requests and assign it to its own PHDR */
    PROVIDE_HIDDEN(__kernel_limine_start = .);

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    PROVIDE_HIDDEN(__kernel_limine_end = .);

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    PROVIDE_HIDDEN(__kernel_text_start = .);

    .text : {
        *(.text .text.*)
    } :text

    PROVIDE_HIDDEN(__kernel_text_end = .);

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    PROVIDE_HIDDEN(__kernel_rodata_start = .);

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .init_array : ALIGN(16) {
        PROVIDE_HIDDEN(__init_array = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    	KEEP(*(.init_array .ctors))
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

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    PROVIDE_HIDDEN(__kernel_data_start = .);

    .data : {
        *(.data .data.*)
    } :data

    .init_node ALIGN(8) : {
    	PROVIDE_HIDDEN(__kernel_init_node_start = .);
    	KEEP(*(.init_node .init_node.*))
    	PROVIDE_HIDDEN(__kernel_init_node_end = .);
    } :data

    .init_stage ALIGN(8) : {
    	PROVIDE_HIDDEN(__kernel_init_stage_start = .);
    	KEEP(*(.init_stage .init_stage.*))
	PROVIDE_HIDDEN(__kernel_init_stage_end = .);
    } :data

    /* store percpu variables on a seperate page */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .percpu : {
	PROVIDE_HIDDEN(__kernel_percpu_start = .);
	KEEP(*(.percpu.cpudata))
	KEEP(*(.percpu .percpu.*))
	PROVIDE_HIDDEN(__kernel_percpu_end = .);
    } :data

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    PROVIDE_HIDDEN(__kernel_data_end = .);

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .stack (NOLOAD) : ALIGN(4K) {
        PROVIDE_HIDDEN(__kernel_entry_stack_guard_start = .);
        . += CONSTANT(MAXPAGESIZE);
        PROVIDE_HIDDEN(__kernel_entry_stack_start = .);
        . += 32K;
        PROVIDE_HIDDEN(__kernel_entry_stack_end = .);
    } :data

    /DISCARD/ : {
        *(.note .note.*)
    }
}
