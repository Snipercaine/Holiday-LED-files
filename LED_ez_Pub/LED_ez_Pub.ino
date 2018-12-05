//########  ########  ######## ######## ####  ######     ##       ######## ########          ######## ######## 
//##     ## ##     ##      ##       ##  #### ##    ##    ##       ##       ##     ##         ##            ##  
//##     ## ##     ##     ##       ##    ##  ##          ##       ##       ##     ##         ##           ##   
//##     ## ########     ##       ##    ##    ######     ##       ######   ##     ## ####### ######      ##    
//##     ## ##   ##     ##       ##                ##    ##       ##       ##     ##         ##         ##     
//##     ## ##    ##   ##       ##           ##    ##    ##       ##       ##     ##         ##        ##      
//########  ##     ## ######## ########       ######     ######## ######## ########          ######## ######## 
//
// Thanks To Luma for pieces of the code, Bruh for LED effects and initial Sketch ..... etc.
//

//esp8266 ver 2.4.2 board manager 
///wifi manager tzapu latest not beta

#include <FS.h>
//#include <WiFiUdp.h>
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
#include <MQTT.h>
#include <FastLED.h>
#include <ESP8266mDNS.h>
#include <string>
#include "Definitions.h"  // also includes const and variables
////////////////////////////////////////////////////////////

WiFiClient espClient; //this needs to be unique for each controller
MQTTClient client(256);

////////////////////////////////////////////////////////////

#include "Webhandles.h" 
#include "Led_mqtt.h"
#include "Led_effects.h"


void setup() {
  
  Serial.begin(115200);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////
//read configuration from FS json
/////////////////////////////////////////////////////////////////////////////////////////////////////////

  
  // build hostname with last 6 of MACID
  os_strcpy(mcuHostName, getDeviceID());

  // ~~~~ Set MQTT Topics
   SetTopics();

  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  readSavedConfig();
/////////////////////////////////////////////////////////////////////////////////////////////////////////
  setup_wifi();

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Webserver handles
/////////////////////////////////////////////////////////////////////////////////////////////////////////
  httpOTAUpdate.setup(&webServer, "/update");
  webServer.on("/", webHandleRoot);
  webServer.on("/saveConfig", webHandleSaveConfig);
  webServer.on("/resetConfig", webHandleResetConfig);
  webServer.on("/LEDroutine", webHandleLEDroutine);
  webServer.on("/espfirmware", webHandleEspFirmware);
  webServer.on("/espfirmware", webHandleEspFirmware);
  webServer.on("/firmware", webHandleFirmware);
  webServer.on("/MQtt", webHandleMQtt);
  webServer.on("/reboot", webHandleReboot);
  webServer.on("/colour", Webhandlecolour);
  
  //webServer.onNotFound(webHandleNotFound);
  webServer.begin();


/////////////////////////////////////////////////////////////////////////////////////////////////////////

  debuglineprint(String(mqtt_server));
  Serial.begin(115200);
  debuglineprint(String(numberLEDs));
  //clean FS, for testing
  //SPIFFS.format();
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTA
/////////////////////////////////////////////////////////////////////////////////////////////////////////

  MDNS.begin(mcuHostName);
  httpUpdater.setup(&webServer);
  webServer.begin();
  MDNS.addService("http", "tcp", 81);
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
     debuglineprint("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    debuglineprint("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      debuglineprint("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      debuglineprint("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      debuglineprint("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      debuglineprint("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      debuglineprint("End Failed");
    }
  });
  ArduinoOTA.begin();
  debuglineprint("Ready");
  Serial.print("IP address: ");
  debuglineprint(WiFi.localIP().toString());
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup telnet server for remote debug output
/////////////////////////////////////////////////////////////////////////////////////////////////////////

  telnetServer.setNoDelay(true);
  telnetServer.begin();
   debuglineprint(String(F("TELNET: debug server enabled at telnet:")) + WiFi.localIP().toString());
   debuglineprint( "MQTTServer");
   debuglineprint( String(mqtt_server));

 ///////////////////////////////////////////////////////////////////////////////////////
 // MQTT   
 ///////////////////////////////////////////////////////////////////////////////////////
 ConnectMQtt();
///////////////////////////////////////////////////////////////////////////////////////
// Set The LED Type and No of LED's   
///////////////////////////////////////////////////////////////////////////////////////
 Ledstringtype();

}

void loop() {

  webServer.handleClient();
  
  while ((WiFi.status() != WL_CONNECTED) || (WiFi.localIP().toString() == "0.0.0.0"))
  { // Check WiFi is connected and that we have a valid IP, retry until we do.
    if (WiFi.status() == WL_CONNECTED)
    { // If we're currently connected, disconnect so we can try again
      WiFi.disconnect();
    }
    setup_wifi();
  }
  
  webServer.handleClient(); // webServer loop
    
  ArduinoOTA.handle();
   SetTheEffect();
     handleTelnetClient();

if (mqtt_server[0] !=0){
    if (!client.connected())
  { // Check MQTT connection
    mqttConnect();
  client.loop();
  }
  
   
  }else{
      FastLED.clear (); //Turns off startup LEDs after connection is made
      FastLED.show();

    }

 }

void reconnect() {
  // Loop until we're reconnected

  
    while (mqtt_server[0] == 0)
    { // Handle HTTP and OTA while we're waiting for MQTT to be configured
      yield();
      webServer.handleClient();
      ArduinoOTA.handle();
    }
  
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(mcuHostName, mqtt_user, mqtt_password)){//, lwtTopic, 1, 1, "Offline")) {
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.connected());
      debuglineprint(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
