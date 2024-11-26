/*
    Video: https://youtu.be/7gmIzQ6lQZ8
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
Using Board ESP32-S3-USB-OTG
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String txValue = "";
String Received_data = ""; 
//int txValue = 0;

int incomingByte = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

uint8_t greenLED = 2;
//uint8_t redLED = 5;
#define RGB_BUILTIN 48

//bool greenLight = 0;
//bool redLight = 0;

String Command_Received;
String Old_Command_Received;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Client has connected");
    rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);  // Green
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Client has disconnected");
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);  // Off / black
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++)
        Serial.print(rxValue[i]);

      Serial.println();
      Serial.println("*********");

      if (rxValue[0] == 'R') {
        rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);  // Red
        Command_Received = "Red";
      }
      else if(rxValue[0] == 'B') {
        rgbLedWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);  // Blue
        Command_Received = "Blue";
      }
      else {
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0);  // Off / black
        Command_Received = "black";
      }
      Received_data = rxValue[0];
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("Marco BLE control");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  /*
  pServer->getAdvertising()->start();
  Serial.println("Waiting on a client connection to notify...");
  */

  // Code below added to get the service advertising


  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();


  pinMode(greenLED, OUTPUT);
  //pinMode(redLED, OUTPUT);
}

void loop() {

  if (deviceConnected) {
    if (Serial.available())
      delay(2);
    {
      //incomingByte = Serial.read();
      //if(incomingByte == 65)
      if (Received_data != Old_Command_Received)
      {
        Serial.print("Value: ");
        Serial.println(Command_Received);
        pTxCharacteristic->setValue(Command_Received);
        pTxCharacteristic->notify();
        delay(1000);  // bluetooth stack will go into congestion, if too many packets are sent
        Received_data = Old_Command_Received;
      }
    }
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}