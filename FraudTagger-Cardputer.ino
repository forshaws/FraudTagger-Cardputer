/*

FRAUDTAGGER CARDPUTER UI & API DEMO V1.0


You need a APIKEY if you want more than 10 calls per day of the TQNN FRAUDTAGGER API

MIT License

Copyright (c) 2025 Scot D Forshaw @softwareguru1 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights  
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is  
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in  
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN  
THE SOFTWARE.

*/

#include <SD.h>
#include <M5GFX.h>
#include <SPI.h>
#include "M5Cardputer.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

String inputText = "";
String responseText = "";
bool isFraud = false;
bool isResult = false;
bool isDemo = true;
bool inError = false;

String ssid = "";
String password = "";
String apiKey = "<YOUR_API_KEY>";
String apiKeyBackup = "<YOUR_API_KEY>";

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  SPI.begin(
    M5.getPin(m5::pin_name_t::sd_spi_sclk),
    M5.getPin(m5::pin_name_t::sd_spi_miso),
    M5.getPin(m5::pin_name_t::sd_spi_mosi),
    M5.getPin(m5::pin_name_t::sd_spi_ss)
  );

  while (!SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI)) {
    delay(1);
  }

  M5.Lcd.setRotation(1);
  M5.Lcd.setBrightness(200);
  M5.Lcd.drawPngFile(SD, "/fraudtagger_loading_screen.png");
  delay(3000);

 
  M5Cardputer.Display.clear();
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(0, 0);
  M5Cardputer.Display.setTextColor(GREEN);
  M5Cardputer.Display.println("");
  M5Cardputer.Display.setTextColor(WHITE);

  // Re-initialize SD with manual pin setup
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    M5Cardputer.Display.println("Card failed, or not present");
    while (1);
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    M5Cardputer.Display.println("No SD card attached");
    return;
  }

  M5Cardputer.Display.print("SD Card Type: ");
  switch (cardType) {
    case CARD_MMC:  M5Cardputer.Display.println("MMC"); break;
    case CARD_SD:   M5Cardputer.Display.println("SDSC"); break;
    case CARD_SDHC: M5Cardputer.Display.println("SDHC"); break;
    default:        M5Cardputer.Display.println("UNKNOWN"); break;
  }

  // Read config file
  File file = SD.open("/fraudtagger.cfg");
  if (!file) {
    M5Cardputer.Display.println("Failed to open fraudtagger.cfg");
    return;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.startsWith("ssid=")) {
      ssid = line.substring(5);
    } else if (line.startsWith("password=")) {
      password = line.substring(9);
    } else if (line.startsWith("apikey=")) {
      apiKey = line.substring(7);
      apiKeyBackup = line.substring(7);
      if (apiKey != "<YOUR_API_KEY>") {
        isDemo = false;
      }
    }
  }

  file.close();

  if(isDemo){
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.println("No APIKEY\n10 CALL LIMIT PER DAY\n");
    delay(2000);
  }


  M5Cardputer.Display.println("\n" + ssid + "\n" + apiKey);
  M5Cardputer.Display.print("\nConnecting WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    M5Cardputer.Display.print(".");
  }

  M5Cardputer.Display.println("\nWiFi connected!");
  delay(1000);

  M5.Lcd.setRotation(1);
  M5.Lcd.setBrightness(200);
  M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");

  drawUI();
}

void loop() {
  M5Cardputer.update();

   // Handle GO button (BtnA) to force free API mode
  if (M5Cardputer.BtnA.wasPressed()) {

      apiKey = "<YOUR_API_KEY>";
      isDemo = true;
      inError = false;
      M5Cardputer.Display.clear();
      M5.Lcd.setRotation(1);
      M5.Lcd.setBrightness(200);
      M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");
        M5Cardputer.Display.setCursor(0, 0);
      M5Cardputer.Display.setTextSize(2);
      M5Cardputer.Display.setTextColor(WHITE);
      M5Cardputer.Display.println("FREE mode.\nReboot to\nswitch back.");
      delay(2000);
      M5Cardputer.Display.clear();
      M5.Lcd.setRotation(1);
      M5.Lcd.setBrightness(200);
      M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");
        M5Cardputer.Display.setCursor(0, 0);
      drawUI();
      return;  // Optional: skip rest of loop after switch
    



  }

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    if (!isResult) {
      for (auto c : status.word) {
        inputText += c;
      }
    }

    if (status.del && inputText.length() > 0) {
      inputText.remove(inputText.length() - 1);
      M5Cardputer.Display.clear();
      M5.Lcd.setRotation(1);
      M5.Lcd.setBrightness(200);
      M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");
      M5Cardputer.Display.println("\n");
    }

    if (status.enter) {
      if (isResult) {
        isResult = false;
        M5Cardputer.Display.clear();
        M5.Lcd.setRotation(1);
        M5.Lcd.setBrightness(200);
        M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");
      } else {
        sendRequest(inputText);
      }
    }

    drawUI();
  }
}

void drawUI() {
  if (inError) {
    M5Cardputer.Display.clear();
    M5.Lcd.setRotation(1);
    M5.Lcd.setBrightness(200);
    M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");
    inError = false;
  }

  M5Cardputer.Display.setCursor(0, 0);

  if (isResult) {
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.println(inputText + "\n");

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(isFraud ? RED : GREEN);
    M5Cardputer.Display.println(responseText);
    M5Cardputer.Display.println("\nPress Return");
  } else {
    M5Cardputer.Display.setTextSize(3);
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.println("");

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.println("\nUsername:>\n" + inputText + "\n");
  }
}

void sendRequest(String username) {
  if (WiFi.status() != WL_CONNECTED) {
    responseText = "WiFi not connected";
    isResult = true;
    M5Cardputer.Display.clear();
    M5.Lcd.setRotation(1);
    M5.Lcd.setBrightness(200);
    M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");
    drawUI();
    inputText = "";
    return;
  }

  HTTPClient http;
  String encodedUsername = urlEncode(username);
  String url = isDemo
    ? "https://api.toridion.com/v1/scoreUsername/?email=" + encodedUsername
    : "https://api.toridion.com/v1/scoreUsername/?email=" + encodedUsername + "&apikey=" + apiKey;

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);

    if (doc.containsKey("error")) {
      M5Cardputer.Display.clear();
      M5Cardputer.Display.setCursor(0, 0);
      M5Cardputer.Display.println("Invalid Email");
      delay(2000);
      inError = true;
      inputText = "";
      drawUI();
      return;
    }

    if (doc.containsKey("status") && doc["status"] == "error") {
      M5Cardputer.Display.clear();
      M5Cardputer.Display.setCursor(0, 0);
      M5Cardputer.Display.println("API KEY ERR");
      delay(2000);
      inError = true;
      inputText = "";
      drawUI();
      return;
    }

    if (!error) {
      JsonObject data = doc["data"];
      float score = data["score"];
      const char* classification = data["classification"];

      responseText = "Score: " + String(score, 1) + "\nClass: " + classification;
      if (score >= 5.0 || String(classification) == "SPAM") {
        responseText += "\n";
        isFraud = true;
      } else {
        responseText += "\n";
        isFraud = false;
      }
    } else {
      responseText = "JSON parse error!";
    }
  } else {
    responseText = "HTTP error: " + String(httpCode);
  }

  http.end();
  isResult = true;

  M5Cardputer.Display.clear();
  M5.Lcd.setRotation(1);
  M5.Lcd.setBrightness(200);
  M5.Lcd.drawPngFile(SD, "/fraudtaggerwallpaper.png");

  drawUI();
  inputText = "";
}

String urlEncode(const String &str) {
  String encoded = "";
  char c;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      char code0 = ((c >> 4) & 0xF) + '0';
      char code1 = (c & 0xF) + '0';
      if (((c >> 4) & 0xF) > 9) code0 = ((c >> 4) & 0xF) - 10 + 'A';
      if ((c & 0xF) > 9) code1 = (c & 0xF) - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}
