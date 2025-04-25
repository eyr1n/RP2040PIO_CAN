#pragma once

#include <cstdint>

struct CanMsg {
  uint32_t id;
  uint8_t data_length;
  uint8_t data[8];
};