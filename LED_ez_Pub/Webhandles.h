

////////////////////////////////////////////////////////////////////////////////////////////////////
void saveConfigCallback()
{ // Callback notifying us of the need to save config
  Serial.println(F("SPIFFS: Configuration changed, flagging for save"));
  shouldSaveConfig = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void saveUpdatedConfig()
{ // Save the custom parameters to LED.json
  DynamicJsonBuffer jsonBuffer(256);
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
    Serial.println(F("SPIFFS: Failed to open config file for writing"));
  }
  else
  {
    json.printTo(configFile);
    configFile.close();
  }


  json["NumberLEDUser"] = NumberLEDUser;
  json["LED_TYPEUSER"] = LED_TYPEUSER;

  
  File configFile1 = SPIFFS.open("/LED.json", "w");
  if (!configFile1)
  {
    Serial.println(F("SPIFFS: Failed to open config file for writing LED"));
  }
  else
  {
    json.printTo(configFile1);
    configFile.close();
  }
  
  shouldSaveConfig = false;
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
  saveConfigCallback();
  saveUpdatedConfig();
  ESP.reset();
  delay(5000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void clearSavedConfig()
{ // Clear out all local storage
  //SPIFFS.format();
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
void startEspOTA(String espOtaUrl)
{ // Update ESP firmware from HTTP
  
  WiFiUDP::stopAll(); // Keep mDNS responder from breaking things

  t_httpUpdate_return returnCode = ESPhttpUpdate.update("https://github.com/GeradB/Holiday-LED-files/blob/development/BruhZzs_LEDs_Pub/LED_ez_Pub.ino.d1_mini.bin");
   Serial.println(espOtaUrl);
  switch (returnCode)
  {
  case HTTP_UPDATE_FAILED:
    Serial.println("ESPFW: HTTP_UPDATE_FAILED error " + String(ESPhttpUpdate.getLastError()) + " " + ESPhttpUpdate.getLastErrorString());
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
              
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println(F("ESPFW: HTTP_UPDATE_NO_UPDATES"));
    break;

  case HTTP_UPDATE_OK:
    Serial.println(F("ESPFW: HTTP_UPDATE_OK"));
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
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

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
      Serial.println(F("WIFI: Failed to connect and hit timeout"));
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
     numberLEDs = atol( custom_mqttNumleds.getValue() );

  Serial.println(String(numberLEDs));
  Serial.println(String(mqtt_server));
    

 DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["LED_TYPEUSER"] = String(LED_TYPEUSER);
    json["NumberLEDUser"] = NumberLEDUser;
    File configFile = SPIFFS.open("/LED.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
 // if (strlen(mqtt_server) !=  "xxx.xxx.xxx.xxx"){
 // client.begin(mqtt_server, atoi(mqtt_port), wifiClient);
 // client.onMessage(mqttCallback);
 //  mqttConnect();

 // }

}





  
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
void webHandleRoot()
{ 

  if (webServer.arg("Seteffect") != ""){
  client.publish(setpowerPubTopic, "ON");
 setPower = "ON";
  setEffect = webServer.arg("Seteffect");

  }else
  {
      client.publish(setpowerPubTopic, "OFF");
      setPower = "OFF";
  
  }
  // If we haven't collected the Nextion model, try now
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(F("<link href='data:image/jpeg;base64,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' rel='icon' type='image/x-icon' />"));
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String(F("<h1>"));
  httpMessage += String("LED-ez");
  httpMessage += String(F("</h1>"));
  httpMessage += String(F("<img src='data:image/jpeg;base64,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' />"));
  httpMessage += String(F("<br/><b>No of LED's </b><label id='LED' name='LED' ><b>'")) + String(NumberLEDUser) + "'</b></>";
  httpMessage += String(F("<br/><b>LED Type  </b><label id='LED_TYPE' name='LED_TYPE' ><b>'")) +String(LED_TYPEUSER) + "'</b></>";
  httpMessage += String(F("<hr><form method='get' action='LEDroutine'>"));
  httpMessage += String(F("<button type='submit'>Run LED effects</button></form>"));
  httpMessage += String(F("<hr><form method='get' action='reboot'>"));
  httpMessage += String(F("<button type='submit'>reboot device</button></form>"));
  httpMessage += String(F("<hr><form method='get' action='resetConfig'>"));
  httpMessage += String(F("<button type='submit'>factory reset settings</button></form>"));
  httpMessage += String(F("<hr><form method='get' action='firmware'>"));
  httpMessage += String(F("<button type='submit'>update firmware</button></form>"));
  
  httpMessage += String(F("<hr><b>MQTT Status: </b>"));
  if (Mqttconnected)
  { // Check MQTT connection
    httpMessage += String(F("Connected"));
  }
  else
  {
    httpMessage += String(F("<font color='red'><b>Disconnected</b></font>, return code: ")) + String(client.returnCode());
  }
  httpMessage += String(F("<br/><b>MQTT ClientID: </b>")) + String(mcuHostName);
  httpMessage += String(F("<br/><b>Version No: </b>")) + String(versionno);
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
    if (webServer.arg("LED_TYPE") != String(LED_TYPEUSER))
    { // Handle mqtt_password
      shouldSaveConfig = true;
      webServer.arg("LED_TYPE").toCharArray(LED_TYPEUSER, 32);
    }
  
    if (webServer.arg("LED") != String(NumberLEDUser))
    { // Handle mqtt_password
      shouldSaveConfig = true;
      webServer.arg("LED").toCharArray(NumberLEDUser,5);
        numberLEDs = atol( NumberLEDUser );
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
    Serial.println("mounted file system");
    if (SPIFFS.exists("/LED.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/LED.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
         strcpy(LED_TYPEUSER,  json["LED_TYPEUSER"]);
         strcpy(NumberLEDUser, json["NumberLEDUser"]);
         numberLEDs = atol( json["NumberLEDUser"] );
  
  Serial.println(String(numberLEDs));
  Serial.println(String(mqtt_server));
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
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

void webHandleLEDroutine(){

 
if (webServer.arg("Seteffect") != ""){
  client.publish(setpowerPubTopic, "ON");
 setPower = "ON";
  setEffect = webServer.arg("Seteffect");

  }else
  {
      client.publish(setpowerPubTopic, "OFF");
      setPower = "OFF";
  
  }

 String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage += String(F ("<meta charset=utf8 />"));
  httpMessage.replace("{v}", String(espName));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
    httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);

   httpMessage += String(F("<img src='data:image/jpeg;base64,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' />"));
 
    httpMessage += String(F("<h1>LED-ez</h1><b> Clicking an effect should trigger it on your device"));
  
    httpMessage += String(F("<br/><hr><br/><form method='get' action='/'>"));
    httpMessage += String(F("<button type='submit'>return home</button></form>"));
    httpMessage += String(F("<br/><hr><br/><form method='get' action='/LEDroutine'>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Christmas'>Christmas</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='StPatty'>St Patty</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Valentine'>Valentine</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Turkey Day'>Turkey Day</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='USA'>USA</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Independence'>Independence</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Halloween'>Halloween</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Go Blue'>Go Blue</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Hail'>Hail</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Touchdown'>Touchdown</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Punkin'>Punkin</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Lovey Day'>Lovey Day</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Holly Jolly'>Holly Jolly Day</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Sinelon'>Sinelon</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Juggle'>Juggle</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Confetti'>Confetti</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Rainbow'>Rainbow</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Rainbow with Glitter'>Rainbow with Glitter</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Glitter'>Glitter</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='BPM'>BPM</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Solid'>Solid</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Twinkle'>Twinkle</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Dots'>Dots</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Lightning'>Lightning</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Police One'>Police One</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Police All'>Police All</button>"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Candy Cane'>Candy Cane</button>​"));
    httpMessage += String(F("<button type='submit' name='Seteffect' value='Cyclon Rainbow'>Cyclon Rainbow</button>"));
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

void webHandleFirmware()
{

 
  String httpMessage = FPSTR(HTTP_HEAD);
  httpMessage.replace("{v}", (String(espName) + " update"));
  httpMessage += FPSTR(HTTP_SCRIPT);
  httpMessage += FPSTR(HTTP_STYLE);
  httpMessage += String(LED_STYLE);
  httpMessage += FPSTR(HTTP_HEAD_END);
  httpMessage += String(F("<h1>")) + String(espName) + String(F(" firmware</h1>"));

  httpMessage += String(F("<form method='get' action='/espfirmware'>"));

 // httpMessage += String(F("<br/><b>Update ESP8266 from URL</b><small><i> http only</i></small>"));
 // httpMessage += String(F("<br/><input id='espFirmwareURL' name='espFirmware' value='")) + espFirmwareUrl + "'>";
 // httpMessage += String(F("<br/><br/><button type='submit'>Update ESP from URL</button></form>"));

  httpMessage += String(F("<br/><form method='POST' action='/update' enctype='multipart/form-data'>"));
  httpMessage += String(F("<b>Update ESP8266 from file</b><input type='file' id='espSelect' name='espSelect' accept='.bin'>"));
  httpMessage += String(F("<br/><br/><button type='submit' id='espUploadSubmit' onclick='ackEspUploadSubmit()'>Update ESP from file</button></form>"));

  //ttpMessage += String(F("<br/><br/><hr><h1>WARNING!</h1>"));
  //httpMessage += String(F("<b>Nextion LCD firmware updates can be risky.</b> If interrupted, the HASP will need to be manually power cycled which might mean a trip to the breaker box. "));
  //httpMessage += String(F("After a power cycle, the LCD will display an error message until a successful firmware update has completed.<br/>"));

 // httpMessage += String(F("<br/><hr><form method='get' action='lcddownload'>"));
 // if (updateLcdAvailable)
 // {
//    httpMessage += String(F("<font color='green'><b>HASP LCD update available!</b></font>"));
//  }
//  httpMessage += String(F("<br/><b>Update Nextion LCD from URL</b><small><i> http only</i></small>"));
//  httpMessage += String(F("<br/><input id='lcdFirmware' name='lcdFirmware' value='")) + lcdFirmwareUrl + "'>";
//  httpMessage += String(F("<br/><br/><button type='submit'>Update LCD from URL</button></form>"));

  //httpMessage += String(F("<br/><form method='POST' action='/lcdupload' enctype='multipart/form-data'>"));
 // httpMessage += String(F("<br/><b>Update Nextion LCD from file</b><input type='file' id='lcdSelect' name='files[]' accept='.tft'/>"));
 // httpMessage += String(F("<br/><br/><button type='submit' id='lcdUploadSubmit' onclick='ackLcdUploadSubmit()'>Update LCD from file</button></form>"));

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

void webHandleEspFirmware()
{ //espfirmware

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
  httpMessage += "<br/>Updating ESP firmware from: " + String(webServer.arg("espSelect"));
  httpMessage += FPSTR(HTTP_END);
  webServer.send(200, "text/html", httpMessage);
String serv = "http://" + WiFi.localIP().toString() +"/" + webServer.arg("espSelect") ;

  Serial.println("ESPFW: Attempting ESP firmware update from: " + String(serv));
  startEspOTA("/LED_ez_Pub.ino.d1_mini.bin");
 

}

void handleTelnetClient()
{ // Basic telnet client handling code from: https://gist.github.com/tablatronix/4793677ca748f5f584c95ec4a2b10303
  if (telnetServer.hasClient())
  {
    // client is connected
    if (!telnetClient || !telnetClient.connected())
    {
      if (telnetClient)
        telnetClient.stop();                   // client disconnected
      telnetClient = telnetServer.available(); // ready for new client
    }
    else
    {
      telnetServer.available().stop(); // have client, block new conections
    }
  }
  // Handle client input from telnet connection.
  if (telnetClient && telnetClient.connected() && telnetClient.available())
  {
    // client input processing
    while (telnetClient.available())
    {
      // Read data from telnet just to clear out the buffer
      telnetClient.read();
    }
  }
}
