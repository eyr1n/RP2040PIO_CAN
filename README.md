# RP2040PIO_CAN

CAN communication library for RP2040/2350 PIO.

This library was made possible thanks to Kevin O'Connor's PIO-based CAN implementation (https://github.com/KevinOConnor/can2040).

## Supported MCUs

- RP2040
- RP2350

## Getting Started

### Setup

```cpp
#include <RP2040PIO_CAN.h>

void setup() {
  Serial.begin(115200);
  CAN.setRX(4);
  CAN.setTX(5);
  CAN.begin(1000000);
}
```

### Receive CAN Message

```cpp
if (CAN.available()) {
  CanMsg msg = CAN.read();
  Serial.println(msg.id);
}
```

### Transmit CAN Message

```cpp
CanMsg msg;
msg.id = 0x01;
msg.data_length = 4;
msg.data[0] = 0x23;
msg.data[1] = 0x45;
msg.data[2] = 0x67;
msg.data[3] = 0x89;
CAN.write(msg);
```

## License

GNU General Public License v3.0
