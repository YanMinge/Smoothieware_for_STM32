#ifndef __BOARD_PINS_H__
#define __BOARD_PINS_H__

#ifdef SMOOTHIE_V1
#include "board_pins_smoothie_v1.h"
#elif defined(PRNTR_V1)
#include "board_pins_prntr_v1.h"
#elif defined(PRNTR_V1_407)
#include "board_pins_prntr_v1_407.h"
#else
#error "Please define board pin file."
#endif

#endif //__BOARD_PINS_H__