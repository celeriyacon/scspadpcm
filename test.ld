/*OUTPUT_FORMAT(binary)*/
OUTPUT_ARCH(sh2)

SECTIONS
{
 .init		0x06004000 : AT(0x00000000) { *(.init) }
 .text					  : { *(.text) }
 .data					  : { *(.rodata) *(.data) *(.bss) }
}

