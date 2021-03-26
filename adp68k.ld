/*OUTPUT_FORMAT(binary)*/
OUTPUT_ARCH(m68k)

SECTIONS
{
 .exception_table	0x00000000 : AT(0x00000000) { LONG(0x00000100) LONG(_start) }
 .control_block		0x00000100 : AT(0x00000100) { *(.control_block) }
 .stuff			0x00000200 : AT(0x00000200) { *(.data) *(.rodata) *(.text) }
 .padding		0x00001FFF : AT(0x00001FFF) { BYTE(0x00) }
 .dummy ADDR(.stuff) : AT(LOADADDR(.stuff)) { *(.bss) }
}

