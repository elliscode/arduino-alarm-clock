#include "WiFiS3.h"
#include "SimpleTime.h"
#include "Arduino_LED_Matrix.h"
#include "time_vars.h"
#include "songs.h"
ArduinoLEDMatrix matrix;
#include "arduino_secrets.h" 
int alarm_times[7][2] = {{7,2},{6,2},{6,2},{6,2},{6,2},{6,2},{7,2}};
uint32_t alarm_time = 0;
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char time_response[400];
char key_i_want[] = "\"unixtime\":";
uint32_t unix_time = 0;
uint32_t time_screen[3];

int status = WL_IDLE_STATUS;
char server[] = "worldtimeapi.org";
WiFiClient client;

/* -------------------------------------------------------------------------- */
void setup() {
/* -------------------------------------------------------------------------- */
  // Serial.begin(9600); // uncomment this if you want logging turned on
  matrix.begin();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  matrix.loadFrame(test_screen);
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
     
    // wait 10 seconds for connection:
    delay(10000);
  }
  
  printWifiStatus();
}

/* -------------------------------------------------------------------------- */
void read_response() {
/* -------------------------------------------------------------------------- */  
  uint32_t received_data_num = 0;
  bool activate = false;
  while (client.available()) {
    /* actual data reception */
    char c = client.read();
    if (c == '{') {
      activate = true;
    }
    if (activate) {
      time_response[received_data_num] = c;
      received_data_num++;
    }
  }  
}
int i;
bool get_server_time = true;
unsigned long previous_millis = 0;
time_t current_time = 0;
bool colon_on = true;
/* -------------------------------------------------------------------------- */
void loop() {
/* -------------------------------------------------------------------------- */  
  digitalWrite(LED_BUILTIN, LOW); 
  if (get_server_time) {
    Serial.println("\nStarting connection to server...");
    if (client.connect(server, 80)) {
      Serial.println("connected to server");
      // Make a HTTP request:
      client.println("GET /api/timezone/America/New_York.json HTTP/1.1");
      client.println("Host: worldtimeapi.org");
      client.println("Connection: close");
      client.println();
    }
    previous_millis = millis();

    delay(3000);

    read_response();

    boolean key_valid = false;
    uint32_t key_valid_index = 0;
    boolean get_numbers = false;
    unix_time = 0;
    for (i = 0; i < 1000; i++) {
      if (time_response[i] == NULL) {
        break;
      }
      if (time_response[i] == '}') {
        Serial.println();
      }
      if (get_numbers && time_response[i] >= 48 && time_response[i] <= 57) {
        unix_time = unix_time * 10;
        unix_time = unix_time + (time_response[i] - 48);
      } else {
        get_numbers = false;
      }
      key_valid = key_valid && key_i_want[key_valid_index] == time_response[i];
      if (key_valid && key_valid_index >= 10) {
        get_numbers = true;
      }
      key_valid_index++;
      Serial.print(time_response[i]);
      if (time_response[i] == '{' || time_response[i] == ',') {
        Serial.println();
        Serial.print("  ");
        key_valid = true;
        key_valid_index = 0;
      }
    }

    Serial.println();
    Serial.println();
    Serial.print("Time found: ");
    Serial.println(unix_time);

    Serial.println();
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    get_server_time = false;
  }

  current_time = unix_time + ((millis() - previous_millis) / 1000) - (60 * 60 * 5);

  if (alarm_time == 0) {
    determine_next_alarm();
  }

  while (alarm_time > 0 && alarm_time < current_time) {
    for (int thisNote = 0; animal_crossing_notes[thisNote]!=-1; thisNote++) {
      int noteDuration = animal_crossing_speed*animal_crossing_times[thisNote];
      tone(3, animal_crossing_notes[thisNote],noteDuration*.95);
      Serial.println(animal_crossing_notes[thisNote]);
      delay(noteDuration);
      noTone(3);
    }
  }

  Serial.println(current_time);
  calculate_time_screen();
  matrix.loadFrame(time_screen);
  delay(1000);
  Serial.println();

  if ((millis() - previous_millis) > (60 * 60 * 1000)) {
    get_server_time = true;
  }
}

void calculate_time_screen() {
  int wkd = weekday(current_time) % 7;
  int hr = hour(current_time);
  if (hr == 0) {
    hr = 12;
  } else if (hr > 12) {
    hr = hr - 12;
  }
  int mn = minute(current_time);
  int sc = second(current_time);
  Serial.print(wkd);
  Serial.print(" -- ");
  Serial.print(hr);
  Serial.print(':');
  Serial.print(mn);
  Serial.print(':');
  Serial.print(sc);
  for (int i = 0; i < 3; i++) {
    time_screen[i] = 0;
  }

  if (hr >= 10) {
    for (int i = 0; i < 3; i++) {
      time_screen[i] = time_screen[i] | hour_tens[i];
    }
  }
  for (int i = 0; i < 3; i++) {
    time_screen[i] = time_screen[i] | hour_ones[(hr % 10)][i];
  }

  if (colon_on) {
    for (int i = 0; i < 3; i++) {
      time_screen[i] = time_screen[i] | colon[i];
    }
  }

  colon_on = !colon_on;

  for (int i = 0; i < 3; i++) {
    time_screen[i] = time_screen[i] | minute_tens[(mn / 10)][i];
  }
  for (int i = 0; i < 3; i++) {
    time_screen[i] = time_screen[i] | minute_ones[(mn % 10)][i];
  }
}

void determine_next_alarm() {
  int today = weekday(current_time) % 7;
  int tomorrow = (today + 1) % 7;
  int hr = hour(current_time);
  int mn = minute(current_time);

  if (hr < alarm_times[today][0] || (hr == alarm_times[today][0] && mn < alarm_times[today][1])) {
    alarm_time = current_time - (current_time % 60) + ((alarm_times[today][1] - mn) * 60) + ((alarm_times[today][0] - hr) * 60 * 60);
  } else {
    alarm_time = current_time - (current_time % 60) + ((alarm_times[tomorrow][1] - mn) * 60) + ((alarm_times[tomorrow][0] - hr) * 60 * 60) + (24 * 60 * 60);
  }
  Serial.print("Alarm time: ");
  Serial.println(alarm_time);
}

/* -------------------------------------------------------------------------- */
void printWifiStatus() {
/* -------------------------------------------------------------------------- */  
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
