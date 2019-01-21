#include "MRI_Hooks.h"
#include <mbed.h>
#include <mri.h>

// This is used by MRI to turn pins on and off when entering and leaving MRI. Useful for not burning everything down
// See http://smoothieware.org/mri-debugging 

extern "C" {

#ifndef __STM32F4__
    static LPC_GPIO_TypeDef* io;
    #define GPIO_PORT_COUNT     5
    #define GPIO_PIN_MAX        32
#else
    static GPIO_TypeDef* io;

    static GPIO_TypeDef* const gpios[] = {
        GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG
#ifdef GPIOH        
        , GPIOH
#endif
#ifdef GPIOI
        , GPIOI
#endif
    };

    #define GPIO_PORT_COUNT     (sizeof(gpios)/sizeof(gpios[0]))
    #define GPIO_PIN_MAX        16
#endif
    static uint32_t _set_high_on_debug[GPIO_PORT_COUNT] = {0};
    static uint32_t _set_low_on_debug[GPIO_PORT_COUNT]  = {0};
    static uint32_t _previous_state[GPIO_PORT_COUNT];

    static unsigned i;

    void __mriPlatform_EnteringDebuggerHook()
    {
        for (i = 0; i < GPIO_PORT_COUNT; i++)
        {
#ifndef __STM32F4__
            io           = (LPC_GPIO_TypeDef*) (LPC_GPIO_BASE + (0x20 * i));
            io->FIOMASK &= ~(_set_high_on_debug[i] | _set_low_on_debug[i]);

            _previous_state[i] = io->FIOPIN;

            io->FIOSET   = _set_high_on_debug[i];
            io->FIOCLR   = _set_low_on_debug[i];
#else
            io          = gpios[i];
            _previous_state[i] = io->IDR;
            io->BSRR    = _set_high_on_debug[i];
            io->BSRR    = (_set_low_on_debug[i] << 16);
#endif
        }
    }

    void __mriPlatform_LeavingDebuggerHook()
    {
        for (i = 0; i < GPIO_PORT_COUNT; i++)
        {
#ifndef __STM32F4__
            io           = (LPC_GPIO_TypeDef*) (LPC_GPIO_BASE + (0x20 * i));
            io->FIOMASK &= ~(_set_high_on_debug[i] | _set_low_on_debug[i]);
            io->FIOSET   =   _previous_state[i]  & (_set_high_on_debug[i] | _set_low_on_debug[i]);
            io->FIOCLR   = (~_previous_state[i]) & (_set_high_on_debug[i] | _set_low_on_debug[i]);
#else
            io       = gpios[i];
            io->BSRR = _previous_state[i]  & (_set_high_on_debug[i] | _set_low_on_debug[i]);
            io->BSRR = ((~_previous_state[i]) & (_set_high_on_debug[i] | _set_low_on_debug[i])) << 16;
#endif
        }
    }

    void set_high_on_debug(int port, int pin)
    {
        if ((port >= (int)GPIO_PORT_COUNT) || (port < 0))
            return;
        if ((pin >= GPIO_PIN_MAX) || (pin < 0))
            return;
        _set_high_on_debug[port] |= (1<<pin);
    }

    void set_low_on_debug(int port, int pin)
    {
        if ((port >= (int)GPIO_PORT_COUNT) || (port < 0))
            return;
        if ((pin >= GPIO_PIN_MAX) || (pin < 0))
            return;
        _set_low_on_debug[port] |= (1<<pin);
    }
}
