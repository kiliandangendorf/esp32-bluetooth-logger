#include <Preferences.h>
Preferences preferences;

#define MAX_DEVICE_ID_LEN 5

// for WRITE
#define KD_DEVICE_ID "30"

// for READ
char DEVICE_ID[MAX_DEVICE_ID_LEN+1];

void writePrefs() {
    preferences.begin("kd_device_id", false);
    // clear complete namespace
    preferences.clear();
    // set id
    preferences.putString("id", KD_DEVICE_ID);
    // done
    preferences.end();
    Serial.println("Devices ID stored.");
}
void readPrefs() {
    preferences.begin("kd_device_id", true);
    int len = preferences.getString("id", DEVICE_ID, MAX_DEVICE_ID_LEN);
    Serial.println(len);

    Serial.printf("Read %d characters from flash.\n", len);

    preferences.end();
    return;
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    writePrefs();
    readPrefs();
}

void loop() {
    Serial.printf("Retrieved device id: \"%s\" with length of: \"%d\".\n", DEVICE_ID, strlen(DEVICE_ID));
    delay(1000);
}