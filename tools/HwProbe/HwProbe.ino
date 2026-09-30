// 硬體偵測：掃描 I2C 晶片、PSRAM/Flash、按鍵電位，確認是 BOX-3 還是 BOX-Lite
#include <Wire.h>
#include <esp_flash.h>

void scan(int sda, int scl) {
  Wire.end();
  Wire.begin(sda, scl, 100000);
  Serial.printf("I2C SDA=%d SCL=%d:", sda, scl);
  for (int a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) Serial.printf(" 0x%02X", a);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  pinMode(47, OUTPUT); digitalWrite(47, HIGH);   // 背光
  pinMode(46, OUTPUT); digitalWrite(46, LOW);
}

void loop() {
  uint32_t fs = 0; esp_flash_get_size(NULL, &fs);
  Serial.printf("\n=== HwProbe === PSRAM %u bytes, Flash %u bytes\n", ESP.getPsramSize(), fs);
  scan(8, 18);
  scan(18, 8);
  for (int p : {0, 1, 3}) { pinMode(p, INPUT_PULLUP); Serial.printf("GPIO%d=%d ", p, digitalRead(p)); }
  Serial.println();
  delay(3000);
}
