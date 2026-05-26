#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

const char* ssid = "Nazwa_Sieci";
const char* password = "Haslo_Sieci";
#define BOT_TOKEN "token_bota_telegram"
#define CHAT_ID "id_czatu_telegram"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

#define TFT_CS     27
#define TFT_DC     13
#define TFT_MOSI   26
#define TFT_SCLK   25
#define TFT_RST    21
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 18, 5, 17}; 
byte colPins[COLS] = {16, 4, 32, 15}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define GREEN_LED 14 
#define RED_LED 12   

String secretCode = "123456"; 
String inputCode = "";
int wrongAttempts = 0;
bool isLocked = true;

bool isPenalized = false;
unsigned long penaltyStartTime = 0;
const unsigned long PENALTY_DURATION = 30000;
int lastCountdownValue = -1;

#define BG_COLOR       0x10A2
#define PANEL_COLOR    0x2144
#define BORDER_COLOR   0x7BEF
#define TEXT_MUTED     0x52AA
#define ACCENT_YELLOW  0xFEE0
#define ALERT_RED      0xD000
#define SUCCESS_GREEN  0x0400

void drawStaticUI();
void drawStatus(String statusText, uint16_t panelColor, uint16_t textColor);
void drawPINFields(int length);
void checkCode();
void triggerAlarm();
void resetLockState();
void handlePenaltyCountdown();

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH); 
  
  tft.initR(INITR_BLACKTAB); 
  tft.setRotation(3);
  
  tft.fillScreen(BG_COLOR);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(25, 60);
  tft.print("Inicjalizacja systemu...");

  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  bot.sendMessage(CHAT_ID, "Zamek szyfrowy uruchomiony w trybie bezpiecznym!", "");
  
  drawStaticUI();
  resetLockState();
}

void loop() {
  if (isPenalized) {
    handlePenaltyCountdown();
    return; 
  }

  char key = keypad.getKey();

  if (key) {
    if (key == '*') {
      inputCode = "";
      drawPINFields(inputCode.length());
    } 
    else if (key == '#') {
      if (inputCode.length() == 6) {
        checkCode();
      } else {
        drawStatus("ZA KROTKI PIN", ALERT_RED, ST77XX_WHITE);
        delay(1500);
        drawStatus("[ LOCKED ]", PANEL_COLOR, ALERT_RED);
        drawPINFields(inputCode.length());
      }
    }
    else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
      if (inputCode.length() < 6) {
        inputCode += key;
        drawPINFields(inputCode.length());
      }
    }
  }
}

void drawStaticUI() {
  tft.fillScreen(BG_COLOR);
  
  tft.fillRect(0, 0, 160, 16, PANEL_COLOR);
  tft.drawFastHLine(0, 16, 160, BORDER_COLOR);
  tft.setTextColor(TEXT_MUTED);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("SECURE SYSTEM");

  tft.drawFastHLine(0, 112, 160, PANEL_COLOR);
  tft.setCursor(10, 117);
  tft.print("[*] Clear    [#] Enter");
}

void drawStatus(String statusText, uint16_t panelColor, uint16_t textColor) {
  tft.fillRoundRect(8, 24, 144, 28, 4, panelColor);
  tft.drawRoundRect(8, 24, 144, 28, 4, BORDER_COLOR);
  
  tft.setTextColor(textColor);
  tft.setTextSize(1);
  
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(statusText, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((160 - w) / 2, 34); 
  tft.print(statusText);
}

void drawPINFields(int length) {
  int startX = 17;  
  int boxW = 16;    
  int boxH = 20;    
  int spacing = 21; 

  for (int i = 0; i < 6; i++) {
    int x = startX + (i * spacing);
    
    tft.fillRect(x, 64, boxW, boxH, PANEL_COLOR);
    tft.drawRect(x, 64, boxW, boxH, BORDER_COLOR);
    
    if (i < length) {
      tft.setTextColor(ACCENT_YELLOW);
      tft.setTextSize(2);
      tft.setCursor(x + 3, 68);
      tft.print("*");
    }
  }

  tft.fillRect(17, 90, 122, 4, BG_COLOR);
  tft.drawRect(17, 90, 122, 4, PANEL_COLOR);
  if (length > 0) {
    tft.fillRect(17, 90, (122 / 6) * length, 4, ACCENT_YELLOW);
  }
}

void handlePenaltyCountdown() {
  unsigned long elapsed = millis() - penaltyStartTime;
  
  if (elapsed >= PENALTY_DURATION) {
    isPenalized = false;
    wrongAttempts = 0;
    resetLockState();
    bot.sendMessage(CHAT_ID, "🛡️ Blokada czasowa minęła. System zamka ponownie aktywny.", "");
  } else {
    int secondsLeft = (PENALTY_DURATION - elapsed) / 1000;
    
    if (secondsLeft != lastCountdownValue) {
      lastCountdownValue = secondsLeft;
      String lockMsg = "BLOKADA: " + String(secondsLeft) + "s";
      drawStatus(lockMsg, ALERT_RED, ST77XX_WHITE);
    }
  }
}

void checkCode() {
  if (inputCode == secretCode) {
    wrongAttempts = 0; 
    isLocked = false;
    
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    
    drawStatus("[ SUCCESS ]", SUCCESS_GREEN, ST77XX_WHITE); 
    bot.sendMessage(CHAT_ID, "🔓 Zamek został otwarty poprawnym kodem.", "");
    
    delay(5000); 
    resetLockState(); 
  } else {
    wrongAttempts++;
    inputCode = ""; 
    
    if (wrongAttempts >= 3) {
      triggerAlarm();
    } else {
      drawStatus("REJECTED (" + String(wrongAttempts) + "/3)", ALERT_RED, ST77XX_WHITE); 
      delay(2000);
      drawStatus("[ LOCKED ]", PANEL_COLOR, ALERT_RED);
      drawPINFields(0); 
    }
  }
}

void triggerAlarm() {
  bot.sendMessage(CHAT_ID, "🚨 ALARM! 3 nieudane próby. System zablokowany na 30 sekund!", "");
  
  for (int i = 0; i < 6; i++) {
    drawStatus("!!! SYSTEM LOCK !!!", ALERT_RED, ST77XX_WHITE);
    digitalWrite(RED_LED, HIGH);
    delay(150);
    drawStatus("!!! SYSTEM LOCK !!!", ST77XX_WHITE, ALERT_RED);
    digitalWrite(RED_LED, LOW);
    delay(150);
  }
  
  isPenalized = true;
  penaltyStartTime = millis();
  lastCountdownValue = -1;
  digitalWrite(RED_LED, HIGH);
  drawPINFields(0);
}

void resetLockState() {
  inputCode = "";
  isLocked = true;
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  
  drawStatus("[ LOCKED ]", PANEL_COLOR, ALERT_RED); 
  drawPINFields(0); 
}