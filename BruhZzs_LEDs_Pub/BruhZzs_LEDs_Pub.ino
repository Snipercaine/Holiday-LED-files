/*
  .______   .______    __    __   __    __          ___      __    __  .___________.  ______   .___  ___.      ___   .___________. __    ______   .__   __.
  |   _  \  |   _  \  |  |  |  | |  |  |  |        /   \    |  |  |  | |           | /  __  \  |   \/   |     /   \  |           ||  |  /  __  \  |  \ |  |
  |  |_)  | |  |_)  | |  |  |  | |  |__|  |       /  ^  \   |  |  |  | `---|  |----`|  |  |  | |  \  /  |    /  ^  \ `---|  |----`|  | |  |  |  | |   \|  |
  |   _  <  |      /  |  |  |  | |   __   |      /  /_\  \  |  |  |  |     |  |     |  |  |  | |  |\/|  |   /  /_\  \    |  |     |  | |  |  |  | |  . `  |
  |  |_)  | |  |\  \-.|  `--'  | |  |  |  |     /  _____  \ |  `--'  |     |  |     |  `--'  | |  |  |  |  /  _____  \   |  |     |  | |  `--'  | |  |\   |
  |______/  | _| `.__| \______/  |__|  |__|    /__/     \__\ \______/      |__|      \______/  |__|  |__| /__/     \__\  |__|     |__|  \______/  |__| \__|
This is the code I use for my MQTT LED Strip controlled from Home Assistant. It's a work in progress, but works great! Huge shout out to all the people I copied ideas from as a scoured around the internet. If you recoginze your code here and want credit, let me know and I'll get that added. Cheers! 
*/
#include "sample_config.h"           // rename sample_config.h and edit any values needed
#include <FS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>                            // Time library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

//#define FASTLED_INTERRUPT_RETRY_COUNT 0


ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
char wifiConfigAP[19]; 
char wifiConfigPass[9];                          // AP config password, always 8 chars + NUL
bool shouldSaveConfig = false;                   // Flag to save json config to SPIFFS
const char LED_STYLE[] = "<style>button{background-color:#03A9F4;}body{width:60%;margin:auto;}</style>";
WiFiClient wifiClient;
ESP8266WebServer webServer(80);

char wifiSSID[32] = ""; // Leave unset for wireless autoconfig.
char wifiPass[64] = ""; // Note that these values will be lost if auto-update is used,
	   
///////////////DrZzs Palettes for custom BPM effects//////////////////////////
///////////////Add any custom palettes here//////////////////////////////////

// Gradient palette "bhw2_thanks_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_thanks.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw2_thanks_gp ) {
    0,   9,  5,  1,
   48,  25,  9,  1,
   76, 137, 27,  1,
   96,  98, 42,  1,
  124, 144, 79,  1,
  153,  98, 42,  1,
  178, 137, 27,  1,
  211,  23,  9,  1,
  255,   9,  5,  1};

// Gradient palette "bhw2_redrosey_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_redrosey.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw2_redrosey_gp ) {
    0, 103,  1, 10,
   33, 109,  1, 12,
   76, 159,  5, 48,
  119, 175, 55,103,
  127, 175, 55,103,
  178, 159,  5, 48,
  221, 109,  1, 12,
  255, 103,  1, 10};

// Gradient palette "bluered_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/h5/tn/bluered.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( bluered_gp ) {
    0,   0,  0,255,
  127, 255,255,255,
  255, 255,  0,  0};

// Gradient palette "bhw2_xmas_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_xmas.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 48 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw2_xmas_gp ) {
    0,   0, 12,  0,
   40,   0, 55,  0,
   66,   1,117,  2,
   77,   1, 84,  1,
   81,   0, 55,  0,
  119,   0, 12,  0,
  153,  42,  0,  0,
  181, 121,  0,  0,
  204, 255, 12,  8,
  224, 121,  0,  0,
  244,  42,  0,  0,
  255,  42,  0,  0};

// Gradient palette "bhw2_xc_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_xc.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw2_xc_gp ) {
    0,   4,  2,  9,
   58,  16,  0, 47,
  122,  24,  0, 16,
  158, 144,  9,  1,
  183, 179, 45,  1,
  219, 220,114,  2,
  255, 234,237,  1};

// Gradient palette "bhw1_04_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_04.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_04_gp ) {
    0, 229,227,  1,
   15, 227,101,  3,
  142,  40,  1, 80,
  198,  17,  1, 79,
  255,   0,  0, 45};

// Gradient palette "bhw4_051_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw4/tn/bhw4_051.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

// Gradient palette "fs2006_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cl/tn/fs2006.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 56 bytes of program space.

DEFINE_GRADIENT_PALETTE( fs2006_gp ) {
    0,   0, 49,  5,
   34,   0, 49,  5,
   34,  79,168, 66,
   62,  79,168, 66,
   62, 252,168, 92,
  103, 252,168, 92,
  103, 234, 81, 29,
  143, 234, 81, 29,
  143, 222, 30,  1,
  184, 222, 30,  1,
  184,  90, 13,  1,
  238,  90, 13,  1,
  238, 210,  1,  1,
  255, 210,  1,  1};


DEFINE_GRADIENT_PALETTE( bhw4_051_gp ) {
    0,   1,  1,  4,
   28,  16, 24, 77,
   66,  35, 87,160,
  101, 125,187,205,
  127, 255,233, 13,
  145, 125,187,205,
  193,  28, 70,144,
  224,  14, 19, 62,
  255,   1,  1,  4};

// Gradient palette "blue_g2_5_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/go2/webtwo/tn/blue-g2-5.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( blue_g2_5_gp ) {
    0,   2,  6, 63,
  127,   2,  9, 67,
  255,   255, 255, 115,
  255,   255, 255, 0};

// Gradient palette "bhw3_41_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw3/tn/bhw3_41.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw3_41_gp ) {
    0,   0,  0, 45,
   71,   7, 12,255,
   76,  75, 91,255,
   76, 255,255,255,
   81, 255,255,255,
  178, 255,255,255,
  179, 255, 55, 45,
  196, 255,  0,  0,
  255,  42,  0,  0};

DEFINE_GRADIENT_PALETTE( test_gp ) {
    0,  255,  0,  0, // Red
// 32,  171, 85,  0, // Orange
// 64,  171,171,  0, // Yellow
// 96,    0,255,  0, // Green
//128,    0,171, 85, // Aqua
  160,    0,  0,255, // Blue
//192,   85,  0,171, // Purple
//224,  171,  0, 85, // Pink
//255,  255,  0,  0};// and back to Red
};  

// Gradient palette "bhw2_greenman_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_greenman.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw2_greenman_gp ) {
    0,   1, 22,  1,
  130,   1,168,  2,
  255,   1, 22,  1};

// Gradient palette "Pills_3_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/icons/tn/Pills-3.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( Pills_3_gp ) {
    0,   4, 12,122,
  127,  55, 58, 50,
  255, 192,147, 11};

// Gradient palette "Orange_to_Purple_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/icons/tn/Orange-to-Purple.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( Orange_to_Purple_gp ) {
    0, 208, 50,  1,
  127, 146, 27, 45,
  255,  97, 12,178};

/****************************** MQTT TOPICS (change these topics as you wish)  ***************************************/

//#define colorstatuspub "bruh/mqttstrip/colorstatus"
//#define setcolorsub "bruh/mqttstrip/setcolor"
//#define setpowersub "bruh/mqttstrip/setpower"
//#define seteffectsub "bruh/mqttstrip/seteffect"
//#define setbrightness "bruh/mqttstrip/setbrightness"

//#define setcolorpub "bruh/mqttstrip/setcolorpub"
//#define setpowerpub "bruh/mqttstrip/setpowerpub"
//#define seteffectpub "bruh/mqttstrip/seteffectpub"
//#define setbrightnesspub "bruh/mqttstrip/setbrightnesspub"
//#define setanimationspeed "bruh/mqttstrip/setanimationspeed"

/*************************** EFFECT CONTROL VARIABLES AND INITIALIZATIONS ************************************/

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

String setColor ="0,0,150";
String setPower;
String setEffect = "Solid";
String setBrightness = "150";
int brightness = 150;
String setAnimationSpeed;
int animationspeed = 240;
String setColorTemp;
int Rcolor = 0;
int Gcolor = 0;
int Bcolor = 0;
CRGB leds[NUM_LEDS];
char mcuHostName[64]; 
char lwtTopic[96];  
char colorstatusPubTopic[96];    
char setcolorSubTopic[96];  
char setpowerSubTopic[96];   
char seteffectSubTopic[96];   
char setbrightnessTopic[96];   
char setcolorPubTopic[96];   
char setpowerPubTopic[96];   
char seteffectPubTopic[96];   
char setbrightnessPubTopic[96];   
char setanimationspeedTopic[96];   


/****************FOR CANDY CANE-like desings***************/
CRGBPalette16 currentPalettestriped; //for Candy Cane
CRGBPalette16 hailPalettestriped; //for Hail
CRGBPalette16 ThxPalettestriped; //for Thanksgiving
CRGBPalette16 HalloweenPalettestriped; //for Halloween
CRGBPalette16 HJPalettestriped; //for Holly Jolly
CRGBPalette16 IndPalettestriped; //for Independence
CRGBPalette16 gPal; //for fire

/****************FOR NOISE - I'm using this one for Easter***************/
static uint16_t dist;         // A random number for our noise generator.
uint16_t scale = 30;          // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
uint8_t maxChanges = 48;      // Value for blending between palettes.
CRGBPalette16 targetPalette(OceanColors_p);
CRGBPalette16 currentPalette(CRGB::Black);

/*****************For TWINKLE********/
#define DENSITY     80
int twinklecounter = 0;

/*********FOR RIPPLE***********/
uint8_t colour;                                               // Ripple colour is randomized.
int center = 0;                                               // Center of the current ripple.
int step = -1;                                                // -1 is the initializing step.
uint8_t myfade = 255;                                         // Starting brightness.
#define maxsteps 16                                           // Case statement wouldn't allow a variable.
uint8_t bgcol = 0;                                            // Background colour rotates.
int thisdelay = 20;                                           // Standard delay value.

/**************FOR RAINBOW***********/
uint8_t thishue = 0;                                          // Starting hue value.
uint8_t deltahue = 10;

/**************FOR DOTS**************/
uint8_t   count =   0;                                        // Count up to 255 and then reverts to 0
uint8_t fadeval = 224;                                        // Trail behind the LED's. Lower => faster fade.
uint8_t bpm = 30;

/**************FOR LIGHTNING**************/
uint8_t frequency = 50;                                       // controls the interval between strikes
uint8_t flashes = 8;                                          //the upper limit of flashes per strike
unsigned int dimmer = 1;
uint8_t ledstart;                                             // Starting location of a flash
uint8_t ledlen;
int lightningcounter = 0;

/********FOR FUNKBOX EFFECTS**********/
int idex = 0;                //-LED INDEX (0 to NUM_LEDS-1
int TOP_INDEX = int(NUM_LEDS / 2);
int thissat = 255;           //-FX LOOPS DELAY VAR

//////////////////add thishue__ for Police All custom effects here/////////////////////////////////////////////////////////
/////////////////use hsv Hue number for one color, for second color change "thishue__ + __" in the setEffect section//////

uint8_t thishuepolice = 0;
uint8_t thishuehail = 64;
uint8_t thishueLovey = 0;     
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {
    iN = ( i + TOP_INDEX ) % NUM_LEDS;
  }
  return iN;
}

/********FIRE**********/
#define COOLING  55
#define SPARKING 120
bool gReverseDirection = false;

/********BPM**********/
uint8_t gHue = 0;
char message_buff[100];


WiFiClient espClient; //this needs to be unique for each controller
PubSubClient client(espClient); //this needs to be unique for each controller

////////////////////////////////////////////////////////////

const char *getDeviceID() {
  char *identifier = new char[30];
  os_strcpy(identifier, espName);
  strcat_P(identifier, PSTR("-"));

  char cidBuf[7];
  sprintf(cidBuf, "%06X", ESP.getChipId());
  os_strcat(identifier, cidBuf);

  return identifier;
}

void setup() {
  Serial.begin(115200);
  // build hostname with last 6 of MACID
  os_strcpy(mcuHostName, getDeviceID());

  // ~~~~ Set MQTT Topics
  sprintf_P(lwtTopic, PSTR("%s/LWT"), mcuHostName);
  
  sprintf_P(colorstatusPubTopic, PSTR("%s/colorstatus"), mcuHostName); 

  sprintf_P(setcolorSubTopic, PSTR("%s/setcolor"), mcuHostName);    
  sprintf_P(setpowerSubTopic, PSTR("%s/setpower"), mcuHostName);    
  sprintf_P(seteffectSubTopic, PSTR("%s/seteffect"), mcuHostName);    
  sprintf_P(setbrightnessTopic, PSTR("%s/setbrightness"), mcuHostName);    

  sprintf_P(setcolorPubTopic, PSTR("%s/setcolorpub"), mcuHostName);    
  sprintf_P(setpowerPubTopic, PSTR("%s/setpowerpub"), mcuHostName);    
  sprintf_P(seteffectPubTopic, PSTR("%s/seteffectpub"), mcuHostName);    
  sprintf_P(setbrightnessPubTopic, PSTR("%s/setbrightnesspub"), mcuHostName);    

  sprintf_P(setanimationspeedTopic, PSTR("%s/setanimationspeed"), mcuHostName);    


  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(12, 10000); //experimental for power management. Feel free to try in your own setup.
  FastLED.setBrightness(brightness);

  setupStripedPalette( CRGB::Red, CRGB::Red, CRGB::White, CRGB::White); //for CANDY CANE
  setupThxPalette( CRGB::OrangeRed, CRGB::Olive, CRGB::Maroon, CRGB::Maroon); //for Thanksgiving
  setupHailPalette( CRGB::Blue, CRGB::Blue, CRGB::Yellow, CRGB::Yellow); //for HAIL
  setupHalloweenPalette( CRGB::DarkOrange, CRGB::DarkOrange, CRGB::Indigo, CRGB::Indigo); //for Halloween
  setupHJPalette( CRGB::Red, CRGB::Red, CRGB::Green, CRGB::Green); //for Holly Jolly
  setupIndPalette( CRGB::FireBrick, CRGB::Cornsilk, CRGB::MediumBlue, CRGB::MediumBlue); //for Independence

/////////////////////////////////////////////////////////////////////////////////////////////////////////

  setup_wifi();
  
  webServer.on("/", webHandleRoot);
  webServer.on("/saveConfig", webHandleSaveConfig);
  webServer.on("/resetConfig", webHandleResetConfig);
  webServer.on("/LEDroutine", webHandleLEDroutine);
 // webServer.on("/espfirmware", webHandleEspFirmware);
 // webServer.on("/reboot", webHandleReboot);
  //webServer.onNotFound(webHandleNotFound);
  webServer.begin();

  
  gPal = HeatColors_p; //for FIRE

  fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0)); //Startup LED Lights
  FastLED.show();

//  setup_wifi();

  client.setServer(mqtt_server, 1883); //CHANGE PORT HERE IF NEEDED
  client.setCallback(callback);

  MDNS.begin(mcuHostName);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);

   ArduinoOTA.setPort(OTAport);
   ArduinoOTA.setHostname(mcuHostName);
   ArduinoOTA.setPassword((const char *)OTApassword);
   ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
     Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
}


void setup_wifi() {

  delay(10);
//  Serial.println();
//  Serial.print("Connecting to ");
//  Serial.println(wifi_ssid);

//  WiFi.mode(WIFI_STA);
//  WiFi.hostname(mcuHostName);
//  WiFi.begin(wifi_ssid, wifi_password);

//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//
//  Serial.println("");
//  Serial.println("WiFi connected");
//  Serial.println("IP address: ");
// Serial.println(WiFi.localIP());
  
  
  // Assign our hostname (default esp_name + left 6 MAC) before connecting to WiFi
  WiFi.hostname(mcuHostName);

  if ( WiFi.status() != WL_CONNECTED) /// (String(wifiSSID) == "")
  { // If the sketch has no defined a static wifiSSID to connect to,
    // use WiFiManager to collect required information from the user.

    // id/name, placeholder/prompt, default value, length, extra tags
    WiFiManagerParameter custom_LEDNodeHeader("<br/><br/><b>DRZZZ's Holiday LED's</b>");
    WiFiManagerParameter custom_mqttHeader("<br/><br/><b>MQTT Broker</b>");
    WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", mqtt_server, 63, " maxlength=39");
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", mqtt_port, 5, " maxlength=5 type='number'");
    WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", mqtt_user, 31, " maxlength=31");
    WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password", mqtt_password, 31, " maxlength=31 type='password'");
    WiFiManagerParameter custom_LEDtpe("LEDTPE", "LED Type WS2811", Param1, 63, " maxlength=39");
    WiFiManagerParameter custom_mqttColororder("COLOR_ORDER", "RGB", Param2, 31, " maxlength=39");
    WiFiManagerParameter custom_mqttNumleds("NUM_LEDS", "Number of LED's", Param3, 5, " maxlength=5 type='number'");
    WiFiManagerParameter custom_espName("Sensor", "Devicename", espName, 63, " maxlength=39");


    // WiFiManager local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    // set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    wifiManager.setCustomHeadElement(LED_STYLE);

    // Add all your parameters here
    wifiManager.addParameter(&custom_LEDNodeHeader);
    wifiManager.addParameter(&custom_mqttHeader);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_LEDtpe);
    wifiManager.addParameter(&custom_mqttColororder);
    wifiManager.addParameter(&custom_mqttNumleds);
    wifiManager.addParameter(&custom_espName);


    // Timeout until configuration portal gets turned off
    wifiManager.setTimeout(180);

   
    // Construct AP name
    String strWifiConfigAP = String("DRzz's Holiday LED's");
    strWifiConfigAP.toCharArray(wifiConfigAP, (strWifiConfigAP.length() + 1));
    
    String strConfigPass = "123" ;
    strConfigPass.toCharArray(wifiConfigPass, 9);

    // Fetches SSID and pass from EEPROM and tries to connect
    // If it does not connect it starts an access point with the specified name
    // and goes into a blocking loop awaiting configuration.
    if (!wifiManager.autoConnect(wifiConfigAP, wifiConfigPass))
    { // Reset and try again
      Serial.println(F("WIFI: Failed to connect and hit timeout"));
      espReset();
      
}}}
  
//}

void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;

  if (String(topic) == setpowerSubTopic) {
    for (i = 0; i < length; i++) {
      message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';
    setPower = String(message_buff);
    Serial.println("Set Power: " + setPower);
    if (setPower == "OFF") {
      client.publish(setpowerPubTopic, "OFF");
    }

    if (setPower == "ON") {
      client.publish(setpowerPubTopic, "ON");
    }
  }


  if (String(topic) == seteffectSubTopic) {
    for (i = 0; i < length; i++) {
      message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';
    setEffect = String(message_buff);
    Serial.println("Set Effect: " + setEffect);
    setPower = "ON";
    client.publish(setpowerPubTopic, "ON");
    if (setEffect == "Twinkle") {
      twinklecounter = 0;
    }
    if (setEffect == "Lightning") {
      twinklecounter = 0;
    }
  }


  if (String(topic) == setbrightnessTopic) {
    for (i = 0; i < length; i++) {
      message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';
    setBrightness = String(message_buff);
    Serial.println("Set Brightness: " + setBrightness);
    brightness = setBrightness.toInt();
    setPower = "ON";
    client.publish(setpowerPubTopic, "ON");
  }

  if (String(topic) == setcolorSubTopic) {
    for (i = 0; i < length; i++) {
      message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';
    client.publish(setcolorPubTopic, message_buff);
    setColor = String(message_buff);
    Serial.println("Set Color: " + setColor);
    setPower = "ON";
    client.publish(setpowerPubTopic, "ON");
    }

  if (String(topic) == setanimationspeedTopic) {
    for (i = 0; i < length; i++) {
      message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';
    setAnimationSpeed = String(message_buff);
    animationspeed = setAnimationSpeed.toInt();
  }
}



void loop() {

//  if (!client.connected()) {
//    reconnect();
//  }
//  client.loop();  // commented out block when hand merging
  
  httpServer.handleClient();
  
  while ((WiFi.status() != WL_CONNECTED) || (WiFi.localIP().toString() == "0.0.0.0"))
  { // Check WiFi is connected and that we have a valid IP, retry until we do.
    if (WiFi.status() == WL_CONNECTED)
    { // If we're currently connected, disconnect so we can try again
      WiFi.disconnect();
    }
    setup_wifi();
  }
  webServer.handleClient(); // webServer loop
  client.loop();
  
  
  ArduinoOTA.handle();
  
  int Rcolor = setColor.substring(0, setColor.indexOf(',')).toInt();
  int Gcolor = setColor.substring(setColor.indexOf(',') + 1, setColor.lastIndexOf(',')).toInt();
  int Bcolor = setColor.substring(setColor.lastIndexOf(',') + 1).toInt();

  if (setPower == "OFF") {
    setEffect = "Solid";
    for ( int i = 0; i < NUM_LEDS; i++) {
      leds[i].fadeToBlackBy( 8 );   //FADE OFF LEDS
    }
  }
  
/////////////////////////////////////////  
//////DrZzs custom effects//////////////
///////////////////////////////////////

  if (setEffect == "Christmas") {                                  // colored stripes pulsing in Shades of GREEN and RED 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw2_xmas_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}
  
  if (setEffect == "St Patty") {                                  // colored stripes pulsing in Shades of GREEN 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw2_greenman_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Valentine") {                                  // colored stripes pulsing in Shades of PINK and RED 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw2_redrosey_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Turkey Day") {                                  // colored stripes pulsing in Shades of Brown and ORANGE 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw2_thanks_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Thanksgiving") {                                  // colored stripes pulsing in Shades of Red and ORANGE and Green 
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 16, /* higher = narrower stripes */
                  ThxPalettestriped, 255, LINEARBLEND);
}
  
  if (setEffect == "USA") {                                  // colored stripes pulsing in Shades of Red White & Blue 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw3_41_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Independence") {                        // colored stripes of Red White & Blue
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 16, /* higher = narrower stripes */
                  IndPalettestriped, 255, LINEARBLEND);
}


  if (setEffect == "Halloween") {                                  // colored stripes pulsing in Shades of Purple and Orange
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = Orange_to_Purple_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Go Blue") {                                  // colored stripes pulsing in Shades of Maize and Blue
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = Pills_3_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Hail") {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 16, /* higher = narrower stripes */
                  hailPalettestriped, 255, LINEARBLEND);
}
  
  if (setEffect == "Touchdown") {                 //Maize and Blue with POLICE ALL animation
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexY = idex;
    int idexB = antipodal_index(idexY);
    int thathue = (thishuehail + 96) % 255;
    leds[idexY] = CHSV(thishuehail, thissat, 255);
    leds[idexB] = CHSV(thathue, thissat, 255);
  }

  if (setEffect == "Punkin") {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 16, /* higher = narrower stripes */
                  HalloweenPalettestriped, 255, LINEARBLEND);
  }

    if (setEffect == "Lovey Day") {                 //Valentine's Day colors (TWO COLOR SOLID)
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishueLovey + 244) % 255;
    leds[idexR] = CHSV(thishueLovey, thissat, 255);
    leds[idexB] = CHSV(thathue, thissat, 255);
  }

  if (setEffect == "Holly Jolly") {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 16, /* higher = narrower stripes */
                  HJPalettestriped, 255, LINEARBLEND);
  }

/////////////////End DrZzs effects/////////////
///////////////////////////////////////////////

////////Place your custom effects below////////////




/////////////end custom effects////////////////

///////////////////////////////////////////////
/////////fastLED & Bruh effects///////////////
/////////////////////////////////////////////
  
  if (setEffect == "Sinelon") {
    fadeToBlackBy( leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS);
    leds[pos] += CRGB(Rcolor, Gcolor, Bcolor);
  }

  if (setEffect == "Juggle" ) {                           // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 20);
    byte dothue = 0;
    for ( int i = 0; i < 8; i++) {
      leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CRGB(Rcolor, Gcolor, Bcolor);
      dothue += 32;
    }
  }

  if (setEffect == "Confetti" ) {                       // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CRGB(Rcolor + random8(64), Gcolor, Bcolor);
  }


  if (setEffect == "Rainbow") {
    // FastLED's built-in rainbow generator
    static uint8_t starthue = 0;    thishue++;
    fill_rainbow(leds, NUM_LEDS, thishue, deltahue);
  }


  if (setEffect == "Rainbow with Glitter") {               // FastLED's built-in rainbow generator with Glitter
    static uint8_t starthue = 0;
    thishue++;
    fill_rainbow(leds, NUM_LEDS, thishue, deltahue);
    addGlitter(80);
  }


  if (setEffect == "Glitter") {
    fadeToBlackBy( leds, NUM_LEDS, 20);
    addGlitterColor(80, Rcolor, Gcolor, Bcolor);
  }


  if (setEffect == "BPM") {                                  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Solid" & setPower == "ON" ) {          //Fill entire strand with solid color
    fill_solid(leds, NUM_LEDS, CRGB(Rcolor, Gcolor, Bcolor));
  }

  

  if (setEffect == "Twinkle") {
    twinklecounter = twinklecounter + 1;
    if (twinklecounter < 2) {                               //Resets strip if previous animation was running
      FastLED.clear();
      FastLED.show();
    }
    const CRGB lightcolor(8, 7, 1);
    for ( int i = 0; i < NUM_LEDS; i++) {
      if ( !leds[i]) continue; // skip black pixels
      if ( leds[i].r & 1) { // is red odd?
        leds[i] -= lightcolor; // darken if red is odd
      } else {
        leds[i] += lightcolor; // brighten if red is even
      }
    }
    if ( random8() < DENSITY) {
      int j = random16(NUM_LEDS);
      if ( !leds[j] ) leds[j] = lightcolor;
    }
  }

  if (setEffect == "Dots") {
    uint8_t inner = beatsin8(bpm, NUM_LEDS / 4, NUM_LEDS / 4 * 3);
    uint8_t outer = beatsin8(bpm, 0, NUM_LEDS - 1);
    uint8_t middle = beatsin8(bpm, NUM_LEDS / 3, NUM_LEDS / 3 * 2);
    leds[middle] = CRGB::Purple;
    leds[inner] = CRGB::Blue;
    leds[outer] = CRGB::Aqua;
    nscale8(leds, NUM_LEDS, fadeval);
  }

  if (setEffect == "Lightning") {
    twinklecounter = twinklecounter + 1;                     //Resets strip if previous animation was running
    Serial.println(twinklecounter);
    if (twinklecounter < 2) {
      FastLED.clear();
      FastLED.show();
    }
    ledstart = random8(NUM_LEDS);           // Determine starting location of flash
    ledlen = random8(NUM_LEDS - ledstart);  // Determine length of flash (not to go beyond NUM_LEDS-1)
    for (int flashCounter = 0; flashCounter < random8(3, flashes); flashCounter++) {
      if (flashCounter == 0) dimmer = 5;    // the brightness of the leader is scaled down by a factor of 5
      else dimmer = random8(1, 3);          // return strokes are brighter than the leader
      fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 255 / dimmer));
      FastLED.show();                       // Show a section of LED's
      delay(random8(4, 10));                // each flash only lasts 4-10 milliseconds
      fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 0)); // Clear the section of LED's
      FastLED.show();
      if (flashCounter == 0) delay (150);   // longer delay until next flash after the leader
      delay(50 + random8(100));             // shorter delay between strokes
    }
    delay(random8(frequency) * 100);        // delay between strikes
  }



  if (setEffect == "Police One") {                    //POLICE LIGHTS (TWO COLOR SINGLE LED)
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishuepolice + 160) % 255;
    for (int i = 0; i < NUM_LEDS; i++ ) {
      if (i == idexR) {
        leds[i] = CHSV(thishuepolice, thissat, 255);
      }
      else if (i == idexB) {
        leds[i] = CHSV(thathue, thissat, 255);
      }
      else {
        leds[i] = CHSV(0, 0, 0);
      }
    }

  }

  if (setEffect == "Police All") {                 //POLICE LIGHTS (TWO COLOR SOLID)
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishuepolice + 160) % 255;
    leds[idexR] = CHSV(thishuepolice, thissat, 255);
    leds[idexB] = CHSV(thathue, thissat, 255);
  }


  if (setEffect == "Candy Cane") {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, NUM_LEDS,
                  startIndex, 16, /* higher = narrower stripes */
                  currentPalettestriped, 255, LINEARBLEND);
  }


  if (setEffect == "Cyclon Rainbow") {                    //Single Dot Down
  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}

  if (setEffect == "Fire") { 
      Fire2012WithPalette();
  }
     random16_add_entropy( random8());




  EVERY_N_MILLISECONDS(10) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // FOR NOISE ANIMATION
    { gHue++; }


    if (setEffect == "Easter") {
      setPower = "ON";
      for (int i = 0; i < NUM_LEDS; i++) {                                     // Just ONE loop to fill up the LED array as all of the pixels change.
        uint8_t index = inoise8(i * scale, dist + i * scale) % 255;            // Get a value from the noise function. I'm using both x and y axis.
        leds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
      }
      dist += beatsin8(10, 1, 4);                                              // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
      // In some sketches, I've used millis() instead of an incremented counter. Works a treat.
    }


    if (setEffect == "Ripple") {
      for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(bgcol++, 255, 15);  // Rotate background colour.
      switch (step) {
        case -1:                                                          // Initialize ripple variables.
          center = random(NUM_LEDS);
          colour = random8();
          step = 0;
          break;
        case 0:
          leds[center] = CHSV(colour, 255, 255);                          // Display the first pixel of the ripple.
          step ++;
          break;
        case maxsteps:                                                    // At the end of the ripples.
          step = -1;
          break;
        default:                                                             // Middle of the ripples.
          leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade / step * 2);   // Simple wrap from Marc Miller
          leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade / step * 2);
          step ++;                                                         // Next step.
          break;
      }
    }

    
  }

  EVERY_N_SECONDS(5) {
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
  }

  FastLED.setBrightness(brightness);  //EXECUTE EFFECT COLOR
  FastLED.show();

  if (animationspeed > 0 && animationspeed < 150) {  //Sets animation speed based on receieved value
    FastLED.delay(1000 / animationspeed);
  }

}


////////////////////////place setup__Palette and __Palettestriped custom functions here - for Candy Cane effects ///////////////// 
///////You can use up to 4 colors and change the pattern of A's AB's B's and BA's as you like//////////////

void setupStripedPalette( CRGB A, CRGB AB, CRGB B, CRGB BA)
{
  currentPalettestriped = CRGBPalette16(
                            A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                          );
}

void setupHailPalette( CRGB A, CRGB AB, CRGB B, CRGB BA)
{
  hailPalettestriped = CRGBPalette16(
                            A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                          );
}

void setupHJPalette( CRGB A, CRGB AB, CRGB B, CRGB BA)
{
  HJPalettestriped = CRGBPalette16(
                            A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                          );
}

void setupIndPalette( CRGB A, CRGB AB, CRGB B, CRGB BA)
{
  IndPalettestriped = CRGBPalette16(
                            A, A, A, A, A, AB, AB, AB, AB, AB, B, B, B, B, B, B
                          );
}

void setupThxPalette( CRGB A, CRGB AB, CRGB B, CRGB BA)
{
  ThxPalettestriped = CRGBPalette16(
                            A, A, A, A, A, A, A, AB, AB, AB, B, B, B, B, B, B
                          );
}

void setupHalloweenPalette( CRGB A, CRGB AB, CRGB B, CRGB BA)
{
  HalloweenPalettestriped = CRGBPalette16(
                            A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                          );
}

////////////////////////////////////////////////////////

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } } //for CYCLON


void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}


void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void addGlitterColor( fract8 chanceOfGlitter, int Rcolor, int Gcolor, int Bcolor) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB(Rcolor, Gcolor, Bcolor);
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("MQTT: Attempting connection to " + String(mqtt_server) + " as " + mcuHostName);
    // Attempt to connect
    if (client.connect(mcuHostName, mqtt_user, mqtt_password, lwtTopic, 1, 1, "Offline")) {
      client.publish(lwtTopic,"Online", true);
      Serial.println("MQTT: Connected");

      FastLED.clear (); //Turns off startup LEDs after connection is made
      FastLED.show();

      client.subscribe(setcolorSubTopic);
      client.subscribe(setbrightnessTopic);
      //client.subscribe(setcolortemp);
      client.subscribe(setpowerSubTopic);
      client.subscribe(seteffectSubTopic);
      client.subscribe(setanimationspeedTopic);
      client.publish(setpowerPubTopic, "OFF");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void saveConfigCallback()
{ // Callback notifying us of the need to save config
  Serial.println(F("SPIFFS: Configuration changed, flagging for save"));
  shouldSaveConfig = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void saveUpdatedConfig()
{ // Save the custom parameters to config.json
  DynamicJsonBuffer jsonBuffer(256);
  JsonObject &json = jsonBuffer.createObject();
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;
  json["mqtt_user"] = mqtt_user;
  json["mqtt_password"] = mqtt_password;
  
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    Serial.println(F("SPIFFS: Failed to open config file for writing"));
  }
  else
  {
    json.printTo(configFile);
    configFile.close();
  }
  shouldSaveConfig = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void clearSavedConfig()
{ // Clear out all local storage
  SPIFFS.format();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  EEPROM.begin(512);
  for (uint16_t i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
  espReset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void espReset()
{
  if (client.connected())
  {
    client.publish(setpowerSubTopic, "OFF");
    client.disconnect();
  }
  delay(2000);
  Serial1.print("rest");
  Serial1.flush();
  ESP.reset();
  delay(5000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleRoot()
{ 
  // If we haven't collected the Nextion model, try now
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String(F("<h1>"));
  httpMessage += String(espName);
  httpMessage += String(F("</h1>"));

  httpMessage += String(F("<form method='POST' action='saveConfig'>"));
  httpMessage += String(F("<b>WiFi SSID</b> <i><small>(required)</small></i><input id='wifiSSID' required name='wifiSSID' maxlength=32 placeholder='WiFi SSID' value='")) + String(WiFi.SSID()) + "'>";
  httpMessage += String(F("<br/><b>WiFi Password</b> <i><small>(required)</small></i><input id='wifiPass' required name='wifiPass' type='password' maxlength=64 placeholder='WiFi Password' value='")) + String("********") + "'>";
  httpMessage += String(F("<br/><br/><b>LED Node Name</b> <i><small>(required)</small></i><input id='espName' required name='espName' maxlength=15 placeholder='LED Node Name' value='")) + String(espName) + "'>";
 httpMessage += String(F("<br/><br/><b>MQTT Broker</b> <i><small>(required)</small></i><input id='mqtt_server' required name='mqtt_server' maxlength=63 placeholder='mqtt_server' value='")) + String(mqtt_server) + "'>";
  httpMessage += String(F("<br/><b>MQTT Port</b> <i><small>(required)</small></i><input id='mqtt_port' required name='mqtt_port' type='number' maxlength=5 placeholder='mqtt_port' value='")) + String(mqtt_port) + "'>";
  httpMessage += String(F("<br/><b>MQTT User</b> <i><small>(optional)</small></i><input id='mqtt_user' name='mqtt_user' maxlength=31 placeholder='mqtt_user' value='")) + String(mqtt_user) + "'>";
  httpMessage += String(F("<br/><b>MQTT Password</b> <i><small>(optional)</small></i><input id='mqtt_password' name='mqtt_password' type='password' maxlength=31 placeholder='mqtt_password'>"));
  httpMessage += String(F("<br/><hr><button type='submit'>save settings</button></form>"));



  httpMessage += String(F("<hr><form method='get' action='LEDroutine'>"));
  httpMessage += String(F("<button type='submit'>Run LED effects</button></form>"));

  httpMessage += String(F("<hr><form method='get' action='reboot'>"));
  httpMessage += String(F("<button type='submit'>reboot device</button></form>"));

  httpMessage += String(F("<hr><form method='get' action='resetConfig'>"));
  httpMessage += String(F("<button type='submit'>factory reset settings</button></form>"));

  httpMessage += String(F("<hr><b>MQTT Status: </b>"));
  if (client.connected())
  { // Check MQTT connection
    httpMessage += String(F("Connected"));
  }
  else
  {
   // httpMessage += String(F("<font color='red'><b>Disconnected</b></font>, return code: ")) + String(client.returnCode());
  }
 // httpMessage += String(F("<br/><b>MQTT ClientID: </b>")) + String(mqttClientId);
  httpMessage += String(F("<br/><b>CPU Frequency: </b>")) + String(ESP.getCpuFreqMHz()) + String(F("MHz"));
  httpMessage += String(F("<br/><b>Sketch Size: </b>")) + String(ESP.getSketchSize()) + String(F(" bytes"));
  httpMessage += String(F("<br/><b>Free Sketch Space: </b>")) + String(ESP.getFreeSketchSpace()) + String(F(" bytes"));
  httpMessage += String(F("<br/><b>Heap Free: </b>")) + String(ESP.getFreeHeap());
  httpMessage += String(F("<br/><b>IP Address: </b>")) + String(WiFi.localIP().toString());
  httpMessage += String(F("<br/><b>Signal Strength: </b>")) + String(WiFi.RSSI());
  httpMessage += String(F("<br/><b>Uptime: </b>")) + String(long(millis() / 1000));
  httpMessage += String(F("<br/><b>Last reset: </b>")) + String(ESP.getResetInfo());

  httpMessage += FPSTR(HTTP_END);
  webServer.send(200, "text/html", httpMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleSaveConfig()
{ 
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);

  bool shouldSaveWifi = false;
  // Check required values
  if (webServer.arg("wifiSSID") != "" && webServer.arg("wifiSSID") != String(WiFi.SSID()))
  { // Handle WiFi update
    shouldSaveConfig = true;
    shouldSaveWifi = true;
    webServer.arg("wifiSSID").toCharArray(wifiSSID, 32);
    webServer.arg("wifiPass").toCharArray(wifiPass, 64);
  }
  if (webServer.arg("mqtt_server") != "" && webServer.arg("mqtt_server") != String(mqtt_server))
  { // Handle mqtt_server
    shouldSaveConfig = true;
    webServer.arg("mqtt_server").toCharArray(mqtt_server, 64);
  }
  if (webServer.arg("mqtt_port") != "" && webServer.arg("mqtt_port") != String(mqtt_port))
  { // Handle mqtt_port
    shouldSaveConfig = true;
    webServer.arg("mqtt_port").toCharArray(mqtt_port, 6);
  }
  if (webServer.arg("espName") != "" && webServer.arg("espName") != String(espName))
  { // Handle espName
    shouldSaveConfig = true;
    webServer.arg("espName").toCharArray(espName, 16);
  }

  // Check optional values
  if (webServer.arg("mqtt_user") != String(mqtt_user))
  { // Handle mqtt_user
    shouldSaveConfig = true;
    webServer.arg("mqtt_user").toCharArray(mqtt_user, 32);
  }
  if (webServer.arg("mqtt_password") != String(mqtt_password))
  { // Handle mqtt_password
    shouldSaveConfig = true;
    webServer.arg("mqtt_password").toCharArray(mqtt_password, 32);
  }


  if (shouldSaveConfig)
  { // Config updated, notify user and trigger write to SPIFFS
    httpMessage += String(F("<meta http-equiv='refresh' content='15;url=/' />"));
    httpMessage += FPSTR(HTTP_HEAD_END);
    httpMessage += String(F("<h1>")) + String(espName) + String(F("</h1>"));
    httpMessage += String(F("<br/>Saving updated configuration values and restarting device"));
    httpMessage += FPSTR(HTTP_END);
    webServer.send(200, "text/html", httpMessage);
    saveUpdatedConfig();
    if (shouldSaveWifi)
    {
     
      setup_wifi();
    }
    espReset();
  }
  else
  { // No change found, notify user and link back to config page
    httpMessage += String(F("<meta http-equiv='refresh' content='3;url=/' />"));
    httpMessage += FPSTR(HTTP_HEAD_END);
    httpMessage += String(F("<h1>")) + String(espName) + String(F("</h1>"));
    httpMessage += String(F("<br/>No changes found, returning to <a href='/'>home page</a>"));
    httpMessage += FPSTR(HTTP_END);
    webServer.send(200, "text/html", httpMessage);
  }
}

void webHandleLEDroutine(){

 
if (webServer.arg("Seteffect") != ""){
  setEffect = webServer.arg("Seteffect");
  
  }

 String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);

 
    httpMessage += String(F("<h1>LED effects</h1><b> Clicking an effect should trigger it on your device"));
    httpMessage += String(F("<br/><hr><br/><form method='get' action='/LEDroutine'>"));

    httpMessage += String(F("<button type='submit' name='Seteffect' value='Christmas'>Christmas</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='StPatty'>St Patty</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Valentine'>Valentine</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='TurkeyDay'>Turkey Day</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='USA'>USA</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Independence'>Independence</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Halloween'>Halloween</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='GoBlue'>Go Blue</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Hail'>Hail</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Touchdown'>Touchdown</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Punkin'>Punkin</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='LoveyDay'>Lovey Day</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='HollyJolly'>Holly Jolly Day</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Sinelon'>Sinelon</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Juggle'>Juggle</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Confetti'>Confetti</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Rainbow'>Rainbow</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='RainbowwithGlitter'>Rainbow with Glitter</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Glitter'>Glitter</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='BPM'>BPM</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Solid'>Solid</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Twinkle'>Twinkle</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Dots'>Dots</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Lightning'>Lightning</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='PoliceOne'>Police One</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='PoliceAll'>Police All</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='CandyCane'>Candy Cane</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='CyclonRainbow'>Cyclon Rainbow</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Fire'>Fire</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Easter'>Easter</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Ripple'>Ripple</button>"));
    httpMessage += String(F("</form>"));
    httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
    httpMessage += String(F("<button type='submit'>return home</button></form>"));
    httpMessage += FPSTR(HTTP_END);
    webServer.send(200, "text/html", httpMessage);
 


}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleResetConfig()
{ 
 
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);

  if (webServer.arg("confirm") == "yes")
  { // User has confirmed, so reset everything
    httpMessage += String(F("<h1>"));
    httpMessage += String(espName);
    httpMessage += String(F("</h1><b>Resetting all saved settings and restarting device into WiFi AP mode</b>"));
    httpMessage += FPSTR(HTTP_END);
    webServer.send(200, "text/html", httpMessage);
    delay(1000);
    clearSavedConfig();
  }
  else
  {
    httpMessage += String(F("<h1>Warning</h1><b>This process will reset all settings to the default values and restart the device.  You may need to connect to the WiFi AP displayed on the panel to re-configure the device before accessing it again."));
    httpMessage += String(F("<br/><hr><br/><form method='get' action='resetConfig'>"));
    httpMessage += String(F("<br/><br/><button type='submit' name='confirm' value='yes'>reset all settings</button></form>"));
    httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
    httpMessage += String(F("<button type='submit'>return home</button></form>"));
    httpMessage += FPSTR(HTTP_END);
    webServer.send(200, "text/html", httpMessage);
  }
}
