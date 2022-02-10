#include "HAS-code.h"

#ifdef ESP8266
#include <BlynkSimpleEsp8266.h>
#elif defined(ESP32)
#include <BlynkSimpleEsp32.h>
#else
#error "Board not found"
#endif
#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"

#include <map>
credentials Credentials;


#define APP_KEY           "86d3c19b-7819-4128-bd55-6fb79c44fee4"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "7d070644-a12d-4ae6-b239-0c41e66a1af9-8b94ee54-325e-4cad-afa2-eb76616de8d8"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

//Variables
char auth_token[33];
bool connected_to_internet = 0;
const int Erasing_button = 0;


//Provide credentials for your ESP server
char* esp_ssid = "yug";
char* esp_pass = "";


//Enter the device IDs here
#define device_ID_1   "61b98cac0df86e5c8fe7bcbb"
#define device_ID_2   "61b98c8d695fce5c874c0142"

// define the GPIO connected with Relays and switches
#define RelayPin1 5  //D1
#define RelayPin2 4  //D2

// comment the following line if you use a toggle switches instead of tactile buttons
//#define TACTILE_BUTTON 1

#define BAUD_RATE   9600

#define DEBOUNCE_TIME 250

typedef struct {      // struct for the std::map below
  int relayPIN;
} deviceConfig_t;

std::map<String, deviceConfig_t> devices = {
    //{deviceId, {relayPIN,  flipSwitchPIN}}
    {device_ID_1, {  RelayPin1}},
    {device_ID_2, {  RelayPin2}}, 
};


void setupRelays() { 
  for (auto &device : devices) {           // for each device (relay, flipSwitch combination)
    int relayPIN = device.second.relayPIN; // get the relay pin
    pinMode(relayPIN, OUTPUT);             // set relay pin to OUTPUT
    digitalWrite(relayPIN, HIGH);
  }
}

bool onPowerState(String deviceId, bool &state)
{
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = devices[deviceId].relayPIN; // get the relay pin for corresponding device
  digitalWrite(relayPIN, !state);             // set the new relay state
  return true;
}



void setupSinricPro()
{
  for (auto &device : devices)
  {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}





void setup()

{

  Serial.begin(115200);
  pinMode(Erasing_button, INPUT);


  for (uint8_t t = 4; t > 0; t--) {
    Serial.println(t);
    delay(1000);
  }

  // Press and hold the button to erase all the credentials
  if (digitalRead(Erasing_button) == LOW)
  {
    Credentials.Erase_eeprom();

  }

  String auth_string = Credentials.EEPROM_Config();
  auth_string.toCharArray(auth_token, 33);

  if (Credentials.credentials_get())
  {

    Blynk.config(auth_token);
    connected_to_internet = 1;

  }
  else
  {
    Credentials.setupAP(esp_ssid, esp_pass);
    connected_to_internet = 0;
  }


  if (connected_to_internet)
  {


      Serial.begin(BAUD_RATE);

      setupRelays();
      setupSinricPro();

  }



}




void loop()
{
  Credentials.server_loops();

  if (connected_to_internet)
  {





    
    SinricPro.handle();
  }
}
