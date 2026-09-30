// ============================================================================
//  box3_hw.h — ESP32-S3-BOX-3 板載硬體（腳位取自 Espressif esp-bsp esp-box-3）
//    螢幕 ILI9342C 320x240 SPI、觸控 GT911、喇叭 ES8311 + 功放、麥克風 ES7210（雙麥）
//    ES8311 與 ES7210 共用同一組 I2S（同一個取樣率），因此錄音與播放都用 24kHz
// ============================================================================
#pragma once
#include <Wire.h>

// ---- 腳位（固定，不要改）----
#define BOX_I2C_SDA    8
#define BOX_I2C_SCL    18
#define BOX_I2S_MCLK   2
#define BOX_I2S_BCLK   17
#define BOX_I2S_WS     45
#define BOX_I2S_DOUT   15      // → ES8311（喇叭）
#define BOX_I2S_DIN    16      // ← ES7210（麥克風）
#define BOX_PA_EN      46      // 功放開關，HIGH = 開
#define BOX_LCD_SCK    7
#define BOX_LCD_MOSI   6
#define BOX_LCD_CS     5
#define BOX_LCD_DC     4
#define BOX_LCD_RST    48      // 高電位重置
#define BOX_LCD_BL     47
#define BOX_TOUCH_INT  3
#define BOX_BTN_MUTE   1       // 頂部靜音鍵（硬體切斷麥克風，這支腳可讀狀態）
#define BOX_BTN_BOOT   0       // 側邊 BOOT 鍵

#define ES8311_ADDR    0x18
#define ES7210_ADDR    0x40

#define I2S_RATE       24000   // 共用取樣率（OpenAI TTS pcm 固定 24kHz）

static bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}
static int i2cRead(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom(addr, (uint8_t)1) != 1) return -1;
  return Wire.read();
}
static bool i2cPresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

// ---------------------------------------------------------------- ES8311 ----
// 從屬模式，MCLK = 256fs（24kHz → 6.144MHz），I2S 16-bit
bool es8311Init() {
  if (!i2cPresent(ES8311_ADDR)) { Serial.println("ES8311 位址 0x18 沒有回應"); return false; }
  const uint8_t seq[][2] = {
    {0x44, 0x08}, {0x44, 0x08},              // 加強 I2C 抗雜訊
    {0x00, 0x1F}, {0x00, 0x00},              // 重置
    {0x01, 0x30}, {0x02, 0x00}, {0x03, 0x10}, {0x16, 0x24}, {0x04, 0x10}, {0x05, 0x00},
    {0x0B, 0x00}, {0x0C, 0x00}, {0x10, 0x1F}, {0x11, 0x7F},
    {0x00, 0x80},                            // 從屬模式、開機
    {0x01, 0x3F},                            // 時脈取自 MCLK 腳，全部開啟
    {0x06, 0x03}, {0x07, 0x00}, {0x08, 0xFF}, // BCLK / LRCK 分頻（從屬模式不使用）
    {0x09, 0x0C}, {0x0A, 0x0C},              // DAC / ADC 格式：I2S 16-bit
    {0x13, 0x10}, {0x1B, 0x0A}, {0x1C, 0x6A},
    {0x0D, 0x01}, {0x0E, 0x02}, {0x12, 0x00}, {0x14, 0x1A}, {0x15, 0x40},
    {0x37, 0x08}, {0x45, 0x00},
    {0x32, 0xBF},                            // DAC 音量 0dB（實際音量用軟體調）
    {0x31, 0x00},                            // 取消靜音
  };
  bool ok = true;
  for (auto &r : seq) {
    bool w = false;
    for (int k = 0; k < 3 && !w; k++) { w = i2cWrite(ES8311_ADDR, r[0], r[1]); if (!w) delay(5); }
    if (!w) { Serial.printf("ES8311 寫入失敗 reg 0x%02X\n", r[0]); ok = false; }
    if (r[0] == 0x00 && r[1] == 0x1F) delay(20);
  }
  Serial.printf("ES8311 ID=%02X%02X\n", i2cRead(ES8311_ADDR, 0xFD), i2cRead(ES8311_ADDR, 0xFE));
  return ok;
}

// ---------------------------------------------------------------- ES7210 ----
// 從屬模式，MCLK = 256fs，I2S 16-bit 立體聲：左 = MIC1、右 = MIC2
bool es7210Init(uint8_t pgaGain) {
  if (!i2cPresent(ES7210_ADDR)) return false;
  uint8_t g = 0x10 | (pgaGain > 14 ? 14 : pgaGain);
  const uint8_t seq[][2] = {
    {0x00, 0xFF}, {0x00, 0x41},              // 重置
    {0x01, 0x3F},                            // 先關時脈
    {0x09, 0x30}, {0x0A, 0x30},              // 開機時序
    {0x23, 0x2A}, {0x22, 0x0A}, {0x20, 0x0A}, {0x21, 0x2A},  // 高通濾波
    {0x08, 0x10},                            // 從屬模式（bit4 必須設，否則每 8 個樣本只輸出 1 個，實測）
    {0x40, 0x43},                            // VMID
    {0x41, 0x70}, {0x42, 0x70},              // 麥克風偏壓 2.87V
    {0x07, 0x20},                            // OSR
    {0x02, 0xC1},                            // 主時脈分頻：256fs（DLL + 倍頻，實測）
    {0x04, 0x01}, {0x05, 0x00},              // LRCK = MCLK / 256
    {0x11, 0x60},                            // I2S 16-bit
    {0x12, 0x00},                            // 不使用 TDM
    {0x43, g}, {0x44, g}, {0x45, 0x00}, {0x46, 0x00},        // MIC1/2 增益，MIC3/4 關閉
    {0x47, 0x08}, {0x48, 0x08}, {0x49, 0x08}, {0x4A, 0x08},
    {0x4B, 0x00},                            // MIC1/2 上電
    {0x4C, 0xFF},                            // MIC3/4 斷電
    {0x01, 0x34},                            // 開啟 ADC1/2 時脈
    {0x06, 0x00},                            // 上電
    {0x00, 0x71}, {0x00, 0x41},              // 啟動
  };
  bool ok = true;
  for (auto &r : seq) ok &= i2cWrite(ES7210_ADDR, r[0], r[1]);
  return ok;
}

// ----------------------------------------------------------------- GT911 ----
uint8_t gt911Addr = 0;

static bool gtRead(uint16_t reg, uint8_t *buf, int n) {
  Wire.beginTransmission(gt911Addr);
  Wire.write(reg >> 8); Wire.write(reg & 0xFF);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(gt911Addr, (uint8_t)n) != n) return false;
  for (int i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}
static void gtWrite(uint16_t reg, uint8_t v) {
  Wire.beginTransmission(gt911Addr);
  Wire.write(reg >> 8); Wire.write(reg & 0xFF); Wire.write(v);
  Wire.endTransmission();
}

bool gt911Init() {
  pinMode(BOX_TOUCH_INT, INPUT);
  for (uint8_t a : {0x14, 0x5D}) {
    gt911Addr = a;
    uint8_t id[4];
    if (i2cPresent(a) && gtRead(0x8140, id, 4)) {
      Serial.printf("GT911 @0x%02X id=%c%c%c%c\n", a, id[0], id[1], id[2], id[3]);
      return true;
    }
  }
  gt911Addr = 0;
  return false;
}

// 讀觸控：回傳 true 表示有手指在螢幕上；homeKey = 螢幕下方紅圈
struct TouchState { bool touched; int x, y; bool homeKey; };
TouchState gt911Read() {
  TouchState t = {false, 0, 0, false};
  if (!gt911Addr) return t;
  uint8_t st;
  if (!gtRead(0x814E, &st, 1)) return t;
  if (!(st & 0x80)) return t;               // 資料尚未準備好：沿用上一次狀態由呼叫端處理
  int n = st & 0x0F;
  t.homeKey = st & 0x10;
  if (n > 0 && n <= 5) {
    uint8_t p[8];
    if (gtRead(0x8150, p, 6)) {
      t.x = p[0] | (p[1] << 8);
      t.y = p[2] | (p[3] << 8);
      t.touched = true;
      if (t.y >= 240) { t.homeKey = true; t.touched = false; }   // 紅圈位於顯示區下方
    }
  }
  gtWrite(0x814E, 0);
  return t;
}
