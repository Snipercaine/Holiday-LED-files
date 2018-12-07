

////////////////////////////////////////////////////////////////////////////////////////////////////
void readSavedConfig()
{
  debuglineprint(F("SPIFFS: mounting FS..."));

  if (SPIFFS.begin())
  {
    debuglineprint(F("SPIFFS: mounted file system"));
    if (SPIFFS.exists("/config.json"))
    {
      // File exists, reading and loading
      debuglineprint(F("SPIFFS: reading config file"));
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        debuglineprint(F("SPIFFS: opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer configJsonBuffer(256);
        JsonObject &configJson = configJsonBuffer.parseObject(buf.get());
        if (configJson.success())
        {
          if (configJson["mqtt_server"].success())
          {
            strcpy(mqtt_server, configJson["mqtt_server"]);
          }
          if (configJson["mqtt_port"].success())
          {
            strcpy(mqtt_port, configJson["mqtt_port"]);
          }
          if (configJson["mqtt_user"].success())
          {
            strcpy(mqtt_user, configJson["mqtt_user"]);
          }
          if (configJson["mqtt_password"].success())
          {
            strcpy(mqtt_password, configJson["mqtt_password"]);
          }
          if (configJson["NumberLEDUser"].success())
          {
            strcpy(NumberLEDUser, configJson["NumberLEDUser"]);
           NUM_LEDS1= String(NumberLEDUser).toInt(); 
          }
          if (configJson["LED_TYPEUSER"].success())
          {
            strcpy(LED_TYPEUSER, configJson["LED_TYPEUSER"]);
          }
          //          if (configJson["Future"].success())
          //          {
          //            strcpy(configPassword, configJson["Future"]);
          //          }
          String configJsonStr;
          configJson.printTo(configJsonStr);
          debuglineprint(String(F("SPIFFS: parsed json:")) + configJsonStr);
        }
        else
        {
          debuglineprint(F("SPIFFS: [ERROR] Failed to load json config"));
        }
      }
    }
  }
  else
  {
    debuglineprint(F("SPIFFS: [ERROR] Failed to mount FS"));
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void saveConfigCallback()
{ // Callback notifying us of the need to save config
  debuglineprint(F("SPIFFS: Configuration changed, flagging for save"));
  shouldSaveConfig = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void saveUpdatedConfig()
{ // Save the custom parameters to config.json
  SPIFFS.begin();
  DynamicJsonBuffer jsonBuffer(512);
  JsonObject &json = jsonBuffer.createObject();
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;
  json["mqtt_user"] = mqtt_user;
  json["mqtt_password"] = mqtt_password;
  json["NumberLEDUser"] = NumberLEDUser;
  json["LED_TYPEUSER"] = LED_TYPEUSER;


  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    debuglineprint(F("SPIFFS: Failed to open config file for writing"));
  }
  else
  {
    json.printTo(configFile);
    Serial1.print(configFile);
    configFile.close();
  }

  SPIFFS.end();

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void espReset()
{
  if (MQTTclient.connected())
  {
    MQTTclient.publish(setpowerSubTopic, "OFF");
    MQTTclient.disconnect();
  }
  delay(2000);
  // Serial1.print("rest");
  // Serial1.flush();
  //  saveConfigCallback();
  // saveUpdatedConfig();
  SPIFFS.end();
  delay(5000);
  ESP.restart() ;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void clearSavedConfig()
{ // Clear out all local storage

  SPIFFS.format();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  //EEPROM.begin(512);
  // for (uint16_t i = 0; i < EEPROM.length(); i++)
  // {
  //   EEPROM.write(i, 0);
  // }
  espReset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void startEspOTA(String espOtaUrl)
{ // Update ESP firmware from HTTP

  WiFiUDP::stopAll(); // Keep mDNS responder from breaking things

  t_httpUpdate_return returnCode = ESPhttpUpdate.update("https://github.com/GeradB/Holiday-LED-files/blob/development/BruhZzs_LEDs_Pub/LED_ez_Pub.ino.d1_mini.bin");
  debuglineprint(espOtaUrl);
  switch (returnCode)
  {
    case HTTP_UPDATE_FAILED:
      debuglineprint("ESPFW: HTTP_UPDATE_FAILED error " + String(ESPhttpUpdate.getLastError()) + " " + ESPhttpUpdate.getLastErrorString());
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());

      break;

    case HTTP_UPDATE_NO_UPDATES:
      debuglineprint(F("ESPFW: HTTP_UPDATE_NO_UPDATES"));
      break;

    case HTTP_UPDATE_OK:
      debuglineprint(F("ESPFW: HTTP_UPDATE_OK"));
      espReset();
  }
  delay(5000);

}

////////////////////////////////////////////////////////////////////////////////////////////////////


void webHandleReboot()
{
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", (String(espName) + "  reboot"));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += String(F("<meta http-equiv='refresh' content='10;url=/' />"));
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String(F("<h1>")) + String(espName) + String(F("</h1>"));
  httpMessage += String(F("<br/>Rebooting device"));
  httpMessage += FPSTR(HTTP_END);

  espReset();
}


void setup_wifi() {

  delay(10);
  //();
  debuglineprint("Connecting to ");
  debuglineprint(wifi_ssid);

  //  WiFi.mode(WIFI_STA);
  //  WiFi.hostname(mcuHostName);
  //  WiFi.begin(wifi_ssid, wifi_password);

  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }
  //
  //  debuglineprint("");
  //  debuglineprint("WiFi connected");
  //  debuglineprint("IP address: ");
  // debuglineprint(WiFi.localIP());


  // Assign our hostname (default esp_name + left 6 MAC) before connecting to WiFi
  WiFi.hostname(mcuHostName);

  if ( WiFi.status() != WL_CONNECTED) /// (String(wifiSSID) == "")
  { // If the sketch has no defined a static wifiSSID to connect to,
    // use WiFiManager to collect required information from the user.

    // id/name, placeholder/prompt, default value, length, extra tags
    WiFiManagerParameter custom_LEDNodeHeader("<br/><br/><b>LED-EZ</b>");
    WiFiManagerParameter custom_mqttHeader("<br/><br/><b>MQTT Broker</b>");
    WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", mqtt_server, 63, " maxlength=39");
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", mqtt_port, 5, " maxlength=5 type='number'");
    WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", mqtt_user, 31, " maxlength=31");
    WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password", mqtt_password, 31, " maxlength=31 type='password'");
    WiFiManagerParameter custom_LEDtpe("LED_TYPEUSER", "LED Type WS2811", LED_TYPEUSER, 63, " maxlength=39");
    //   WiFiManagerParameter custom_mqttColororder("COLOR_ORDER", "RGB", Param2, 31, " maxlength=39");
    WiFiManagerParameter custom_mqttNumleds("NumberLEDUser", "Number of LED's", NumberLEDUser, 5, " maxlength=5 type='number'");
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
    //  wifiManager.addParameter(&custom_mqttColororder);
    wifiManager.addParameter(&custom_mqttNumleds);
    wifiManager.addParameter(&custom_espName);


    // Timeout until configuration portal gets turned off
    wifiManager.setTimeout(180);


    // Construct AP name
    String strWifiConfigAP = String("LED-ez");
    strWifiConfigAP.toCharArray(wifiConfigAP, (strWifiConfigAP.length() + 1));

    String strConfigPass = "123" ;
    strConfigPass.toCharArray(wifiConfigPass, 9);

    // Fetches SSID and pass from EEPROM and tries to connect
    // If it does not connect it starts an access point with the specified name
    // and goes into a blocking loop awaiting configuration.
    if (!wifiManager.autoConnect(wifiConfigAP, wifiConfigPass))
    { // Reset and try again
      debuglineprint(F("WIFI: Failed to connect and hit timeout"));
      espReset();

    }
    // Read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    strcpy(espName, custom_espName.getValue());
    strcpy(LED_TYPEUSER, custom_LEDtpe.getValue());
    strcpy(NumberLEDUser, custom_mqttNumleds.getValue());
    NUM_LEDS1 = atol( custom_mqttNumleds.getValue() );

    debuglineprint(String(NUM_LEDS1));
    debuglineprint(String(mqtt_server));

    if (shouldSaveConfig)
    { // Save the custom parameters to FS
      saveUpdatedConfig();
    }
 }

  }


//}
////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleRoot()
{
  if (webServer.arg("Seteffect") != "") {
if(MQTTclient.connected()){
 MQTTclient.publish(setpowerPubTopic, "ON");}

    //MQTTclient.publish(setpowerPubTopic, "ON");
    setPower = "ON";
    setEffect = webServer.arg("Seteffect");
  } else
  {
if(MQTTclient.connected()){
 MQTTclient.publish(setpowerPubTopic, "OFF");}
    //  MQTTclient.publish(setpowerPubTopic, "OFF");
    setPower = "OFF";
    setEffect = "OFF";
    SetTheEffect();
  }
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage += String(F ("<meta charset=utf8 />"));
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(F("<link href='data:image/jpeg;base64,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' rel='icon' type='image/x-icon' />"));
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String("<h1>LED-ez <img src='data:image/jpeg;base64,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' /></h1><hr>");
  //  httpMessage += String(F("<img src='data:image/jpeg;base64,/9j/4AAQSkZJRgABAQEAYABgAAD/4QAiRXhpZgAATU0AKgAAAAgAAQESAAMAAAABAAEAAAAAAAD/4gKgSUNDX1BST0ZJTEUAAQEAAAKQbGNtcwQwAABtbnRyUkdCIFhZWiAH4QABAAMAEgA4ABNhY3NwQVBQTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA9tYAAQAAAADTLWxjbXMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAtkZXNjAAABCAAAADhjcHJ0AAABQAAAAE53dHB0AAABkAAAABRjaGFkAAABpAAAACxyWFlaAAAB0AAAABRiWFlaAAAB5AAAABRnWFlaAAAB+AAAABRyVFJDAAACDAAAACBnVFJDAAACLAAAACBiVFJDAAACTAAAACBjaHJtAAACbAAAACRtbHVjAAAAAAAAAAEAAAAMZW5VUwAAABwAAAAcAHMAUgBHAEIAIABiAHUAaQBsAHQALQBpAG4AAG1sdWMAAAAAAAAAAQAAAAxlblVTAAAAMgAAABwATgBvACAAYwBvAHAAeQByAGkAZwBoAHQALAAgAHUAcwBlACAAZgByAGUAZQBsAHkAAAAAWFlaIAAAAAAAAPbWAAEAAAAA0y1zZjMyAAAAAAABDEoAAAXj///zKgAAB5sAAP2H///7ov///aMAAAPYAADAlFhZWiAAAAAAAABvlAAAOO4AAAOQWFlaIAAAAAAAACSdAAAPgwAAtr5YWVogAAAAAAAAYqUAALeQAAAY3nBhcmEAAAAAAAMAAAACZmYAAPKnAAANWQAAE9AAAApbcGFyYQAAAAAAAwAAAAJmZgAA8qcAAA1ZAAAT0AAACltwYXJhAAAAAAADAAAAAmZmAADypwAADVkAABPQAAAKW2Nocm0AAAAAAAMAAAAAo9cAAFR7AABMzQAAmZoAACZmAAAPXP/bAEMAAgEBAgEBAgICAgICAgIDBQMDAwMDBgQEAwUHBgcHBwYHBwgJCwkICAoIBwcKDQoKCwwMDAwHCQ4PDQwOCwwMDP/bAEMBAgICAwMDBgMDBgwIBwgMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDP/AABEIACgAKAMBIgACEQEDEQH/xAAfAAABBQEBAQEBAQAAAAAAAAAAAQIDBAUGBwgJCgv/xAC1EAACAQMDAgQDBQUEBAAAAX0BAgMABBEFEiExQQYTUWEHInEUMoGRoQgjQrHBFVLR8CQzYnKCCQoWFxgZGiUmJygpKjQ1Njc4OTpDREVGR0hJSlNUVVZXWFlaY2RlZmdoaWpzdHV2d3h5eoOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4eLj5OXm5+jp6vHy8/T19vf4+fr/xAAfAQADAQEBAQEBAQEBAAAAAAAAAQIDBAUGBwgJCgv/xAC1EQACAQIEBAMEBwUEBAABAncAAQIDEQQFITEGEkFRB2FxEyIygQgUQpGhscEJIzNS8BVictEKFiQ04SXxFxgZGiYnKCkqNTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqCg4SFhoeIiYqSk5SVlpeYmZqio6Slpqeoqaqys7S1tre4ubrCw8TFxsfIycrS09TV1tfY2dri4+Tl5ufo6ery8/T19vf4+fr/2gAMAwEAAhEDEQA/AP38zXm/iT9pLTYNYl03w9Y3XifUITiX7KQttCfRpTkZ+gI7ZB4r5R/4LL/8FgvBP/BPzXfB/wAONctvG1xrnxEt5btToGlrdMtnG/lsoZ5YlDM2c4YlUU52hlNeJfDD/gvR+zraaXb6bNq3ifwNCmF/4nvhm7jhUn+KW5gWa3X3Z5Ao9QK+B4ozzNKVX6vl9Kagl71SMHN37QVmtOsmmlta97fS5Tl+DlT9rippye0HK2neT0evRJpve9t/uT43ftT+Lvgr8KfEHjK68MeGf7P8OWTX90moeIRYQxQpgyM8+yQLtTcRiNixAUDLCvgz9tz43+A/2yfjX8LtD+MXwW8WfC03WjWut+NNXn8Falr2tParMZbTw5Y3NjaSSI8ryM9xIBGY0LxKfMkbb9AfF39oP4U/Ff8AZvuNZ8aaz4N1j4T6ktveSarfanD/AGNceVOk0EguA4Qss8UbLtbO9BjkYr5z8T/8FtP2X/DfgSHwrpOpeKPEGj2CtHapo3hvUZY4fmJ3xXN0saPySdyyMGB6kV5PDHiBi8OpVMvVbEVYt3urW0tbmhBKLvd2cd7O9lY7sw4fw05KOK5KcGls9d97Sk7/AH/5nvvxq/ba/af+DX7WPgH4VWGk/s/+JNW+IeovLpmj6UmsNqWk6BHKQ+pX28pFbxpGNued8gZY1cqQCvj/AP4IZfHD44ftV/Fr4veP/h5rXwY8UeNLnWrG38aax490nXotbt7AgrBYWTRkW6wRpDIQi5YMYzKWHlGiv3GtjKFSEPYwp6LV2esuujta2y0u9+tl8I8LKnNqUn5a9Om39dD2v/gvT+wTJ/wUSh8QaJ4E1O2b4yeBxbap4d0rUJVs21BlgDSW1pcthB58LupjcgeYsbsyKmT/ADt3HgH4zaf8YT8P5PBfxOXx2s32f/hGjot62rGXAIQW2zzCxBBwF5BB6Gv60P29vgp4dtfE1j4+1TxFdeGo5kh0u4mXSJdRt/NVnaF3WHDoxztDk7QVQcEjO3+yt8QLrUfDHiLd4wHjJItXWKPUFglgbYLG0xG4kAcsM8s2SQRkk5NfmuT4qvRzKtl9WlGKlKU4uM4u6b6wcueLfVpcrd3o3r9lmmDpVcvpY+lOT5Yxg04SVmrKymoqMkuivzJWWvT8RPiH/wAG1HxN8Jf8EwdL1TT/AIX3U3x60G+bxhqS2moWlxNdxPnzNLiiSc+Y8MIjZVRGZpopFjz5oB/MXwX4F+L3xZ+Lp8B+HvCfxB1zxp5zQvoVppl0+oROpwwkhC7o9ufmLABepIFf2ir4nk/vfrXkX7XPi+6ul8P2v/Cd/wDCC2t5FfR3V19mmuXu4/8ARz5KiHEg5wxKspwpGcEg+nUovKcJWxDnKpq52lKKs5Ne6m+WMYp7Juy7nlUP+FHFUsOoqGnL7qk9k3dpczb9Fr2Pnz/ghX+yPB+wV4U0H4c6xrFvqHxIk0K61LxVZaXtntLC8muIpZxcXI+WW4jZ4rddhIVIcHcCrkr6I/4J+/BHQ9Bg1DxxpOtXGuQalB/ZVlN/ZL6ZbiKOTMhiiky5UuqjcTjMZx60VjwzGt9S9rXhGDnKUrRlzq0ne/NdqTe91prZJLQ6eIvZLF+yoylJQjGLco8rvFWa5Wk0ltZ66Xbb1PoXxd4R03x74ZvtG1izh1DS9SiaC5tplykqHqD/ADBHIIBGCK+Lfif/AME+fiP8JdQu5vhb4s8QXnh+6k85tJh1t9NvInwFHzEiGbCqq72KNhVBDY3UUVtm2Q4bMLSqOUJxTSnCTjNJ7pNdH2d15EZRxBisuvCnyyhLVwmuaLa2dns/NNPzscDH8Jfj9LdfZxD8ZPOzjB8Q7Y8/75l2fjmvUvhH/wAE9fGnxB1iz1L4s+KNauNLs9xi0WTWpb+4cNt3q8udkQbaobyizMBgOlFFfMZXwfQnKX1vEVq0U/hqVHKLtqrx0vqtndd0fT5pxdXpxisJQpUpNfFCFpK+mjbdvVa9mfZGk6Ta6DpdvY2NvDaWdnEsEEEKBI4Y1AVUVRwFAAAA4AFFFFfoCSSsj8+lJt3Z/9k=' />"));
  //-----------------------------------------------------------------------------------------------------
  httpMessage += String(F("<form method='get' action='colour'>"));
  httpMessage += String(F("<button type='submit'>colour</button></form>"));

  httpMessage += String(F("<hr><form method='get' action='LEDroutine'>"));
  httpMessage += String(F("<button type='submit'>effects</button></form>"));
  httpMessage += String(F("<hr><form method='get' action='reboot'>"));
  httpMessage += String(F("<button type='submit'>reboot device</button></form>"));
  httpMessage += String(F("<hr><form method='get' action='resetConfig'>"));
  httpMessage += String(F("<button type='submit'>factory reset settings</button></form>"));
  httpMessage += String(F("<hr><form method='get' action='firmware'>"));
  httpMessage += String(F("<button type='submit'>update firmware</button></form>"));
  httpMessage += String(F("<hr><form method='get' action='MQtt'>"));
  httpMessage += String(F("<button type='submit'>MQTT Setup</button></form>"));
  //httpMessage += String(F("<hr><form method='get' action='boardinfo'>"));
  //httpMessage += String(F("<button type='submit'>Board Info</button></form>"));


  httpMessage += String(F("<hr><br/><b>No of LEDs: </b><label id='LED' name='LED' >")) + String(NumberLEDUser) + "</label></label></>";
  httpMessage += String(F("<br/><b>LED Type: </b><label id='LED_TYPE' name='LED_TYPE' >")) + String(LED_TYPEUSER) + "</label></><br/>";
  httpMessage += String(F("<b>MQTT Status: </b>"));
  if (Mqttconnected)
  { // Check MQTT connection
    httpMessage += String(F("Connected"));
  }
  else
  {
    httpMessage += String(F("<font color='red'><b>Disconnected</b></font>, return code: ")) + String(MQTTclient.returnCode());
  }
  httpMessage += String(F("<br/><b>MQTT ClientID: </b>")) + String(mcuHostName);
  httpMessage += String(F("<br/><b>Version No: </b>")) + String(versionno);
  httpMessage += String(F("<br/><b>CPU Frequency: </b>")) + String(ESP.getCpuFreqMHz()) + String(F("MHz"));
  httpMessage += String(F("<br/><b>Sketch Size: </b>")) + String(ESP.getSketchSize()) + String(F(" bytes"));
  httpMessage += String(F("<br/><b>Free Sketch Space: </b>")) + String(ESP.getFreeSketchSpace()) + String(F(" bytes"));
  httpMessage += String(F("<br/><b>Heap Free: </b>")) + String(ESP.getFreeHeap());

  httpMessage += String(F("<br/><b>Chip Id: </b>")) + String(ESP.getChipId());
  httpMessage += String(F("<br/><b>Core Version: </b>")) + String(ESP.getCoreVersion());
  httpMessage += String(F("<br/><b>Flash Chip Size: </b>")) + String(ESP.getFlashChipSize()) + String(F(" bytes"));
  
  httpMessage += String(F("<br/><b>IP Address: </b>")) + String(WiFi.localIP().toString());
  httpMessage += String(F("<br/><b>Signal Strength: </b>")) + String(WiFi.RSSI());
  httpMessage += String(F("<br/><b>Uptime: </b>")) + String(long(millis() / 1000));
  httpMessage += String(F("<br/><b>Last reset: </b>")) + String(ESP.getResetInfo());

  httpMessage += FPSTR(HTTP_END);
  webServer.send(200, "text/html", httpMessage);
}


void colorConverter(String hex)
{

  char  str[8];
  hex.toCharArray(str, 8);
  char red[5] = {0};
  char green[5] = {0};
  char blue[5] = {0};

  red[0] = green[0] = blue[0] = '0';
  red[1] = green[1] = blue[1] = 'X';

  red[2] = str[1];
  red[3] = str[2];

  green[2] = str[3];
  green[3] = str[4];

  blue[2] = str[5];
  blue[3] = str[6];

  int r = strtol(red, NULL, 16);
  int g = strtol(green, NULL, 16);
  int b = strtol(blue, NULL, 16);

  setColor = String(r) + "," + String(g) + "," + String(b);
  Rcolor = r;
  Gcolor = g;
  Bcolor  = b;

  debuglineprint("RED");
  debuglineprint(String(r));
  debuglineprint("Green");
  debuglineprint(String(g));
  debuglineprint("Blue");
  debuglineprint(String(b));
if(MQTTclient.connected()){
 MQTTclient.publish(setpowerPubTopic, "ON");}
  setPower = "ON";
  setEffect = "Solid";

}
void Webhandlecolour()
{


  String httpMessage = FPSTR(HTTP_HEAD);

  String PrevColor;
  if (webServer.arg("brightness") != "") {
    brightness = webServer.arg("brightness").toInt();
  }
  if (webServer.arg("color") != "") {
    colorConverter("#" + webServer.arg("color"));
    PrevColor = webServer.arg("color");
  }

  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += "<script src='//cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.js'></script>";
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String("<h1>LED-ez <img src='data:image/jpeg;base64,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' /></h1><hr>");
  httpMessage += String(F("<b> Choose a color"));
  httpMessage += String("<hr><form method='set'action='/colour'><label>Color:</label> <input class='jscolor  value='" + PrevColor + "' name='color'> ");
  httpMessage += String("<label>Brighness:</label><br/> <input type='range' min='1' max='100' value='" + String(brightness) + "' class='slider' name='brightness'/>");
  httpMessage += String(F("<br/><button type='submit'>Set</input></button></form>"));
  httpMessage += String(F("<script>function update(picker) { document.getElementById('rgb').innerHTML =  Math.round(picker.rgb[0]) + ', ' +        Math.round(picker.rgb[1]) + ', ' +        Math.round(picker.rgb[2]);}</script>"));
  httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
  httpMessage += String(F("<button type='submit'>return home</button></form>"));

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
  if (webServer.arg("LED_TYPE") != String(LED_TYPEUSER))
  { // Handle mqtt_password
    shouldSaveConfig = true;
    webServer.arg("LED_TYPE").toCharArray(LED_TYPEUSER, 32);
  }

  if (webServer.arg("LED") != String(NumberLEDUser))
  { // Handle mqtt_password
    shouldSaveConfig = true;
    webServer.arg("LED").toCharArray(NumberLEDUser, 5);
    NUM_LEDS1 = atol( NumberLEDUser );
  }

  if (shouldSaveConfig)
  { // Config updated, notify user and trigger write to SPIFFS
    httpMessage += String(F("<meta http-equiv='refresh' content='15;url=/' />"));
    httpMessage += FPSTR(HTTP_HEAD_END);
    httpMessage += String(F("<h1>")) + String(espName) + String(F("</h1>"));
    httpMessage += String(F("<br/>Saving updated configuration values and restarting device"));
    httpMessage += FPSTR(HTTP_END);
    httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
    httpMessage += String(F("<button type='submit'>return home</button></form>"));
    webServer.send(200, "text/html", httpMessage);
    saveUpdatedConfig();
    if (shouldSaveWifi)
    {

      if (SPIFFS.begin()) {
        debuglineprint("mounted file system");
        if (SPIFFS.exists("/config.json")) {
          //file exists, reading and loading
          debuglineprint("reading config file");
          File configFile = SPIFFS.open("/config.json", "r");
          if (configFile) {
            debuglineprint("opened config file");
            size_t size = configFile.size();
            // Allocate a buffer to store contents of the file.
            std::unique_ptr<char[]> buf(new char[size]);
            configFile.readBytes(buf.get(), size);
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.parseObject(buf.get());
            json.printTo(Serial);
            if (json.success()) {
              debuglineprint("\nparsed json");
              strcpy(LED_TYPEUSER,  json["LED_TYPEUSER"]);
              strcpy(NumberLEDUser, json["NumberLEDUser"]);
              NUM_LEDS1 = atol( json["NumberLEDUser"] );

              debuglineprint(String(NUM_LEDS1));
              debuglineprint(String(mqtt_server));
            } else {
              debuglineprint("failed to load json config");
            }
            configFile.close();
          }
        }
      } else {
        debuglineprint("failed to mount FS");
      }


    }
    // espReset();
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

void webHandleLEDroutine() {
  if (webServer.arg("Speed") != "") {

    animationspeed = webServer.arg("Speed").toInt();
  }


  if (webServer.arg("Brightness") != "") {
    brightness = webServer.arg("Brightness").toInt();
  }
  if (webServer.arg("Seteffect") != "") {
   


    // MQTTclient.publish(setpowerPubTopic, "ON");
    setPower = "ON";
    setEffect = webServer.arg("Seteffect");

if(MQTTclient.connected()){
 MQTTclient.publish(seteffectPubTopic, webServer.arg("Seteffect"));}
  } else
  {
if(MQTTclient.connected()){
 MQTTclient.publish(setpowerPubTopic, "off");}

//    MQTTclient.publish(setpowerPubTopic, "OFF");
    setPower = "OFF";
    SetTheEffect();

  }
  String Bright = String(brightness);
  String Speed = String(animationspeed);

  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage += String(F ("<meta charset=utf8 />"));
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);

  //httpMessage += String(F("<img src='data:image/jpeg;base64,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' />"));
  httpMessage += String("<h1>LED-ez <img src='data:image/jpeg;base64,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' /></h1><hr>");
  httpMessage += String(F("<b> Clicking an effect should trigger it on your device"));
  httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
  httpMessage += String(F("<button type='submit'>return home</button></form>"));
  httpMessage += String(F("<br/><hr><br/><form method='get' action='/LEDroutine'>"));
  httpMessage += String(F("<label>Brightness</label>"));
  httpMessage += String("<input type='range' min='1' max='100' value='" + Bright  + "' name='Brightness' class='slider' id='Brightnes'/>");
  httpMessage += String(F("<label>Speed</label>"));
  httpMessage += String("<input type='range' min='1' max='150' value='" + Speed  + "' class='slider'name='Speed'  id='Speed'/>");
  // httpMessage += String(F("<br/><button type='submit'>Set </button></form>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Christmas'>Christmas</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Holly Jolly'>Holly Jolly</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Candy Cane'>Candy Cane</button>​"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='Turkey Day'>Turkey Day</button>​"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Halloween'>Halloween</button>​"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Punkin'>Punkin</button>​"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='Valentine'>Valentine</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Lovey Day'>Lovey Day</button>"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='St Patty'>St Patty</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Easter'>Easter</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='USA'>USA</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Independence'>Independence</button>"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='Go Blue'>Go Blue</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Hail'>Hail</button>​"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Touchdown'>Touchdown</button>"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='Police One'>Police One</button>​"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Police All'>Police All</button>"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='Sinelon'>Sinelon</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Juggle'>Juggle</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Confetti'>Confetti</button>"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='Rainbow'>Rainbow</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Rainbow with Glitter'>Rainbow with Glitter</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Glitter'>Glitter</button>​"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='BPM'>BPM</button>"));

  httpMessage += String(F("<button type='submit' name='Seteffect' value='Twinkle'>Twinkle</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Dots'>Dots</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Lightning'>Lightning</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Cyclon Rainbow'>Cyclon Rainbow</button>"));
  httpMessage += String(F("<button type='submit' name='Seteffect' value='Fire'>Fire</button>"));
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
    httpMessage += String("<h1>LED-ez <img src='data:image/jpeg;base64,/9j/4AAQSkZJRgABAQEAYABgAAD/4QAiRXhpZgAATU0AKgAAAAgAAQESAAMAAAABAAEAAAAAAAD/4gKgSUNDX1BST0ZJTEUAAQEAAAKQbGNtcwQwAABtbnRyUkdCIFhZWiAH4QABAAMAEgA4ABNhY3NwQVBQTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA9tYAAQAAAADTLWxjbXMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAtkZXNjAAABCAAAADhjcHJ0AAABQAAAAE53dHB0AAABkAAAABRjaGFkAAABpAAAACxyWFlaAAAB0AAAABRiWFlaAAAB5AAAABRnWFlaAAAB+AAAABRyVFJDAAACDAAAACBnVFJDAAACLAAAACBiVFJDAAACTAAAACBjaHJtAAACbAAAACRtbHVjAAAAAAAAAAEAAAAMZW5VUwAAABwAAAAcAHMAUgBHAEIAIABiAHUAaQBsAHQALQBpAG4AAG1sdWMAAAAAAAAAAQAAAAxlblVTAAAAMgAAABwATgBvACAAYwBvAHAAeQByAGkAZwBoAHQALAAgAHUAcwBlACAAZgByAGUAZQBsAHkAAAAAWFlaIAAAAAAAAPbWAAEAAAAA0y1zZjMyAAAAAAABDEoAAAXj///zKgAAB5sAAP2H///7ov///aMAAAPYAADAlFhZWiAAAAAAAABvlAAAOO4AAAOQWFlaIAAAAAAAACSdAAAPgwAAtr5YWVogAAAAAAAAYqUAALeQAAAY3nBhcmEAAAAAAAMAAAACZmYAAPKnAAANWQAAE9AAAApbcGFyYQAAAAAAAwAAAAJmZgAA8qcAAA1ZAAAT0AAACltwYXJhAAAAAAADAAAAAmZmAADypwAADVkAABPQAAAKW2Nocm0AAAAAAAMAAAAAo9cAAFR7AABMzQAAmZoAACZmAAAPXP/bAEMAAgEBAgEBAgICAgICAgIDBQMDAwMDBgQEAwUHBgcHBwYHBwgJCwkICAoIBwcKDQoKCwwMDAwHCQ4PDQwOCwwMDP/bAEMBAgICAwMDBgMDBgwIBwgMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDP/AABEIACgAKAMBIgACEQEDEQH/xAAfAAABBQEBAQEBAQAAAAAAAAAAAQIDBAUGBwgJCgv/xAC1EAACAQMDAgQDBQUEBAAAAX0BAgMABBEFEiExQQYTUWEHInEUMoGRoQgjQrHBFVLR8CQzYnKCCQoWFxgZGiUmJygpKjQ1Njc4OTpDREVGR0hJSlNUVVZXWFlaY2RlZmdoaWpzdHV2d3h5eoOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4eLj5OXm5+jp6vHy8/T19vf4+fr/xAAfAQADAQEBAQEBAQEBAAAAAAAAAQIDBAUGBwgJCgv/xAC1EQACAQIEBAMEBwUEBAABAncAAQIDEQQFITEGEkFRB2FxEyIygQgUQpGhscEJIzNS8BVictEKFiQ04SXxFxgZGiYnKCkqNTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqCg4SFhoeIiYqSk5SVlpeYmZqio6Slpqeoqaqys7S1tre4ubrCw8TFxsfIycrS09TV1tfY2dri4+Tl5ufo6ery8/T19vf4+fr/2gAMAwEAAhEDEQA/AP38zXm/iT9pLTYNYl03w9Y3XifUITiX7KQttCfRpTkZ+gI7ZB4r5R/4LL/8FgvBP/BPzXfB/wAONctvG1xrnxEt5btToGlrdMtnG/lsoZ5YlDM2c4YlUU52hlNeJfDD/gvR+zraaXb6bNq3ifwNCmF/4nvhm7jhUn+KW5gWa3X3Z5Ao9QK+B4ozzNKVX6vl9Kagl71SMHN37QVmtOsmmlta97fS5Tl+DlT9rippye0HK2neT0evRJpve9t/uT43ftT+Lvgr8KfEHjK68MeGf7P8OWTX90moeIRYQxQpgyM8+yQLtTcRiNixAUDLCvgz9tz43+A/2yfjX8LtD+MXwW8WfC03WjWut+NNXn8Falr2tParMZbTw5Y3NjaSSI8ryM9xIBGY0LxKfMkbb9AfF39oP4U/Ff8AZvuNZ8aaz4N1j4T6ktveSarfanD/AGNceVOk0EguA4Qss8UbLtbO9BjkYr5z8T/8FtP2X/DfgSHwrpOpeKPEGj2CtHapo3hvUZY4fmJ3xXN0saPySdyyMGB6kV5PDHiBi8OpVMvVbEVYt3urW0tbmhBKLvd2cd7O9lY7sw4fw05KOK5KcGls9d97Sk7/AH/5nvvxq/ba/af+DX7WPgH4VWGk/s/+JNW+IeovLpmj6UmsNqWk6BHKQ+pX28pFbxpGNued8gZY1cqQCvj/AP4IZfHD44ftV/Fr4veP/h5rXwY8UeNLnWrG38aax490nXotbt7AgrBYWTRkW6wRpDIQi5YMYzKWHlGiv3GtjKFSEPYwp6LV2esuujta2y0u9+tl8I8LKnNqUn5a9Om39dD2v/gvT+wTJ/wUSh8QaJ4E1O2b4yeBxbap4d0rUJVs21BlgDSW1pcthB58LupjcgeYsbsyKmT/ADt3HgH4zaf8YT8P5PBfxOXx2s32f/hGjot62rGXAIQW2zzCxBBwF5BB6Gv60P29vgp4dtfE1j4+1TxFdeGo5kh0u4mXSJdRt/NVnaF3WHDoxztDk7QVQcEjO3+yt8QLrUfDHiLd4wHjJItXWKPUFglgbYLG0xG4kAcsM8s2SQRkk5NfmuT4qvRzKtl9WlGKlKU4uM4u6b6wcueLfVpcrd3o3r9lmmDpVcvpY+lOT5Yxg04SVmrKymoqMkuivzJWWvT8RPiH/wAG1HxN8Jf8EwdL1TT/AIX3U3x60G+bxhqS2moWlxNdxPnzNLiiSc+Y8MIjZVRGZpopFjz5oB/MXwX4F+L3xZ+Lp8B+HvCfxB1zxp5zQvoVppl0+oROpwwkhC7o9ufmLABepIFf2ir4nk/vfrXkX7XPi+6ul8P2v/Cd/wDCC2t5FfR3V19mmuXu4/8ARz5KiHEg5wxKspwpGcEg+nUovKcJWxDnKpq52lKKs5Ne6m+WMYp7Juy7nlUP+FHFUsOoqGnL7qk9k3dpczb9Fr2Pnz/ghX+yPB+wV4U0H4c6xrFvqHxIk0K61LxVZaXtntLC8muIpZxcXI+WW4jZ4rddhIVIcHcCrkr6I/4J+/BHQ9Bg1DxxpOtXGuQalB/ZVlN/ZL6ZbiKOTMhiiky5UuqjcTjMZx60VjwzGt9S9rXhGDnKUrRlzq0ne/NdqTe91prZJLQ6eIvZLF+yoylJQjGLco8rvFWa5Wk0ltZ66Xbb1PoXxd4R03x74ZvtG1izh1DS9SiaC5tplykqHqD/ADBHIIBGCK+Lfif/AME+fiP8JdQu5vhb4s8QXnh+6k85tJh1t9NvInwFHzEiGbCqq72KNhVBDY3UUVtm2Q4bMLSqOUJxTSnCTjNJ7pNdH2d15EZRxBisuvCnyyhLVwmuaLa2dns/NNPzscDH8Jfj9LdfZxD8ZPOzjB8Q7Y8/75l2fjmvUvhH/wAE9fGnxB1iz1L4s+KNauNLs9xi0WTWpb+4cNt3q8udkQbaobyizMBgOlFFfMZXwfQnKX1vEVq0U/hqVHKLtqrx0vqtndd0fT5pxdXpxisJQpUpNfFCFpK+mjbdvVa9mfZGk6Ta6DpdvY2NvDaWdnEsEEEKBI4Y1AVUVRwFAAAA4AFFFFfoCSSsj8+lJt3Z/9k=' /></h1><hr>");
    httpMessage += String(F("<h1>Warning</h1><b>This process will reset all settings to the default values and restart the device.  You will need to connect to the WiFi AP , Once connected to the AP, point your browser at 192.168.4.1 to re-configure the device before accessing it again."));
    httpMessage += String(F("<br/><hr><br/><form method='get' action='resetConfig'>"));
    httpMessage += String(F("<br/><br/><button type='submit' name='confirm' value='yes'>reset all settings</button></form>"));
    httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
    httpMessage += String(F("<button type='submit'>return home</button></form>"));
    httpMessage += FPSTR(HTTP_END);
    webServer.send(200, "text/html", httpMessage);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleFirmware()
{

  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", (String(espName) + " update"));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String(F("<form method='get' action='/espfirmware'>"));
  httpMessage += String(F("<br/><form method='POST' action='/update' enctype='multipart/form-data'>"));
  httpMessage += String("<h1>LED-ez <img src='data:image/jpeg;base64,/9j/4AAQSkZJRgABAQEAYABgAAD/4QAiRXhpZgAATU0AKgAAAAgAAQESAAMAAAABAAEAAAAAAAD/4gKgSUNDX1BST0ZJTEUAAQEAAAKQbGNtcwQwAABtbnRyUkdCIFhZWiAH4QABAAMAEgA4ABNhY3NwQVBQTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA9tYAAQAAAADTLWxjbXMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAtkZXNjAAABCAAAADhjcHJ0AAABQAAAAE53dHB0AAABkAAAABRjaGFkAAABpAAAACxyWFlaAAAB0AAAABRiWFlaAAAB5AAAABRnWFlaAAAB+AAAABRyVFJDAAACDAAAACBnVFJDAAACLAAAACBiVFJDAAACTAAAACBjaHJtAAACbAAAACRtbHVjAAAAAAAAAAEAAAAMZW5VUwAAABwAAAAcAHMAUgBHAEIAIABiAHUAaQBsAHQALQBpAG4AAG1sdWMAAAAAAAAAAQAAAAxlblVTAAAAMgAAABwATgBvACAAYwBvAHAAeQByAGkAZwBoAHQALAAgAHUAcwBlACAAZgByAGUAZQBsAHkAAAAAWFlaIAAAAAAAAPbWAAEAAAAA0y1zZjMyAAAAAAABDEoAAAXj///zKgAAB5sAAP2H///7ov///aMAAAPYAADAlFhZWiAAAAAAAABvlAAAOO4AAAOQWFlaIAAAAAAAACSdAAAPgwAAtr5YWVogAAAAAAAAYqUAALeQAAAY3nBhcmEAAAAAAAMAAAACZmYAAPKnAAANWQAAE9AAAApbcGFyYQAAAAAAAwAAAAJmZgAA8qcAAA1ZAAAT0AAACltwYXJhAAAAAAADAAAAAmZmAADypwAADVkAABPQAAAKW2Nocm0AAAAAAAMAAAAAo9cAAFR7AABMzQAAmZoAACZmAAAPXP/bAEMAAgEBAgEBAgICAgICAgIDBQMDAwMDBgQEAwUHBgcHBwYHBwgJCwkICAoIBwcKDQoKCwwMDAwHCQ4PDQwOCwwMDP/bAEMBAgICAwMDBgMDBgwIBwgMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDP/AABEIACgAKAMBIgACEQEDEQH/xAAfAAABBQEBAQEBAQAAAAAAAAAAAQIDBAUGBwgJCgv/xAC1EAACAQMDAgQDBQUEBAAAAX0BAgMABBEFEiExQQYTUWEHInEUMoGRoQgjQrHBFVLR8CQzYnKCCQoWFxgZGiUmJygpKjQ1Njc4OTpDREVGR0hJSlNUVVZXWFlaY2RlZmdoaWpzdHV2d3h5eoOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4eLj5OXm5+jp6vHy8/T19vf4+fr/xAAfAQADAQEBAQEBAQEBAAAAAAAAAQIDBAUGBwgJCgv/xAC1EQACAQIEBAMEBwUEBAABAncAAQIDEQQFITEGEkFRB2FxEyIygQgUQpGhscEJIzNS8BVictEKFiQ04SXxFxgZGiYnKCkqNTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqCg4SFhoeIiYqSk5SVlpeYmZqio6Slpqeoqaqys7S1tre4ubrCw8TFxsfIycrS09TV1tfY2dri4+Tl5ufo6ery8/T19vf4+fr/2gAMAwEAAhEDEQA/AP38zXm/iT9pLTYNYl03w9Y3XifUITiX7KQttCfRpTkZ+gI7ZB4r5R/4LL/8FgvBP/BPzXfB/wAONctvG1xrnxEt5btToGlrdMtnG/lsoZ5YlDM2c4YlUU52hlNeJfDD/gvR+zraaXb6bNq3ifwNCmF/4nvhm7jhUn+KW5gWa3X3Z5Ao9QK+B4ozzNKVX6vl9Kagl71SMHN37QVmtOsmmlta97fS5Tl+DlT9rippye0HK2neT0evRJpve9t/uT43ftT+Lvgr8KfEHjK68MeGf7P8OWTX90moeIRYQxQpgyM8+yQLtTcRiNixAUDLCvgz9tz43+A/2yfjX8LtD+MXwW8WfC03WjWut+NNXn8Falr2tParMZbTw5Y3NjaSSI8ryM9xIBGY0LxKfMkbb9AfF39oP4U/Ff8AZvuNZ8aaz4N1j4T6ktveSarfanD/AGNceVOk0EguA4Qss8UbLtbO9BjkYr5z8T/8FtP2X/DfgSHwrpOpeKPEGj2CtHapo3hvUZY4fmJ3xXN0saPySdyyMGB6kV5PDHiBi8OpVMvVbEVYt3urW0tbmhBKLvd2cd7O9lY7sw4fw05KOK5KcGls9d97Sk7/AH/5nvvxq/ba/af+DX7WPgH4VWGk/s/+JNW+IeovLpmj6UmsNqWk6BHKQ+pX28pFbxpGNued8gZY1cqQCvj/AP4IZfHD44ftV/Fr4veP/h5rXwY8UeNLnWrG38aax490nXotbt7AgrBYWTRkW6wRpDIQi5YMYzKWHlGiv3GtjKFSEPYwp6LV2esuujta2y0u9+tl8I8LKnNqUn5a9Om39dD2v/gvT+wTJ/wUSh8QaJ4E1O2b4yeBxbap4d0rUJVs21BlgDSW1pcthB58LupjcgeYsbsyKmT/ADt3HgH4zaf8YT8P5PBfxOXx2s32f/hGjot62rGXAIQW2zzCxBBwF5BB6Gv60P29vgp4dtfE1j4+1TxFdeGo5kh0u4mXSJdRt/NVnaF3WHDoxztDk7QVQcEjO3+yt8QLrUfDHiLd4wHjJItXWKPUFglgbYLG0xG4kAcsM8s2SQRkk5NfmuT4qvRzKtl9WlGKlKU4uM4u6b6wcueLfVpcrd3o3r9lmmDpVcvpY+lOT5Yxg04SVmrKymoqMkuivzJWWvT8RPiH/wAG1HxN8Jf8EwdL1TT/AIX3U3x60G+bxhqS2moWlxNdxPnzNLiiSc+Y8MIjZVRGZpopFjz5oB/MXwX4F+L3xZ+Lp8B+HvCfxB1zxp5zQvoVppl0+oROpwwkhC7o9ufmLABepIFf2ir4nk/vfrXkX7XPi+6ul8P2v/Cd/wDCC2t5FfR3V19mmuXu4/8ARz5KiHEg5wxKspwpGcEg+nUovKcJWxDnKpq52lKKs5Ne6m+WMYp7Juy7nlUP+FHFUsOoqGnL7qk9k3dpczb9Fr2Pnz/ghX+yPB+wV4U0H4c6xrFvqHxIk0K61LxVZaXtntLC8muIpZxcXI+WW4jZ4rddhIVIcHcCrkr6I/4J+/BHQ9Bg1DxxpOtXGuQalB/ZVlN/ZL6ZbiKOTMhiiky5UuqjcTjMZx60VjwzGt9S9rXhGDnKUrRlzq0ne/NdqTe91prZJLQ6eIvZLF+yoylJQjGLco8rvFWa5Wk0ltZ66Xbb1PoXxd4R03x74ZvtG1izh1DS9SiaC5tplykqHqD/ADBHIIBGCK+Lfif/AME+fiP8JdQu5vhb4s8QXnh+6k85tJh1t9NvInwFHzEiGbCqq72KNhVBDY3UUVtm2Q4bMLSqOUJxTSnCTjNJ7pNdH2d15EZRxBisuvCnyyhLVwmuaLa2dns/NNPzscDH8Jfj9LdfZxD8ZPOzjB8Q7Y8/75l2fjmvUvhH/wAE9fGnxB1iz1L4s+KNauNLs9xi0WTWpb+4cNt3q8udkQbaobyizMBgOlFFfMZXwfQnKX1vEVq0U/hqVHKLtqrx0vqtndd0fT5pxdXpxisJQpUpNfFCFpK+mjbdvVa9mfZGk6Ta6DpdvY2NvDaWdnEsEEEKBI4Y1AVUVRwFAAAA4AFFFFfoCSSsj8+lJt3Z/9k=' /></h1><hr>");

  httpMessage += String(F("Update the LED-ez <b>")) + String(espName) + String(F("</b> firmware from a file<hr>"));
  httpMessage += String(F("<input type='file' id='espSelect' name='espSelect' accept='.bin'>"));
  httpMessage += String(F("<br/><br/><button type='submit' id='espUploadSubmit' onclick='ackEspUploadSubmit()'>Update ESP from file</button></form>"));

  httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
  httpMessage += String(F("<button type='submit'>return home</button></form>"));

  httpMessage += String(F("<br/><hr>"));


  // Javascript to collect the filesize of the LCD upload and send it to /tftFileSize
  httpMessage += String(F("<script>function handleLcdFileSelect(evt) {"));
  httpMessage += String(F("var uploadFile = evt.target.files[0];"));
  httpMessage += String(F("document.getElementById('lcdUploadSubmit').innerHTML = 'Upload LCD firmware ' + uploadFile.name;"));
  httpMessage += String(F("var tftFileSize = '/tftFileSize?tftFileSize=' + uploadFile.size;"));
  httpMessage += String(F("var xhttp = new XMLHttpRequest();xhttp.open('GET', tftFileSize, true);xhttp.send();}"));
  httpMessage += String(F("function ackLcdUploadSubmit() {document.getElementById('lcdUploadSubmit').innerHTML = 'Uploading LCD firmware...';}"));
  httpMessage += String(F("function handleEspFileSelect(evt) {var uploadFile = evt.target.files[0];document.getElementById('espUploadSubmit').innerHTML = 'Upload ESP firmware ' + uploadFile.name;}"));
  httpMessage += String(F("function ackEspUploadSubmit() {document.getElementById('espUploadSubmit').innerHTML = 'Uploading ESP firmware...';}"));
  httpMessage += String(F("document.getElementById('lcdSelect').addEventListener('change', handleLcdFileSelect, false);"));
  httpMessage += String(F("document.getElementById('espSelect').addEventListener('change', handleEspFileSelect, false);</script>"));

  httpMessage += FPSTR(HTTP_END);
  webServer.send(200, "text/html", httpMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleEspFirmware()
{
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", (String(espName) + " ESP update"));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += String(F("<meta http-equiv='refresh' content='60;url=/' />"));
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String(F("<h1>"));
  httpMessage += String(espName) + " ESP update";
  httpMessage += String(F("</h1>"));
  httpMessage += "<br/>Updating ESP firmware from: " + String(webServer.arg("espFirmware"));
  httpMessage += FPSTR(HTTP_END);
  webServer.send(200, "text/html", httpMessage);

  debuglineprint("ESPFW: Attempting ESP firmware update from: " + String(webServer.arg("espFirmware")));
  startEspOTA(webServer.arg("espFirmware"));
}
void webHandleMQtt()
{

  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(F("<link href='data:image/jpeg;base64,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' rel='icon' type='image/x-icon' />"));
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String("<h1>LED-ez <img src='data:image/jpeg;base64,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' /></h1><hr>");

  httpMessage += String(F("<br/>"));
  httpMessage += String(F("<img src='data:image/png;base64,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' />"));
  httpMessage += String(F("<br/><b>MQTT ClientID: </b>")) + String(mcuHostName);
  httpMessage += String(F("<br/><b>LWT: </b>")) +  mcuHostName + "/LWT";
  httpMessage += String(F("<br/><b>Colour Status </b>")) +  mcuHostName + "/colorstatus";
  httpMessage += String(F("<br/><b>Set Colour </b>")) +  mcuHostName + "/setcolor";
  httpMessage += String(F("<br/><b>Set Power </b>")) +  mcuHostName + "/setpower";
  httpMessage += String(F("<br/><b>Set Effect </b>")) +  mcuHostName + "/seteffect";
  httpMessage += String(F("<br/><b>Set Brightness </b>")) +  mcuHostName + "/setbrightness";
  httpMessage += String(F("<br/><b>Set Colour Pub </b>")) + mcuHostName + "/setcolorpub";

  httpMessage += String(F("<br/><b>Set Power </b>")) + mcuHostName + "/setpowerpub" ;
  httpMessage += String(F("<br/><b>Set Effect </b>")) +   mcuHostName + "/seteffectpub";
  httpMessage += String(F("<br/><b>Set Brightness </b>")) +  mcuHostName + "/setbrightnesspub";
  httpMessage += String(F("<br/><b>Set Colour Pub </b>")) +  mcuHostName + "/setanimationspeed";


  httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
  httpMessage += String(F("<button type='submit'>return home</button></form>"));


  httpMessage += FPSTR(HTTP_END);
  webServer.send(200, "text/html", httpMessage);


}
