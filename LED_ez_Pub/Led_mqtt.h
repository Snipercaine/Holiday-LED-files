void mqttCallback(String &topic, String &payload)

 {
  int i = 0;

  if (String(topic) == setpowerSubTopic) {
    setPower = String(message_buff);
    debuglineprint("Set Power: " + setPower);
    if (setPower == "OFF") {
      MQTTclient.publish(setpowerPubTopic, "OFF");
    }

    if (setPower == "ON") {
      MQTTclient.publish(setpowerPubTopic, "ON");
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
    MQTTclient.publish(setpowerPubTopic, "ON");
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
    MQTTclient.publish(setpowerPubTopic, "ON");
  }

  if (String(topic) == setcolorSubTopic) {
    //for (i = 0; i < length; i++) {
    //  message_buff[i] = payload[i];
    // }
    //   message_buff[i] = '\0';
    MQTTclient.publish(setcolorPubTopic, message_buff);
    setColor = String(payload);
    debuglineprint("Set Color: " + setColor);
    setPower = "ON";
    MQTTclient.publish(setpowerPubTopic, "ON");
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
  static bool mqttFirstConnect = true; // For the first connection, we want to send an OFF/ON state to
                                       // trigger any automations, but skip that if we reconnect while
                                       // still running the sketch
  // Check to see if we have a broker configured 
  if (mqtt_server[0] == 0)
  {
    while (mqtt_server[0] == 0)
    { // Handle HTTP and OTA while we're waiting for MQTT to be configured
      yield();
      webServer.handleClient();
      ArduinoOTA.handle();
    }
  }

  // Loop until we're reconnected to MQTT
  while (!MQTTclient.connected())
  {
    // Create a reconnect counter
    static uint8_t mqttReconnectCount = 0;
    // Generate an MQTT client ID as haspNode + our MAC address
    // Set keepAlive, cleanSession, timeout
    MQTTclient.setOptions(30, true, 5000);
    // declare LWT
    //ient.publish(lwtTopic,"Online", true);
    MQTTclient.setWill(lwtTopic, "OFF");
    if (MQTTclient.connect(mcuHostName, mqtt_user, mqtt_password))
    { // Attempt to connect to broker, setting last will and testament
      // Subscribe to our incoming topics
      Mqttconnected = 1 ;
      MQTTclient.publish(lwtTopic,"Online", true);
      debuglineprint("MQTT: Connected");
      MQTTclient.subscribe(setcolorSubTopic);
      MQTTclient.subscribe(setbrightnessTopic);
      MQTTclient.subscribe(setpowerSubTopic);
      MQTTclient.subscribe(seteffectSubTopic);
      MQTTclient.subscribe(setanimationspeedTopic);
      MQTTclient.publish(setpowerPubTopic, "OFF");

      if (mqttFirstConnect)
      { // Force any subscribed clients to toggle OFF/ON when we first connect to
        // "ON" will be sent by the mqttStatusTopic subscription action.
        debuglineprint(String(F("MQTT: binary_sensor state: [")) + lwtTopic + "] : [OFF]");
        MQTTclient.publish(lwtTopic, "OFF", true, 1);
        mqttFirstConnect = false;
      }
      else
    {
        debuglineprint(String(F("MQTT: binary_sensor state: [")) + lwtTopic + "] : [ON]");
      }
      mqttReconnectCount = 0;
      debuglineprint(F("MQTT: connected"));
    }

    else

    { // Retry until we give up and restart after connectTimeout seconds
      mqttReconnectCount++;
      if (mqttReconnectCount > ((connectTimeout / 10) - 1))
      {
        debuglineprint(String(F("MQTT connection attempt ")) + String(mqttReconnectCount) + String(F(" failed with rc ")) + String(MQTTclient.returnCode()) + String(F(".  Restarting device.")));
        espReset();
      }
      debuglineprint(String(F("MQTT connection attempt ")) + String(mqttReconnectCount) + String(F(" failed with rc ")) + String(MQTTclient.returnCode()) + String(F(".  Trying again in 10 seconds.")));
      unsigned long mqttReconnectTimer = millis(); // record current time for our timeout
      while ((millis() - mqttReconnectTimer) < 10000)
      { // Handle HTTP and OTA while we're waiting 10sec for MQTT to reconnect
        yield();
        webServer.handleClient();
        ArduinoOTA.handle();
      }
    }
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
   mqttConnect();
 }
}
