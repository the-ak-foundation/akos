/**
  ******************************************************************************
  * @file    startup.c
  * @brief   STM32F030F4P6 platform vector table and reset handler.
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
WEAK_DEFAULT_HANDLER(WWDG_IRQHandler);
WEAK_DEFAULT_HANDLER(RTC_IRQHandler);
WEAK_DEFAULT_HANDLER(FLASH_IRQHandler);
WEAK_DEFAULT_HANDLER(RCC_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI0_1_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI2_3_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI4_15_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel1_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel2_3_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel4_5_IRQHandler);
WEAK_DEFAULT_HANDLER(ADC1_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM1_BRK_UP_TRG_COM_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM1_CC_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM3_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM14_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM16_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM17_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C1_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI1_IRQHandler);
WEAK_DEFAULT_HANDLER(USART1_IRQHandler);

/* ============================================================
 * 4. Interrupt vector table
 * ============================================================ */

__attribute__((section(".isr_vector"), used))
const uintptr_t g_pfnVectors[] = {
    (uintptr_t)&_estack,                       /* Initial stack pointer */
    (uintptr_t)Reset_Handler,                  /* Reset                 */
    (uintptr_t)NMI_Handler,                    /* NMI                   */
    (uintptr_t)HardFault_Handler,              /* HardFault             */
    0u,                                        /* Reserved              */
    0u,                                        /* Reserved              */
    0u,                                        /* Reserved              */
    0u,                                        /* Reserved              */
    0u,                                        /* Reserved              */
    0u,                                        /* Reserved              */
    0u,                                        /* Reserved              */
    (uintptr_t)SVC_Handler,                    /* SVCall                */
    0u,                                        /* Reserved              */
    0u,                                        /* Reserved              */
    (uintptr_t)PendSV_Handler,                 /* PendSV                */
    (uintptr_t)SysTick_Handler,                /* SysTick               */

    (uintptr_t)WWDG_IRQHandler,                /* IRQ 0:  WWDG          */
    0u,                                        /* IRQ 1:  Reserved      */
    (uintptr_t)RTC_IRQHandler,                 /* IRQ 2:  RTC           */
    (uintptr_t)FLASH_IRQHandler,               /* IRQ 3:  FLASH         */
    (uintptr_t)RCC_IRQHandler,                 /* IRQ 4:  RCC           */
    (uintptr_t)EXTI0_1_IRQHandler,             /* IRQ 5:  EXTI0_1       */
    (uintptr_t)EXTI2_3_IRQHandler,             /* IRQ 6:  EXTI2_3       */
    (uintptr_t)EXTI4_15_IRQHandler,            /* IRQ 7:  EXTI4_15      */
    0u,                                        /* IRQ 8:  Reserved      */
    (uintptr_t)DMA1_Channel1_IRQHandler,       /* IRQ 9:  DMA1_CH1      */
    (uintptr_t)DMA1_Channel2_3_IRQHandler,     /* IRQ 10: DMA1_CH2_3    */
    (uintptr_t)DMA1_Channel4_5_IRQHandler,     /* IRQ 11: DMA1_CH4_5    */
    (uintptr_t)ADC1_IRQHandler,                /* IRQ 12: ADC1          */
    (uintptr_t)TIM1_BRK_UP_TRG_COM_IRQHandler, /* IRQ 13: TIM1          */
    (uintptr_t)TIM1_CC_IRQHandler,             /* IRQ 14: TIM1_CC       */
    0u,                                        /* IRQ 15: Reserved      */
    (uintptr_t)TIM3_IRQHandler,                /* IRQ 16: TIM3          */
    0u,                                        /* IRQ 17: Reserved      */
    0u,                                        /* IRQ 18: Reserved      */
    (uintptr_t)TIM14_IRQHandler,               /* IRQ 19: TIM14         */
    0u,                                        /* IRQ 20: Reserved      */
    (uintptr_t)TIM16_IRQHandler,               /* IRQ 21: TIM16         */
    (uintptr_t)TIM17_IRQHandler,               /* IRQ 22: TIM17         */
    (uintptr_t)I2C1_IRQHandler,                /* IRQ 23: I2C1          */
    0u,                                        /* IRQ 24: Reserved      */
    (uintptr_t)SPI1_IRQHandler,                /* IRQ 25: SPI1          */
    0u,                                        /* IRQ 26: Reserved      */
    (uintptr_t)USART1_IRQHandler,              /* IRQ 27: USART1        */
    0u,                                        /* IRQ 28: Reserved      */
    0u,                                        /* IRQ 29: Reserved      */
    0u,                                        /* IRQ 30: Reserved      */
    0u,                                        /* IRQ 31: Reserved      */
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
