/**
   talk-to-thingspeak-example.ino

   Hi IEEE team, this is Andrew Quinn talking. I'm making these
   files with a lot of comments so that nobody gets lost on how
   exactly to implement these things, or what these things do.

   This file is to show people how to talk to ThingSpeak and send
   data up to it with the ESP-32.
*/

///////////////////////////////////////////////////////////////////////////////

// #include "whatever.h" means a custom library that Arduino
// doesn't have natively.
//
// You're going to need to install the ThingSpeak library. Go to
//
//    Sketch > Include Library > Manage Libraries ... (Ctrl+Shift+I)
//
// and search for "ThingSpeak". It should be there.
//
// Documentation for the ThingSpeak Arduino library is at
//
//   https://github.com/mathworks/thingspeak-arduino

#include "ThingSpeak.h"
#include <WiFi.h>

///////////////////////////////////////////////////////////////////////////////

// Next step: Log into ThingSpeak, and create a New Channel for us to
// send data to.

//----------------  Fill in your credentials   ---------------------
char ssid[] = "____________________";                // your network SSID (name)
char pass[] = "____________________";                // your network password
unsigned long myChannelNumber = ______;              // Replace the 0 with your channel number
const char * myWriteAPIKey = "________________";     // Paste your ThingSpeak Write API Key between the quotes
//------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////

// This code creates a WiFi client that can connect to a specific IP address,
// which is how we're going to write to ThingSpeak.
WiFiClient client;

// This is a simple incrementer, what we'll be sending up.
int number = 0;

// This is the baud rate. Make sure that under Tools, the Upload Speed is set
// to this number. In addition, make sure the Serial Monitor has this number
// as well.
int BAUD_RATE = 115200;


void setup() {
  // First we open up the Serial connection, and wait until it's actually
  // open before we do anything else.
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Serial port successfully opened.");

  // Next, we want to turn on the WiFi for the board. The ESP-32 can actually
  // operate in *both* station mode, where it just passively connects to a
  // router, and in Access Point mode, where it allows other devices to connect
  // to it itself! In another file we might play around with that.
  WiFi.mode(WIFI_STA);

  // ThingSpeak defines pretty much what you expect it to. There are commands
  // for beginning the client, writing to fields, reading *from* fields, etc.
  // (If you write to multiple fields at once, you want to setField() each one
  // individually and then writeFields() all at once.)
  //
  // Here, we let the library do the heavy lifting for us with the WiFi client.
  ThingSpeak.begin(client);
}

void loop() {

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    //    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeField(myChannelNumber, 1, number, myWriteAPIKey);

  // Check the return code
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  number++;
  if (number > 99) {
    number = 0;
  }

  delay(20000); // Wait 20 seconds before sending a new value
}
