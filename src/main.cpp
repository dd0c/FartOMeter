// FartOMeter
// Libraries needed:
// EasyNTPClient, Timezone, Time, TroykaMQ

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <EasyNTPClient.h>
#include <Timezone.h>
#include <TroykaMQ.h>

// SSID of your network
char ssid[] = ""; //SSID of your Wi-Fi router
char pass[] = ""; //Password of your Wi-Fi router

// NTP client and Timezone library
WiFiUDP udp;
EasyNTPClient ntpClient(udp, "192.168.1.105");
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);
unsigned long utc, localTime;

// MQ-9 Gas detector
#define PIN_MQ9         A0
#define PIN_MQ9_HEATER  D0
MQ9 mq9(PIN_MQ9, PIN_MQ9_HEATER);

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(10);
    Serial.println("\r\n");

    // Connect to Wi-Fi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to...");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Wi-Fi connected successfully");

    // Connect to MQ-9
    mq9.cycleHeat();
}

void loop() {
    // put your main code here, to run repeatedly:
    utc = ntpClient.getUnixTime();
    localTime = CE.toLocal(utc);
    Serial.print("Epoch CET time: ");
    Serial.println(localTime);

    // MQ-9
    if (!mq9.isCalibrated() && mq9.atHeatCycleEnd()) {
      mq9.calibrate();
      Serial.print("Ro = ");
      Serial.println(mq9.getRo());
      mq9.cycleHeat();
    }
    if (mq9.isCalibrated() && mq9.atHeatCycleEnd()) {
      Serial.print("Ratio: ");
      Serial.print(mq9.readRatio());
      Serial.print(" LPG: ");
      Serial.print(mq9.readLPG());
      Serial.print(" ppm ");
      Serial.print(" Methane: ");
      Serial.print(mq9.readMethane());
      Serial.print(" ppm ");
      Serial.print(" CarbonMonoxide: ");
      Serial.print(mq9.readCarbonMonoxide());
      Serial.println(" ppm ");
      delay(100);
      mq9.cycleHeat();
    }

    delay(5000); // wait for 5 seconds before refreshing.
}
