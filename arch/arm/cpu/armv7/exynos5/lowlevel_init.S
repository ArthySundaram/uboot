/*
 * Lowlevel setup for SMDK5250 board based on S5PC520
 *
 * Copyright (C) 2012 Samsung Electronics
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

#include <config.h>
#include <asm/arch/cpu.h>
/*
_TEXT_BASE:
	.word  CONFIG_UBOOT_TEXT_BASE
*/
	.globl lowlevel_init
lowlevel_init:
	/*
	 * Set the stack pointer, although it will be overwriten by the caller
	 * It seems we will not boot if this function is empty.
	 */

/* DISABLE SMP KICKING

	mrc     p15, 0, r0, c0, c0, 5
	and     r0, r0, #15
	cmp     r0, #0
	beq     first_cpu
	
	bl      arch_timer_init
	bl      non_secure_init
	bl      monitor_init
	bl      enter_ns
	bl      enter_hyp
	b      enter_smp_pen

first_cpu:
*/
	ldr	sp, =0x41f00000
	str	pc, [sp]

	ldr	sp, =CONFIG_IRAM_STACK
	stmdb	r13!, {ip, lr}
		
	bl 	arch_timer_init
	bl	non_secure_init

/* DISABLE SMP

	bl 	smp_kick_secondary

*/

	bl	monitor_init

	ldmia   r13!, {r6, r7} 	
	ldr	sp, =CONFIG_IRAM_STACK

	mov     ip, r6
	mov	pc, r7 
