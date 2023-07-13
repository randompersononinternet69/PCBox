#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#ifndef INFINITY
#    define INFINITY (__builtin_inff())
#endif

#include <86box/86box.h>
#include "cpu.h"
#include <86box/timer.h>
#include "x86.h"
#include "x86_ops.h"
#include "x87.h"
#include "x86_flags.h"
#include <86box/dma.h>
#include <86box/io.h>
#include <86box/mem.h>
#include "386_common.h"
#include <86box/nmi.h>
#include <86box/pci.h>
#include <86box/pic.h>
#include <86box/gdbstub.h>
#include "codegen.h"

void
mem_add_ram_mapping(mem_mapping_t *mapping, uint32_t base, uint32_t size);

mem_mapping_t ram_map;

uint8_t* testrom = NULL;
const uint32_t romsize = 0x20000;

uint8_t read_mem_byte(uint32_t addr, void* p)
{
    addr &= 0x1ffff;
    return testrom[addr];
}

uint16_t read_mem_word(uint32_t addr, void* p)
{
    return read_mem_byte(addr, p) | (read_mem_byte(addr+1, p) << 8);
}

uint32_t read_mem_long(uint32_t addr, void* p)
{
    return read_mem_word(addr, p) | (read_mem_word(addr+2, p) << 16);
}

void write_mem_byte(uint32_t addr, uint8_t val, void* p)
{
    addr &= 0x1ffff;
    testrom[addr] = val;
}

void write_mem_word(uint32_t addr, uint16_t val, void* p)
{
    write_mem_byte(addr, val & 0xff, p);
    write_mem_byte(addr + 1, val >> 8, p);
}

void write_mem_long(uint32_t addr, uint32_t val, void* p)
{
    write_mem_word(addr, val & 0xffff, p);
    write_mem_word(addr + 2, val >> 16, p);
}

int main(int ac, char** av)
{
    pc_init(ac, av);
    pc_init_modules();

    cpu_f = cpu_get_family("generic_intel");
    cpu = 0;

    cpu_set();
    mem_init();
    hardresetx86();

    mem_size = 1 << 16;
    mem_reset();

    testrom = malloc(romsize);

    mem_mapping_add(&bios_mapping, 0xe0000, 0x20000, read_mem_byte, read_mem_word, read_mem_long, write_mem_byte, write_mem_word, write_mem_long, testrom, MEM_MAPPING_IS_ROM, NULL);
    mem_mapping_add(&bios_high_mapping, 0xfffe0000, 0x20000, read_mem_byte, read_mem_word, read_mem_long, write_mem_byte, write_mem_word, write_mem_long, testrom, MEM_MAPPING_IS_ROM, NULL);

    testrom[0x1fff0] = 0xea; // JMP FAR F000:0000
    testrom[0x1fff1] = 0x00;
    testrom[0x1fff2] = 0x00;
    testrom[0x1fff3] = 0x00;
    testrom[0x1fff4] = 0xf0;

    testrom[0x10000] = 0xbb; // MOV BX, 1000h
    testrom[0x10001] = 0x00;
    testrom[0x10002] = 0x10;
    testrom[0x10003] = 0x2e; // MOVUPS XMM0, CS:[BX]
    testrom[0x10004] = 0x0f;
    testrom[0x10005] = 0x10;
    testrom[0x10006] = 0x07;
    testrom[0x10007] = 0xf4; // HLT

    testrom[0x11000] = 0x00;
    testrom[0x11001] = 0x01;
    testrom[0x11002] = 0x02;
    testrom[0x11003] = 0x03;
    testrom[0x11004] = 0x04;
    testrom[0x11005] = 0x05;
    testrom[0x11006] = 0x06;
    testrom[0x11007] = 0x07;
    testrom[0x11008] = 0x08;
    testrom[0x11009] = 0x09;
    testrom[0x1100a] = 0x0a;
    testrom[0x1100b] = 0x0b;
    testrom[0x1100c] = 0x0c;
    testrom[0x1100d] = 0x0d;
    testrom[0x1100e] = 0x0e;
    testrom[0x1100f] = 0x0f;

    while(opcode != 0xF4)
    {
        cpu_state.ea_seg = &cpu_state.seg_ds;
        fetchdat = fastreadl_fetch(cs + cpu_state.pc);
    
        opcode = fetchdat & 0xFF;
        fetchdat >>= 8;

        cpu_state.pc++;
        x86_opcodes[(opcode | cpu_state.op32) & 0x3ff](fetchdat);
        sse_xmm = 0;
    }

    if(XMM[0].q[0] == 0x0706050403020100ull && XMM[0].q[1] == 0x0f0e0d0c0b0a0908ull)
        printf("MOVUPS matched expected results\n");
    else
        printf("MOVUPS did not match expected results! XMM low half %016llx XMM high half %016llx \n", XMM[0].q[0], XMM[0].q[1]);

    free(testrom);
    pc_close(NULL);
}