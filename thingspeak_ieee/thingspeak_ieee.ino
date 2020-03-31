#include "ThingSpeak.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <BLEDevice.h>
//#include <BLEUtils.h>
#include <BLEScan.h>
//#include <BLEAdvertisedDevice.h>

#define BAUD_RATE 115200 // Baud rate is used for our Serial connection.
#define LOOP_RATE  20000 // How many milliseconds to pause before running the loop again.
#define DOT_RATE     500 // How quickly the dots will appear over Serial as we go to the next loop.

WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//----------------  Fill in your credentials   ---------------------
// your network SSID (name)
char ssid[] = "YOUR_SSID_HERE";

// your network password
char pass[] = "YOUR_SSID_PASSWORD_HERE";

// Replace the 0 with your ThingSpeak channel number for this device.
unsigned long myChannelNumber = 0;

// Paste your ThingSpeak Write API Key between the quotes.
const char * myWriteAPIKey = "YOUR_WRITE_API_KEY_HERE";

// Paste your ThingSpeak Read API Key between the quotes.
const char * myReadAPIKey = "YOUR_READ_API_KEY_HERE";
//------------------------------------------------------------------

//----------------  Variable Initializations   ---------------------

int scanTime = 10; //In seconds
BLEScan* pBLEScan;
String fields_to_write[8];

//------------------------------------------------------------------

//----------------  User-Defined Functions   ---------------------

void wifi_setup() {
  WiFi.mode(WIFI_STA);
  Serial.println("WiFi station mode activated!");
  Serial.print("Attempting to connect to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(250);
  }
  Serial.println("WiFi connected!");
  ThingSpeak.begin(client);
  Serial.println("ThingSpeak client connected!");
  timeClient.begin();
  Serial.println("NTP time client connected!");
  return;
}

void clear_fields() {
  Serial.println("Clearing ThingSpeak internal fields storage.");
  for (int i = 0; i < 8; i++)
    fields_to_write[i] = "NaN";
  return;
}

void pull_and_set_timestamp() {
  timeClient.update();
  fields_to_write[0] = timeClient.getEpochTime();
}

void record_address() {
  return;
}

void check_fields() {
  Serial.println("Fields Content:");
  for (int i = 0; i < 8; i++) {
    Serial.print("fields_to_write[");
    Serial.print(i);
    Serial.print("]\t::\t");
    Serial.println(fields_to_write[i]);
  }
  return;
}

void write_to_thingspeak() {
  Serial.println("Setting ThingSpeak fields 1 through 8.");
  Serial.println("Return codes (200 = normal, other = problem");
  for (int i = 1; i <= 8; i++)
    Serial.println(ThingSpeak.setField(i, fields_to_write[i-1]));
  Serial.println("Writing fields to the ThingSpeak server.");
  Serial.println("Return codes (200 = normal, other = problem):");
  Serial.println(ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey));
  return;
}

//------------------------------------------------------------------
G
void setup() {
  Serial.begin(BAUD_RATE);
  delay(1000);
  Serial.println("\n===\nSerial connection established\n===\n");
  wifi_setup();
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setActiveScan(true);    //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);          //less or equal setInterval value
  Serial.println("\n===\nEntering loop()\n===\n");
  delay(1000);
}

void loop() {
  Serial.println("Scanning...");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println("Scan Complete!");
  clear_fields(); //clear all fields to prepare them for the next scan
  pull_and_set_timestamp();
  record_address();
  check_fields();
  delay(1000);
  Serial.println("Writing to ThingSpeak!");
  write_to_thingspeak();
  delay(1000);
  Serial.println("Scanning again in...");
  for (int i = 5; i > 0; i--) {
    Serial.print(i);
    Serial.println("...");
    delay(1000);
  }
}
