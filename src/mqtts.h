/*
MQTT with PubSubClient

https://github.com/knolleary/pubsubclient/pull/851/commits/c2098031d1e6d4144cc86e10cf715f1bc6085526
check: https://github.com/knolleary/pubsubclient/issues/462#issuecomment-542911896
*/

// TODO: check for MAX_MESSAGE_SIZE and handle long messages in sendMessage()

#ifndef MQTTS_KD_H
#define MQTTS_KD_H

#include <PubSubClient.h>

#include <sstream>

#include "globals_kd.h"
#include "led_blink.h"

#ifdef SECURE_MQTT
#include <WiFiClientSecure.h>
#else
#include <WiFiClient.h>
#endif  // SECURE_MQTT

#ifdef SECURE_MQTT
WiFiClientSecure client;
uint16_t MQTT_PORT = 8883;
#else
WiFiClient client;
uint16_t MQTT_PORT = 1883;
#endif  // SECURE_MQTT

// forward declaration
char *getCurSsid();                                               // from main
char *getFullDeviceName();                                        // from main
char *getDeviceId();                                              // from main
void onIncomingOtaMessage(byte *payload, unsigned int length);    // from main from ota
void onMessage(char *topic, byte *payload, unsigned int length);  // from below
bool sendMessage(const char *msg, bool admin);                    // from below

char *MQTT_CLIENT_ID;
const char *MQTT_HOST = MQTT_HOST_CN;

PubSubClient mqtt_client(client);

char *sensorsTopic = nullptr;
char *adminTopic = nullptr;

char *otaTopic = nullptr;

bool subscribeToTopics() {
#ifdef OTA_UPDATE
    return mqtt_client.subscribe(otaTopic);
#else
    return true;
#endif  // OTA_UPDATE
}

void setTopicStrings() {
    std::stringstream ss;
    ss << SENSOR_TOPIC_PRE << getCurSsid() << '/' << getDeviceId();
    sensorsTopic = new char[ss.str().size() + 1];
    strcpy(sensorsTopic, ss.str().c_str());

    std::stringstream().swap(ss);
    ss << ADMIN_TOPIC_PRE << getCurSsid() << '/' << getDeviceId();
    adminTopic = new char[ss.str().size() + 1];
    strcpy(adminTopic, ss.str().c_str());
    Serial.printf("- Set up topics to \"%s\" and \"%s\".\n", sensorsTopic, adminTopic);

    std::stringstream().swap(ss);
    ss << OTA_TOPIC_PRE << getDeviceId();
    otaTopic = new char[ss.str().size() + 1];
    strcpy(otaTopic, ss.str().c_str());

    Serial.printf("- Set up ota topic \"%s\".\n", otaTopic);
}

void connectMQTT() {
    while (!mqtt_client.connected()) {
        Serial.println("Connecting MQTT...");
#ifdef MQTT_USERNAME
        // topics are setup already here
        bool connected = mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWD, adminTopic, 0, false, MQTT_LAST_WILL_MSG);
#else
        bool connected = mqtt_client.connect(MQTT_CLIENT_ID);
#endif  // MQTT_USERNAME
        if (!connected) {
            Serial.print("- Failed with rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" retrying...");
            ledBlinkTimes(indicator::MQTT_ERROR);  // MQTT error
        }
    }
    Serial.println("- MQTT Connected!");
    sendMessage(MQTT_CONNECT_MSG, true);
    if (!subscribeToTopics()) {
        // TODO: Handle this case
        Serial.println("- ERR: Could not subscribe topics!");
    }
}

bool loopMQTT() {
    if (!mqtt_client.connected()) {
        connectMQTT();
    }
    return mqtt_client.loop();
}

void initMQTT() {
    Serial.println("Setup MQTT...");
#ifdef SECURE_MQTT
    // Set up the root ca certificate
    client.setCACert((char *)ROOT_CERT);
    Serial.println("- With CA cert");
#endif  // SECURE_MQTT
    Serial.printf("- Connect MQTT to \"%s\" on port \"%d\"\n", MQTT_HOST, MQTT_PORT);
    mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
    mqtt_client.setBufferSize(MAX_MQTT_MESSAGE_SIZE);
    mqtt_client.setCallback(onMessage);

    MQTT_CLIENT_ID = getFullDeviceName();
    Serial.printf("- Client id: \"%s\".\n", MQTT_CLIENT_ID);

    // TODO: check if keepalive needs to be greater than SCAN_TIME_IN_SECONDS

    // setup topic strings
    setTopicStrings();
}

bool sendMessage(const char *msg, bool admin = false) {
    ledOn();
    char *topic = admin ? adminTopic : sensorsTopic;

    // only send messages from info level upwards
    if (CORE_DEBUG_LEVEL > 2) Serial.println(msg);

    bool sent = mqtt_client.publish(topic, msg);
    if (!admin && !sent)
        sendMessage("Publish failed!", true);
    ledOff();
    return sent;
}

void onMessage(char *topic, byte *payload, unsigned int length) {
    Serial.printf("MQTT message received on topic \"%s\".\n", topic);
    if (length < 1) {
        Serial.println("- No payload. Ignore.");
        return;
    }

    if (strncmp(topic, otaTopic, strlen(otaTopic)) == 0) {
        onIncomingOtaMessage(payload, length);
    } else {
        Serial.println("- Unsopported topic. Ignore.");
    }
}

#endif  // MQTTS_KD_H
