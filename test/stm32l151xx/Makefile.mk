CPU		  	= -mthumb -mcpu=cortex-m3
DEFS  		= STM32L151xB
LDSCRIPT  	= test/stm32l151xx/stm32l151xx.ld

OOCD_IF		= stlink
OOCD_TARGET	= stm32l1x

INCS +=
SRCS +=\
	test/stm32l151xx/main.c\
	test/stm32l151xx/startup_stm32l151xx.s
