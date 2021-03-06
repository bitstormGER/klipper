// Main starting point for AVR boards.
//
// Copyright (C) 2016,2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <avr/io.h> // AVR_STACK_POINTER_REG
#include <util/crc16.h> // _crc_ccitt_update
#include "autoconf.h" // CONFIG_MCU
#include "board/misc.h" // dynmem_start
#include "command.h" // DECL_CONSTANT
#include "irq.h" // irq_enable
#include "sched.h" // sched_main

DECL_CONSTANT(MCU, CONFIG_MCU);


/****************************************************************
 * Dynamic memory
 ****************************************************************/

// Return the start of memory available for dynamic allocations
void *
dynmem_start(void)
{
    extern char _end;
    return &_end;
}

// Return the end of memory available for dynamic allocations
void *
dynmem_end(void)
{
    return (void*)ALIGN(AVR_STACK_POINTER_REG, 256) - CONFIG_AVR_STACK_SIZE;
}


/****************************************************************
 * Misc functions
 ****************************************************************/

// Initialize the clock prescaler (if necessary)
void
prescaler_init(void)
{
    if (CONFIG_AVR_CLKPR != -1 && (uint8_t)CONFIG_AVR_CLKPR != CLKPR) {
        irqstatus_t flag = irq_save();
        CLKPR = 0x80;
        CLKPR = CONFIG_AVR_CLKPR;
        irq_restore(flag);
    }
}
DECL_INIT(prescaler_init);

// The "sleep" instruction should cause the processor to enter "idle mode"
void
sleep_init(void)
{
    SMCR = 0x01;
}
DECL_INIT(sleep_init);

// Optimized crc16_ccitt for the avr processor
uint16_t
crc16_ccitt(char *buf, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    while (len--)
        crc = _crc_ccitt_update(crc, *buf++);
    return crc;
}

// Main entry point for avr code.
int
main(void)
{
    irq_enable();
    sched_main();
    return 0;
}
