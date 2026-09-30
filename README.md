# ESP32-S3-BOX-3 AI 語音聊天機器人「小柯」（OpenAI 版）

把樂鑫 **ESP32-S3-BOX-3** 變成會說台灣繁體中文的桌上型 AI 聊天機器人：
直接對它說話 → OpenAI 語音轉文字 → ChatGPT 回答 → OpenAI 語音合成從喇叭講出來，
2.4 吋觸控螢幕上有會眨眼、東張西望、依心情變化的機器人表情，以及日期、農曆與時間。

## 功能

| 功能 | 說明 |
|---|---|
| 聲控聊天 | 直接說話，停頓一下自動送出（不用喚醒詞、不用按鍵）|
| 觸控 | 點螢幕 = 馬上開始聆聽；小柯說話時點螢幕 = 打斷 |
| 功能說明 | 按螢幕下方紅圈或側邊 BOOT 鍵開啟，點螢幕翻頁 |
| 靜音 | 按主機頂部靜音鍵關閉麥克風 |
| 語音指令 | 「大聲一點」「小聲一點」「清除記憶」|
| 對話記憶 | 記得最近 10 則對話 |
| 表情 | 開心、愛心、驚訝、難過、生氣、眨眼、害羞，太久沒互動會想睡 |
| 時間 | 上方顯示日期、星期、農曆與時間（NTP 自動校時）|
| 網頁設定 | 手機開啟螢幕左下角網址，可修改 Wi-Fi、密碼與 API Key |
| 設定熱點 | 連不上 Wi-Fi 時自動開熱點 `XiaoKe-Setup`（密碼 `xiaoke123`），連上後開 `http://192.168.4.1` |

## 硬體

- **ESP32-S3-BOX-3** 主機（ESP32-S3、16MB Flash、16MB Octal PSRAM）
- 底座：BOX-3-DOCK 或 BOX-3-SENSOR 都可以（只負責供電，本程式不使用底座上的感測器）
- USB-C 線

主機板載元件與腳位（程式已內建，不用接線）：

| 元件 | 晶片 | 腳位 |
|---|---|---|
| 2.4" 320x240 螢幕 | ILI9342C（SPI）| SCK 7、MOSI 6、CS 5、DC 4、RST 48（高電位重置）、背光 47 |
| 電容觸控（含紅圈）| GT911（I2C 0x5D）| SDA 8、SCL 18、INT 3 |
| 喇叭 | ES8311（I2C 0x18）+ 功放 | I2S DOUT 15、功放致能 46 |
| 雙麥克風 | ES7210（I2C 0x40）| I2S DIN 16 |
| I2S 共用 | — | MCLK 2、BCLK 17、WS 45 |
| 按鍵 | — | BOOT = GPIO0、靜音鍵狀態 = GPIO1 |

> ES8311 與 ES7210 共用同一組 I2S，所以錄音與播放都跑 24kHz（OpenAI TTS 的 pcm 格式固定 24kHz），
> 錄音送出前再降頻成 16kHz。

## 安裝

### 1. Arduino IDE 與 ESP32 開發板

1. 安裝 [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. 檔案 → 偏好設定 → 額外的開發板管理員網址：
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. 開發板管理員搜尋 **esp32**（by Espressif Systems），安裝 **3.x 版**（本專案以 3.3.12 測試）

### 2. 函式庫（工具 → 管理程式庫）

| 函式庫 | 用途 |
|---|---|
| Adafruit GFX Library | 繪圖 |
| Adafruit ILI9341 | 螢幕驅動（ILI9342C 相容）|
| ArduinoJson（7.x）| 解析 OpenAI 回應 |

### 3. 開發板設定（工具選單）

| 項目 | 設定 |
|---|---|
| 開發板 | **ESP32S3 Dev Module** |
| USB CDC On Boot | **Enabled** |
| USB Mode | **Hardware CDC and JTAG** |
| Flash Size | **16MB (128Mb)** |
| Partition Scheme | **16M Flash (3MB APP/9.9MB FATFS)** |
| PSRAM | **OPI PSRAM**（必選）|

命令列編譯：

```bash
arduino-cli compile --fqbn "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=cdc,USBMode=hwcdc" BOX3_AI_Chatbot
```

### 4. 設定 Wi-Fi 與 API Key

把 `BOX3_AI_Chatbot/config.example.h` 複製成 `config.h`，填入：

- `WIFI_SSID`、`WIFI_PASSWORD`（只支援 2.4GHz）
- `OPENAI_API_KEY`（到 <https://platform.openai.com/api-keys> 建立，帳戶需有儲值額度）

不建立 `config.h` 也可以：開機後連不上 Wi-Fi 會自動開設定熱點，用手機設定即可。

### 5. 第一次燒錄

原廠韌體不支援自動進入燒錄模式，第一次要手動：

1. 拔掉 USB-C 線
2. 按住主機左側上方的 **Boot** 鍵
3. 插回 USB-C 線，再按住約 2 秒後放開
4. 在 Arduino IDE 按「上傳」

之後再上傳就會自動重置，不用再按鍵。

## 調整

所有參數都在 `config.h`：

| 參數 | 說明 |
|---|---|
| `VAD_MIN_RMS` / `VAD_RATIO` | 聲控觸發門檻：常被雜音誤觸就調高，要很大聲才有反應就調低 |
| `MIC_PGA_GAIN` | 麥克風類比增益 0~14（×3dB）|
| `MIC_SOFT_GAIN` | 麥克風數位增益倍數 |
| `CHAT_MODEL` / `TTS_VOICE` / `TTS_INSTRUCTIONS` | 換模型、換聲音、換說話風格 |
| `LCD_MADCTL` / `LCD_INVERT` | 畫面方向或顏色不對時調整 |
| `HW_TEST_MODE` | 設 1 = 開機只跑硬體測試（螢幕、喇叭、麥克風、觸控）|

### 除錯網址（與機器人在同一個網路）

| 網址 | 用途 |
|---|---|
| `http://<IP>/` | 設定網頁 |
| `http://<IP>/last.wav` | 下載最近一次錄音，檢查麥克風音質 |
| `http://<IP>/raw` | 0.5 秒 I2S 原始立體聲資料（16-bit）|

## 開發紀錄：ES7210 麥克風的坑

一開始語音辨識錯得離譜，要貼著麥克風 3 公分內才聽得懂。把 I2S 原始資料抓下來分析後發現，
**87.5%（剛好 7/8）的樣本是 0**，每 16 個樣本只有 2 個是真的聲音。
逐一即時修改暫存器後找到解法（已寫進 `box3_hw.h`）：

- `0x08 = 0x10`：少了 bit4，ES7210 每 8 個樣本只輸出 1 個
- `0x02 = 0xC1`：主時脈開啟 DLL + 倍頻，否則每個樣本會重複兩次（實際只有 12kHz）

另外 ES8311 在軟體重置後需要稍等才會回應 I2C，程式裡對每個暫存器寫入都加了重試。

## 費用參考

每次對話呼叫 3 次 OpenAI API：`gpt-4o-mini-transcribe`（語音轉文字）、`gpt-4o-mini`（對話）、
`gpt-4o-mini-tts`（語音合成），一般問答每次約新台幣幾毛錢，實際以 OpenAI 官網為準。

## 檔案

```
BOX3_AI_Chatbot/
  BOX3_AI_Chatbot.ino   主程式（UI、錄音、OpenAI、網頁設定）
  box3_hw.h             BOX-3 硬體驅動（ES8311、ES7210、GT911、腳位）
  config.example.h      設定範本
  web_page.h            設定網頁
  cjk_font.c/.h         16px 繁體中文點陣字（GNU Unifont）
  lunar_table.h         農曆表
tools/
  HwProbe/              硬體偵測程式（I2C 掃描）
  make_font.py          由 GNU Unifont 產生 cjk_font.c
```

## 授權與致謝

- 中文字型來自 [GNU Unifont](https://unifoundry.com/unifont/)（GPL-2.0+ with font embedding exception / OFL-1.1）
- 腳位參考 Espressif [esp-bsp](https://github.com/espressif/esp-bsp) 的 esp-box-3

## 開發者

此系統是**吳玉柱先生**與 **Claude AI** 共同開發，有任何意見請聯絡：achir1015@gmail.com
