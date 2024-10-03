#include <Adafruit_NeoPixel.h>
#include "MPU9250.h"
#define MPU9250_ADDRESS 0x68
#define ACCEL_CONFIG 0x1C

#define PIN_LED 4
#define NUMLED  32 // 12 + 12 + 4 + 4 LEDs

#define SW_VERSION "0.0.1_23.09.2024"

float AccX = 0;
float AccY = 0;
float AccZ = 0;
float AccMax = 0;
int color = 0;
float lastAccMax = 0;
bool rising = 0;

//Adafruit_NeoPixel pixels(NUMLED, PIN_LED, NEO_GRBW + NEO_KHZ400); // for SK6812 RGBW ideal
Adafruit_NeoPixel pixels(NUMLED, PIN_LED, NEO_GRBW + NEO_KHZ400); // for WS2812 RGB chip
// pixels.setPixelColor(j, pixels.Color(0, 1, 0, 0)); // R, G, B, W

MPU9250 mpu;


void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.beginTransmission(MPU9250_ADDRESS);
    Wire.write(ACCEL_CONFIG);
    Wire.write(0x18); // 0x18 pro ±16g, 0x00 pro 2g
    Wire.endTransmission();
  delay(1000);
  pixels.begin();
  pixels.clear();

  if (!mpu.setup(MPU9250_ADDRESS)) {
    while (1) {
        Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
        delay(5000);
    }
  }
  

}

void loop() {

  if (mpu.update()) {
      static uint32_t prev_ms = millis();
      if (millis() > prev_ms + 0) {
          print_roll_pitch_yaw();
          prev_ms = millis();
      }
  }

  if (AccMax > lastAccMax){
    rising = true;
    }
  else{
    rising = false;
    }
  
  if ((AccMax >= 1.7) && (rising == false)){
    color = map(lastAccMax, 1.7, 5, 10, 255);
    
    Serial.print("Turning LEDs ON, lastAccZ:");
    Serial.println(lastAccMax, 2);
    for (int i=0; i<NUMLED; i++){
      pixels.setPixelColor(i, pixels.Color(color, (255 - color), 0, 0));
      }
    pixels.show();
    Serial.println("Turning LEDs OFF");
    for (int i=NUMLED-1; i>=0; i--){
      pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
      pixels.show();
      delay(10);
      }
    Serial.println("LEDs OFF");
  }

  lastAccMax = AccMax; 
  delay(5);
}


void print_roll_pitch_yaw() {
    AccX = mpu.getAccX();
    AccY = mpu.getAccY();
    AccZ = mpu.getAccZ();
    AccMax = max(AccY, AccY, AccZ);
    Serial.print(AccX, 2);
    Serial.print(", ");
    Serial.print(AccY, 2);
    Serial.print(", ");
    Serial.println(AccZ, 2);
}
