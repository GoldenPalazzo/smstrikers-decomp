#include "PowerPC_EABI_Support/MetroTRK/trk.h"

// clang-format off

// TRKAccessFile = .text:0x80228F70; // type:label scope:global
// fn_80228F70 = .text:0x80228F70; // type:function size:0x8
#pragma function_align 16
ASM void fn_80228F70() 
{
#ifdef __MWERKS__ 
	twi 31, r0, 0
	blr
#endif
}
#pragma function_align 4

// TRKOpenFile = .text:0x80228F78; // type:label scope:global
// fn_80228F78 = .text:0x80228F78; // type:function size:0x8
ASM void fn_80228F78() 
{
#ifdef __MWERKS__ 
	twi 31, r0, 0
	blr
#endif
}

// TRKCloseFile = .text:0x80228F80; // type:label scope:global
// fn_80228F80 = .text:0x80228F80; // type:function size:0x8
ASM void fn_80228F80() 
{
#ifdef __MWERKS__ 
	twi 31, r0, 0
	blr
#endif
}

// TRKPositionFile = .text:0x80228F88; // type:label scope:global
// fn_80228F88 = .text:0x80228F88; // type:function size:0x8
ASM void fn_80228F88()
{
#ifdef __MWERKS__ 
	twi 31, r0, 0
	blr
#endif 
}

// clang-format on
