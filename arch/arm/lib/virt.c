/*
 * (C) Copyright 2013
 * Andre Przywara, Linaro
 *
 * routines to push ARMv7 processors from secure into non-secure state
 * needed to enable ARMv7 virtualization for current hypervisors
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/armv7.h>

#define GICD_CTLR	0x000
#define GICD_TYPER	0x004
#define GICD_IGROUPR0	0x080
#define GICD_SGIR	0xf00

#define CPU_ARM_CORTEX_A15	0x4100c0f0
#define CPU_ARM_CORTEX_A7	0x4100c070

static inline unsigned int read_cpsr(void)
{
	unsigned int reg;

	asm volatile ("mrs %0, cpsr\n" : "=r" (reg));
	return reg;
}

int armv7_switch_hyp(void)
{
	unsigned int reg;
	volatile unsigned int *gicdptr;
	unsigned itlinesnr, i;
	unsigned int *sysflags;

	/* check whether the CPU in hyp mode already */

	if((read_cpsr() & 0x1F) ==0x1A)
		return 1;

	printf("we are not yet in hyp mode already\n");
	/* check whether the CPU supports the virt extensions */
	asm("mrc p15, 0, %0, c0, c1, 1\n" : "=r"(reg));
	if ((reg & 0xF000) != 0x1000)
		return 2;

	printf("We do support virt extn\n");
	/* the timer frequency for the generic timer needs to be
	 * programmed still in secure state, should be done by firmware.
	 * check whether we have the generic timer first
	 */
	
	printf("Do timer init\n");

	arch_timer_init();
/*

#define CONFIG_TIMER_FREQ 24000000


	asm("mrc p15, 0, %0, c0, c1, 1\n" : "=r"(reg));
	if ((reg & 0xF0000) == 0x10000)
	asm("mcr p15, 0, %0, c14, c0, 0\n"
		: : "r"(CONFIG_TIMER_FREQ));
*/

	/* the SCR register will be set directly in the monitor mode handler,
	 * according to the spec one should not tinker with it in secure state
	 * in SVC mode. Do not try to read it once in non-secure state,
	 * any access to it will trap.
	 */

	/* check whether we are an Cortex-A15 or A7.
	 * The actual non-secure switch should work with all CPUs supporting
	 * the security extension, but we need the GIC address,
	 * which we know only for sure for those two CPUs.
	 */

	asm("mrc p15, 0, %0, c0, c0, 0\n" : "=r"(reg));
	if (((reg & 0xFF00FFF0) != 0x4100C0F0) &&
	    ((reg & 0xFF00FFF0) != 0x4100C070))
		return 3;

	printf("Processor support two CPUs\n");

	/* get the GIC base address from the A15 PERIPHBASE register */
	asm("mrc p15, 4, %0, c15, c0, 0\n" : "=r" (reg));

	/* the PERIPHBASE can be mapped above 4 GB (lower 8 bits used to
	 * encode this). Bail out here since we cannot access this without
	 * enabling paging.
	 */

	if ((reg & 0xff) != 0)
		return 4;

	printf("PERIPHBASE is accessible < 4GB\n");
	/* GIC distributor registers start at offset 0x1000 */
	gicdptr = (unsigned *)(reg + 0x1000);

	/* enable the GIC distributor */
	gicdptr[GICD_CTLR / 4] |= 0x03;

	/* TYPER[4:0] contains an encoded number of all interrupts */
	itlinesnr = gicdptr[GICD_TYPER / 4] & 0x1f;

	/* set all bits in the GIC group registers to one to allow access
	 * from non-secure state
	 */
	for (i = 0; i <= itlinesnr; i++)
		gicdptr[GICD_IGROUPR0 / 4 + i] = (unsigned)-1;

	sysflags = (void *) 0x02020000;
	sysflags[1] = (unsigned) -1;
	sysflags[0] = (uintptr_t) _smp_pen;
	gicdptr[GICD_SGIR / 4] = 1U << 24;

	/* call the non-sec switching code and hyp mode tranistion on this CPU */

	printf("invoking _hyp_gic_switch\n");
	_hyp_gic_switch();

	if((read_cpsr() & 0x1F) != 0x1A) {
		printf("Failed");
		return 5;
        }

	printf("successful hyp entry - cpu0\n");
	return 0;
}

void do_software_interrupt_print(void) {

	printf("SWI trace1 invoked\n");
	return;
}
void trace_me(void) {

printf("trace me invoked\n");
return;
}

void trace_out(void){

printf("trace out\n");
return;

}
