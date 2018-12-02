/************ WIFI and MQTT INFORMATION (CHANGE THESE FOR YOUR SETUP) ******************/
#define wifi_ssid "xxxx" //enter your WIFI SSID
#define wifi_password "xxxxx" //enter your WIFI Password

//#define mqtt_server "xxx.xxx.xx.xx" // Enter your MQTT server adderss or IP. I use my DuckDNS adddress (yourname.duckdns.org) in this field
//#define mqtt_user "xxxx" //enter your MQTT username
//#define mqtt_port "" //enter your MQTT Port
//#define mqtt_password "xxxx" //enter your password


char mqtt_server[64] = ""; // These defaults may be overwritten with values saved by the web interface
char mqtt_port[6] = "1883";
char mqtt_user[32] = "DVES_USER";
char mqtt_password[32] = "DVES_PASS";
char versionno[8] = "x.x.05";
//const int NUM_LEDS =4;



#define DATA_PIN    D4 //on the NodeMCU 1.0, FastLED will default to the D5 pin after throwing an error during compiling. Leave as is. 
#define LED_TYPE    WS2811 //change to match your LED type WS2812
#define LED_TYPE1    WS2812 //change to match your LED type WS2812
#define COLOR_ORDER RGB //change to match your LED configuration // RGB for 2811's | GRB for 2812's //
#define COLOR_ORDER1 GRB //change to match your LED configuration // RGB for 2811's | GRB for 2812's //

#define NUM_LEDS8    1000 //change to match your setup


#define espName "LEDstrip" //change this to whatever you want to call your device
#define OTApassword "" //the password you will need to enter to upload remotely via the ArduinoIDE
int OTAport = 8266;
/************ FastLED Defintions ******************/
