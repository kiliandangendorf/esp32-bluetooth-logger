/**
 *
 * https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/BLEScan.h
 * https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/src/BLEAdvertisedDevice.cpp#L492
 * */

#ifndef BLE_KD_H
#define BLE_KD_H

#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include <sstream>

// Watchdog
#include <esp_task_wdt.h>

#include "get_time.h"
#include "globals_kd.h"

// BLE
BLEScan *pBLEScan;

// forward declaration from main
bool transmitSensorsData(const char *msg);
// forward declaration see below
void addBleDeviceToStringStream(std::stringstream &ss, BLEAdvertisedDevice device);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // we need to do this in callback, since we only have here the correct timestamp
        std::stringstream ss;
        addBleDeviceToStringStream(ss, advertisedDevice);
        // Serial.print("Found device:");
        // Serial.println(ss.str());
        transmitSensorsData(ss.str().c_str());

        // feed/reset watchdog
        esp_task_wdt_reset();
    }
};
void deinitBLE() {
    BLEDevice::deinit();
}

void initBLE() {
    Serial.println("Setup BLE...");
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();  // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(SCAN_ACTIVE);  // active scan uses more power, but get results faster
    pBLEScan->setInterval(SCAN_INTERVAL_MS);
    pBLEScan->setWindow(SCAN_WINDOW_MS);  // less or equal setInterval value
    Serial.printf("- With interval=%d ms, window=%d ms, active-scan=%s.\n", SCAN_INTERVAL_MS, SCAN_WINDOW_MS, SCAN_ACTIVE?"true":"false");
}

void scanBleDevicesForXSeconds(int seconds) {
    Serial.printf("Start scan for %i seconds...\n", seconds);
    // second param to false for deleting scanresults afterwards
    BLEScanResults foundDevices = pBLEScan->start(seconds, false);

    int count = foundDevices.getCount();
    Serial.printf("- Scan done, found %d devices.\n", count);

    pBLEScan->stop();
    // delete results from BLEScan buffer to release memory
    pBLEScan->clearResults();
    delay(100);
}

std::string addKeyValuePair(std::string const &key, const std::string &value, bool first = false) {
    // JSON
    std::string s = (first ? "" : ", ") + std::string("\"") + key + std::string("\": \"") + value + std::string("\"");
    return s;
}
std::string addKeyValuePair(const char *key, const char *value, bool first = false) {
    return addKeyValuePair(std::string(key), std::string(value), first);
}
std::string addKeyValuePair(std::string const &key, int const &value, bool first = false) {
    std::stringstream temp;
    temp << value;
    return addKeyValuePair(key, temp.str(), first);
}

void addBleDeviceToStringStream(std::stringstream &ss, BLEAdvertisedDevice device) {
    ss << "{";

    ss << addKeyValuePair("address", device.getAddress().toString(), true);
    if (device.haveName()) {
        ss << addKeyValuePair("name", device.getName().c_str());
    }

    if (device.haveAppearance()) {
        char val[6];
        snprintf(val, sizeof(val), "%d", device.getAppearance());
        ss << addKeyValuePair("appearance", val);
    }
    if (device.haveManufacturerData()) {
        std::string md = device.getManufacturerData();
        uint8_t *mdp = (uint8_t *)device.getManufacturerData().data();
        char *pHex = BLEUtils::buildHexData(nullptr, mdp, md.length());
        ss << addKeyValuePair("manufData", pHex);
        free(pHex);
    }
    if (device.haveServiceUUID()) {
        ss << addKeyValuePair("serviceUUID", device.getServiceUUID().toString());
    }
    if (device.haveTXPower()) {
        char val[6];
        snprintf(val, sizeof(val), "%d", device.getTXPower());
        ss << addKeyValuePair("txPower", val);
    }

    // add rSSI, payloadLength, addressType
    if (device.haveRSSI()) {
        ss << addKeyValuePair("rssi", device.getRSSI());
    }
    ss << addKeyValuePair("payloadLength", device.getPayloadLength());
    ss << addKeyValuePair("addrType", device.getAddressType());

    // don't use scan and payload from now since these are pointer ;)

    // don't use serviceDataUUID

    // add timestamp and micros
    unsigned long seconds, microseconds;
    getTimeInSecAndUsec(seconds, microseconds);
    ss << addKeyValuePair("timestamp", seconds);
    ss << addKeyValuePair("micros", microseconds);

    ss << "}";
}

#endif  // BLE_KD_H