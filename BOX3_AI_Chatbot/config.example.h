// ============================================================================
//  config.example.h — 設定範本（複製成 config.h 再填入自己的值；沒有 config.h 時自動使用本檔）
//  ⚠ config.h 含 API Key，請勿上傳到 GitHub 或分享給他人
//  Wi-Fi 與 API Key 也可以開機後用手機開啟螢幕上的網址修改
// ============================================================================
#pragma once

// ---------------------------------------------------------------- Wi-Fi ----
#define WIFI_SSID       "你的WiFi名稱"
#define WIFI_PASSWORD   "你的WiFi密碼"

// --------------------------------------------------------------- OpenAI ----
#define OPENAI_API_KEY  "sk-請填入你的OpenAI金鑰"

#define STT_MODEL       "gpt-4o-mini-transcribe"   // 或 "whisper-1"
#define CHAT_MODEL      "gpt-4o-mini"
#define TTS_MODEL       "gpt-4o-mini-tts"          // 或 "tts-1"
#define TTS_VOICE       "nova"                     // alloy / ash / coral / echo / fable / nova / onyx / sage / shimmer
#define TTS_INSTRUCTIONS "用可愛、活潑、溫暖的台灣口音中文說話，語氣像貼心的小朋友，語速稍快。"

#define ROBOT_NAME      "小柯"
#define MAX_HISTORY_MSGS 10        // 記住最近幾則對話（user+assistant 各算 1 則）
#define CHAT_MAX_TOKENS  300

// ------------------------------------------------------------- 功能開關 ----
#define HW_TEST_MODE    0          // 1 = 開機只跑硬體測試（螢幕/觸控/喇叭/麥克風），先確認硬體用

// ------------------------------------------------------------ 聲控（免按鍵）----
#define VOICE_ACTIVATION  1        // 1 = 直接說話就會開始聆聽；0 = 只用觸控螢幕
#define VAD_MIN_RMS       300      // 觸發錄音的最低音量（環境吵、常誤觸就調高；要喊很大聲才有反應就調低）
#define VAD_RATIO         3.0      // 音量需高於背景噪音幾倍才觸發
#define VAD_SILENCE_MS    900      // 停頓多久視為說完
#define VAD_MIN_SPEECH_MS 400      // 少於這個長度的聲音視為雜音
#define SLEEPY_AFTER_SEC  90       // 多久沒互動表情變想睡

// ------------------------------------------------------------------ 音訊 ----
#define MAX_RECORD_SEC   12
#define DEFAULT_VOLUME   70        // 0~100
#define MIC_PGA_GAIN     10        // ES7210 麥克風類比增益 0~14（x3dB，10 = 30dB）
#define MIC_SOFT_GAIN    2         // 數位增益倍數（錄音太小聲調大）

// ------------------------------------------------------------------ 螢幕 ----
#define LCD_INVERT       0         // 顏色反相（黑白顛倒時改 1）
#define LCD_MADCTL       0xC8      // 畫面左右/上下顛倒時試 0x08 / 0x48 / 0x88
