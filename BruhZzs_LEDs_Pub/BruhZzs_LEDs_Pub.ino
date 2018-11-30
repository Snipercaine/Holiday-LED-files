#include "sample_config.h"           // rename sample_config.h and edit any values needed
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
//#include <WiFiClient.h>
#include "Definitions.h"  // also includes const and variables

WiFiClient espClient; //this needs to be unique for each controller
//MQTTClient client(espClient); //this needs to be unique for each controller
MQTTClient client(256);
////////////////////////////////////////////////////////////
#include "Webhandles.h" 



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

/////////////////////////////////////////////////////////////////////////////////////////////////////////

  setup_wifi();
  
  webServer.on("/", webHandleRoot);
  webServer.on("/saveConfig", webHandleSaveConfig);
  webServer.on("/resetConfig", webHandleResetConfig);
  webServer.on("/LEDroutine", webHandleLEDroutine);
  webServer.on("/espfirmware", webHandleEspFirmware);
  webServer.on("/firmware", webHandleFirmware);
  webServer.on("/reboot", webHandleReboot);
  //webServer.onNotFound(webHandleNotFound);
  webServer.begin();


  Serial.println(String(mqtt_server));
    






  //reconnect()

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


  // Setup telnet server for remote debug output
  telnetServer.setNoDelay(true);
  telnetServer.begin();
   Serial.println(String(F("TELNET: debug server enabled at telnet:")) + WiFi.localIP().toString());

 Serial.println( "MQTTServer");
  
  Serial.println( String(mqtt_server));
 
 if (mqtt_server[0] !=0){
  client.begin(mqtt_server, atoi(mqtt_port), wifiClient);
  client.onMessage(mqttCallback);
   mqttConnect();
 }
 
  // client.setServer(mqtt_server, 1883); //CHANGE PORT HERE IF NEEDED
 // client.setCallback(callback);

if(String(LED_TYPEUSER) == "WS2811"){
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS8).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(12, 10000); //experimental for power management. Feel free to try in your own setup.
  FastLED.setBrightness(brightness);
}

if(String(LED_TYPEUSER) == "WS2812") {
  FastLED.addLeds<LED_TYPE1, DATA_PIN, COLOR_ORDER1>(leds, NUM_LEDS8).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(12, 10000); //experimental for power management. Feel free to try in your own setup.
  FastLED.setBrightness(brightness);
}

  setupStripedPalette( CRGB::Red, CRGB::Red, CRGB::White, CRGB::White); //for CANDY CANE
  setupThxPalette( CRGB::OrangeRed, CRGB::Olive, CRGB::Maroon, CRGB::Maroon); //for Thanksgiving
  setupHailPalette( CRGB::Blue, CRGB::Blue, CRGB::Yellow, CRGB::Yellow); //for HAIL
  setupHalloweenPalette( CRGB::DarkOrange, CRGB::DarkOrange, CRGB::Indigo, CRGB::Indigo); //for Halloween
  setupHJPalette( CRGB::Red, CRGB::Red, CRGB::Green, CRGB::Green); //for Holly Jolly
  setupIndPalette( CRGB::FireBrick, CRGB::Cornsilk, CRGB::MediumBlue, CRGB::MediumBlue); //for Independence


  gPal = HeatColors_p; //for FIRE

  fill_solid(leds, String(NumberLEDUser).toInt(), CRGB(255, 0, 0)); //Startup LED Lights
  FastLED.show();


}


//void callback(char* topic, byte* payload, unsigned int length)
void mqttCallback(String &topic, String &payload)

 {
  int i = 0;

  if (String(topic) == setpowerSubTopic) {
//    for (i = 0; i < length; i++) {
//      message_buff[i] = payload[i];
//    }
//    message_buff[i] = '\0';
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
  //  for (i = 0; i < length; i++) {
 //     message_buff[i] = payload[i];
  //  }
  //  message_buff[i] = '\0';
    setEffect = String(payload);
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
   // for (i = 0; i < length; i++) {
   //   message_buff[i] = payload[i];
   // }
  //  message_buff[i] = '\0';
    setBrightness = String(payload);
    Serial.println("Set Brightness: " + setBrightness);
    brightness = setBrightness.toInt();
    setPower = "ON";
    client.publish(setpowerPubTopic, "ON");
  }

  if (String(topic) == setcolorSubTopic) {
    //for (i = 0; i < length; i++) {
    //  message_buff[i] = payload[i];
   // }
 //   message_buff[i] = '\0';
    client.publish(setcolorPubTopic, message_buff);
    setColor = String(payload);
    Serial.println("Set Color: " + setColor);
    setPower = "ON";
    client.publish(setpowerPubTopic, "ON");
    }

  if (String(topic) == setanimationspeedTopic) {
  //  for (i = 0; i < length; i++) {
 //     message_buff[i] = payload[i];
 //   }
  //  message_buff[i] = '\0';
    setAnimationSpeed = String(payload);
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


  
  
  ArduinoOTA.handle();
  
  int Rcolor = setColor.substring(0, setColor.indexOf(',')).toInt();
  int Gcolor = setColor.substring(setColor.indexOf(',') + 1, setColor.lastIndexOf(',')).toInt();
  int Bcolor = setColor.substring(setColor.lastIndexOf(',') + 1).toInt();

  if (setPower == "OFF") {
    setEffect = "Solid";
    for ( int i = 0; i < String(NumberLEDUser).toInt(); i++) {
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
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}
  
  if (setEffect == "St Patty") {                                  // colored stripes pulsing in Shades of GREEN 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw2_greenman_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Valentine") {                                  // colored stripes pulsing in Shades of PINK and RED 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw2_redrosey_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Turkey Day") {                                  // colored stripes pulsing in Shades of Brown and ORANGE 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw2_thanks_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Thanksgiving") {                                  // colored stripes pulsing in Shades of Red and ORANGE and Green 
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, String(NumberLEDUser).toInt(),
                  startIndex, 16, /* higher = narrower stripes */
                  ThxPalettestriped, 255, LINEARBLEND);
}
  
  if (setEffect == "USA") {                                  // colored stripes pulsing in Shades of Red White & Blue 
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = bhw3_41_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Independence") {                        // colored stripes of Red White & Blue
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, String(NumberLEDUser).toInt(),
                  startIndex, 16, /* higher = narrower stripes */
                  IndPalettestriped, 255, LINEARBLEND);
}


  if (setEffect == "Halloween") {                                  // colored stripes pulsing in Shades of Purple and Orange
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = Orange_to_Purple_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Go Blue") {                                  // colored stripes pulsing in Shades of Maize and Blue
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = Pills_3_gp;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Hail") {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* higher = faster motion */

    fill_palette( leds, String(NumberLEDUser).toInt(),
                  startIndex, 16, /* higher = narrower stripes */
                  hailPalettestriped, 255, LINEARBLEND);
}
  
  if (setEffect == "Touchdown") {                 //Maize and Blue with POLICE ALL animation
    idex++;
    if (idex >= String(NumberLEDUser).toInt()) {
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

    fill_palette( leds, String(NumberLEDUser).toInt(),
                  startIndex, 16, /* higher = narrower stripes */
                  HalloweenPalettestriped, 255, LINEARBLEND);
  }

    if (setEffect == "Lovey Day") {                 //Valentine's Day colors (TWO COLOR SOLID)
    idex++;
    if (idex >= String(NumberLEDUser).toInt()) {
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

    fill_palette( leds, String(NumberLEDUser).toInt(),
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
    fadeToBlackBy( leds, String(NumberLEDUser).toInt(), 20);
    int pos = beatsin16(13, 0, String(NumberLEDUser).toInt());
    leds[pos] += CRGB(Rcolor, Gcolor, Bcolor);
  }

  if (setEffect == "Juggle" ) {                           // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, String(NumberLEDUser).toInt(), 20);
    byte dothue = 0;
    for ( int i = 0; i < 8; i++) {
      leds[beatsin16(i + 7, 0, String(NumberLEDUser).toInt())] |= CRGB(Rcolor, Gcolor, Bcolor);
      dothue += 32;
    }
  }

  if (setEffect == "Confetti" ) {                       // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( leds, String(NumberLEDUser).toInt(), 10);
    int pos = random16(String(NumberLEDUser).toInt());
    leds[pos] += CRGB(Rcolor + random8(64), Gcolor, Bcolor);
  }


  if (setEffect == "Rainbow") {
    // FastLED's built-in rainbow generator
    static uint8_t starthue = 0;    thishue++;
    fill_rainbow(leds, String(NumberLEDUser).toInt(), thishue, deltahue);
  }


  if (setEffect == "Rainbow with Glitter") {               // FastLED's built-in rainbow generator with Glitter
    static uint8_t starthue = 0;
    thishue++;
    fill_rainbow(leds, String(NumberLEDUser).toInt(), thishue, deltahue);
    addGlitter(80);
  }


  if (setEffect == "Glitter") {
    fadeToBlackBy( leds, String(NumberLEDUser).toInt(), 20);
    addGlitterColor(80, Rcolor, Gcolor, Bcolor);
  }


  if (setEffect == "BPM") {                                  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < String(NumberLEDUser).toInt(); i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

  if (setEffect == "Solid" & setPower == "ON" ) {          //Fill entire strand with solid color
    fill_solid(leds, String(NumberLEDUser).toInt(), CRGB(Rcolor, Gcolor, Bcolor));
  }

  

  if (setEffect == "Twinkle") {
    twinklecounter = twinklecounter + 1;
    if (twinklecounter < 2) {                               //Resets strip if previous animation was running
      FastLED.clear();
      FastLED.show();
    }
    const CRGB lightcolor(8, 7, 1);
    for ( int i = 0; i < String(NumberLEDUser).toInt(); i++) {
      if ( !leds[i]) continue; // skip black pixels
      if ( leds[i].r & 1) { // is red odd?
        leds[i] -= lightcolor; // darken if red is odd
      } else {
        leds[i] += lightcolor; // brighten if red is even
      }
    }
    if ( random8() < DENSITY) {
      int j = random16(String(NumberLEDUser).toInt());
      if ( !leds[j] ) leds[j] = lightcolor;
    }
  }

  if (setEffect == "Dots") {
    uint8_t inner = beatsin8(bpm, String(NumberLEDUser).toInt() / 4, String(NumberLEDUser).toInt() / 4 * 3);
    uint8_t outer = beatsin8(bpm, 0, String(NumberLEDUser).toInt() - 1);
    uint8_t middle = beatsin8(bpm, String(NumberLEDUser).toInt() / 3, String(NumberLEDUser).toInt() / 3 * 2);
    leds[middle] = CRGB::Purple;
    leds[inner] = CRGB::Blue;
    leds[outer] = CRGB::Aqua;
    nscale8(leds, String(NumberLEDUser).toInt(), fadeval);
  }

  if (setEffect == "Lightning") {
    twinklecounter = twinklecounter + 1;                     //Resets strip if previous animation was running
    Serial.println(twinklecounter);
    if (twinklecounter < 2) {
      FastLED.clear();
      FastLED.show();
    }
    ledstart = random8(String(NumberLEDUser).toInt());           // Determine starting location of flash
    ledlen = random8(String(NumberLEDUser).toInt() - ledstart);  // Determine length of flash (not to go beyond NumberLEDUser-1)
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
    if (idex >= String(NumberLEDUser).toInt()) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishuepolice + 160) % 255;
    for (int i = 0; i < String(NumberLEDUser).toInt(); i++ ) {
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
    if (idex >= String(NumberLEDUser).toInt()) {
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

    fill_palette( leds, String(NumberLEDUser).toInt(),
                  startIndex, 16, /* higher = narrower stripes */
                  currentPalettestriped, 255, LINEARBLEND);
  }


  if (setEffect == "Cyclon Rainbow") {                    //Single Dot Down
  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < String(NumberLEDUser).toInt(); i++) {
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
  for(int i = (String(NumberLEDUser).toInt())-1; i >= 0; i--) {
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
      for (int i = 0; i < String(NumberLEDUser).toInt(); i++) {                                     // Just ONE loop to fill up the LED array as all of the pixels change.
        uint8_t index = inoise8(i * scale, dist + i * scale) % 255;            // Get a value from the noise function. I'm using both x and y axis.
        leds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
      }
      dist += beatsin8(10, 1, 4);                                              // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
      // In some sketches, I've used millis() instead of an incremented counter. Works a treat.
    }


    if (setEffect == "Ripple") {
      for (int i = 0; i < String(NumberLEDUser).toInt(); i++) leds[i] = CHSV(bgcol++, 255, 15);  // Rotate background colour.
      switch (step) {
        case -1:                                                          // Initialize ripple variables.
          center = random(String(NumberLEDUser).toInt());
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
          leds[(center + step + String(NumberLEDUser).toInt()) % String(NumberLEDUser).toInt()] += CHSV(colour, 255, myfade / step * 2);   // Simple wrap from Marc Miller
          leds[(center - step + String(NumberLEDUser).toInt()) % String(NumberLEDUser).toInt()] += CHSV(colour, 255, myfade / step * 2);
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

  // telnetClient loop
  handleTelnetClient();
if (mqtt_server[0] !=0){
    if (!client.connected())
  { // Check MQTT connection
    mqttConnect();
  }
  
   client.loop();
  }else{
//      FastLED.clear (); //Turns off startup LEDs after connection is made
 //     FastLED.show();

    }
    
   
   
  
  
}


/////////////////

  ///////place setup__Palette and __Palettestriped custom functions here - for Candy Cane effects ///////////////// 
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

void fadeall() { for(int i = 0; i < String(NumberLEDUser).toInt(); i++) { leds[i].nscale8(250); } } //for CYCLON


void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS8];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < String(NumberLEDUser).toInt(); i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / String(NumberLEDUser).toInt()) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= String(NumberLEDUser).toInt() - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < String(NumberLEDUser).toInt(); j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (String(NumberLEDUser).toInt()-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}


void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(String(NumberLEDUser).toInt()) ] += CRGB::White;
  }
}

void addGlitterColor( fract8 chanceOfGlitter, int Rcolor, int Gcolor, int Bcolor) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(String(NumberLEDUser).toInt()) ] += CRGB(Rcolor, Gcolor, Bcolor);
  }
}


void reconnect() {
  // Loop until we're reconnected

   if (mqtt_server[0] == 0)
  {
    while (mqtt_server[0] == 0)
    { // Handle HTTP and OTA while we're waiting for MQTT to be configured
      yield();
      webServer.handleClient();
      ArduinoOTA.handle();
    }
  }
  while (!client.connected()) {
  
    // Attempt to connect
    if (client.connect(mcuHostName, mqtt_user, mqtt_password)){//, lwtTopic, 1, 1, "Offline")) {
 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.connected());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttConnect()
{ // MQTT connection and subscriptions

  // Check to see if we have a broker configured and notify the user if not
  if (mqtt_server[0] == 0)
  {
    while (mqtt_server[0] == 0)
    { // Handle HTTP and OTA while we're waiting for MQTT to be configured
      yield();
      webServer.handleClient();
      ArduinoOTA.handle();
    }
  }
  Serial.println( "MQTTServer");
  
  Serial.println( String(mqtt_server));
    // Loop until we're reconnected to MQTT
    if (mqtt_server[0] != 0){
  while (!client.connected())
  {

       static uint8_t mqttReconnectCount = 0;
    mqttClientId = String(espName);
    // Set keepAlive, cleanSession, timeout
  if(String(mqtt_server)!=""){
  Serial.println("MQTT: Attempting connection to " + String(mqtt_server) + " as " + mcuHostName);
    if (client.connect(espName, mqtt_user, mqtt_password))
    { // Attempt to connect to broker, setting last will and testament
      // Subscribe to our incoming topics
      Mqttconnected = 1 ;
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
      mqttReconnectCount = 0;
      unsigned long mqttReconnectTimeout = 10000;  // timeout for MQTT reconnect
      unsigned long mqttReconnectTimer = millis(); // record current time for our timeout
      while ((millis() - mqttReconnectTimer) < mqttReconnectTimeout)
      { // Handle HTTP and OTA while we're waiting for MQTT to reconnect
        yield();
        webServer.handleClient();
        ArduinoOTA.handle();
      }
    }}}
  }else{
      FastLED.clear (); //Turns off startup LEDs after connection is made
      FastLED.show();

    }
}
