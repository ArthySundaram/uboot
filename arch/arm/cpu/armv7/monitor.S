
#include <config.h>

.syntax	unified
.arch_extension sec
.arch_extension virt
.text

.align 5

/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
mon_vectors:
	.word 0	/* reset */
	.word 0	/* undef */
	b	2f /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	b	1f
	.word 0 /* irq */
	.word 0 /* fiq */

/* Return directly back to the caller without leaving Hyp mode: */
1:	mrs	lr, elr_hyp
	mov	pc, lr

/* In monitor mode, set up HVBAR and SCR then return to caller in NS-SVC. */
2:
	mov     r0, lr
	mrc	p15, 0, r1, c1, c1, 0		@ SCR
	/*
	 * Set SCR.NS=1 (needed for setting HVBAR and also returning to NS state)
	 *        .IRQ,FIQ,EA=0 (don't take aborts/exceptions to Monitor mode)
	 *        .FW,AW=1 (CPSR.A,F modifiable in NS state)
	 *        .nET=0 (early termination OK)
	 *        .SCD=0 (SMC in NS mode OK, so we can call secure firmware)
	 *        .HCE=1 (HVC does Hyp call)
	 */
	bic	r1, r1, #0x07f
	ldr	r2, =0x131
	orr	r1, r1, r2
	mcr	p15, 0, r2, c1, c1, 0		@ SCR
	isb
	ldr	r2, =mon_vectors
	mcr	p15, 4, r2, c12, c0, 0		@ set HVBAR
	/* ...and return to calling code in NS state */
	movs	pc, r0

/******************************************************************************
 * This code is called from u-boot into the above handler
 */

	.globl monitor_init
monitor_init:
	mov     r0, lr
	ldr	ip, =mon_vectors
	mcr	p15, 0, ip, c12, c0, 1		@ Monitor vector base address
	mov	pc, r0 

	/* Try to go into NS-SVC: void enter_ns(void); */
	.globl enter_ns
enter_ns:
	mov     r0, lr
	b	tracens
	smc	#0
	b 	tracens
	mov	pc, r0 

	/* void enter_hyp(void); */
	.globl enter_hyp
enter_hyp:
	/* Now we're in NS-SVC, make a Hyp call to get into Hyp mode */
	mov	r0, lr
	mov	r1, sp
	b	tracehyp
	hvc	#0
	b	tracehyp
	/* We will end up here in NS-Hyp. */
	mov	sp, r1
	mov	pc, r0
