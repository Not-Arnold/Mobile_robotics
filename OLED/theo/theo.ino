#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 11
#define SCL_PIN 21

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---- OLED PRINT FUNCTION ----
void printOLED(const char* label, float value, uint8_t textSize) {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s %.1f", label, value);

  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(buffer);
  display.display();
}


void setup() {
  Serial.begin(115200);

  // Start I2C on custom pins
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED NOT detected");
    while (true);
  }
 

  
}

int count = 0;

void loop() {
  printOLED("Distance:", count, 2);
  count++;
}
