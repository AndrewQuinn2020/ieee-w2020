/*
 * Code
 *
 * Scans the environment periodically for Bluetooth devices.
 * Writes the Bluetooth MAC addresses, comma-separated,
 * into NUM_DATA_FIELDS different Strings. Then sends them up
 * to ThingSpeak. All of this is clear over Serial.
 *
 */

// an elephant. isn't she lovely?

//              .,;>>%%%%%>>;,.                                              
//           .>%%%%%%%%%%%%%%%%%%%%>,.                                       
//         .>%%%%%%%%%%%%%%%%%%>>,%%%%%%;,.                                  
//       .>>>>%%%%%%%%%%%%%>>,%%%%%%%%%%%%,>>%%,.                            
//     .>>%>>>>%%%%%%%%%>>,%%%%%%%%%%%%%%%%%,>>%%%%%,.                       
//   .>>%%%%%>>%%%%>>,%%>>%%%%%%%%%%%%%%%%%%%%,>>%%%%%%%,                    
//  .>>%%%%%%%%%%>>,%%%%%%>>%%%%%%%%%%%%%%%%%%,>>%%%%%%%%%%.                 
// .>>%%%%%%%%%%>>,>>>>%%%%%%%%%%'..`%%%%%%%%,;>>%%%%%%%%%>%%.               
//.>>%%%>>>%%%%%>,%%%%%%%%%%%%%%.%%%,`%%%%%%,;>>%%%%%%%%>>>%%%%.             
//>>%%>%>>>%>%%%>,%%%%%>>%%%%%%%%%%%%%`%%%%%%,>%%%%%%%>>>>%%%%%%%.           
//>>%>>>%%>>>%%%%>,%>>>%%%%%%%%%%%%%%%%`%%%%%%%%%%%%%%%%%%%%%%%%%%.          
//>>%%%%%%%%%%%%%%,>%%%%%%%%%%%%%%%%%%%'%%%,>>%%%%%%%%%%%%%%%%%%%%%.         
//>>%%%%%%%%%%%%%%%,>%%%>>>%%%%%%%%%%%%%%%,>>%%%%%%%%>>>>%%%%%%%%%%%.        
//>>%%%%%%%%;%;%;%%;,%>>>>%%%%%%%%%%%%%%%,>>>%%%%%%>>;";>>%%%%%%%%%%%%.      
//`>%%%%%%%%%;%;;;%;%,>%%%%%%%%%>>%%%%%%%%,>>>%%%%%%%%%%%%%%%%%%%%%%%%%%.    
// >>%%%%%%%%%,;;;;;%%>,%%%%%%%%>>>>%%%%%%%%,>>%%%%%%%%%%%%%%%%%%%%%%%%%%%.  
// `>>%%%%%%%%%,%;;;;%%%>,%%%%%%%%>>>>%%%%%%%%,>%%%%%%'%%%%%%%%%%%%%%%%%%%>>.
//  `>>%%%%%%%%%%>,;;%%%%%>>,%%%%%%%%>>%%%%%%';;;>%%%%%,`%%%%%%%%%%%%%%%>>%%>.
//   >>>%%%%%%%%%%>> %%%%%%%%>>,%%%%>>>%%%%%';;;;;;>>,%%%,`%     `;>%%%%%%>>%%
//   `>>%%%%%%%%%%>> %%%%%%%%%>>>>>>>>;;;;'.;;;;;>>%%'  `%%'          ;>%%%%%>
//    >>%%%%%%%%%>>; %%%%%%%%>>;;;;;;''    ;;;;;>>%%%                   ;>%%%%
//    `>>%%%%%%%>>>, %%%%%%%%%>>;;'        ;;;;>>%%%'                    ;>%%%
//     >>%%%%%%>>>':.%%%%%%%%%%>>;        .;;;>>%%%%                    ;>%%%'
//     `>>%%%%%>>> ::`%%%%%%%%%%>>;.      ;;;>>%%%%'                   ;>%%%' 
//      `>>%%%%>>> `:::`%%%%%%%%%%>;.     ;;>>%%%%%                   ;>%%'  
//       `>>%%%%>>, `::::`%%%%%%%%%%>,   .;>>%%%%%'                   ;>%'   
//        `>>%%%%>>, `:::::`%%%%%%%%%>>. ;;>%%%%%%                    ;>%,   
//         `>>%%%%>>, :::::::`>>>%%%%>>> ;;>%%%%%'                     ;>%,  
//          `>>%%%%>>,::::::,>>>>>>>>>>' ;;>%%%%%                       ;%%, 
//            >>%%%%>>,:::,%%>>>>>>>>'   ;>%%%%%.                        ;%% 
//             >>%%%%>>``%%%%%>>>>>'     `>%%%%%%.                           
//             >>%%%%>> `@@a%%%%%%'     .%%%%%%%%%.                          
//             `a@@a%@'    `%a@@'       `a@@a%a@@a'


#include "ThingSpeak.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//------------------------------------------------------------------

//----------------  #define Initializations   ----------------------

#define BLE_NAME "__________________" // Other BLE devices will see this name as the ESP32's name.

#define BAUD_RATE 115200 // (lol) How quickly to resolve the 1s and 0s sent over Serial into ASCII.
#define LOOP_RATE  20000 // (ms)  How many milliseconds to pause before running the loop again.
#define DOT_RATE     250 // (ms)  How quickly the dots will appear over Serial as we go to the next loop.
#define SCAN_TIME     10 // (s)   How long the BLE device will scan the area for, before stopping.


/*
 * For those unaware of how ThingSpeak works, each channel has up to 8 fields available for both
 * reading and for writing. Each of these fields can store a maximum of 255 characters. A MAC address
 * takes up a minimum space of 16 characters, so along with a trailing "," in order to differentiate
 * them, we can move a maximum of 19 MAC addresses per field devoted to it.
 * 
 * In this code, we decide to use the first ThingSpeak field to upload a timestamp from the ESP32's
 * point of view, and the remaining 7 are all devoted to MAC addresses. We can theoretically store up
 * to 133 MAC addresses with this setup (7 * 19), but we decided to arbitrarily cap the maximum number
 * of MAC addresses a given ESP 32 will scan in and upload at 100.
 */

#define NUM_DATA_FIELDS   7 // How many different Strings we actually want to be writing into.
#define MAX_MACS     100 // Maximum number of MAC addresses to store.



//----------------  Variable Initializations   ---------------------

WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BLEScan* pBLEScan;
String data_field[NUM_DATA_FIELDS];
String timestamp;

int c = 0;                    // Moves between 0 and NUM_DATA_FIELDS-1 to store MAC addresses.
int c_max = 0;                // total number of MAC addresses found this cycle; past MAX_MACS, not recorded

int dot_time; // Just used to track the final part of the loop().






























//----------------  Fill in your credentials   ---------------------

// your network SSID (name)
char ssid[] = "__________________";

// your network password
char pass[] = "__________________";

// Replace the 0 with your ThingSpeak channel number for this device.
unsigned long myChannelNumber = 0;

// Paste your ThingSpeak Write API Key between the quotes.
const char * myWriteAPIKey = "_________________";

// Paste your ThingSpeak Read API Key between the quotes.
const char * myReadAPIKey = "________________";

//------------------------------------------------------------------





































//------------------------------------------------------------------

//----------------  User-Defined Functions   -----------------------

// Code to get the WiFi and timestamp client both set up.

void wifi_setup() {
  WiFi.mode(WIFI_STA);
  Serial.println("WiFi station mode activated!");


  Serial.print("Attempting to connect to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(DOT_RATE);
  }

  Serial.println("WiFi connected!");


  ThingSpeak.begin(client);
  Serial.println("ThingSpeak client connected!");


  timeClient.begin();
  Serial.println("NTP time client connected!");


  return;
}

void scan_area() {
  Serial.println("Scanning the area.");

  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);

  Serial.println("Scan complete.\,");

  return;
}
/*
 * Here, we inherit the "usual" class used for storing data and methods
 * when we get a Bluetooth callback, and we change its onResult method
 * around (which is called every time a new callback instance is finished
 * up) so that it concatenates to the correct place every time.
 */

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (c_max > MAX_MACS) {
        return;
      }

      /*
       * We're not super constrained for resources here, so these next two
       * lines construct a String object, out of a C-formatted std::string,
       * so that we can use the replace function on it.
       *
       * Also, we're going to make them upper case, just because it looks nicer.
       */
      String mac = String(advertisedDevice.getAddress().toString().c_str());
      mac.replace(":","");

      // Okay, *now* let's put the MAC address in its proper place.
      data_field[c] = data_field[c] + mac + ",";
      c = (c + 1) % NUM_DATA_FIELDS;
      c_max = c_max + 1;
    }
};


void clear_data_fields() {
  Serial.println("Clearing internal data fields.");

  for (int i = 0; i < NUM_DATA_FIELDS; i++) {
    data_field[i] = "";
  }

  return;
}

void print_data_fields() {
  Serial.println("Internal data fields (these are the MAC addresses that will get uploaded next unless changed):");

  for (int i = 0; i < NUM_DATA_FIELDS; i++) {
    Serial.println(data_field[i]);
  }
}


void pull_and_set_timestamp() {
  Serial.println("Updating and setting timestamp.");

  timeClient.update();
  timestamp = timeClient.getFormattedTime();

  return;
}

void print_timestamp() {
  Serial.println("Internal timestamp (this is what timestamp will get uploaded next unless changed):");
  Serial.println(timestamp);
}


void set_thingspeak_fields() {
  Serial.println("Setting ThingSpeak fields to internal data fields.");

  Serial.println("Return codes (200 = normal, other = problem)");

  Serial.println(ThingSpeak.setField(1, timestamp));
  for (int i = 2; i <= 8; i++)
    Serial.println(ThingSpeak.setField(i, data_field[i-2]));

  return;
}

void write_to_thingspeak() {
  Serial.println("Writing ThingSpeak fields to server.");

  Serial.println("Return codes (200 = normal, other = problem):");

  Serial.println(ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey));

  return;
}

//------------------------------------------------------------------


void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial) { ; } // wait for serial port to connect.
  Serial.println("\n===\nSerial connection established\n===\n");

  wifi_setup();

  BLEDevice::init(BLE_NAME);
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);    //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);          // less or equal setInterval value

  Serial.println("\n===\nEntering loop()\n===\n");
  delay(1000);

}

void loop() {
  Serial.println("-------------------------------------------\n");
  c = 0;

  clear_data_fields();

  pull_and_set_timestamp();

  scan_area();

  Serial.println();
  print_timestamp();
  print_data_fields();
  Serial.println();

  Serial.println();
  set_thingspeak_fields();
  write_to_thingspeak();
  Serial.println();

  Serial.println();
  Serial.print("Loop done.\n\nNext skip coming");
  for (dot_time = LOOP_RATE - (SCAN_TIME * 1000); dot_time > 0; dot_time = dot_time - DOT_RATE) {
    Serial.print(".");
    delay(DOT_RATE);
    Serial.print(".");
    delay(DOT_RATE);
  }
  Serial.println();
}

// she had a baby!

// __                 
//'. \                
// '- \               
//  / /_         .---.
// / | \\,.\/--.//    )
// |  \//        )/  / 
//  \  ' ^ ^    /    )____.----..  6
//   '.____.    .___/            \._) 
//      .\/.                      )
//       '\                       /
//       _/ \/    ).        )    (
//      /#  .!    |        /\    /
//      \  C// #  /'-----''/ #  / 
//   .   'C/ |    |    |   |    |mrf  ,
//   \), .. .'OOO-'. ..'OOO'OOO-'. ..\(,
