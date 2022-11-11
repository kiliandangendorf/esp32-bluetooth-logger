#ifndef GET_TIME_KD_H
#define GET_TIME_KD_H

#include <Arduino.h>
#include <sys/time.h>

#define NTP_SERVER "pool.ntp.org"

// FORWARD DECLARATIONS
bool getTimeInSecAndUsec(unsigned long& seconds, unsigned long& microseconds);

void initTimeNTP() {
    Serial.print("Wait for timesync...");
    // The first and second arguments correspond to the GMT time offset and daylight saving time
    // choose 0,0 for epoch
    configTime(0, 0, NTP_SERVER);
    // after this call, time is syncing periodically each hour 
    // (see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html#sntp-time-synchronization)

    while (time(nullptr) < 10000) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("!");

    unsigned long seconds, microseconds;
    getTimeInSecAndUsec(seconds, microseconds);

    Serial.print("- Time synced in seconds: ");
    Serial.print(seconds);
    Serial.print(", plus micros: ");
    Serial.println(microseconds);
}

bool getTimeInSecAndUsec(unsigned long& seconds, unsigned long& microseconds) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        Serial.println("Failed to obtain time");
        return false;
    }
    seconds = tv.tv_sec;
    microseconds = tv.tv_usec;
    return true;
}

// Function that gets current epoch time
unsigned long getTime() {
    unsigned long seconds, microseconds;
    if (!getTimeInSecAndUsec(seconds, microseconds)) {
        return 0;
    }
    return seconds;
}

/* inspired by
- https://fipsok.de/Projekt/esp8266-ntp-zeit
- Rui Santos: Complete project details at https://RandomNerdTutorials.com/epoch-unix-time-esp32-arduino/
*/

#endif  // GET_TIME_KD_H