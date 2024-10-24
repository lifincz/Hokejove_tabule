#include <Adafruit_NeoPixel.h>
#include "MPU9250.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <string>


#define MPU9250_ADDRESS 0x68
#define ACCEL_CONFIG 0x1C // Register for changing the accelerometer range

#define PIN_LED 4
#define NUMLED  32 // 12 + 12 + 4 + 4 LEDs

#define SW_VERSION "0.0.2_09.10.2024"

// ESP32 Wifi/BT comm with phone app
// BLE UUID for communication
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

float AccX = 0;
float AccY = 0;
float AccZ = 0;
float AccMax = 0;
int color = 0;
int color_R = 0;
int color_G = 0;
float lastAccMax = 0;
bool rising = 0;

bool deviceConnected      = 0;      // if BLE device is connected
float txValue             = 0;

BLECharacteristic *pCharacteristic;
std::string prijataZprava;

//Adafruit_NeoPixel pixels(NUMLED, PIN_LED, NEO_GRBW + NEO_KHZ400); // for SK6812 RGBW ideal
Adafruit_NeoPixel pixels(NUMLED, PIN_LED, NEO_GRBW + NEO_KHZ400); // for WS2812 RGB chip
// pixels.setPixelColor(j, pixels.Color(0, 1, 0, 0)); // R, G, B, W

MPU9250 mpu;

class MyServerCallbacks: public BLEServerCallbacks {        // Class for checking BLE connection
    void onConnect(BLEServer* pServer) {
      deviceConnected = 1;
      Serial.println("Device connected...");
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = 0;
      Serial.println("Device disconnected...");
      pServer->getAdvertising()->start();   // aby se to pri kazdem odpojeni clienta zase zacalo nabizet k pripojeni
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {      // Callback for BLE messages
  void onRead(BLECharacteristic *pCharacteristic) {
    char txString[12];
    dtostrf(txValue, 1, 2, txString);                       // float_val, min_width, digits_after_decimal, char_buffer
    pCharacteristic->setValue(txString);
  }
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxMsg = pCharacteristic->getValue();
    std::string rxValue = rxMsg.c_str();  // Convert String to std::string

  //  String rxMsg = rxValue.c_str(); // for future avoid String, use char array istead to avoid possible heap fragmentation
    if (rxValue.length() > 0) {
      Serial.print("BLE Rx: ");
      for (int i = 0; i < rxValue.length(); i++) {
        Serial.print(rxValue[i]);
      }
      Serial.println();

      // Do stuff based on the command received from the app
      if (rxMsg.startsWith("RESTART_ALL") == 1)   {
        Serial.println("STRING1");
        }
      else if (rxMsg.startsWith("RESTART_TIME") == 1)   {
        Serial.println("STRING2");
        }
    }
  }
};

void send_message_BLE(char txString[]){
  Serial.print("*** BLE Sent value: ");
  pCharacteristic->setValue(txString);
  Serial.println(txString);
  pCharacteristic->notify();                      // Send the value to the app via BLE
}

void initialize_BLE(){
  BLEDevice::init("HockeyTable_1");
  BLEServer *pServer = BLEDevice::createServer();                         // create BLE Server
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);            // create BLE service
  pCharacteristic = pService->createCharacteristic(                       // vytvoření BLE komunikačního kanálu pro odesílání (TX)
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                    ); 
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());                       // assign callback function
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(    // vytvoření BLE komunikačního kanálu pro prijimani (RX)
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  pService->start();                                                      // Turn ON BLE
  pServer->getAdvertising()->start();                                     // Device visibility for BLE enabled

  esp_err_t errRc = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); // Set BLE power to maximum
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9); 
  }

void show_init_animation(){
  for (int i = 0; i < NUMLED; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 255, 0, 0));
        pixels.show();
        delay(10);
  }
  for (int fadeValue = 255; fadeValue >= 0; fadeValue-=5) {
        for (int j = 0; j < NUMLED; j++) {
            // Set the color with the current fade value
            pixels.setPixelColor(j, pixels.Color(0, fadeValue, 0, 0));
        }
        pixels.show();
        delay(10);
    }
}

void fade_out_leds(byte color_R,
                   byte color_G,
                   byte color_B,
                   byte color_W){
    
    // Fade out LEDs
    int fadeSteps = 51; // Number of steps for fade (e.g., 255/5)
    for (int step = 0; step < fadeSteps; step++) {
        // Calculate the fade value, starting from max brightness
        int fadeValue = map(step, 0, fadeSteps - 1, color_R, 0);
        
        for (int j = 0; j < NUMLED; j++) {
            // Calculate the current RGB values based on the fadeValue
            int current_R = map(fadeValue, color_R, 0, 0, color_R);
            int current_G = map(fadeValue, color_G, 0, 0, color_G);
            int current_B = map(fadeValue, color_B, 0, 0, color_B);
            int current_W = map(fadeValue, color_W, 0, 0, color_W);
            
            pixels.setPixelColor(j, pixels.Color(current_R, current_G, current_B, current_W));
        }
        pixels.show();
        delay(10); // Adjust the delay for the fade speed
    }

    // Optionally turn off the LEDs completely at the end
    for (int j = 0; j < NUMLED; j++) {
        pixels.setPixelColor(j, pixels.Color(0, 0, 0, 0));
    }
    pixels.show();
  }

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
  initialize_BLE();
  show_init_animation();
  

}

void loop() {

  if (mpu.update()) {
      static uint32_t prev_ms = millis();
      if (millis() > prev_ms + 0) {
          read_roll_pitch_yaw();
          prev_ms = millis();
      }
  }

  if (AccMax > lastAccMax){
    rising = true;
    }
  else{
    rising = false;
    }
  
  if ((AccMax >= 1.7) && (rising == false)) {
    color = map(lastAccMax, 1.7, 5, 10, 255);
    color_R = color;
    color_G = 255 - color;

    Serial.print("Turning LEDs ON, lastAccZ:");
    Serial.println(lastAccMax, 2);

    // Turn on all LEDs at once
    for (int i = 0; i < NUMLED; i++) {
        pixels.setPixelColor(i, pixels.Color(color_R, color_G, 0, 0));
    }
    pixels.show();

    fade_out_leds(color_R, color_G, 0, 0);
    
  }

  lastAccMax = AccMax; 
}


void read_roll_pitch_yaw() {
    AccX = mpu.getAccX();
    AccY = mpu.getAccY();
    AccZ = mpu.getAccZ();
    AccMax = max(max(AccY, AccY), AccZ);
    /*Serial.print(AccX, 2);
    Serial.print(", ");
    Serial.print(AccY, 2);
    Serial.print(", ");
    Serial.println(AccZ, 2);*/
}
