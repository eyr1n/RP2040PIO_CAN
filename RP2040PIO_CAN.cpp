#include "RP2040PIO_CAN.h"

#include <cstring>

#include <hardware/clocks.h>
#include <hardware/irq.h>

void CAN0_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg) {
  if (notify == CAN2040_NOTIFY_RX) {
    queue_try_add(&CAN.queue, msg);
  }
}

void CAN1_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg) {
  if (notify == CAN2040_NOTIFY_RX) {
    queue_try_add(&CAN1.queue, msg);
  }
}

#ifdef PICO_RP2350
void CAN2_Handler(can2040 *cd, uint32_t notify, can2040_msg *msg) {
  if (notify == CAN2040_NOTIFY_RX) {
    queue_try_add(&CAN2.queue, msg);
  }
}
#endif

void PIO0_IRQHandler() { can2040_pio_irq_handler(&CAN.cd); }

void PIO1_IRQHandler() { can2040_pio_irq_handler(&CAN1.cd); }

#ifdef PICO_RP2350
void PIO2_IRQHandler() { can2040_pio_irq_handler(&CAN2.cd); }
#endif

RP2040PIO_CAN::RP2040PIO_CAN(uint32_t pio, pin_size_t rx, pin_size_t tx)
    : pio{pio}, rx{rx}, tx{tx} {}

void RP2040PIO_CAN::setRX(pin_size_t pin) { rx = pin; }

void RP2040PIO_CAN::setTX(pin_size_t pin) { tx = pin; }

void RP2040PIO_CAN::begin(uint32_t bitrate, size_t queueSize) {
  queue_init(&queue, sizeof(can2040_msg), queueSize);

  can2040_setup(&cd, pio);
  switch (pio) {
  case 0:
    can2040_callback_config(&cd, CAN0_Handler);
    break;
  case 1:
    can2040_callback_config(&cd, CAN1_Handler);
    break;
#ifdef PICO_RP2350
  case 2:
    can2040_callback_config(&cd, CAN2_Handler);
    break;
#endif
  }

  switch (pio) {
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

  can2040_start(&cd, clock_get_hz(clk_sys), bitrate, rx, tx);
}

void RP2040PIO_CAN::end() {
  can2040_stop(&cd);
  queue_free(&queue);
}

int RP2040PIO_CAN::write(const CanMsg &msg) {
  can2040_msg tmp;
  tmp.id = msg.id;
  tmp.dlc = msg.data_length;
  std::memcpy(tmp.data, msg.data, sizeof(msg.data));
  if (can2040_check_transmit(&cd) == 0) {
    return 0;
  }
  can2040_transmit(&cd, &tmp);
  return 1;
}

size_t RP2040PIO_CAN::available() { return queue_get_level(&queue); }

CanMsg RP2040PIO_CAN::read() {
  can2040_msg tmp;
  queue_remove_blocking(&queue, &tmp);
  CanMsg msg;
  msg.id = tmp.id;
  msg.data_length = tmp.dlc;
  std::memcpy(msg.data, tmp.data, sizeof(tmp.data));
  return msg;
}

RP2040PIO_CAN CAN(0, 4, 5);
RP2040PIO_CAN CAN1(1, 8, 9);
#ifdef PICO_RP2350
RP2040PIO_CAN CAN2(2, 12, 13);
#endif
