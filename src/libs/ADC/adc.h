/* mbed Library - ADC
 * Copyright (c) 2010, sblandford
 * released under MIT license http://mbed.org/licence/mit
 */

#ifndef MBED_ADC_H
#define MBED_ADC_H

#include "PinNames.h" // mbed.h lib
#define XTAL_FREQ       12000000
#define MAX_ADC_CLOCK   13000000
#define CLKS_PER_SAMPLE 64
#ifdef __STM32F4__
#define ADC_CHANNEL_COUNT   16
#else
#define ADC_CHANNEL_COUNT   6
#endif

namespace mbed {

class ADC {
public:

    //Initialize ADC with ADC maximum sample rate of
    //sample_rate and system clock divider of cclk_div
    //Maximum recommened sample rate is 184000
    ADC(int sample_rate, int cclk_div);

    //Enable/disable ADC on pin according to state
    //and also select/de-select for next conversion
    void setup(PinName pin, int state);

    //Enable/disable burst mode according to state
    void burst(int state);

    //Return burst mode enabled/disabled
    int burst(void);

    //Return startmode state according to mode_edge=0: mode and mode_edge=1: edge
    int startmode(int mode_edge);

    //Set interrupt enable/disable for pin to state
    void interrupt_state(PinName pin, int state);

    //Append custom global interrupt handler
    void append(void(*fptr)(int chan, uint32_t value));

    int _pin_to_channel(PinName pin);
private:
    void adcisr(void);
    static void _adcisr(void);
    static ADC *instance;

    void(*_adc_g_isr)(int chan, uint32_t value);
#ifndef __STM32F4__    
    uint32_t _adc_data[8];
    int _adc_clk_freq;
#else
    //Callback for attaching to slowticker as scan start timer 
    uint32_t on_tick(uint32_t dummy);

    bool attached;
    uint8_t scan_chan_lut[ADC_CHANNEL_COUNT];
    uint8_t scan_count_next;
    uint32_t interrupt_mask;
    bool active;
    bool paused;
    uint8_t skipped_ticks;
    uint16_t adc_data[ADC_CHANNEL_COUNT];
#endif
};

} /* namespace mbed */

#endif
