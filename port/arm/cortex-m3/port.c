/**
 * @file port.c
 * @brief Cortex-M3 port implementation (context switching and tick hook).
 */

#include "port.h"
#include "thread.h"
#include "core.h"
#include "config.h"

#include "core_cm3.h"
#include "core_cmFunc.h"

/**
 * @brief Initialize SysTick to generate 1ms OS ticks.
 * @param cpu_freq Core clock frequency in Hz.
 */
void akos_port_systick_init_freq(uint32_t cpu_freq)
{
	volatile uint32_t ticks = cpu_freq / 1000u;

	SysTick->LOAD = ticks - 1u;
	SysTick->VAL = 0u;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
			SysTick_CTRL_TICKINT_Msk |
			SysTick_CTRL_ENABLE_Msk;

	*(volatile uint32_t *)0xE000ED20UL &= ~(0xFFUL << 24);
	*(volatile uint32_t *)0xE000ED20UL |= ((uint32_t)1u << (8U - __NVIC_PRIO_BITS)) << 24;
}

/**
 * @brief Start the first thread by restoring the initial exception context.
 */
void akos_port_start_first_task(void)
{
	__asm volatile(
		" ldr r0, =0xE000ED08 	    \n" /* Use the NVIC offset register to locate the stack. */
		" ldr r0, [r0] 	            \n"
		" ldr r0, [r0] 			    \n"
		" msr msp, r0			    \n" /* Set the msp back to the start of the stack. */
		" cpsie i			        \n" /* Globally enable interrupts. */
		" cpsie f		            \n"
		" dsb			            \n"
		" isb			            \n"
		" svc 0		                \n" /* System call to start first thread. */
		" nop			            \n"
		" .ltorg                    \n"
	);
}

/**
 * @brief Build the initial Cortex-M thread stack frame.
 * @param p_stack Base stack buffer.
 * @param stack_size Stack size in 32-bit words.
 * @param pf_task Thread entry function.
 * @param p_arg Task argument.
 * @return Initial top-of-stack pointer.
 */
uint32_t *akos_port_task_stack_init(uint32_t *p_stack,
                               size_t stack_size,
                               void (*pf_task)(void *),
                               void *p_arg)
{
	uint32_t *p_stack_ptr;

	p_stack_ptr = &p_stack[stack_size - (uint32_t)1];
	p_stack_ptr = (uint32_t *)(((uint32_t)p_stack_ptr) & (~((uint32_t)0x007)));

	*(--p_stack_ptr) = (uint32_t)0x01000000UL;
	*(--p_stack_ptr) = ((uint32_t)pf_task) & ((uint32_t)0xfffffffeUL);
	*(--p_stack_ptr) = (uint32_t)0x0000000EUL;
	p_stack_ptr -= 5;
	*p_stack_ptr = (uint32_t)p_arg;
	p_stack_ptr -= 8;

	return p_stack_ptr;
}

#ifdef __cplusplus
extern "C"
{
#endif
	/**
	 * @brief SVC handler used to restore the first thread context.
	 */
	void port_SVCHandler(void)
	{
		__asm volatile(
		    "CPSID   I						\n" // Prevent interruption during context switch
		    "LDR     R1, =tcb_curr_ptr		\n" // get pointer to TCB current
		    "LDR     R1, [R1]				\n" // get TCB current	= pointer to StkPtr
		    "LDR     R0, [R1]				\n" // get StkPtr
		    "LDMIA R0!, {R4-R11}			\n" //
		    "MSR     PSP, R0				\n" //
		    "ORR LR, #0xD					\n" // LR = 0xFFFFFFFD return to threadmode
		    "CPSIE   I						\n" //
		    "BX      LR						\n" //
		);
	}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	/**
	 * @brief PendSV handler used for context switching between threads.
	 */
	void port_PendSVHandler(void)
	{
		__asm volatile(
		    //"CPSID   I								\n"	//Prevent interruption during context switch
		    "MRS     R0, PSP							\n" // PSP is process stack pointer
		    "CBZ     R0, OS_CPU_PendSVHandler_nosave   	\n" // Skip register save the first time

		    "SUBS    R0, R0, #0x20						\n" // Save remaining regs r4-11 on process stack
		    "STM     R0, {R4-R11}						\n" //

		    "LDR     R1, =tcb_curr_ptr					\n" // OSTCBCur->OSTCBStkPtr = SP;
		    "LDR     R1, [R1]\n"				    		//
		    "STR     R0, [R1]							\n" // R0 is SP of process being switched out

		    /* At this point, entire context of process has been saved	*/

		    "OS_CPU_PendSVHandler_nosave:				\n" //
		    "LDR     R0, =tcb_curr_ptr					\n" // OSTCBCur  = OSTCBHighRdy;
		    "LDR     R1, =tcb_high_rdy_ptr				\n" //
		    "LDR     R2, [R1]							\n" //
		    "STR     R2, [R0]							\n" //

		    "LDR     R0, [R2]							\n" // R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;
		    "LDM     R0, {R4-R11}						\n" // Restore r4-11 from new process stack
		    "ADDS    R0, R0, #0x20						\n" //
		    "MSR     PSP, R0							\n" // Load PSP with new process SP
		    "ORR     LR, LR, #0x04						\n" // Ensure exception return uses process stack
		    //"CPSIE   I								\n"  	//
		    "BX      LR									\n" // Exception return will restore remaining context
		);
	}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	/**
	 * @brief SysTick ISR hook for scheduler tick updates.
	 */
	void port_SysTickHandler()
	{
		port_disable_interrupts
		/* Increment the RTOS tick. */
		if (akos_thread_increment_tick() == OS_TRUE)
		{
			/* A context switch is required.  Context switching is performed in
			 * the PendSV interrupt.  Pend the PendSV interrupt. */
			port_trigger_PendSV();
		}
		port_enable_interrupts
	}

#ifdef __cplusplus
}
#endif
