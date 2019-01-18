#include "gpio.h"

#ifndef __STM32F4__
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#else
#include "stm32f4xx.h"

static GPIO_TypeDef* const gpios[] = {
	GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG
#ifdef GPIOH
	, GPIOH
#endif
#ifdef GPIOI
	, GPIOI
#endif
	};

extern "C" uint32_t Set_GPIO_Clock(uint32_t port_idx);
#endif

GPIO::GPIO(PinName pin) {
#ifndef __STM32F4__	
    this->port = (pin >> 5) & 7;
    this->pin = pin & 0x1F;
#else
    this->port = STM_PORT(pin);
	this->pin = STM_PIN(pin);
#endif
    setup();
}

GPIO::GPIO(uint8_t port, uint8_t pin) {
	GPIO::port = port;
	GPIO::pin = pin;

	setup();
}

GPIO::GPIO(uint8_t port, uint8_t pin, uint8_t direction) {
	GPIO::port = port;
	GPIO::pin = pin;

	setup();

	set_direction(direction);
}
// GPIO::~GPIO() {}

void GPIO::setup() {
#ifndef __STM32F4__
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = GPIO::port;
	PinCfg.Pinnum = GPIO::pin;
	PINSEL_ConfigPin(&PinCfg);
#else
	Set_GPIO_Clock(port);
	gpios[port]->OTYPER &= ~(1 << pin);
    gpios[port]->PUPDR &= ~(0x3 << (2*pin));    
#endif	
}

void GPIO::set_direction(uint8_t direction) {
#ifndef __STM32F4__	
	FIO_SetDir(port, 1UL << pin, direction);
#else
    if (direction) {
		output();
	} else {
		input();
	}
#endif
}

void GPIO::output() {
#ifndef __STM32F4__
	set_direction(1);
#else	
    gpios[port]->MODER = (gpios[port]->MODER & ~(0x3<<(2*pin))) | (0x1<<(2*pin));
#endif	
}

void GPIO::input() {
#ifndef __STM32F4__
	set_direction(0);
#else
    gpios[port]->MODER &= ~(0x3<<(2*pin));
#endif	
}

void GPIO::write(uint8_t value) {
	output();
	if (value)
		set();
	else
		clear();
}

void GPIO::set() {
#ifndef __STM32F4__
	FIO_SetValue(port, 1UL << pin);
#else	
	gpios[port]->BSRR = (0x1 << pin);
#endif
}

void GPIO::clear() {
#ifndef __STM32F4__
	FIO_ClearValue(port, 1UL << pin);
#else
    gpios[port]->BSRR = ((0x1 << 16) << pin);
#endif
}

uint8_t GPIO::get() {
#ifndef __STM32F4__
	return (FIO_ReadValue(port) & (1UL << pin))?255:0;
#else
    return (gpios[port]->IDR & (1UL << pin)) ? 0xFF : 0;
#endif
}

int GPIO::operator=(int value) {
    if (value)
        set();
    else
        clear();
    return value;
}
