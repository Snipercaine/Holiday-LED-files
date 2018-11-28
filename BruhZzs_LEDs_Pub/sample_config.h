/************ WIFI and MQTT INFORMATION (CHANGE THESE FOR YOUR SETUP) ******************/

#define wifi_ssid "xxxx"                // Enter your WIFI SSID
#define wifi_password "xxxxx"           // Enter your WIFI Password

#define mqtt_server "xxx.xxx.xxx.xxx"   // Enter your MQTT server adderss or IP. I use my DuckDNS adddress (yourname.duckdns.org) in this field
#define mqtt_user "xxxx"                // Enter your MQTT username
#define mqtt_port "1883"                // Enter your MQTT Port 1883 is the default non SSL port 8883 for SSL
#define mqtt_password "xxxx"            // Enter your password
#define Param1 ""                       // For Future use
#define Param2 ""                       // For Future use
#define Param3 ""                       // For Future use

#define espName "LEDstrip"              // Change this to whatever you want to call your device
#define OTApassword ""                  // The password you will need to enter to upload remotely via the ArduinoIDE
int OTAport = 8266;

/************ FastLED Defintions ******************/

//#define FASTLED_ESP8266_RAW_PIN_ORDER // Uncomment me if you are having issues with the nodemcu not working
#define DATA_PIN    D4                  // On the NodeMCU 1.0, FastLED will default to the D5 pin after throwing an error during compiling. Leave as is.
#define LED_TYPE    WS2811              // Change to match your LED type WS2812
#define COLOR_ORDER RGB                 // Change to match your LED configuration // RGB for 2811's | GRB for 2812's //
#define NUM_LEDS    175                 // Change to match your setup
