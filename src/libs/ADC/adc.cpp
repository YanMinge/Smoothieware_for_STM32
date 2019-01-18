/* mbed Library - ADC
 * Copyright (c) 2010, sblandford
 * released under MIT license http://mbed.org/licence/mit
 */
#include "mbed.h"
#undef ADC
#include "adc.h"

#ifdef __STM32F4__
#include "pinmap.h"
#include "PeripheralPins.h"
#include "mri.h"
#include <vector>
#include "SlowTicker.h"
#include "Kernel.h"
#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_bus.h"
//#include "stm32f4xx_ll_gpio.h"

#define STM_ADC ADC1
extern "C" uint32_t Set_GPIO_Clock(uint32_t port);
#define ADC_SQR3_SQ2_Pos          (5U)                                         
#define ADC_SQR2_SQ8_Pos          (5U)                                         
#define ADC_SQR1_SQ14_Pos         (5U)                                         
#endif

using namespace mbed;

ADC *ADC::instance;

#ifndef __STM32F4__
ADC::ADC(int sample_rate, int cclk_div){
    int i, adc_clk_freq, pclk, clock_div, max_div=1;

    //Work out CCLK
    adc_clk_freq=CLKS_PER_SAMPLE*sample_rate;
    int m = (LPC_SC->PLL0CFG & 0xFFFF) + 1;
    int n = (LPC_SC->PLL0CFG >> 16) + 1;
    int cclkdiv = LPC_SC->CCLKCFG + 1;
    int Fcco = (2 * m * XTAL_FREQ) / n;
    int cclk = Fcco / cclkdiv;

    //Power up the ADC
    LPC_SC->PCONP |= (1 << 12);
    //Set clock at cclk / 1.
    LPC_SC->PCLKSEL0 &= ~(0x3 << 24);
    switch (cclk_div) {
        case 1:
            LPC_SC->PCLKSEL0 |= 0x1 << 24;
            break;
        case 2:
            LPC_SC->PCLKSEL0 |= 0x2 << 24;
            break;
        case 4:
            LPC_SC->PCLKSEL0 |= 0x0 << 24;
            break;
        case 8:
            LPC_SC->PCLKSEL0 |= 0x3 << 24;
            break;
        default:
            printf("ADC Warning: ADC CCLK clock divider must be 1, 2, 4 or 8. %u supplied.\n",
                cclk_div);
            printf("Defaulting to 1.\n");
            LPC_SC->PCLKSEL0 |= 0x1 << 24;
            break;
    }
    pclk = cclk / cclk_div;
    clock_div=pclk / adc_clk_freq;

    if (clock_div > 0xFF) {
        printf("ADC Warning: Clock division is %u which is above 255 limit. Re-Setting at limit.\n", clock_div);
        clock_div=0xFF;
    }
    if (clock_div == 0) {
        printf("ADC Warning: Clock division is 0. Re-Setting to 1.\n");
        clock_div=1;
    }

    _adc_clk_freq=pclk / clock_div;
    if (_adc_clk_freq > MAX_ADC_CLOCK) {
        printf("ADC Warning: Actual ADC sample rate of %u which is above %u limit\n",
            _adc_clk_freq / CLKS_PER_SAMPLE, MAX_ADC_CLOCK / CLKS_PER_SAMPLE);
        while ((pclk / max_div) > MAX_ADC_CLOCK) max_div++;
        printf("ADC Warning: Maximum recommended sample rate is %u\n", (pclk / max_div) / CLKS_PER_SAMPLE);
    }

    LPC_ADC->ADCR =
        ((clock_div - 1 ) << 8 ) |    //Clkdiv
        ( 1 << 21 );                  //A/D operational

    //Default no channels enabled
    LPC_ADC->ADCR &= ~0xFF;
    //Default NULL global custom isr
    _adc_g_isr = NULL;
    //Initialize arrays
    for (i=7; i>=0; i--) {
        _adc_data[i] = 0;
    }

    //* Attach IRQ
    instance = this;
    NVIC_SetVector(ADC_IRQn, (uint32_t)&_adcisr);

    //Disable global interrupt
    LPC_ADC->ADINTEN &= ~0x100;
};

void ADC::adcisr(void)
{
    uint32_t stat;
    int chan;

    // Read status
    stat = LPC_ADC->ADSTAT;
    //Scan channels for over-run or done and update array
    if (stat & 0x0101) _adc_data[0] = LPC_ADC->ADDR0;
    if (stat & 0x0202) _adc_data[1] = LPC_ADC->ADDR1;
    if (stat & 0x0404) _adc_data[2] = LPC_ADC->ADDR2;
    if (stat & 0x0808) _adc_data[3] = LPC_ADC->ADDR3;
    if (stat & 0x1010) _adc_data[4] = LPC_ADC->ADDR4;
    if (stat & 0x2020) _adc_data[5] = LPC_ADC->ADDR5;
    if (stat & 0x4040) _adc_data[6] = LPC_ADC->ADDR6;
    if (stat & 0x8080) _adc_data[7] = LPC_ADC->ADDR7;

    // Channel that triggered interrupt
    chan = (LPC_ADC->ADGDR >> 24) & 0x07;
    //User defined interrupt handlers
    if (_adc_g_isr != NULL)
        _adc_g_isr(chan, _adc_data[chan]);
    return;
}

int ADC::_pin_to_channel(PinName pin) {
    int chan;
    switch (pin) {
        case p15://=p0.23 of LPC1768
        default:
            chan=0;
            break;
        case p16://=p0.24 of LPC1768
            chan=1;
            break;
        case p17://=p0.25 of LPC1768
            chan=2;
            break;
        case p18://=p0.26 of LPC1768
            chan=3;
            break;
        case p19://=p1.30 of LPC1768
            chan=4;
            break;
        case p20://=p1.31 of LPC1768
            chan=5;
            break;
    }
    return(chan);
}

//Enable or disable an ADC pin
void ADC::setup(PinName pin, int state) {
    int chan;
    chan=_pin_to_channel(pin);
    if ((state & 1) == 1) {
        switch(pin) {
            case p15://=p0.23 of LPC1768
            default:
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 14);
                LPC_PINCON->PINSEL1 |= (unsigned int)0x1 << 14;
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 14);
                LPC_PINCON->PINMODE1 |= (unsigned int)0x2 << 14;
                break;
            case p16://=p0.24 of LPC1768
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 16);
                LPC_PINCON->PINSEL1 |= (unsigned int)0x1 << 16;
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 16);
                LPC_PINCON->PINMODE1 |= (unsigned int)0x2 << 16;
                break;
            case p17://=p0.25 of LPC1768
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 18);
                LPC_PINCON->PINSEL1 |= (unsigned int)0x1 << 18;
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 18);
                LPC_PINCON->PINMODE1 |= (unsigned int)0x2 << 18;
                break;
            case p18://=p0.26 of LPC1768:
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 20);
                LPC_PINCON->PINSEL1 |= (unsigned int)0x1 << 20;
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 20);
                LPC_PINCON->PINMODE1 |= (unsigned int)0x2 << 20;
                break;
            case p19://=p1.30 of LPC1768
                LPC_PINCON->PINSEL3 &= ~((unsigned int)0x3 << 28);
                LPC_PINCON->PINSEL3 |= (unsigned int)0x3 << 28;
                LPC_PINCON->PINMODE3 &= ~((unsigned int)0x3 << 28);
                LPC_PINCON->PINMODE3 |= (unsigned int)0x2 << 28;
                break;
            case p20://=p1.31 of LPC1768
                LPC_PINCON->PINSEL3 &= ~((unsigned int)0x3 << 30);
                LPC_PINCON->PINSEL3 |= (unsigned int)0x3 << 30;
                LPC_PINCON->PINMODE3 &= ~((unsigned int)0x3 << 30);
                LPC_PINCON->PINMODE3 |= (unsigned int)0x2 << 30;
               break;
        }
        //Only one channel can be selected at a time if not in burst mode
        if (!burst()) LPC_ADC->ADCR &= ~0xFF;
        //Select channel
        LPC_ADC->ADCR |= (1 << chan);
    } else {
        switch(pin) {
            case p15://=p0.23 of LPC1768
            default:
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 14);
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 14);
                break;
            case p16://=p0.24 of LPC1768
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 16);
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 16);
                break;
            case p17://=p0.25 of LPC1768
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 18);
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 18);
                break;
            case p18://=p0.26 of LPC1768:
                LPC_PINCON->PINSEL1 &= ~((unsigned int)0x3 << 20);
                LPC_PINCON->PINMODE1 &= ~((unsigned int)0x3 << 20);
                break;
            case p19://=p1.30 of LPC1768
                LPC_PINCON->PINSEL3 &= ~((unsigned int)0x3 << 28);
                LPC_PINCON->PINMODE3 &= ~((unsigned int)0x3 << 28);
                break;
            case p20://=p1.31 of LPC1768
                LPC_PINCON->PINSEL3 &= ~((unsigned int)0x3 << 30);
                LPC_PINCON->PINMODE3 &= ~((unsigned int)0x3 << 30);
                break;
        }
        LPC_ADC->ADCR &= ~(1 << chan);
    }
}

//Enable or disable burst mode
void ADC::burst(int state) {
    if ((state & 1) == 1) {
        if (startmode(0) != 0)
            fprintf(stderr, "ADC Warning. startmode is %u. Must be 0 for burst mode.\n", startmode(0));
        LPC_ADC->ADCR |= (1 << 16);
    } else {
        LPC_ADC->ADCR &= ~(1 << 16);
    }
}

//Return burst mode state
int  ADC::burst(void) {
    return((LPC_ADC->ADCR & (1 << 16)) >> 16);
}

//Return startmode state according to mode_edge=0: mode and mode_edge=1: edge
int ADC::startmode(int mode_edge){
    switch (mode_edge) {
        case 0:
        default:
            return((LPC_ADC->ADCR >> 24) & 0x07);
        case 1:
            return((LPC_ADC->ADCR >> 27) & 0x01);
    }
}

//Set interrupt enable/disable for pin to state
void ADC::interrupt_state(PinName pin, int state) {
    int chan;

    chan = _pin_to_channel(pin);
    if (state == 1) {
        LPC_ADC->ADINTEN &= ~0x100;
        LPC_ADC->ADINTEN |= 1 << chan;
        /* Enable the ADC Interrupt */
        NVIC_EnableIRQ(ADC_IRQn);
    } else {
        LPC_ADC->ADINTEN &= ~( 1 << chan );
        //Disable interrrupt if no active pins left
        if ((LPC_ADC->ADINTEN & 0xFF) == 0)
            NVIC_DisableIRQ(ADC_IRQn);
    }
}

#else  // defined(__STM32F4__)

static void MX_DMA_Init(void) {
  /* Init with LL driver */
  /* DMA controller clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

  /* DMA interrupt enable */
  NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

static void MX_ADC_Init(void* adc_buffer) {
  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
  /* STM_ADC DMA Init */
  
  /* STM_ADC Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_0, LL_DMA_CHANNEL_0);
  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_HALFWORD);
  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_HALFWORD);
  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_0);
  LL_DMA_ConfigAddresses(
    DMA2,
    LL_DMA_STREAM_0,
    LL_ADC_DMA_GetRegAddr(STM_ADC, LL_ADC_DMA_REG_REGULAR_DATA),
    (uint32_t)adc_buffer,
    LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
}

static uint32_t adcChannelIDMap[] = {
    LL_ADC_CHANNEL_0,
    LL_ADC_CHANNEL_1,
    LL_ADC_CHANNEL_2,
    LL_ADC_CHANNEL_3,
    LL_ADC_CHANNEL_4,
    LL_ADC_CHANNEL_5,
    LL_ADC_CHANNEL_6,
    LL_ADC_CHANNEL_7,
    LL_ADC_CHANNEL_8,
    LL_ADC_CHANNEL_9,
    LL_ADC_CHANNEL_10,
    LL_ADC_CHANNEL_11,
    LL_ADC_CHANNEL_12,
    LL_ADC_CHANNEL_13,
    LL_ADC_CHANNEL_14,
    LL_ADC_CHANNEL_15,
    LL_ADC_CHANNEL_16,
    LL_ADC_CHANNEL_17,
    LL_ADC_CHANNEL_18
};

static uint32_t adcChannelRankMap[] = {
    LL_ADC_REG_RANK_1,
    LL_ADC_REG_RANK_2,
    LL_ADC_REG_RANK_3,
    LL_ADC_REG_RANK_4,
    LL_ADC_REG_RANK_5,
    LL_ADC_REG_RANK_6,
    LL_ADC_REG_RANK_7,
    LL_ADC_REG_RANK_8,
    LL_ADC_REG_RANK_9,
    LL_ADC_REG_RANK_10,
    LL_ADC_REG_RANK_11,
    LL_ADC_REG_RANK_12,
    LL_ADC_REG_RANK_13,
    LL_ADC_REG_RANK_14,
    LL_ADC_REG_RANK_15,
    LL_ADC_REG_RANK_16
};

static void MX_ADC_Init_Channels(uint8_t num_channels, uint8_t *channel_ids) {
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct;
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct;
    LL_ADC_InitTypeDef ADC_InitStruct;

    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    // If we have only one channel, disable scan mode
    if (num_channels < 2) {
        ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
        ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
        ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
        LL_ADC_Init(STM_ADC, &ADC_InitStruct);
        ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
    } else {
        ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
        ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
        ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_ENABLE;
        LL_ADC_Init(STM_ADC, &ADC_InitStruct);
        ADC_REG_InitStruct.SequencerLength = (num_channels - 1) << ADC_SQR1_L_Pos;//LL_ADC_REG_SEQ_SCAN_ENABLE_2RANKS;
    }
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_LIMITED;
    LL_ADC_REG_Init(STM_ADC, &ADC_REG_InitStruct);
    LL_ADC_REG_SetFlagEndOfConversion(STM_ADC, LL_ADC_REG_FLAG_EOC_SEQUENCE_CONV);
    LL_ADC_DisableIT_EOCS(STM_ADC);

    ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV8;
    ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
    LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(STM_ADC), &ADC_CommonInitStruct);

    for (uint8_t idx = 0; idx < ADC_CHANNEL_COUNT; idx++) {
        uint8_t channel_rank = channel_ids[idx];
        if (channel_rank != 0xff) {
            uint32_t channel_id = adcChannelIDMap[idx];
            /**Configure Regular Channel 
             */
            LL_ADC_REG_SetSequencerRanks(
                STM_ADC,
                adcChannelRankMap[channel_rank],
                channel_id);
            LL_ADC_SetChannelSamplingTime(
                STM_ADC,
                channel_id,
                LL_ADC_SAMPLINGTIME_480CYCLES);
        }
    }

    LL_ADC_Enable(STM_ADC);
}

ADC::ADC(int sample_rate, int cclk_div) {
    scan_count_next = 0;
    skipped_ticks = 0;
    attached = false;
    interrupt_mask = 0;

    memset(scan_chan_lut, 0xFF, sizeof(scan_chan_lut));
    for (uint8_t idx = 0; idx < ADC_CHANNEL_COUNT; idx++) {
        adc_data[idx] = 4095;
    }

    MX_DMA_Init();
    MX_ADC_Init(adc_data);
    NVIC_SetVector(DMA2_Stream0_IRQn, (uint32_t)&_adcisr);

    _adc_g_isr = NULL;
    instance = this;
    active = false;
    paused = false;
}

int ADC::_pin_to_channel(PinName pin) {
    uint32_t function = pinmap_function(pin, PinMap_ADC);
    uint8_t chan = 0xFF;

    if (function != (uint32_t)NC)
        chan = scan_chan_lut[STM_PIN_CHANNEL(function)];

    return chan;
}

// enable or disable burst mode
void ADC::burst(int state) {
    if (state && !attached && scan_count_next > 0) {
        // Finish configuring the ADC peripheral.
        MX_ADC_Init_Channels(scan_count_next, scan_chan_lut);
        // Schedule adc scan at 1kHz rate
        THEKERNEL->slow_ticker->attach(1000, this, &ADC::on_tick);
        attached = true;
    }
}

// enable or disable an ADC pin
void ADC::setup(PinName pin, int state) {
    uint32_t function = pinmap_function(pin, PinMap_ADC);
    uint8_t stm_chan = 0xFF;
    uint8_t chan = 0xFF;

    // The pin is not ADC compatible
    if (function == (uint32_t)NC) return;

    if (attached) {
        // The ADC is running, we have to be careful
        paused = true;
        // Wait until the current cycle is complete
        while (active) wait_us(1);

        // Disable DMA channel && the ADC peripheral
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
        LL_ADC_Disable(STM_ADC);
    }

    stm_chan = STM_PIN_CHANNEL(function);

    if (state) {
        // We have not used all ADC channels
        if (scan_count_next < ADC_CHANNEL_COUNT) {
            // set analog mode for gpio (b11)
            GPIO_TypeDef *gpio = (GPIO_TypeDef *) Set_GPIO_Clock(STM_PORT(pin));
            gpio->MODER |= (0x3 << (2*STM_PIN(pin)));

            // Assign next channel ID
            chan = scan_count_next++;
            scan_chan_lut[stm_chan] = chan;
        }
    } else {
        // Clear the ADC mode
        chan = scan_chan_lut[stm_chan];
        if (chan != 0xff) {
            scan_chan_lut[stm_chan] = 0xff; // clear this channel ID mapping
            for (int idx = 0; idx < ADC_CHANNEL_COUNT; idx++) {
                if (scan_chan_lut[idx] != 0xff && scan_chan_lut[idx] > chan) {
                    // Recalculate the new ID
                    scan_chan_lut[idx]--;
                }
            }
            // Make the last ID available
            scan_count_next--;
        }
    }

    if (attached) {
        // Reconfigure the ADC peripheral
        MX_ADC_Init_Channels(scan_count_next, scan_chan_lut);

        // Re-start the ADC scan
        paused = false;
    }
}

// set interrupt enable/disable for pin to state
void ADC::interrupt_state(PinName pin, int state) {
    int chan = _pin_to_channel(pin);

    if (chan < ADC_CHANNEL_COUNT) {
        if (state)
            interrupt_mask |= (1 << chan);
        else
            interrupt_mask &= ~(1 << chan);
     }    
}

void ADC::adcisr(void) {
    if (LL_DMA_IsActiveFlag_TC0(DMA2)) {
        //clear the ADC->CR2 DDS and DMA flag to disable further DMA requests
        STM_ADC->CR2 &= ~(ADC_CR2_DDS|ADC_CR2_DMA);
        //do processing
        LL_DMA_ClearFlag_TC0(DMA2);
        LL_DMA_ClearFlag_HT0(DMA2);
        //abjust the DMA NDTR counter
        LL_DMA_SetDataLength(DMA2,LL_DMA_STREAM_0, scan_count_next);
        LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
        //set the DMA bit
        STM_ADC->CR2 |= (ADC_CR2_DMA|ADC_CR2_DDS);
        //clear the overload
        STM_ADC->SR &= ~(ADC_SR_OVR|ADC_SR_STRT);

        if (_adc_g_isr) {
            for (uint8_t scan_index = 0; scan_index < scan_count_next; scan_index++) {
                if (interrupt_mask & (1 << scan_index)) {
                    _adc_g_isr(scan_index, adc_data[scan_index]);
                }
            }
        }
        active = false;
    }
}

//Callback for attaching to slowticker as scan start timer 
uint32_t ADC::on_tick(uint32_t dummy) {
    // If ADC conversion or ISR is still running, skip this
    if (!active) {
        if (!paused) {
            if (!LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_0)) {
                LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, scan_count_next);
                LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
            }
            LL_ADC_REG_StartConversionSWStart(STM_ADC);
            active = true;
        }
        skipped_ticks = 0;
    } else {
        skipped_ticks++;
        if (skipped_ticks > 10) {
            // Something is off, ADC can't be running that long
            __debugbreak();
        }
    }
    return dummy;
}

#endif // __STM32F4__

void ADC::_adcisr(void) {
    instance->adcisr();
}

//append global interrupt handler to function isr
void ADC::append(void(*fptr)(int chan, uint32_t value)) {
    _adc_g_isr = fptr;
}
