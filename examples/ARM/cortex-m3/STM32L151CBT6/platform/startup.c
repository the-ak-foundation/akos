/**
  ******************************************************************************
  * @file    startup.c
  * @brief   STM32L151CBT6 platform vector table and reset handler.
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
extern void __libc_init_array(void);

/* ============================================================
 * 2. Forward declarations
 * ============================================================ */

void Reset_Handler(void);
void Default_Handler(void);
void _init(void) __attribute__((weak));

#define WEAK_DEFAULT_HANDLER(name) \
    void name(void) __attribute__((weak, alias("Default_Handler")))

/* ============================================================
 * 3. Weak exception and IRQ handlers
 * ============================================================ */

WEAK_DEFAULT_HANDLER(NMI_Handler);
WEAK_DEFAULT_HANDLER(HardFault_Handler);
WEAK_DEFAULT_HANDLER(MemManage_Handler);
WEAK_DEFAULT_HANDLER(BusFault_Handler);
WEAK_DEFAULT_HANDLER(UsageFault_Handler);
WEAK_DEFAULT_HANDLER(SVC_Handler);
WEAK_DEFAULT_HANDLER(DebugMon_Handler);
WEAK_DEFAULT_HANDLER(PendSV_Handler);
WEAK_DEFAULT_HANDLER(SysTick_Handler);
WEAK_DEFAULT_HANDLER(WWDG_IRQHandler);
WEAK_DEFAULT_HANDLER(PVD_IRQHandler);
WEAK_DEFAULT_HANDLER(TAMPER_STAMP_IRQHandler);
WEAK_DEFAULT_HANDLER(RTC_WKUP_IRQHandler);
WEAK_DEFAULT_HANDLER(FLASH_IRQHandler);
WEAK_DEFAULT_HANDLER(RCC_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI0_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI1_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI2_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI3_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI4_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel1_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel2_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel3_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel4_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel5_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel6_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Channel7_IRQHandler);
WEAK_DEFAULT_HANDLER(ADC1_IRQHandler);
WEAK_DEFAULT_HANDLER(USB_HP_IRQHandler);
WEAK_DEFAULT_HANDLER(USB_LP_IRQHandler);
WEAK_DEFAULT_HANDLER(DAC_IRQHandler);
WEAK_DEFAULT_HANDLER(COMP_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI9_5_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM9_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM10_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM11_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM2_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM3_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM4_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C1_EV_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C1_ER_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C2_EV_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C2_ER_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI1_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI2_IRQHandler);
WEAK_DEFAULT_HANDLER(USART1_IRQHandler);
WEAK_DEFAULT_HANDLER(USART2_IRQHandler);
WEAK_DEFAULT_HANDLER(USART3_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI15_10_IRQHandler);
WEAK_DEFAULT_HANDLER(RTC_Alarm_IRQHandler);
WEAK_DEFAULT_HANDLER(USB_FS_WKUP_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM6_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM7_IRQHandler);

/* ============================================================
 * 4. Interrupt vector table
 * ============================================================ */

__attribute__((section(".isr_vector"), used))
const uintptr_t g_pfnVectors[] = {
    (uintptr_t)&_estack,                 /* Initial stack pointer */
    (uintptr_t)Reset_Handler,            /* Reset                 */
    (uintptr_t)NMI_Handler,              /* NMI                   */
    (uintptr_t)HardFault_Handler,        /* HardFault             */
    (uintptr_t)MemManage_Handler,        /* MemManage             */
    (uintptr_t)BusFault_Handler,         /* BusFault              */
    (uintptr_t)UsageFault_Handler,       /* UsageFault            */
    0u,                                  /* Reserved              */
    0u,                                  /* Reserved              */
    0u,                                  /* Reserved              */
    0u,                                  /* Reserved              */
    (uintptr_t)SVC_Handler,              /* SVCall                */
    (uintptr_t)DebugMon_Handler,         /* DebugMon              */
    0u,                                  /* Reserved              */
    (uintptr_t)PendSV_Handler,           /* PendSV                */
    (uintptr_t)SysTick_Handler,          /* SysTick               */

    (uintptr_t)WWDG_IRQHandler,          /* IRQ 0:  WWDG          */
    (uintptr_t)PVD_IRQHandler,           /* IRQ 1:  PVD           */
    (uintptr_t)TAMPER_STAMP_IRQHandler,  /* IRQ 2:  TAMPER_STAMP  */
    (uintptr_t)RTC_WKUP_IRQHandler,      /* IRQ 3:  RTC_WKUP      */
    (uintptr_t)FLASH_IRQHandler,         /* IRQ 4:  FLASH         */
    (uintptr_t)RCC_IRQHandler,           /* IRQ 5:  RCC           */
    (uintptr_t)EXTI0_IRQHandler,         /* IRQ 6:  EXTI0         */
    (uintptr_t)EXTI1_IRQHandler,         /* IRQ 7:  EXTI1         */
    (uintptr_t)EXTI2_IRQHandler,         /* IRQ 8:  EXTI2         */
    (uintptr_t)EXTI3_IRQHandler,         /* IRQ 9:  EXTI3         */
    (uintptr_t)EXTI4_IRQHandler,         /* IRQ 10: EXTI4         */
    (uintptr_t)DMA1_Channel1_IRQHandler, /* IRQ 11: DMA1_CH1      */
    (uintptr_t)DMA1_Channel2_IRQHandler, /* IRQ 12: DMA1_CH2      */
    (uintptr_t)DMA1_Channel3_IRQHandler, /* IRQ 13: DMA1_CH3      */
    (uintptr_t)DMA1_Channel4_IRQHandler, /* IRQ 14: DMA1_CH4      */
    (uintptr_t)DMA1_Channel5_IRQHandler, /* IRQ 15: DMA1_CH5      */
    (uintptr_t)DMA1_Channel6_IRQHandler, /* IRQ 16: DMA1_CH6      */
    (uintptr_t)DMA1_Channel7_IRQHandler, /* IRQ 17: DMA1_CH7      */
    (uintptr_t)ADC1_IRQHandler,          /* IRQ 18: ADC1          */
    (uintptr_t)USB_HP_IRQHandler,        /* IRQ 19: USB_HP        */
    (uintptr_t)USB_LP_IRQHandler,        /* IRQ 20: USB_LP        */
    (uintptr_t)DAC_IRQHandler,           /* IRQ 21: DAC           */
    (uintptr_t)COMP_IRQHandler,          /* IRQ 22: COMP          */
    (uintptr_t)EXTI9_5_IRQHandler,       /* IRQ 23: EXTI9_5       */
    0u,                                  /* IRQ 24: Reserved      */
    (uintptr_t)TIM9_IRQHandler,          /* IRQ 25: TIM9          */
    (uintptr_t)TIM10_IRQHandler,         /* IRQ 26: TIM10         */
    (uintptr_t)TIM11_IRQHandler,         /* IRQ 27: TIM11         */
    (uintptr_t)TIM2_IRQHandler,          /* IRQ 28: TIM2          */
    (uintptr_t)TIM3_IRQHandler,          /* IRQ 29: TIM3          */
    (uintptr_t)TIM4_IRQHandler,          /* IRQ 30: TIM4          */
    (uintptr_t)I2C1_EV_IRQHandler,       /* IRQ 31: I2C1_EV       */
    (uintptr_t)I2C1_ER_IRQHandler,       /* IRQ 32: I2C1_ER       */
    (uintptr_t)I2C2_EV_IRQHandler,       /* IRQ 33: I2C2_EV       */
    (uintptr_t)I2C2_ER_IRQHandler,       /* IRQ 34: I2C2_ER       */
    (uintptr_t)SPI1_IRQHandler,          /* IRQ 35: SPI1          */
    (uintptr_t)SPI2_IRQHandler,          /* IRQ 36: SPI2          */
    (uintptr_t)USART1_IRQHandler,        /* IRQ 37: USART1        */
    (uintptr_t)USART2_IRQHandler,        /* IRQ 38: USART2        */
    (uintptr_t)USART3_IRQHandler,        /* IRQ 39: USART3        */
    (uintptr_t)EXTI15_10_IRQHandler,     /* IRQ 40: EXTI15_10     */
    (uintptr_t)RTC_Alarm_IRQHandler,     /* IRQ 41: RTC_Alarm     */
    (uintptr_t)USB_FS_WKUP_IRQHandler,   /* IRQ 42: USB_FS_WKUP   */
    (uintptr_t)TIM6_IRQHandler,          /* IRQ 43: TIM6          */
    (uintptr_t)TIM7_IRQHandler,          /* IRQ 44: TIM7          */
    0u,                                  /* IRQ 45: Reserved      */
    0u,                                  /* IRQ 46: Reserved      */
    0u,                                  /* IRQ 47: Reserved      */
    0u,                                  /* IRQ 48: Reserved      */
    0u,                                  /* IRQ 49: Reserved      */
    0u,                                  /* IRQ 50: Reserved      */
    0u,                                  /* IRQ 51: Reserved      */
    0u,                                  /* IRQ 52: Reserved      */
    0u,                                  /* IRQ 53: Reserved      */
    0u,                                  /* IRQ 54: Reserved      */
    0u,                                  /* IRQ 55: Reserved      */
    0u,                                  /* IRQ 56: Reserved      */
    0u,                                  /* IRQ 57: Reserved      */
    0u,                                  /* IRQ 58: Reserved      */
    0u,                                  /* IRQ 59: Reserved      */
    0u,                                  /* IRQ 60: Reserved      */
    0u,                                  /* IRQ 61: Reserved      */
    0xF108F85Fu,                         /* BootRAM marker        */
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

    __libc_init_array();
    (void)main();

    for (;;)
    {
    }
}

/* ============================================================
 * 6. Init / default handlers
 * ============================================================ */

void _init(void)
{
}

void Default_Handler(void)
{
    for (;;)
    {
    }
}
