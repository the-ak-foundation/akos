/**
  ******************************************************************************
  * @file    startup.c
  * @brief   LP-MSPM0C1104 vector table and reset handler.
  *
  *   Reset sequence:
  *     1. SystemInit()
  *     2. Copy .data from FLASH to SRAM
  *     3. Zero .bss
  *     4. Run C/C++ init arrays
  *     5. main()
  ******************************************************************************
  */

#include <stdint.h>

/* ============================================================
 * 1. External linker symbols
 * ============================================================ */

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

extern int main(void);
extern void SystemInit(void);
extern void __libc_init_array(void) __attribute__((weak));

/* ============================================================
 * 2. Forward declarations
 * ============================================================ */

void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_DEFAULT_HANDLER(name) \
    void name(void) __attribute__((weak, alias("Default_Handler")))

/* ============================================================
 * 3. Weak exception and IRQ handlers
 * ============================================================ */

WEAK_DEFAULT_HANDLER(NMI_Handler);
WEAK_DEFAULT_HANDLER(HardFault_Handler);
WEAK_DEFAULT_HANDLER(SVC_Handler);
WEAK_DEFAULT_HANDLER(PendSV_Handler);
WEAK_DEFAULT_HANDLER(SysTick_Handler);
WEAK_DEFAULT_HANDLER(GROUP0_IRQHandler);
WEAK_DEFAULT_HANDLER(GPIOA_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMG8_IRQHandler);
WEAK_DEFAULT_HANDLER(ADC0_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI0_IRQHandler);
WEAK_DEFAULT_HANDLER(UART0_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMG14_IRQHandler);
WEAK_DEFAULT_HANDLER(TIMA0_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C0_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA_IRQHandler);

/* ============================================================
 * 4. Interrupt vector table
 * ============================================================ */

__attribute__((section(".intvecs"), used))
const uintptr_t g_pfnVectors[] = {
    (uintptr_t)&_estack,          /* Initial stack pointer */
    (uintptr_t)Reset_Handler,     /* Reset                 */
    (uintptr_t)NMI_Handler,       /* NMI                   */
    (uintptr_t)HardFault_Handler, /* HardFault             */
    0u,                           /* Reserved              */
    0u,                           /* Reserved              */
    0u,                           /* Reserved              */
    0u,                           /* Reserved              */
    0u,                           /* Reserved              */
    0u,                           /* Reserved              */
    0u,                           /* Reserved              */
    (uintptr_t)SVC_Handler,       /* SVCall                */
    0u,                           /* Reserved              */
    0u,                           /* Reserved              */
    (uintptr_t)PendSV_Handler,    /* PendSV                */
    (uintptr_t)SysTick_Handler,   /* SysTick               */

    (uintptr_t)GROUP0_IRQHandler, /* IRQ 0:  GROUP0        */
    (uintptr_t)GPIOA_IRQHandler,  /* IRQ 1:  GPIOA         */
    (uintptr_t)TIMG8_IRQHandler,  /* IRQ 2:  TIMG8         */
    0u,                           /* IRQ 3:  Reserved      */
    (uintptr_t)ADC0_IRQHandler,   /* IRQ 4:  ADC0          */
    0u,                           /* IRQ 5:  Reserved      */
    0u,                           /* IRQ 6:  Reserved      */
    0u,                           /* IRQ 7:  Reserved      */
    0u,                           /* IRQ 8:  Reserved      */
    (uintptr_t)SPI0_IRQHandler,   /* IRQ 9:  SPI0          */
    0u,                           /* IRQ 10: Reserved      */
    0u,                           /* IRQ 11: Reserved      */
    0u,                           /* IRQ 12: Reserved      */
    0u,                           /* IRQ 13: Reserved      */
    0u,                           /* IRQ 14: Reserved      */
    (uintptr_t)UART0_IRQHandler,  /* IRQ 15: UART0         */
    (uintptr_t)TIMG14_IRQHandler, /* IRQ 16: TIMG14        */
    0u,                           /* IRQ 17: Reserved      */
    (uintptr_t)TIMA0_IRQHandler,  /* IRQ 18: TIMA0         */
    0u,                           /* IRQ 19: Reserved      */
    0u,                           /* IRQ 20: Reserved      */
    0u,                           /* IRQ 21: Reserved      */
    0u,                           /* IRQ 22: Reserved      */
    0u,                           /* IRQ 23: Reserved      */
    (uintptr_t)I2C0_IRQHandler,   /* IRQ 24: I2C0          */
    0u,                           /* IRQ 25: Reserved      */
    0u,                           /* IRQ 26: Reserved      */
    0u,                           /* IRQ 27: Reserved      */
    0u,                           /* IRQ 28: Reserved      */
    0u,                           /* IRQ 29: Reserved      */
    0u,                           /* IRQ 30: Reserved      */
    (uintptr_t)DMA_IRQHandler,    /* IRQ 31: DMA           */
};

/* ============================================================
 * 5. Reset handler
 * ============================================================ */

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    SystemInit();

    while (dst < &_edata)
    {
        *dst++ = *src++;
    }

    for (dst = &_sbss; dst < &_ebss; ++dst)
    {
        *dst = 0u;
    }

    if (__libc_init_array != 0)
    {
        __libc_init_array();
    }

    (void)main();

    for (;;)
    {
    }
}

/* ============================================================
 * 6. Default handler
 * ============================================================ */

void Default_Handler(void)
{
    for (;;)
    {
    }
}
