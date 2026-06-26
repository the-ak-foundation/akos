#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdint.h>

#include "mspm0c110x.h"

#define BLINK_LED_GPIO GPIOA
#define BLINK_LED_PIN  (1u << 22u)

static inline void board_gpio_write(GPIO_Regs *gpio, uint32_t pin, bool on)
{
    if (on)
    {
        gpio->DOUTCLR31_0 = pin;
    }
    else
    {
        gpio->DOUTSET31_0 = pin;
    }
}

static inline void board_gpio_toggle(GPIO_Regs *gpio, uint32_t pin)
{
    gpio->DOUTTGL31_0 = pin;
}

static inline void board_leds_init(void)
{
    BLINK_LED_GPIO->GPRCM.RSTCTL =
        GPIO_RSTCTL_KEY_UNLOCK_W |
        GPIO_RSTCTL_RESETSTKYCLR_CLR |
        GPIO_RSTCTL_RESETASSERT_ASSERT;

    BLINK_LED_GPIO->GPRCM.PWREN =
        GPIO_PWREN_KEY_UNLOCK_W |
        GPIO_PWREN_ENABLE_ENABLE;

    for (volatile uint32_t delay = 0u; delay < 16u; ++delay)
    {
    }

    IOMUX->SECCFG.PINCM[IOMUX_PINCM23] =
        IOMUX_PINCM_PC_CONNECTED |
        IOMUX_PINCM_INENA_ENABLE |
        IOMUX_PINCM23_PF_GPIOA_DIO22;

    board_gpio_write(BLINK_LED_GPIO, BLINK_LED_PIN, false);
    BLINK_LED_GPIO->DOESET31_0 = BLINK_LED_PIN;
}

static inline void board_leds_toggle(void)
{
    board_gpio_toggle(BLINK_LED_GPIO, BLINK_LED_PIN);
}

#endif /* __BOARD_H__ */
