/**
 *
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/Update/examples/HTTPS_OTA_Update/HTTPS_OTA_Update.ino
 * */

#ifndef OTA_KD_H
#define OTA_KD_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HttpsOTAUpdate.h>

#include "globals_kd.h"

static HttpsOTAStatus_t otastatus;

static bool updateAvailable = false;
static bool updateInProgress = false;

static StaticJsonDocument<256> doc;

bool transmitAdminInfo(const char* msg);             // main
int versionCompare(const char* v1, const char* v2);  // ota

bool isUpdateAvailable() { return updateAvailable; }
bool isUpdating() { return updateInProgress; }

void onIncomingOtaMessage(byte* payload, unsigned int length) {
    // make sure there will be no race condition
    if (!updateAvailable) {
        deserializeJson(doc, (const byte*)payload, length);
        // print
        Serial.print("- Received data: ");
        serializeJson(doc, Serial);
        Serial.println();

        if (!doc.containsKey("url")) {
            Serial.println("- No key \"url\" contained, ignore.");
            return;
        }
        if (!doc.containsKey("version")) {
            Serial.println("- No key \"version\" contained, ignore.");
            return;
        }
        const char* url = doc["url"];
        const char* version = doc["version"];
        // -1:v1<v2, 0:v1=v2, 1:v1>v2
        int versionDiff = versionCompare(version, FW_VERSION);
        if (versionDiff == 0) {
            Serial.printf("- Same version remote (%s) and local (%s), ignore.\n", version, FW_VERSION);
            return;
        }
        if (versionDiff == -1) {
            Serial.printf("- Older version remote (%s) than local (%s), ignore.\n", version, FW_VERSION);
            return;
        }
        Serial.printf("- Newer version remote (%s) than local (%s), will prepare update.\n", version, FW_VERSION);

        std::stringstream ss;
        ss << "OTA update info received for"
           << "\n - "
           << "version \"" << version << "\"";
        ss << "\n - "
           << "url \"" << url << "\"";
        ss << "\n Will start update now.";
        transmitAdminInfo(ss.str().c_str());

        updateAvailable = true;
    }
}

void HttpEvent(HttpEvent_t* event) {
    switch (event->event_id) {
        case HTTP_EVENT_ERROR:
            Serial.println("- Http Event Error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            Serial.println("- Http Event On Connected");
            break;
        case HTTP_EVENT_HEADER_SENT:
            Serial.println("- Http Event Header Sent");
            break;
        case HTTP_EVENT_ON_HEADER:
            Serial.printf("- Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            break;
        case HTTP_EVENT_ON_FINISH:
            Serial.println("- Http Event On Finish");
            break;
        case HTTP_EVENT_DISCONNECTED:
            Serial.println("- Http Event Disconnected");
            break;
    }
}

void initOta() {
    // make sure to call this only once
    if (!updateInProgress) {
        HttpsOTA.onHttpEvent(HttpEvent);
        const char* url = doc["url"];
        Serial.printf("Starting OTA from \"%s\"...\n", url);
        HttpsOTA.begin(url, (char*)ROOT_CERT);
        Serial.println("- Please wait, OTA takes some time...");
        updateInProgress = true;
    }
}

bool loopOta() {
    otastatus = HttpsOTA.status();
    if (otastatus == HTTPS_OTA_SUCCESS) {
        Serial.println("- Firmware written successfully. Restart...");
        ESP.restart();
    } else if (otastatus == HTTPS_OTA_FAIL) {
        Serial.println("- Upgrade failed!");
        delay(1000);
        return false;
    }
    delay(1000);
    return true;
}

/**
 * Method to compare two version strings
 * adapted from: https://stackoverflow.com/a/54067471/11438489
 * v1 <  v2  -> -1
 * v1 == v2  ->  0
 * v1 >  v2  -> +1
 */
int versionCompare(const char* v1, const char* v2) {
    size_t i = 0, j = 0;
    while (i < strlen(v1) || j < strlen(v2)) {
        int acc1 = 0, acc2 = 0;
        // parse until '.'
        while (i < strlen(v1) && v1[i] != '.') {
            // multiply by 10 for each next position
            // substract '0' to get diget value from ascii char
            acc1 = acc1 * 10 + (v1[i] - '0');
            i++;
        }
        while (j < strlen(v2) && v2[j] != '.') {
            acc2 = acc2 * 10 + (v2[j] - '0');
            j++;
        }

        if (acc1 < acc2) return -1;
        if (acc1 > acc2) return +1;
        // if equal go on with next digit
        i++;
        j++;
    }
    // case equal
    return 0;
}

#endif  // OTA_KD_H