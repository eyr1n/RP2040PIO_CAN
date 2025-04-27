#pragma once

#include <cstddef>
#include <cstdint>

#include <pico/util/queue.h>

#include <api/HardwareCAN.h>

using arduino::CanExtendedId;
using arduino::CanMsg;
using arduino::CanStandardId;

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

class RP2040PIO_CAN : public arduino::HardwareCAN {
public:
  RP2040PIO_CAN(uint32_t pio, pin_size_t rx_pin, pin_size_t tx_pin);

  void setRX(pin_size_t rx_pin);

  void setTX(pin_size_t tx_pin);

  bool begin(CanBitRate const can_bitrate) override;

  bool begin(CanBitRate const can_bitrate, size_t rx_queue_len);

  void end() override;

  int write(CanMsg const &msg) override;

  size_t available() override;

  CanMsg read() override;

private:
  uint32_t pio_;
  pin_size_t rx_pin_;
  pin_size_t tx_pin_;

  can2040 cd_;
  queue_t rx_queue_;

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
