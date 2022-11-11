#ifndef GLOBALS_KD_H
#define GLOBALS_KD_H

//----------------------------
// DEVICE
//----------------------------
#define DEVICE_NAME_PRE "esp32_ble_scan_"

//#define KD_DEVICE_ID "1000" // this needs to be a char* to concat it with other defines
// obsolete since now stored in flash. If no id was found in flash, a substring of the MAC address is chosen.

// max. 5 chars long (max topic length will depend on)
#define MAX_DEVICE_ID_LEN 5

//----------------------------
// BLE
//----------------------------
#define SCAN_TIME_IN_SECONDS 10
#define SCAN_ACTIVE 1  // true=1, false=0
#define SCAN_INTERVAL_MS 100
#define SCAN_WINDOW_MS 100 // less or equal SCAN_INTERVAL_MS value

//----------------------------
// WIFI
//----------------------------
// known WiFi networks
#define SSID_AP_1 "ssid_from_AP_1"
#define PW_AP_1 "your_password_for_AP_1"
//#define SSID_AP_2 "ssid_from_AP_2"
//#define PW_AP_2 "your_password_for_AP_2"
//#define SSID_AP_3 "ssid_from_AP_3"
//#define PW_AP_3 "your_password_for_AP_3"
//#define SSID_AP_4 "ssid_from_AP_4"
//#define PW_AP_4 "your_password_for_AP_4"

//----------------------------
// MQTT
//----------------------------
#define SECURE_MQTT  // Comment this line if you are not using MQTT over SSL
#define MQTT_HOST_CN "example.com"

// TODO: max packet size: https://github.com/knolleary/pubsubclient#limitations
#define MAX_MQTT_MESSAGE_SIZE 512

#define SENSOR_TOPIC_PRE "sensor/BLE/Scanner/"
#define ADMIN_TOPIC_PRE "admin/BLE/Scanner/"

#define OTA_TOPIC_PRE "ota/BLE/Scanner/"

#define MQTT_LAST_WILL_MSG "{\"status\": \"offline\"}"
#define MQTT_CONNECT_MSG "{\"status\": \"online\", \"firmware\": \"" FW_VERSION "\"}"

#define MQTT_USERNAME "sensor_ble"
#define MQTT_PASSWD "Password for sensor_ble"

//----------------------------
// OTA
//----------------------------
#define OTA_UPDATE  // Comment this line if you are not using OTA

//----------------------------
// TLS
//----------------------------
#if defined SECURE_MQTT || defined OTA_UPDATE
/*static */ const unsigned char ROOT_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootCe
YourRootCertificateGoesHereYourRootCertificateGoesHereYourRootC=
-----END CERTIFICATE-----
)EOF";
#endif  // SECURE_MQTT || defined OTA_UPDATE

#endif  // GLOBALS_KD_H