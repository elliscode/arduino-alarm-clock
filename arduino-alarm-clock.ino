#include "Arduino_LED_Matrix.h"
#include "arduino_secrets.h"
#include "songs.h"
#include "time_vars.h"
#include "RTC.h"
#include <NTPClient.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h"
#include <algorithm>

ArduinoLEDMatrix matrix;
WiFiClient client;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

const bool DEBUG = false;
const int timeZoneOffsetHours = -4; // todo get this from a API call
const unsigned long NTP_REFRESH_RATE = 29 * 60 * 1000;
RTCTime currentTime;
RTCTime alarmTime;
uint32_t timeScreen[3];
bool colon_on = true;
const int ALARM_TIMEOUT = 20 * 60 * 1000; // ms
const int alarmTimes[7][2] = {{7,0},{5,45},{5,45},{5,45},{5,45},{5,45},{7,0}}; // {{hh,mm}}
int random_index;
unsigned long millisThisLoop = 0;
unsigned long hourTicker = 0;

int wifiStatus = WL_IDLE_STATUS;
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectToWiFi(){
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (wifiStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    wifiStatus = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to WiFi");
  printWifiStatus();
}

void setup(){
  if (DEBUG) {
    Serial.begin(9600);
    while (!Serial);
  }
  Serial.println("Communication with serial port succeeded!");
  matrix.begin();
  matrix.loadFrame(test_screen);

  connectToWiFi();
  RTC.begin();
  Serial.println("\nStarting connection to server...");

  timeClient.begin();
  while (!timeClient.isTimeSet() || timeClient.getEpochTime() == 0) {
    updateRtcModuleFromNtpServer();
  }
  timeClient.setUpdateInterval(NTP_REFRESH_RATE);

  unsigned long unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
  Serial.print("Unix time = ");
  Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);

  // Retrieve the date and time from the RTC and print them
  RTC.getTime(currentTime); 
  Serial.println("The RTC was just set to: " + String(currentTime));

  calculateNextAlarmTime();
}

void updateRtcModuleFromNtpServer() {
    Serial.println("timeClient not set yet, waiting 5 second before attempting...");
    delay(5000);
    timeClient.forceUpdate();
    if (timeClient.isTimeSet() && timeClient.getEpochTime() != 0) {
      hourTicker = hourTicker + 1;
    }
}

void calculateNextAlarmTime() {
  alarmTime = RTCTime(currentTime);
  alarmTime.setHour(0);
  alarmTime.setMinute(0);
  alarmTime.setSecond(0);
  int dayOfWeekIndex = (int) currentTime.getDayOfWeek();
  int nextAlarm[] = {alarmTimes[dayOfWeekIndex][0], alarmTimes[dayOfWeekIndex][1]};
  bool todaysAlarmAlreadyPassed = currentTime.getHour() > nextAlarm[0] || (currentTime.getHour() == nextAlarm[0] && currentTime.getMinutes() >= nextAlarm[1]);
  if (todaysAlarmAlreadyPassed) {
    alarmTime.setUnixTime(alarmTime.getUnixTime() + 86400);
    dayOfWeekIndex = (dayOfWeekIndex + 1) % 7;
    nextAlarm[0] = alarmTimes[dayOfWeekIndex][0];
    nextAlarm[1] = alarmTimes[dayOfWeekIndex][1];
  }
  alarmTime.setHour(nextAlarm[0]);
  alarmTime.setMinute(nextAlarm[1]);
  Serial.println("The alarm was just set to: " + String(alarmTime));
}

void loop(){
  millisThisLoop = millis();
  if (alarmTime.getUnixTime() <= currentTime.getUnixTime()) {
    playAlarm();
    calculateNextAlarmTime();
  }
  calculateTimeScreen();
  matrix.loadFrame(timeScreen);
  if (timeClient.update()) { // doesn't actually update, but will update when updating is available
    Serial.println("Updated the NTP time");
    unsigned long unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
    Serial.print("Unix time = ");
    Serial.println(unixTime);
    RTCTime timeToSet = RTCTime(unixTime);
    RTC.setTime(timeToSet);
  } 
  delay(1000);
}

void calculateTimeScreen() {
  RTC.getTime(currentTime);
  int hr = currentTime.getHour();
  if (hr > 12) {
    hr = hr - 12;
  } else if (hr == 0) {
    hr = 12;
  }
  int mn = currentTime.getMinutes();
  Serial.print((int) currentTime.getDayOfWeek());
  Serial.print(" -- ");
  Serial.print(hr);
  Serial.print(':');
  Serial.print(mn);
  Serial.print(':');
  Serial.print(currentTime.getSeconds());
  Serial.println();
  for (int i = 0; i < 3; i++) {
    timeScreen[i] = 0;
  }
  if (hr >= 10) {
    for (int i = 0; i < 3; i++) {
      timeScreen[i] = timeScreen[i] | hour_tens[i];
    }
  }
  for (int i = 0; i < 3; i++) {
    timeScreen[i] = timeScreen[i] | hour_ones[(hr % 10)][i];
  }
  if (colon_on) {
    for (int i = 0; i < 3; i++) {
      timeScreen[i] = timeScreen[i] | colon[i];
    }
  }
  colon_on = !colon_on;
  for (int i = 0; i < 3; i++) {
    timeScreen[i] = timeScreen[i] | minute_tens[(mn / 10)][i];
  }
  for (int i = 0; i < 3; i++) {
    timeScreen[i] = timeScreen[i] | minute_ones[(mn % 10)][i];
  }
}

void playAlarm() {
  int happyFaceIndex = 0;
  unsigned long alarmMiillis = millisThisLoop;
  unsigned long previousTimeTicked = millisThisLoop;
  while (alarmMiillis > 0 && alarmMiillis <= millisThisLoop && (alarmMiillis + ALARM_TIMEOUT) >= millisThisLoop) {
    millisThisLoop = millis();
    for (int thisNote = 0; animal_crossing_notes[thisNote]!=END; thisNote++) {
      int noteToPlay = animal_crossing_notes[thisNote];
      int noteDuration = animal_crossing_speed*animal_crossing_times[thisNote];
      tone(3, noteToPlay,noteDuration*.95);
      delay(noteDuration);
      noTone(3);
      if (millisThisLoop > previousTimeTicked + 1000) {
        matrix.loadFrame(happy_face[happyFaceIndex]);
        happyFaceIndex = (happyFaceIndex + 1) % 4;
        previousTimeTicked = millisThisLoop;
      }
      millisThisLoop = millis();
    }
  }
}