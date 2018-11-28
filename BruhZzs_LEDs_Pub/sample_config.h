/************ WIFI and MQTT INFORMATION (CHANGE THESE FOR YOUR SETUP) ******************/

#define wifi_ssid "xxxx"                //enter your WIFI SSID
#define wifi_password "xxxxx"           //enter your WIFI Password

#define mqtt_server "xxx.xxx.xxx.xxx"   // Enter your MQTT server adderss or IP. I use my DuckDNS adddress (yourname.duckdns.org) in this field
#define mqtt_user "xxxx"                //enter your MQTT username
#define mqtt_port "1883"                //enter your MQTT Port 1883 is the default non SSL port 8883 for SSL
#define mqtt_password "xxxx"            //enter your password
#define Param1 ""                       //
#define Param2 ""                       //
#define Param3 ""                       //

#define espName "LEDstrip"              //change this to whatever you want to call your device
#define OTApassword ""                  //the password you will need to enter to upload remotely via the ArduinoIDE
int OTAport = 8266;

/************ FastLED Defintions ******************/

//#define FASTLED_ESP8266_RAW_PIN_ORDER // uncomment me if you are having issues with the nodemcu not working
#define DATA_PIN    D4                  //on the NodeMCU 1.0, FastLED will default to the D5 pin after throwing an error during compiling. Leave as is.
#define LED_TYPE    WS2811              //change to match your LED type WS2812
#define COLOR_ORDER RGB                 //change to match your LED configuration // RGB for 2811's | GRB for 2812's //
#define NUM_LEDS    175                 //change to match your setup
