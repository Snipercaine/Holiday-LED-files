void mqttCallback(String &topic, String &payload)

 {
  int i = 0;

  if (String(topic) == setpowerSubTopic) {
    setPower = String(message_buff);
    debuglineprint("Set Power: " + setPower);
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
    debuglineprint("Set Effect: " + setEffect);
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
    debuglineprint("Set Brightness: " + setBrightness);
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
    debuglineprint("Set Color: " + setColor);
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
  debuglineprint( "MQTTServer");
  
  debuglineprint( String(mqtt_server));
    // Loop until we're reconnected to MQTT
    if (mqtt_server[0] != 0){
  while (!client.connected())
  {

       static uint8_t mqttReconnectCount = 0;
    mqttClientId = String(espName);
    // Set keepAlive, cleanSession, timeout
  if(String(mqtt_server)!=""){
  debuglineprint("MQTT: Attempting connection to " + String(mqtt_server) + " as " + mcuHostName);
    if (client.connect(espName, mqtt_user, mqtt_password))
    { // Attempt to connect to broker, setting last will and testament
      // Subscribe to our incoming topics
      Mqttconnected = 1 ;
      client.publish(lwtTopic,"Online", true);
      debuglineprint("MQTT: Connected");

      FastLED.clear (); //Turns off startup LEDs after connection is made
      FastLED.show();

      client.subscribe(setcolorSubTopic);
      client.subscribe(setbrightnessTopic);
      //client.subscribe(setcolortemp);
      client.subscribe(setpowerSubTopic);
      client.subscribe(seteffectSubTopic);
      client.subscribe(setanimationspeedTopic);
      client.publish(setpowerPubTopic, "OFF");
      mqttReconnectCount = mqttReconnectCount + 1;
      unsigned long mqttReconnectTimeout = 10000;  // timeout for MQTT reconnect
      unsigned long mqttReconnectTimer = millis(); // record current time for our timeout
      while ((millis() - mqttReconnectTimer) < mqttReconnectTimeout)
      { // Handle HTTP and OTA while we're waiting for MQTT to reconnect
        yield();
        webServer.handleClient();
        ArduinoOTA.handle();
      }
      if(mqttReconnectCount >5 ){
    WiFi.disconnect();  
        }
    }}}
  }else{
      FastLED.clear (); //Turns off startup LEDs after connection is made
      FastLED.show();

    }
}

void SetTopics(){
  
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

  }
void ConnectMQtt(){
  if (mqtt_server[0] !=0){
  client.begin(mqtt_server, atoi(mqtt_port), wifiClient);
  client.onMessage(mqttCallback);
   mqttConnect();
 }
}
