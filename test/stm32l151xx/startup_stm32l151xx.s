#####################################################################
# @author: Tien Ta
# @date  : 30/03/2026
#####################################################################

#####################################################################
# Initiliaze syntax and processor
#####################################################################

.syntax unified
.cpu cortex-m3
.arch armv7-m
.fpu softvfp
.thumb
.global vector_table
.global default_handler

.equ  BootRAM, 0xF108F85F
#####################################################################
# Vector table
#####################################################################

.section .vector,"a",%progbits
.type vector_table, %object

vector_table:
	.word _estack
	.word Reset_Handler
	.word NMI_Handler
	.word HardFault_Handler
	.word MemManage_Handler
	.word BusFault_Handler
	.word UsageFault_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word SVC_Handler
	.word DebugMon_Handler
	.word 0
	.word PendSV_Handler
	.word SysTick_Handler
	.word WWDG_IRQHandler
	.word PVD_IRQHandler
	.word TAMPER_STAMP_IRQHandler
	.word RTC_WKUP_IRQHandler
	.word FLASH_IRQHandler
	.word RCC_IRQHandler
	.word EXTI0_IRQHandler
	.word EXTI1_IRQHandler
	.word EXTI2_IRQHandler
	.word EXTI3_IRQHandler
	.word EXTI4_IRQHandler
	.word DMA1_Channel1_IRQHandler
	.word DMA1_Channel2_IRQHandler
	.word DMA1_Channel3_IRQHandler
	.word DMA1_Channel4_IRQHandler
	.word DMA1_Channel5_IRQHandler
	.word DMA1_Channel6_IRQHandler
	.word DMA1_Channel7_IRQHandler
	.word ADC1_IRQHandler
	.word USB_HP_IRQHandler
	.word USB_LP_IRQHandler
	.word DAC_IRQHandler
	.word COMP_IRQHandler
	.word EXTI9_5_IRQHandler
	.word 0
	.word TIM9_IRQHandler
	.word TIM10_IRQHandler
	.word TIM11_IRQHandler
	.word TIM2_IRQHandler
	.word TIM3_IRQHandler
	.word TIM4_IRQHandler
	.word I2C1_EV_IRQHandler
	.word I2C1_ER_IRQHandler
	.word I2C2_EV_IRQHandler
	.word I2C2_ER_IRQHandler
	.word SPI1_IRQHandler
	.word SPI2_IRQHandler
	.word USART1_IRQHandler
	.word USART2_IRQHandler
	.word USART3_IRQHandler
	.word EXTI15_10_IRQHandler
	.word RTC_Alarm_IRQHandler
	.word USB_FS_WKUP_IRQHandler
	.word TIM6_IRQHandler
	.word TIM7_IRQHandler
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word BootRAM

#####################################################################
# Reset Handler
#####################################################################

.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function

Reset_Handler:
	bl SystemInit

	movs r0, #0x00
	ldr r1, =_sdata
	ldr r2, =_edata
	ldr r3, =_sidata
	b copy_data_init_loop

copy_data_init:
	ldr r4, [r3, r0]
	str r4, [r1, r0]
	adds r0, r0, #0x04

copy_data_init_loop:
	adds r4, r0, r1
	cmp r4, r2
	bcc copy_data_init

	movs r0, #0x00
	ldr r1, =_sbss
	ldr r2, =_ebss
	b reset_bss_loop

reset_bss:
	str r0, [r1]
	adds r1, r1, #4

reset_bss_loop:
	cmp r1, r2
	bcc reset_bss

	bl main

.size Reset_Handler, .-Reset_Handler

#####################################################################
# Default Handler
#####################################################################

Default_Handler:

infinite_loop:
	b infinite_loop
	.size Default_Handler, .-Default_Handler

#####################################################################
# Alias for each exception handler to default handler
#####################################################################

	.weak NMI_Handler
	.thumb_set NMI_Handler,Default_Handler

	.weak HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler

	.weak MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler

	.weak BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler

	.weak UsageFault_Handler
	.thumb_set UsageFault_Handler,Default_Handler

	.weak SVC_Handler
	.thumb_set SVC_Handler,Default_Handler

	.weak DebugMon_Handler
	.thumb_set DebugMon_Handler,Default_Handler

	.weak PendSV_Handler
	.thumb_set PendSV_Handler,Default_Handler

	.weak SysTick_Handler
	.thumb_set SysTick_Handler,Default_Handler

	.weak WWDG_IRQHandler
	.thumb_set WWDG_IRQHandler,Default_Handler

	.weak PVD_IRQHandler
	.thumb_set PVD_IRQHandler,Default_Handler

	.weak TAMPER_STAMP_IRQHandler
	.thumb_set TAMPER_STAMP_IRQHandler,Default_Handler

	.weak RTC_WKUP_IRQHandler
	.thumb_set RTC_WKUP_IRQHandler,Default_Handler

	.weak FLASH_IRQHandler
	.thumb_set FLASH_IRQHandler,Default_Handler

	.weak RCC_IRQHandler
	.thumb_set RCC_IRQHandler,Default_Handler

	.weak EXTI0_IRQHandler
	.thumb_set EXTI0_IRQHandler,Default_Handler

	.weak EXTI1_IRQHandler
	.thumb_set EXTI1_IRQHandler,Default_Handler

	.weak EXTI2_IRQHandler
	.thumb_set EXTI2_IRQHandler,Default_Handler

	.weak EXTI3_IRQHandler
	.thumb_set EXTI3_IRQHandler,Default_Handler

	.weak EXTI4_IRQHandler
	.thumb_set EXTI4_IRQHandler,Default_Handler

	.weak DMA1_Channel1_IRQHandler
	.thumb_set DMA1_Channel1_IRQHandler,Default_Handler

	.weak DMA1_Channel2_IRQHandler
	.thumb_set DMA1_Channel2_IRQHandler,Default_Handler

	.weak DMA1_Channel3_IRQHandler
	.thumb_set DMA1_Channel3_IRQHandler,Default_Handler

	.weak DMA1_Channel4_IRQHandler
	.thumb_set DMA1_Channel4_IRQHandler,Default_Handler

	.weak DMA1_Channel5_IRQHandler
	.thumb_set DMA1_Channel5_IRQHandler,Default_Handler

	.weak DMA1_Channel6_IRQHandler
	.thumb_set DMA1_Channel6_IRQHandler,Default_Handler

	.weak DMA1_Channel7_IRQHandler
	.thumb_set DMA1_Channel7_IRQHandler,Default_Handler

	.weak ADC1_IRQHandler
	.thumb_set ADC1_IRQHandler,Default_Handler

	.weak USB_HP_IRQHandler
	.thumb_set USB_HP_IRQHandler,Default_Handler

	.weak USB_LP_IRQHandler
	.thumb_set USB_LP_IRQHandler,Default_Handler

	.weak DAC_IRQHandler
	.thumb_set DAC_IRQHandler,Default_Handler

	.weak COMP_IRQHandler
	.thumb_set COMP_IRQHandler,Default_Handler

	.weak EXTI9_5_IRQHandler
	.thumb_set EXTI9_5_IRQHandler,Default_Handler

	.weak TIM9_IRQHandler
	.thumb_set TIM9_IRQHandler,Default_Handler

	.weak TIM10_IRQHandler
	.thumb_set TIM10_IRQHandler,Default_Handler

	.weak TIM11_IRQHandler
	.thumb_set TIM11_IRQHandler,Default_Handler

	.weak TIM2_IRQHandler
	.thumb_set TIM2_IRQHandler,Default_Handler

	.weak TIM3_IRQHandler
	.thumb_set TIM3_IRQHandler,Default_Handler

	.weak TIM4_IRQHandler
	.thumb_set TIM4_IRQHandler,Default_Handler

	.weak I2C1_EV_IRQHandler
	.thumb_set I2C1_EV_IRQHandler,Default_Handler

	.weak I2C1_ER_IRQHandler
	.thumb_set I2C1_ER_IRQHandler,Default_Handler

	.weak I2C2_EV_IRQHandler
	.thumb_set I2C2_EV_IRQHandler,Default_Handler

	.weak I2C2_ER_IRQHandler
	.thumb_set I2C2_ER_IRQHandler,Default_Handler

	.weak SPI1_IRQHandler
	.thumb_set SPI1_IRQHandler,Default_Handler

	.weak SPI2_IRQHandler
	.thumb_set SPI2_IRQHandler,Default_Handler

	.weak USART1_IRQHandler
	.thumb_set USART1_IRQHandler,Default_Handler

	.weak USART2_IRQHandler
	.thumb_set USART2_IRQHandler,Default_Handler

	.weak USART3_IRQHandler
	.thumb_set USART3_IRQHandler,Default_Handler

	.weak EXTI15_10_IRQHandler
	.thumb_set EXTI15_10_IRQHandler,Default_Handler

	.weak RTC_Alarm_IRQHandler
	.thumb_set RTC_Alarm_IRQHandler,Default_Handler

	.weak USB_FS_WKUP_IRQHandler
	.thumb_set USB_FS_WKUP_IRQHandler,Default_Handler

	.weak TIM6_IRQHandler
	.thumb_set TIM6_IRQHandler,Default_Handler

	.weak TIM7_IRQHandler
	.thumb_set TIM7_IRQHandler,Default_Handler
