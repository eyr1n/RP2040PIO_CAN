#include <algorithm>
#include <cstring>

#include <hardware/clocks.h>
#include <hardware/irq.h>

#include "RP2040PIO_CAN.h"

void CAN0_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg) {
  if (notify == CAN2040_NOTIFY_RX) {
    queue_try_add(&CAN.rx_queue_, msg);
  }
}

void CAN1_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg) {
  if (notify == CAN2040_NOTIFY_RX) {
    queue_try_add(&CAN1.rx_queue_, msg);
  }
}

#ifdef PICO_RP2350
void CAN2_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg) {
  if (notify == CAN2040_NOTIFY_RX) {
    queue_try_add(&CAN2.rx_queue_, msg);
  }
}
#endif

void PIO0_IRQHandler() { can2040_pio_irq_handler(&CAN.cd_); }

void PIO1_IRQHandler() { can2040_pio_irq_handler(&CAN1.cd_); }

#ifdef PICO_RP2350
void PIO2_IRQHandler() { can2040_pio_irq_handler(&CAN2.cd_); }
#endif

RP2040PIO_CAN::RP2040PIO_CAN(uint32_t pio, pin_size_t rx_pin, pin_size_t tx_pin)
    : pio_{pio}, rx_pin_{rx_pin}, tx_pin_{tx_pin} {}

void RP2040PIO_CAN::setRX(pin_size_t rx_pin) { rx_pin_ = rx_pin; }

void RP2040PIO_CAN::setTX(pin_size_t tx_pin) { tx_pin_ = tx_pin; }

bool RP2040PIO_CAN::begin(CanBitRate const can_bitrate) {
  return begin(can_bitrate, 32);
}

bool RP2040PIO_CAN::begin(CanBitRate const can_bitrate, size_t rx_queue_len) {
  queue_init(&rx_queue_, sizeof(can2040_msg), rx_queue_len);

  uint32_t bitrate;
  switch (can_bitrate) {
  case CanBitRate::BR_125k:
    bitrate = 125000;
    break;
  case CanBitRate::BR_250k:
    bitrate = 250000;
    break;
  case CanBitRate::BR_500k:
    bitrate = 500000;
    break;
  case CanBitRate::BR_1000k:
    bitrate = 1000000;
    break;
  }

  can2040_setup(&cd_, pio_);
  switch (pio_) {
  case 0:
    can2040_callback_config(&cd_, CAN0_Handler);
    break;
  case 1:
    can2040_callback_config(&cd_, CAN1_Handler);
    break;
#ifdef PICO_RP2350
  case 2:
    can2040_callback_config(&cd_, CAN2_Handler);
    break;
#endif
  }

  switch (pio_) {
  case 0:
    irq_set_exclusive_handler(PIO0_IRQ_0, PIO0_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, 1);
    irq_set_enabled(PIO0_IRQ_0, true);
    break;
  case 1:
    irq_set_exclusive_handler(PIO1_IRQ_0, PIO1_IRQHandler);
    irq_set_priority(PIO1_IRQ_0, 1);
    irq_set_enabled(PIO1_IRQ_0, true);
    break;
#ifdef PICO_RP2350
  case 2:
    irq_set_exclusive_handler(PIO2_IRQ_0, PIO2_IRQHandler);
    irq_set_priority(PIO2_IRQ_0, 1);
    irq_set_enabled(PIO2_IRQ_0, true);
    break;
#endif
  }

  can2040_start(&cd_, clock_get_hz(clk_sys), bitrate, rx_pin_, tx_pin_);
  return true;
}

void RP2040PIO_CAN::end() {
  can2040_stop(&cd_);
  queue_free(&rx_queue_);
}

int RP2040PIO_CAN::write(CanMsg const &msg) {
  can2040_msg cd_msg;
  cd_msg.id = msg.isStandardId() ? msg.getStandardId()
                                 : (msg.getExtendedId() | CAN2040_ID_EFF);
  cd_msg.dlc =
      std::min<uint32_t>(msg.data_length, CanMsg::CanMsg::MAX_DATA_LENGTH);
  std::memcpy(cd_msg.data, msg.data, cd_msg.dlc);
  if (can2040_check_transmit(&cd_) == 0) {
    return 0;
  }
  int res = can2040_transmit(&cd_, &cd_msg);
  if (res != 0) {
    return res;
  }
  return 1;
}

size_t RP2040PIO_CAN::available() { return queue_get_level(&rx_queue_); }

CanMsg RP2040PIO_CAN::read() {
  can2040_msg cd_msg;
  queue_remove_blocking(&rx_queue_, &cd_msg);
  return {(cd_msg.id & CAN2040_ID_EFF) == 0 ? CanStandardId(cd_msg.id)
                                            : CanExtendedId(cd_msg.id),
          static_cast<uint8_t>(cd_msg.dlc), cd_msg.data};
}

RP2040PIO_CAN CAN(0, 4, 5);
RP2040PIO_CAN CAN1(1, 8, 9);
#ifdef PICO_RP2350
RP2040PIO_CAN CAN2(2, 12, 13);
#endif
