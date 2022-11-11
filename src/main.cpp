/**
 * Publishes BLE device info on topic:
 * "sensor/BLE/Scanner/"<connected_SSID>"/"<KD_DEVICE_ID>"
 *
 * ESP/system-info on:
 * "admin/BLE/Scanner/"<connected_SSID>"/"<KD_DEVICE_ID>"
 *
 * WifiMulti example from: https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiMulti/WiFiMulti.ino
 * Based on example "BLE_scan"
 *
 * KD 2022-06-07
 */

//----------------------------
// INCLUDES
//----------------------------
#include <Arduino.h>
#include <Preferences.h>

#include "ble.h"
#include "get_time.h"
#include "globals_kd.h"
#include "led_blink.h"
#include "mqtts.h"
#include "ota.h"

// WiFi
#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

// Watchdog
#include <esp_task_wdt.h>
// strings
#include <sstream>
//#include <string>

//----------------------------
// FORWARD DECLARATIONS
//----------------------------
bool sendMessage(const char *msg, bool admin);                  // mqtts
bool connectWiFi();                                             // main
bool initDeviceNameFromFlash();                                 // main
void initWiFi();                                                // main
bool transmitAdminInfo(const char *msg);                        // main
void onIncomingOtaMessage(byte *payload, unsigned int length);  // ota

//----------------------------
// GLOBALS
//----------------------------
// stores current SSID for topic names
static char curSsid[33];  // max 32 byte
char *getCurSsid() { return curSsid; }
// holding device id
Preferences preferences;
char KD_DEVICE_ID[MAX_DEVICE_ID_LEN+1];
char *FULL_DEVICE_NAME;
char *getDeviceId() { return KD_DEVICE_ID; }
char *getFullDeviceName() { return FULL_DEVICE_NAME; }

//----------------------------
// SETUP
//----------------------------
void setup() {
    Serial.begin(115200);
    Serial.println("\n################################\n\n");
    Serial.printf("Booting (v%s)...\n", FW_VERSION);
    initLed(true);

    initDeviceNameFromFlash();
    delay(100);

    // connect WiFi (first before BLE)
    initWiFi();
    if (!connectWiFi()) {
        ledBlinkTimes(indicator::WIFI_ERROR);  // WiFi error
        ESP.restart();
    }
    delay(100);

    // WiFi connection is needed
    initMQTT();
    if (!loopMQTT()) {
        ledBlinkTimes(indicator::MQTT_ERROR);  // MQTT error
    }
    delay(100);

    // Note to start WiFi first and afterwards BLE ("strange issue")
    initBLE();
    delay(100);

    // Sync time (blocks until sync)
    initTimeNTP();
    delay(100);

    // Inform about restart with unix epoch
    std::stringstream helloMsg;
    helloMsg << "Up now " << getTime();
    helloMsg << " (v" << FW_VERSION << ")";
    helloMsg << " after " << millis() / 1000 << " seconds of booting.";
    transmitAdminInfo(helloMsg.str().c_str());

    ledOff();

    // Watchdog
    Serial.printf("Setup Watchdog with %d seconds timout, panic=%s.\n", 5 * SCAN_TIME_IN_SECONDS, true ? "true" : "false");
    esp_task_wdt_init(10 * SCAN_TIME_IN_SECONDS, true);
    esp_task_wdt_add(NULL);  // No special task needed

    Serial.printf("Setup done after %d seconds.\n", millis() / 1000);
    Serial.println("\n################################\n\n");
}

//----------------------------
// LOOP
//----------------------------
void loop() {
    // WiFi at first
    if (!connectWiFi())
        return;

    // NORMAL LOOP
    if (!isUpdateAvailable()) {
        loopMQTT();

        // scan for x seconds
        scanBleDevicesForXSeconds(SCAN_TIME_IN_SECONDS);

        // feed/reset watchdog
        esp_task_wdt_reset();
    }
    // OTA LOOP
    else {
        // ota first run
        if (!isUpdating()) {
            Serial.println("\n################################\n\n");
            Serial.println("Prepare OTA Update...");
            // stop ble and free memory
            deinitBLE();

            // keep mqtt running to inform about error

            // update watchdog for 5 minutes, if ota fails
            esp_task_wdt_init(300, true);

            initOta();
        }
        // ota other runs
        else {
            // performs the update (delay is inside)
            if (!loopOta()) {
                transmitAdminInfo("Error updating ota. Will reboot now and continue old firmware...");
                delay(1000);
                // Reboot in old version
                ESP.restart();
            }
        }
    }
}

//----------------------------
// DEVICE
//----------------------------
bool initDeviceNameFromFlash() {
    // Load device id from flash (readonly)
    Serial.println("Retrieving device id...");
    preferences.begin("kd_device_id", true);
    size_t len = preferences.getString("id", KD_DEVICE_ID, MAX_DEVICE_ID_LEN);
    preferences.end();
    if (len > 0) {
        Serial.printf("- Retrieved device id \"%s\".\n", KD_DEVICE_ID);
    } else {
        // Retrieve ID from last digits of MAC address (eg. "30:AE:A4:07:0D:64")
        String mac = WiFi.macAddress();
        // get last 5 digits ("7:0D:64")
        String subMac = WiFi.macAddress().substring(10);
        // without colons ("7:0D64")
        subMac.remove(4, 1);
        // without colons ("70D64")
        subMac.remove(1, 1);

        // no id found
        Serial.printf("- No device id found in flash. Will fallback to substring of MAC address \"%s\". (full MAC: \"%s\")\n", subMac.c_str(), mac.c_str());
        // set DEVICE_ID
        strcpy(KD_DEVICE_ID, subMac.c_str());
    }
    std::stringstream ss;
    ss << DEVICE_NAME_PRE << KD_DEVICE_ID;
    FULL_DEVICE_NAME = new char[ss.str().size() + 1];
    strcpy(FULL_DEVICE_NAME, ss.str().c_str());
    Serial.printf("- Retrieved device name: \"%s\".\n", FULL_DEVICE_NAME);
    return len;
}

//----------------------------
// WIFI
//----------------------------
void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(FULL_DEVICE_NAME);
#ifdef SSID_AP_1
    wifiMulti.addAP(SSID_AP_1, PW_AP_1);
#endif
#ifdef SSID_AP_2
    wifiMulti.addAP(SSID_AP_2, PW_AP_2);
#endif
#ifdef SSID_AP_3
    wifiMulti.addAP(SSID_AP_3, PW_AP_3);
#endif
#ifdef SSID_AP_4
    wifiMulti.addAP(SSID_AP_4, PW_AP_4);
#endif
}

bool connectWiFi() {
    // if (wifiMulti.run()!=WL_CONNECTED) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting WiFi...");

        int retries = 0;
        int POSSIBLE_RETRIES = 10;
        while (wifiMulti.run() != WL_CONNECTED) {
            Serial.print(".");
            // delay(100); //dely is inside blink-logic
            ledBlinkTimes(indicator::WIFI_ERROR);  // WiFi error
            if (retries > POSSIBLE_RETRIES)
                return false;
            retries++;
        }
        Serial.println("!");
        Serial.println("- WiFi connected");
        Serial.print("- IP address: ");
        Serial.println(WiFi.localIP());

        // TODO: check if it's the same ssid
        // if yes: go on
        // if no: reset topics etc. (or easily restart esp)

        // store SSID in curSsid (as not evil String ;))
        strcpy(curSsid, WiFi.SSID().c_str());
        // sanitize slashes in SSID
        for (size_t i = 0; i < strlen(curSsid); i++) {
            if (curSsid[i] == '/')
                curSsid[i] = '-';
        }
        Serial.print("- Sanitized SSID: ");
        Serial.println(WiFi.SSID());
    }
    return true;
}

//----------------------------
// MESSAGES
//----------------------------
bool transmitSensorsData(const char *msg) {
    return sendMessage(msg, false);
}
enum adminInfo { INFO,
                 ERR };
bool transmitAdminInfo(const char *msg) {  //, adminInfo infoLevel) {
    return sendMessage(msg, true);
}