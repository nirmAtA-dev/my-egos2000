/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: wrapping the CPU interface for interrupts
 * Initialize the trap entry, enable interrupts, and reset the timer.
 */

#include "egos.h"
/* EXPLAINATION OF BELOW DEFINITIONS 
 * CLINT_BASE is the base address of a region in memory where core-local interruptor lives 
 * which provides basic software and timer interrupts to CPU , check  SiFive FU540-C000 to understand it
 * the mtime is a 64bit memory region which is 0xBFF8 away from CLINT_BASE
 * similarly the 4 cores provided by SiFive hardware (implements The RISC‑V Instruction Set Manual,
 * Volume II: Privileged Architecture, Version 1.10, has mtimecmp memory region in 0x2004000 as BASE
 * also address of CORE 0, *B away then we have address 0x2004008 which has CORE 1 ...
 * QUANTUM is time quanta of interrupt and if we are running on QEMU it has lower value and if we are 
 * running in SiFive hardware it has higher value
 */
#define MTIME_BASE    (CLINT_BASE + 0xBFF8)
#define MTIMECMP_BASE (CLINT_BASE + 0x4000)
#define QUANTUM       (earth->platform == QEMU ? 100000UL : 50000000UL)
/* EXPLAINATION OF CODE BELOW 
 *
 * the mtime register which holds current time in processor and in multicore it has single mtime but multiple mtimecmp,
 * is of 64bit size and our qemu is setup for 32bit machine
 * hence we need to read high part and the low part and merge it using
 * (ulonglong)high << 32 | low
 * ulonglong is a combination of 2- 32bit register working together
 * the only question lies that why reading the high twice ?
 *
 * consider a case when current mtime = 0x00000000FFFFFFFF
 * say once you read high = 0x000000000
 * then clock ticks and mtime becomes = 0x0000000100000000
 * then we read the low = 0x00000000
 * then clock value is 0x0000000000000000 = 0 but the actual value is 0x0000000100000000
 * hence to avoid such boundary condition we read high again and compare it to earlier high and then we return the merge
 *
 */
ulonglong mtime_get() {
    uint low, high;
    do {
        high = REGW(MTIME_BASE, 4);
        low  = REGW(MTIME_BASE, 0);
    } while (REGW(MTIME_BASE, 4) != high);

    return (((ulonglong)high) << 32) | low;
}

/* EXPLAINATION OF CODE BELOW
 *
 * the mtimecmp register whose size is 64 bits bold value when the value need to be raised for interrupt
 * hence when mtime >= mtimecmp then interrupt occurs 
 * multicore processor has a single mtimecmp for each of its core however all have same global clock (mtime)
 * in the function mtimecmp_set sets the mtimecmp region with a value
 * the line 2 and 3 inside the function sets the lower and higher region nicely only confusion is why the 1st line
 * say we want to have an interrupt after mtimecmp = 0x00000001 00000010, the current mtime is say 0x00000000 FFFFFF00
 * there can be an earlier mtimecmp value say it be  0x00000000 00FFFFFF, once we update the lower value we get mtimecmp = 0x00000000 00000010
 * resulting in mtime >= mtimecmp and raising of interrupt where reality is we are still not at correct time
 * hence we assign 0xFFFFFFFF which forces the interrupt to a far away time and if we update lower bits we still are far away and then we update 
 * higher address
 * 
 */
static void mtimecmp_set(ulonglong time, uint core_id) {
    REGW(MTIMECMP_BASE, core_id * 8 + 4) = 0xFFFFFFFF;
    REGW(MTIMECMP_BASE, core_id * 8 + 0) = (uint)time;
    REGW(MTIMECMP_BASE, core_id * 8 + 4) = (uint)(time >> 32);
}

// This code simply moves the new timer value to a QUANTUM value farter in future, next timer value
static void timer_reset(uint core_id) {
    mtimecmp_set(mtime_get() + QUANTUM, core_id);
}

void trap_entry(); /* See grass/kernel.s */
void intr_init(uint core_id) {
    /* Initialize the timer. */
    // if looked at egos.h we have earth->timer_reset a pointer to function
    earth->timer_reset = timer_reset;
    // set timer to very future time nice way to disable timer
    mtimecmp_set(0x0FFFFFFFFFFFFFFFUL, core_id);

    /* Setup the interrupt/exception handling entry. */
    // mtvec holds the starting address of interrupt handler
    asm("csrw mtvec, %0" ::"r"(trap_entry));
    INFO("Use direct mode and put the address of the trap_entry into mtvec");

    /* Enable timer interrupt. */
    // csrw overwrites the control and status register while csrs sets bits value
    // mip == pending interrupts is zeroed
    asm("csrw mip, %0" ::"r"(0));
    //enables timer interrupt by flipping the MTIE interrupt enable as 1
    asm("csrs mie, %0" ::"r"(0x80));
    asm("csrs mstatus, %0" ::"r"(0x88));

    /* Student's code goes here (Ethernet & TCP/IP). */

    /* Enable external interrupt. Find the IRQ number corresponding to the
     * Ethernet controller device in PLIC, enable external interrupts from
     * this IRQ number, and then set the priority of this IRQ number to 1. */

    /* Student's code ends here. */
}
