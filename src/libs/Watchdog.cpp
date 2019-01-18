#include "Watchdog.h"
#include "Kernel.h"
#include <mri.h>

#include "mbed.h"
#ifndef DISABLELEDS
extern DigitalOut leds[];
#endif

#ifdef __STM32F4__
static WWDG_HandleTypeDef m_wdt_handle;
extern "C" void WWDG_IRQHandler(void);
#endif

// TODO : comment this
// Basically, when stuff stop answering, reset, or enter MRI mode, or something

Watchdog::Watchdog(uint32_t timeout, WDT_ACTION action)
{
#ifndef __STM32F4__
    WDT_Init(WDT_CLKSRC_IRC, (action == WDT_MRI)?WDT_MODE_INT_ONLY:WDT_MODE_RESET);
    WDT_Start(timeout);
    WDT_Feed();
    if(action == WDT_MRI) {
        // enable the interrupt
        NVIC_EnableIRQ(WDT_IRQn);
        NVIC_SetPriority(WDT_IRQn, 1);
    }
#else
    m_wdt_handle.Instance         = WWDG;
    m_wdt_handle.Init.Prescaler   = WWDG_CFR_WDGTB; // prescale /8
    m_wdt_handle.Init.Window      = WWDG_CFR_W; // load max values, still timeout ~ 100ms
    m_wdt_handle.Init.Counter     = WWDG_CR_T;  // TODO rewrite to use IWDG for longer timeouts
    m_wdt_handle.Init.EWIMode     = (action == WDT_MRI) ? WWDG_CFR_EWI : 0;   

    __HAL_RCC_WWDG_CLK_ENABLE();

    //HAL_WWDG_Init(&m_wdt_handle);
    feed();
    if(action == WDT_MRI) {
        // enable the interrupt
        NVIC_SetVector(WWDG_IRQn, (uint32_t)WWDG_IRQHandler);
        NVIC_EnableIRQ(WWDG_IRQn);
    }
#endif
}

void Watchdog::feed()
{
#ifndef __STM32F4__
    WDT_Feed();
#else
    HAL_WWDG_Refresh(&m_wdt_handle);
#endif
}

void Watchdog::on_module_loaded()
{
    register_for_event(ON_IDLE);
    feed();
}

void Watchdog::on_idle(void*)
{
    feed();
}


// when watchdog triggers, set a led pattern and enter MRI which turns everything off into a safe state
// TODO handle when MRI is disabled
#ifndef __STM32F4__
extern "C" void WDT_IRQHandler(void)
{
#ifndef DISABLELEDS
    if(THEKERNEL->is_using_leds()) {
        // set led pattern to show we are in watchdog timeout
        leds[0]= 0;
        leds[1]= 1;
        leds[2]= 0;
        leds[3]= 1;
    }
#endif
    WDT_ClrTimeOutFlag(); // bootloader uses this flag to enter DFU mode
    WDT_Feed();
    __debugbreak();
}
#else
extern "C" void WWDG_IRQHandler(void)
{
#ifndef DISABLELEDS
    if(THEKERNEL->is_using_leds()) {
        // set led pattern to show we are in watchdog timeout
        leds[0]= 0;
        leds[1]= 1;
        leds[2]= 0;
        leds[3]= 1;
    }
#endif
    HAL_WWDG_IRQHandler(&m_wdt_handle); // clears int flag
    //HAL_WWDG_Refresh(&m_wdt_handle);  // don't refresh, let wdt reset device
    __debugbreak();                     // but trigger breakpoint if we're on a debugger
}
#endif // __STM32F4__
