# 🔐 Inteligentny Zamek Szyfrowy (ESP32 Smart Lock System)

Projekt symulatora zamka szyfrowego oparty na mikrokontrolerze **ESP32**. System posiada zaawansowany interfejs graficzny wyświetlany na ekranie TFT, reaguje na wprowadzany kod za pomocą diod LED oraz komunikuje się z właścicielem przez internet, wysyłając powiadomienia na komunikator **Telegram**.

Projekt zrealizowany jako odpowiedź na "Temat 5: Symulacja zamykania i otwierania zamka szyfrowego z wizualizacją stanów jego pracy".

## ✨ Główne funkcje
* **Nowoczesny interfejs graficzny (GUI):** Estetyczny panel na ekranie TFT pokazujący wpisywane znaki (ukryte pod postacią `*`), pasek postępu oraz dynamiczne komunikaty statusowe.
* **Wizualizacja stanu sprzętowego:** Czerwona dioda LED sygnalizuje zablokowanie zamka, zielona – poprawne otwarcie.
* **Integracja z Telegramem:** System natychmiast wysyła powiadomienia na telefon właściciela o poprawnym otwarciu drzwi oraz o próbach włamania.
* **Zabezpieczenie przed włamaniem (Anti-bruteforce):** Po 3 błędnych próbach wpisania kodu (PIN), system uruchamia stroboskopowy alarm świetlny, wysyła alert na Telegram i blokuje możliwość wpisywania kodu na 30 sekund (wyświetlając na ekranie odliczanie).
* **Bezpieczna, nieblokująca pętla:** Oczekiwanie na koniec kary czasowej jest realizowane bez użycia funkcji `delay()`, co zapobiega zawieszaniu się urządzenia.

## 🛠 Wymagany sprzęt
* Płytka rozwojowa **ESP32** (np. DOIT DevKit v1)
* Wyświetlacz **TFT 1.8"** (sterownik ST7735)
* Klawiatura matrycowa membranowa **4x4**
* Diody LED (1x Czerwona, 1x Zielona) + 2 rezystory 220Ω
* Płytka stykowa i przewody połączeniowe

## 🔌 Schemat podłączeń (Pinout)

Z uwagi na konflikt sprzętowy, wyświetlacz TFT korzysta z programowej szyny SPI (Software SPI), co gwarantuje pełną stabilność działania razem z klawiaturą.

| Komponent | Pin na ESP32 | Uwagi |
| :--- | :--- | :--- |
| **Wyświetlacz TFT (S-SPI)** | | |
| VCC / LED | 3V3 | Zasilanie i podświetlenie |
| GND | GND | Masa |
| CS | GPIO 27 | Chip Select |
| DC / A0 | GPIO 13 | Data/Command |
| MOSI / SDA | GPIO 26 | Data |
| SCLK / SCK | GPIO 25 | Clock |
| RST / RES | GPIO 21 | Reset |
| **Klawiatura 4x4** | | Od lewej do prawej na złączu |
| Rząd 1-4 | GPIO 19, 18, 5, 17 | |
| Kolumna 1-4 | GPIO 16, 4, 32, 15 | |
| **Diody LED** | | |
| Dioda Zielona | GPIO 14 | + Rezystor 220Ω do GND |
| Dioda Czerwona | GPIO 12 | + Rezystor 220Ω do GND |

## 🚀 Środowisko i Biblioteki
Projekt został napisany w języku C++ z wykorzystaniem środowiska **PlatformIO** (VS Code). 

Wymagane zależności (konfiguracja `platformio.ini`):
* `chris--a/Keypad@^3.1.1`
* `adafruit/Adafruit GFX Library@^1.11.9`
* `adafruit/Adafruit ST7735 and ST7789 Library@^1.10.3`
* `witnessmenow/UniversalTelegramBot@^1.3.0`
* `bblanchon/ArduinoJson@^6.21.5` *(Ważne: Wymagana wersja 6.x dla pełnej kompatybilności z botem Telegram)*

## ⚙️ Konfiguracja przed uruchomieniem
Przed wgraniem kodu na ESP32, należy uzupełnić własne dane dostępowe w pliku `main.cpp`:
1. Zmień `TWOJA_NAZWA_WIFI` i `TWOJE_HASLO_WIFI` na dane swojej sieci domowej (2.4 GHz).
2. Stwórz bota na Telegramie używając *BotFather* i wklej jego token w miejscu `TWÓJ_TOKEN_BOTA`.
3. Sprawdź swoje ID na Telegramie używając *IDBot* i wklej je w miejscu `TWÓJ_CHAT_ID`.
4. Domyślny kod PIN w systemie to `123456`.