#include <RP2040PIO_CAN.h>

void setup() {
  Serial.begin(115200);
  CAN.begin(1000000);
}

void loop() {
  if (CAN.available()) {
    CanMsg msg = CAN.read();
    Serial.println(msg.id);
  }

  CanMsg msg;
  msg.id = 0x01;
  msg.data_length = 4;
  msg.data[0] = 0x23;
  msg.data[1] = 0x45;
  msg.data[2] = 0x67;
  msg.data[3] = 0x89;
  CAN.write(msg);

  delay(10);
}
