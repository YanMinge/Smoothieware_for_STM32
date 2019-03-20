/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if defined(TARGET_STM32F4) && !defined(USB_STM_HAL)

#include "USBHAL.h"
#include "USBRegs_STM32.h"
#include "pinmap.h"

USBHAL *USBHAL::instance;

static volatile int epComplete = 0;

static uint32_t bufferEnd = 0;
static const uint32_t rxFifoSize = 512;
static uint32_t rxFifoCount = 0;

static uint32_t setupBuffer[MAX_PACKET_SIZE_EP0 >> 2];

#if defined(TARGET_DISCO_F407VG) || defined(TARGET_STM32F401RE) || defined(TARGET_STM32F411RE) || defined(TARGET_STM32F412ZG) || defined(TARGET_STM32F429ZI)
#define USBHAL_IRQn  OTG_FS_IRQn
#elif defined(TARGET_TEST_F407ZG)
#define USBHAL_IRQn  OTG_HS_IRQn
#else
#define USBHAL_IRQn  OTG_FS_IRQn
#endif

uint32_t USBHAL::endpointReadcore(uint8_t endpoint, uint8_t *buffer)
{
    return 0;
}

USBHAL::USBHAL(void)
{
    NVIC_DisableIRQ(USBHAL_IRQn);
    epCallback[0] = &USBHAL::EP1_OUT_callback;
    epCallback[1] = &USBHAL::EP1_IN_callback;
    epCallback[2] = &USBHAL::EP2_OUT_callback;
    epCallback[3] = &USBHAL::EP2_IN_callback;
    epCallback[4] = &USBHAL::EP3_OUT_callback;
    epCallback[5] = &USBHAL::EP3_IN_callback;

#if defined(TARGET_DISCO_F407VG) || defined(TARGET_STM32F401RE) || defined(TARGET_STM32F411RE) || defined(TARGET_STM32F412ZG) || defined(TARGET_STM32F429ZI)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    pin_function(PA_11, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG_FS)); // DM
    pin_function(PA_12, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG_FS)); // DP
    pin_function(PA_9, STM_PIN_DATA(STM_MODE_INPUT, GPIO_NOPULL, GPIO_AF10_OTG_FS));  // VBUS
    pin_function(PA_10, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_PULLUP, GPIO_AF10_OTG_FS)); // ID
    pin_function(PA_8, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG_FS));  // SOF
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#elif defined(TARGET_TEST_F407ZG)
    __HAL_RCC_GPIOB_CLK_ENABLE();
    pin_function(PB_14, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_OTG_HS_FS)); // DM
    pin_function(PB_15, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_OTG_HS_FS)); // DP
    __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
#else
    __HAL_RCC_GPIOA_CLK_ENABLE();
    pin_function(PA_8, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG_FS));
    pin_function(PA_9, STM_PIN_DATA(STM_MODE_INPUT, GPIO_PULLDOWN, GPIO_AF10_OTG_FS));
    pin_function(PA_10, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_PULLUP, GPIO_AF10_OTG_FS));
    pin_function(PA_11, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG_FS));
    pin_function(PA_12, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF10_OTG_FS));
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#endif
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* USB 2.0 high-speed ULPI PHY or USB 1.1 full-speed serial transceiver select */
    USB_OTG_REGS->GREGS.GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;

    /* Forced peripheral mode */
    USB_OTG_REGS->GREGS.GUSBCFG &= ~(USB_OTG_GUSBCFG_FHMOD | USB_OTG_GUSBCFG_FDMOD); 
    USB_OTG_REGS->GREGS.GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;

    // Enable interrupts
    USB_OTG_REGS->GREGS.GAHBCFG |= USB_OTG_GAHBCFG_GINT;

    // Turnaround time to maximum value - too small causes packet loss
    USB_OTG_REGS->GREGS.GUSBCFG |= USB_OTG_GUSBCFG_TRDT;

    // Unmask global interrupts
    USB_OTG_REGS->GREGS.GINTMSK |= USB_OTG_GINTMSK_SOFM |    // Start of frame mask
                             USB_OTG_GINTMSK_RXFLVLM | // Receive FIFO nonempty mask 
                             USB_OTG_GINTMSK_USBRST;   // USB reset mask

    USB_OTG_REGS->DREGS.DCFG |= USB_OTG_DCFG_DSPD |          // Full speed
                          USB_OTG_DCFG_NZLSOHSK;       // Non-zero-length status OUT handshake

    USB_OTG_REGS->GREGS.GCCFG |= USB_OTG_GCCFG_NOVBUSSENS |  // Disable VBUS sensing
                           USB_OTG_GCCFG_PWRDWN;       // Power down

    instance = this;
    NVIC_SetVector(USBHAL_IRQn, (uint32_t)&_usbisr);
    NVIC_SetPriority(USBHAL_IRQn, 1);
}

USBHAL::~USBHAL(void)
{
}

void USBHAL::connect(void)
{
    NVIC_EnableIRQ(USBHAL_IRQn);
}

void USBHAL::disconnect(void)
{
    NVIC_DisableIRQ(USBHAL_IRQn);
}

void USBHAL::configureDevice(void)
{
    // Not needed
}

void USBHAL::unconfigureDevice(void)
{
    // Not needed
}

void USBHAL::setAddress(uint8_t address)
{
    USB_OTG_REGS->DREGS.DCFG &= ~ (USB_OTG_DCFG_DAD);
    USB_OTG_REGS->DREGS.DCFG |= (address << 4) & USB_OTG_DCFG_DAD;
    EP0write(0, 0);
}

bool USBHAL::realiseEndpoint(uint8_t endpoint, uint32_t maxPacket,
                             uint32_t flags)
{
    uint32_t epIndex = endpoint >> 1;

    uint32_t type;
    switch (endpoint) {
        case EP0IN:
        case EP0OUT:
            type = 0;
            break;
        case EPISO_IN:
        case EPISO_OUT:
            type = 1;
        case EPBULK_IN:
        case EPBULK_OUT:
            type = 2;
            break;
        case EPINT_IN:
        case EPINT_OUT:
            type = 3;
            break;
    }

    // Generic in or out EP controls
    uint32_t control = (maxPacket << 0) | // Packet size
                       USB_OTG_DIEPCTL_USBAEP | // Active endpoint
                       type << USB_OTG_DIEPCTL_EPTYP_Pos;   // Endpoint type

    if (endpoint & 0x1) { // In Endpoint
        // Set up the Tx FIFO
        if (endpoint == EP0IN) {
            USB_OTG_REGS->GREGS.DIEPTXF0_HNPTXFSIZ = ((maxPacket >> 2) << 16) |
                                               (bufferEnd << 0);
        } else {
            USB_OTG_REGS->GREGS.DIEPTXF[epIndex - 1] = ((maxPacket >> 2) << 16) |
                                                 (bufferEnd << 0);
        }
        bufferEnd += maxPacket >> 2;

        // Set the In EP specific control settings
        if (endpoint != EP0IN) {
            control |= USB_OTG_DIEPCTL_SD0PID_SEVNFRM; // SD0PID
        }

        control |= (epIndex << 22) | // TxFIFO index
                   USB_OTG_DIEPCTL_SNAK; // SNAK
        USB_OTG_REGS->INEP_REGS[epIndex].DIEPCTL = control;

        // Unmask the interrupt
        USB_OTG_REGS->DREGS.DAINTMSK |= (1 << epIndex);
    } else { // Out endpoint
        // Set the out EP specific control settings
        control |= USB_OTG_DIEPCTL_CNAK; // CNAK
        USB_OTG_REGS->OUTEP_REGS[epIndex].DOEPCTL = control;

        // Unmask the interrupt
        USB_OTG_REGS->DREGS.DAINTMSK |= (1 << (epIndex + 16));
    }
    return true;
}

// read setup packet
void USBHAL::EP0setup(uint8_t *buffer)
{
    memcpy(buffer, setupBuffer, MAX_PACKET_SIZE_EP0);
}

void USBHAL::EP0readStage(void)
{
}

void USBHAL::EP0read(void)
{
}

uint32_t USBHAL::EP0getReadResult(uint8_t *buffer)
{
    uint32_t *buffer32 = (uint32_t *) buffer;
    uint32_t length = rxFifoCount;
    for (uint32_t i = 0; i < length; i += 4) {
        buffer32[i >> 2] = USB_OTG_REGS->FIFO[0][0];
    }

    rxFifoCount = 0;
    return length;
}

void USBHAL::EP0write(uint8_t *buffer, uint32_t size)
{
    endpointWrite(0, buffer, size);
}

void USBHAL::EP0getWriteResult(void)
{
}

void USBHAL::EP0stall(void)
{
    // If we stall the out endpoint here then we have problems transferring
    // and setup requests after the (stalled) get device qualifier requests.
    // TODO: Find out if this is correct behavior, or whether we are doing
    // something else wrong
    stallEndpoint(EP0IN);
//    stallEndpoint(EP0OUT);
}

EP_STATUS USBHAL::endpointRead(uint8_t endpoint, uint32_t maximumSize)
{
    uint32_t epIndex = endpoint >> 1;
    uint32_t size = (1 << 19) | // 1 packet
                    (maximumSize << 0); // Packet size
//    if (endpoint == EP0OUT) {
    epComplete &= ~(1 << endpoint);
    size |= (1 << 29); // 1 setup packet
//    }
    USB_OTG_REGS->OUTEP_REGS[epIndex].DOEPTSIZ = size;
    USB_OTG_REGS->OUTEP_REGS[epIndex].DOEPCTL |= USB_OTG_DOEPCTL_EPENA | // Enable endpoint
                                                 USB_OTG_DOEPCTL_CNAK;   // Clear NAK

    return EP_PENDING;
}

EP_STATUS USBHAL::endpointReadResult(uint8_t endpoint, uint8_t *buffer, uint32_t *bytesRead)
{
    if (!(epComplete & (1 << endpoint))) {
        return EP_PENDING;
    }

    uint32_t *buffer32 = (uint32_t *) buffer;
    uint32_t length = rxFifoCount;
    for (uint32_t i = 0; i < length; i += 4) {
        buffer32[i >> 2] = USB_OTG_REGS->FIFO[endpoint >> 1][0];
    }
    rxFifoCount = 0;
    *bytesRead = length;
    return EP_COMPLETED;
}

EP_STATUS USBHAL::endpointWrite(uint8_t endpoint, uint8_t *data, uint32_t size)
{
    uint32_t epIndex = endpoint >> 1;
    USB_OTG_REGS->INEP_REGS[epIndex].DIEPTSIZ = (1 << 19) | // 1 packet
                                          (size << 0);      // Size of packet
    USB_OTG_REGS->INEP_REGS[epIndex].DIEPCTL |= USB_OTG_DIEPCTL_EPENA | // Enable endpoint
                                                USB_OTG_DIEPCTL_CNAK;   // CNAK
    USB_OTG_REGS->DREGS.DIEPEMPMSK = (1 << epIndex);
    epComplete &= ~(1 << endpoint);
    while ((USB_OTG_REGS->INEP_REGS[epIndex].DTXFSTS & 0XFFFF) < ((size + 3) >> 2));

    for (uint32_t i = 0; i < (size + 3) >> 2; i++, data += 4) {
        USB_OTG_REGS->FIFO[epIndex][0] = *(uint32_t *)data;
    }

    return EP_PENDING;
}

EP_STATUS USBHAL::endpointWriteResult(uint8_t endpoint)
{
    if (epComplete & (1 << endpoint)) {
        epComplete &= ~(1 << endpoint);
        return EP_COMPLETED;
    }

    return EP_PENDING;
}

void USBHAL::stallEndpoint(uint8_t endpoint)
{
    if (endpoint & 0x1) { // In EP
        USB_OTG_REGS->INEP_REGS[endpoint >> 1].DIEPCTL |= USB_OTG_DIEPCTL_EPDIS | // Disable
                                                          USB_OTG_DIEPCTL_STALL;  // Stall
    } else { // Out EP
        USB_OTG_REGS->DREGS.DCTL |= USB_OTG_DCTL_SGONAK; // Set global out NAK
        USB_OTG_REGS->OUTEP_REGS[endpoint >> 1].DOEPCTL |= USB_OTG_DOEPCTL_EPDIS | // Disable
                                                           USB_OTG_DOEPCTL_STALL;  // Stall
    }
}

void USBHAL::unstallEndpoint(uint8_t endpoint)
{

}

bool USBHAL::getEndpointStallState(uint8_t endpoint)
{
    return false;
}

void USBHAL::remoteWakeup(void)
{
}


void USBHAL::_usbisr(void)
{
    instance->usbisr();
}


void USBHAL::usbisr(void)
{
    if (USB_OTG_REGS->GREGS.GINTSTS & USB_OTG_GINTSTS_USBSUSP) { // USB Suspend
        suspendStateChanged(1);
    };

    if (USB_OTG_REGS->GREGS.GINTSTS & USB_OTG_GINTSTS_USBRST) { // USB Reset
        suspendStateChanged(0);

        // Set SNAK bits
        USB_OTG_REGS->OUTEP_REGS[0].DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
        USB_OTG_REGS->OUTEP_REGS[1].DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
        USB_OTG_REGS->OUTEP_REGS[2].DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
        USB_OTG_REGS->OUTEP_REGS[3].DOEPCTL |= USB_OTG_DOEPCTL_SNAK;

        USB_OTG_REGS->DREGS.DIEPMSK = USB_OTG_DIEPMSK_XFRCM;

        bufferEnd = 0;

        // Set the receive FIFO size
        USB_OTG_REGS->GREGS.GRXFSIZ = rxFifoSize >> 2;
        bufferEnd += rxFifoSize >> 2;

        busReset();
        // Create the endpoints, and wait for setup packets on out EP0
        realiseEndpoint(EP0IN, MAX_PACKET_SIZE_EP0, 0);
        realiseEndpoint(EP0OUT, MAX_PACKET_SIZE_EP0, 0);
        endpointRead(EP0OUT, MAX_PACKET_SIZE_EP0);

        USB_OTG_REGS->GREGS.GINTSTS = (1 << 12);
    }

    if (USB_OTG_REGS->GREGS.GINTSTS & USB_OTG_GINTSTS_RXFLVL) { // RX FIFO not empty
        uint32_t status = USB_OTG_REGS->GREGS.GRXSTSP;

        uint32_t endpoint = (status & 0xF) << 1;
        uint32_t length = (status >> 4) & 0x7FF;
        uint32_t type = (status >> 17) & 0xF;

        rxFifoCount = length;

        if (type == 0x6) {
            // Setup packet
            for (uint32_t i = 0; i < length; i += 4) {
                setupBuffer[i >> 2] = USB_OTG_REGS->FIFO[0][i >> 2];
            }
            rxFifoCount = 0;
        }

        if (type == 0x4) {
            // Setup complete
            EP0setupCallback();
            endpointRead(EP0OUT, MAX_PACKET_SIZE_EP0);
        }

        if (type == 0x2) {
            // Out packet
            if (endpoint == EP0OUT) {
                EP0out();
            } else {
                epComplete |= (1 << endpoint);
                if ((instance->*(epCallback[endpoint - 2]))()) {
                    //epComplete &= ~(1 << endpoint);
                }
            }
        }

        for (uint32_t i = 0; i < rxFifoCount; i += 4) {
            (void) USB_OTG_REGS->FIFO[0][0];
        }
        USB_OTG_REGS->GREGS.GINTSTS = (1 << 4);
    }

    if (USB_OTG_REGS->GREGS.GINTSTS & USB_OTG_GINTSTS_IEPINT) { // In endpoint interrupt
        // Loop through the in endpoints
        for (uint32_t i = 0; i < 4; i++) {
            if (USB_OTG_REGS->DREGS.DAINT & (1 << i)) { // Interrupt is on endpoint

                if (USB_OTG_REGS->INEP_REGS[i].DIEPINT & USB_OTG_DIEPINT_TXFE) {// Tx FIFO empty
                    // If the Tx FIFO is empty on EP0 we need to send a further
                    // packet, so call EP0in()
                    if (i == 0) {
                        EP0in();
                    }
                    // Clear the interrupt
                    USB_OTG_REGS->INEP_REGS[i].DIEPINT = USB_OTG_DIEPINT_TXFE;
                    // Stop firing Tx empty interrupts
                    // Will get turned on again if another write is called
                    USB_OTG_REGS->DREGS.DIEPEMPMSK &= ~(1 << i);
                }

                // If the transfer is complete
                if (USB_OTG_REGS->INEP_REGS[i].DIEPINT & USB_OTG_DIEPINT_XFRC) { // Tx Complete
                    if (i != 0){
                    epComplete |= (1 << (1 + (i << 1)));
                        if((instance->*(epCallback[(1 + (i << 1)) - 2]))()){

                        }
                    }
                    USB_OTG_REGS->INEP_REGS[i].DIEPINT = USB_OTG_DIEPINT_XFRC;
                }
            }
        }
        USB_OTG_REGS->GREGS.GINTSTS = USB_OTG_GINTSTS_IEPINT;
    }

    if (USB_OTG_REGS->GREGS.GINTSTS & USB_OTG_GINTSTS_SOF) { // Start of frame
        SOF((USB_OTG_REGS->GREGS.GRXSTSR >> 17) & 0xF);
        USB_OTG_REGS->GREGS.GINTSTS = USB_OTG_GINTSTS_SOF;
    }
}


#endif
