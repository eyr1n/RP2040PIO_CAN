#pragma once

#include <cstddef>
#include <cstdint>

#include <Arduino.h>

#include <pico/util/queue.h>

#include "CanMsg.h"

extern "C" {
#include "can2040.h"

void CAN0_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg);
void CAN1_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg);
#ifdef PICO_RP2350
void CAN2_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg);
#endif

void PIO0_IRQHandler();
void PIO1_IRQHandler();
#ifdef PICO_RP2350
void PIO2_IRQHandler();
#endif
}

class RP2040PIO_CAN {
public:
  RP2040PIO_CAN(uint32_t pio, pin_size_t rx, pin_size_t tx);

  void setRX(pin_size_t pin);

  void setTX(pin_size_t pin);

  void begin(uint32_t bitrate, size_t queueSize = 32);

  void end();

  int write(const CanMsg &msg);

  size_t available();

  CanMsg read();

private:
  uint32_t pio;
  pin_size_t rx;
  pin_size_t tx;

  can2040 cd;
  queue_t queue;

  friend void CAN0_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg);
  friend void CAN1_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg);
#ifdef PICO_RP2350
  friend void CAN2_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg);
#endif

  friend void PIO0_IRQHandler();
  friend void PIO1_IRQHandler();
#ifdef PICO_RP2350
  friend void PIO2_IRQHandler();
#endif
};

extern RP2040PIO_CAN CAN;
extern RP2040PIO_CAN CAN1;
#ifdef PICO_RP2350
extern RP2040PIO_CAN CAN2;
#endif
