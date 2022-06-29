#include <LoRa.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define NSS 15
#define RST 16
#define DIO0 7

int counter = 0;

TinyGPSPlus gps;
SoftwareSerial ss(4, 5);

float lat, lon;
int year, month, date, hour, minute, second;
String timestamp_str, date_str, time_str, lat_str, lon_str;

void setup() {
    Serial.begin(9600);
    ss.begin(9600);
    Serial.println("LoRa Sender");
    LoRa.setPins(NSS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }
}

void loop() {
    while (ss.available() > 0) {
        if (gps.encode(ss.read())) {
            bool gps_is_valid = gps.location.isValid();
            if (gps_is_valid) {
                lat = gps.location.lat();
                lat_str = String(lat, 5);
                lon = gps.location.lng();
                lon_str = String(lon, 6);
            }
            date_str = "";
            if (gps.date.isValid()) {
                date = gps.date.day();
                month = gps.date.month();
                year = gps.date.year();
                date_str += String(year);
                date_str += "-";
                if (month < 10) date_str += "0";
                date_str += String(month);
                date_str += "-";
                if (date < 10) date_str += "0";
                date_str += String(date);
            }
            time_str = "";
            if (gps.time.isValid()) {
                hour = gps.time.hour();
                minute = gps.time.minute();
                second = gps.time.second();
                if (hour < 10) time_str += "0";
                time_str += String(hour);
                time_str += ":";
                if (minute < 10) time_str += "0";
                time_str += String(minute);
                time_str += ":";
                if (second < 10) time_str += "0";
                time_str += String(second);
            }
            timestamp_str = date_str + "T" + time_str + "Z";
            Serial.print("lat=");
            Serial.print(lat_str);
            Serial.print(" lon=");
            Serial.print(lon_str);
            Serial.print(" timestamp=");
            Serial.println(timestamp_str);
            LoRa.beginPacket();
            LoRa.print("lat=");
            LoRa.print(lat_str);
            LoRa.print(" lon=");
            LoRa.print(lon_str);
            LoRa.print(" timestamp=");
            LoRa.print(timestamp_str);
            LoRa.endPacket();
        }
    }
    delay(5000);
}
