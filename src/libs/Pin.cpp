#include "Pin.h"
#include "utils.h"

// mbed libraries for hardware pwm
#include "PwmOut.h"
#include "InterruptIn.h"
#include "PinNames.h"
#include "pinmap.h"
#include "port_api.h"

#ifndef __STM32F4__
#define MAX_PIN 32
#else
#include "PeripheralPins.h"
#define MAX_PIN 16
extern "C" uint32_t Set_GPIO_Clock(uint32_t port_idx);
#endif

Pin::Pin(){
    this->inverting= false;
    this->valid= false;
    this->pin= MAX_PIN;
    this->port= nullptr;
}

// Make a new pin object from a string
Pin* Pin::from_string(std::string value){
    if(value == "nc") {
        this->valid= false;
        return this; // optimize the nc case
    }

#ifndef __STM32F4__
    static LPC_GPIO_TypeDef* gpios[] = {
        LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4};
#else
    static GPIO_TypeDef* const gpios[] = {
        GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF
#ifdef GPIOH
        , GPIOH
#endif
#ifdef GPIOI
        , GPIOI
#endif
    };
#endif
    // cs is the current position in the string
    const char* cs = value.c_str();
    // cn is the position of the next char after the number we just read
    char* cn = NULL;
    valid= true;

    // grab first integer as port. pointer to first non-digit goes in cn
    char port_letter = *cs;
    if (port_letter >= 'a' && port_letter <= 'i') {
        this->port_number = port_letter - 'a';
        cn = const_cast<char*>(cs) + 1;
    } else if (port_letter >= 'A' && port_letter <= 'I') {
        this->port_number = port_letter - 'A';
        cn = const_cast<char*>(cs) + 1;
    } else {
        this->port_number = strtol(cs, &cn, 10);
    }
    // if cn > cs then strtol read at least one digit
    if ((cn > cs) && (port_number < (sizeof(gpios)/sizeof(gpios[0])))){
#ifdef __STM32F4__
        Set_GPIO_Clock(port_number); // enable clock domain
#endif
        // translate port index into something useful
        this->port = gpios[(unsigned int) this->port_number];
        // if the char after the first integer is a . then we should expect a pin index next
        if (*cn == '.'){
            // move pointer to first digit (hopefully) of pin index
            cs = ++cn;

            // grab pin index.
            this->pin = strtol(cs, &cn, 10);

            // if strtol read some numbers, cn will point to the first non-digit
            if ((cn > cs) && (pin < MAX_PIN)){
#ifndef __STM32F4__
                this->port->FIOMASK &= ~(1 << this->pin);
#endif
                // now check for modifiers:-
                // ! = invert pin
                // o = set pin to open drain
                // ^ = set pin to pull up
                // v = set pin to pull down
                // - = set pin to no pull up or down
                // @ = set pin to repeater mode
                for (;*cn;cn++) {
                    switch(*cn) {
                        case '!':
                            this->inverting = true;
                            break;
                        case 'o':
                            as_open_drain();
                            break;
                        case '^':
                            pull_up();
                            break;
                        case 'v':
                            pull_down();
                            break;
                        case '-':
                            pull_none();
                            break;
#ifndef __STM32F4__
                        case '@':
                            as_repeater();
                            break;
#endif
                        default:
                            // skip any whitespace following the pin index
                            if (!is_whitespace(*cn))
                                return this;
                    }
                }
                return this;
            }
        }
    }

    // from_string failed. TODO: some sort of error
    valid= false;
    port_number = 0;
    port = gpios[0];
    pin = MAX_PIN;
    inverting = false;
    return this;
}

// Configure this pin as OD
Pin* Pin::as_open_drain(){
    if (!this->valid) return this;
#ifndef __STM32F4__
    if( this->port_number == 0 ){ LPC_PINCON->PINMODE_OD0 |= (1<<this->pin); }
    if( this->port_number == 1 ){ LPC_PINCON->PINMODE_OD1 |= (1<<this->pin); }
    if( this->port_number == 2 ){ LPC_PINCON->PINMODE_OD2 |= (1<<this->pin); }
    if( this->port_number == 3 ){ LPC_PINCON->PINMODE_OD3 |= (1<<this->pin); }
    if( this->port_number == 4 ){ LPC_PINCON->PINMODE_OD4 |= (1<<this->pin); }
#else
    this->port->OTYPER |= (1 << this->pin);
#endif    
    pull_none(); // no pull up by default
    return this;
}

#ifndef __STM32F4__
// Configure this pin as a repeater
Pin* Pin::as_repeater(){
    if (!this->valid) return this;
    // Set the two bits for this pin as 01
    if( this->port_number == 0 && this->pin < 16  ){ LPC_PINCON->PINMODE0 |= (1<<( this->pin*2)); LPC_PINCON->PINMODE0 &= ~(2<<( this->pin    *2)); }
    if( this->port_number == 0 && this->pin >= 16 ){ LPC_PINCON->PINMODE1 |= (1<<( this->pin*2)); LPC_PINCON->PINMODE1 &= ~(2<<((this->pin-16)*2)); }
    if( this->port_number == 1 && this->pin < 16  ){ LPC_PINCON->PINMODE2 |= (1<<( this->pin*2)); LPC_PINCON->PINMODE2 &= ~(2<<( this->pin    *2)); }
    if( this->port_number == 1 && this->pin >= 16 ){ LPC_PINCON->PINMODE3 |= (1<<( this->pin*2)); LPC_PINCON->PINMODE3 &= ~(2<<((this->pin-16)*2)); }
    if( this->port_number == 2 && this->pin < 16  ){ LPC_PINCON->PINMODE4 |= (1<<( this->pin*2)); LPC_PINCON->PINMODE4 &= ~(2<<( this->pin    *2)); }
    if( this->port_number == 3 && this->pin >= 16 ){ LPC_PINCON->PINMODE7 |= (1<<( this->pin*2)); LPC_PINCON->PINMODE7 &= ~(2<<((this->pin-16)*2)); }
    if( this->port_number == 4 && this->pin >= 16 ){ LPC_PINCON->PINMODE9 |= (1<<( this->pin*2)); LPC_PINCON->PINMODE9 &= ~(2<<((this->pin-16)*2)); }
    return this;
}
#endif    

// Configure this pin as no pullup or pulldown
Pin* Pin::pull_none(){
	if (!this->valid) return this;
#ifndef __STM32F4__
	// Set the two bits for this pin as 10
	if( this->port_number == 0 && this->pin < 16  ){ LPC_PINCON->PINMODE0 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE0 &= ~(1<<( this->pin    *2)); }
	if( this->port_number == 0 && this->pin >= 16 ){ LPC_PINCON->PINMODE1 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE1 &= ~(1<<((this->pin-16)*2)); }
	if( this->port_number == 1 && this->pin < 16  ){ LPC_PINCON->PINMODE2 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE2 &= ~(1<<( this->pin    *2)); }
	if( this->port_number == 1 && this->pin >= 16 ){ LPC_PINCON->PINMODE3 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE3 &= ~(1<<((this->pin-16)*2)); }
	if( this->port_number == 2 && this->pin < 16  ){ LPC_PINCON->PINMODE4 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE4 &= ~(1<<( this->pin    *2)); }
	if( this->port_number == 3 && this->pin >= 16 ){ LPC_PINCON->PINMODE7 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE7 &= ~(1<<((this->pin-16)*2)); }
	if( this->port_number == 4 && this->pin >= 16 ){ LPC_PINCON->PINMODE9 |= (2<<( this->pin*2)); LPC_PINCON->PINMODE9 &= ~(1<<((this->pin-16)*2)); }
#else
    // Set the two bits for this pin as b00
    this->port->PUPDR &= ~(0x3 << (2*this->pin));
#endif    
	return this;
}

// Configure this pin as a pullup
Pin* Pin::pull_up(){
    if (!this->valid) return this;
#ifndef __STM32F4__
    // Set the two bits for this pin as 00
    if( this->port_number == 0 && this->pin < 16  ){ LPC_PINCON->PINMODE0 &= ~(3<<( this->pin    *2)); }
    if( this->port_number == 0 && this->pin >= 16 ){ LPC_PINCON->PINMODE1 &= ~(3<<((this->pin-16)*2)); }
    if( this->port_number == 1 && this->pin < 16  ){ LPC_PINCON->PINMODE2 &= ~(3<<( this->pin    *2)); }
    if( this->port_number == 1 && this->pin >= 16 ){ LPC_PINCON->PINMODE3 &= ~(3<<((this->pin-16)*2)); }
    if( this->port_number == 2 && this->pin < 16  ){ LPC_PINCON->PINMODE4 &= ~(3<<( this->pin    *2)); }
    if( this->port_number == 3 && this->pin >= 16 ){ LPC_PINCON->PINMODE7 &= ~(3<<((this->pin-16)*2)); }
    if( this->port_number == 4 && this->pin >= 16 ){ LPC_PINCON->PINMODE9 &= ~(3<<((this->pin-16)*2)); }
#else
    // Set the two bits for this pin as b01
    this->port->PUPDR = (this->port->PUPDR & ~(0x3 << (2*this->pin))) | (0x1 << (2*this->pin));
#endif    
    return this;
}

// Configure this pin as a pulldown
Pin* Pin::pull_down(){
    if (!this->valid) return this;
#ifndef __STM32F4__
    // Set the two bits for this pin as 11
    if( this->port_number == 0 && this->pin < 16  ){ LPC_PINCON->PINMODE0 |= (3<<( this->pin    *2)); }
    if( this->port_number == 0 && this->pin >= 16 ){ LPC_PINCON->PINMODE1 |= (3<<((this->pin-16)*2)); }
    if( this->port_number == 1 && this->pin < 16  ){ LPC_PINCON->PINMODE2 |= (3<<( this->pin    *2)); }
    if( this->port_number == 1 && this->pin >= 16 ){ LPC_PINCON->PINMODE3 |= (3<<((this->pin-16)*2)); }
    if( this->port_number == 2 && this->pin < 16  ){ LPC_PINCON->PINMODE4 |= (3<<( this->pin    *2)); }
    if( this->port_number == 3 && this->pin >= 16 ){ LPC_PINCON->PINMODE7 |= (3<<((this->pin-16)*2)); }
    if( this->port_number == 4 && this->pin >= 16 ){ LPC_PINCON->PINMODE9 |= (3<<((this->pin-16)*2)); }
#else
    // Set the two bits for this pin as b10 
    this->port->PUPDR = (this->port->PUPDR & ~(0x3 << (2*this->pin))) | (0x2 << (2*this->pin));
#endif
    return this;
}

// If available on this pin, return mbed hardware pwm class for this pin
mbed::PwmOut* Pin::hardware_pwm()
{
#ifndef __STM32F4__
    if (port_number == 1)
    {
        if (pin == 18) { return new mbed::PwmOut(P1_18); }
        if (pin == 20) { return new mbed::PwmOut(P1_20); }
        if (pin == 21) { return new mbed::PwmOut(P1_21); }
        if (pin == 23) { return new mbed::PwmOut(P1_23); }
        if (pin == 24) { return new mbed::PwmOut(P1_24); }
        if (pin == 26) { return new mbed::PwmOut(P1_26); }
    }
    else if (port_number == 2)
    {
        if (pin == 0) { return new mbed::PwmOut(P2_0); }
        if (pin == 1) { return new mbed::PwmOut(P2_1); }
        if (pin == 2) { return new mbed::PwmOut(P2_2); }
        if (pin == 3) { return new mbed::PwmOut(P2_3); }
        if (pin == 4) { return new mbed::PwmOut(P2_4); }
        if (pin == 5) { return new mbed::PwmOut(P2_5); }
    }
    else if (port_number == 3)
    {
        if (pin == 25) { return new mbed::PwmOut(P3_25); }
        if (pin == 26) { return new mbed::PwmOut(P3_26); }
    }
#else
    PinName pin = port_pin((PortName)this->port_number, this->pin);
    if (pinmap_peripheral(pin, PinMap_PWM) != (uint32_t)NC)
        return new mbed::PwmOut(pin);
#endif    
    return nullptr;
}

mbed::InterruptIn* Pin::interrupt_pin()
{
    if(!this->valid) return nullptr;

    // set as input
    as_input();

#ifndef __STM32F4__
    if (port_number == 0 || port_number == 2) {
        PinName pinname = port_pin((PortName)port_number, pin);
        return new mbed::InterruptIn(pinname);

    } else {
        this->valid= false;
        return nullptr;
    }
    return nullptr;
#else
    // all pins support interrupts on stm32
    PinName pinname = port_pin((PortName)port_number, pin);
    return new mbed::InterruptIn(pinname);
#endif    
}
