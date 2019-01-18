#ifndef PIN_H
#define PIN_H

#include <stdlib.h>
#include <stdio.h>
#include <string>

#ifndef __STM32F4__
#include "libs/LPC17xx/sLPC17xx.h" // smoothed mbed.h lib
#endif
#include "PinNames.h"

namespace mbed {
    class PwmOut;
    class InterruptIn;
}

class Pin {
    public:
        Pin();

        Pin* from_string(std::string value);

        inline bool connected(){
            return this->valid;
        }

        inline bool equals(const Pin& other) const {
            return (this->pin == other.pin) && (this->port == other.port);
        }

        inline Pin* as_output(){
            if (this->valid) {
#ifdef __STM32F4__
                this->port->MODER = (this->port->MODER & ~(0x3<<(2*this->pin))) | (0x1<<(2*this->pin));
#else                
                this->port->FIODIR |= 1<<this->pin;
#endif
            }
            return this;
        }

        inline Pin* as_input(){
            if (this->valid) {
#ifdef __STM32F4__
                this->port->MODER &= ~(0x3<<(2*this->pin));
#else                
                this->port->FIODIR &= ~(1<<this->pin);
#endif                
            }
            return this;
        }

        Pin* as_open_drain(void);
#ifndef __STM32F4__
        Pin* as_repeater(void);
#endif
        Pin* pull_up(void);

        Pin* pull_down(void);

        Pin* pull_none(void);

        inline bool get() const{
            if (!this->valid) return false;
#ifdef __STM32F4__
            return this->inverting ^ (( this->port->IDR >> this->pin ) & 1);
#else                
            return this->inverting ^ (( this->port->FIOPIN >> this->pin ) & 1);
#endif            
        }

        inline void set(bool value){
            if (!this->valid) return;
            if ( this->inverting ^ value ) {
#ifdef __STM32F4__
                this->port->BSRR = 1 << this->pin;
#else
                this->port->FIOSET = 1 << this->pin;
#endif                
            } else {
#ifdef __STM32F4__
                this->port->BSRR = (1 << 16) << this->pin;
#else
                this->port->FIOCLR = 1 << this->pin;
#endif                
            }
        }

        mbed::PwmOut *hardware_pwm();

        mbed::InterruptIn *interrupt_pin();

        bool is_inverting() const { return inverting; }
        void set_inverting(bool f) { inverting= f; }

        // these should be private, and use getters
#ifdef __STM32F4__
        GPIO_TypeDef* port;
#else        
        LPC_GPIO_TypeDef* port;
#endif
        uint8_t pin;
        uint8_t port_number;

    private:
        struct {
            bool inverting:1;
            bool valid:1;
        };
};




#endif
