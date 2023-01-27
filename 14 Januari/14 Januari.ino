#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
const char EOPmarker = '=';  //This is the end of packet marker
char serialbuf[8];           //This gives
#include <EEPROM.h>
#include <TM1637TinyDisplay6.h>
#include <TM1637TinyDisplay.h>
#include <string.h>        // we'll need this for subString
#define MAX_STRING_LEN 20  // like 3 lines above, change as needed.
#define DIO1 22            // pin 6
#define CLK1 19            // pin 5
#define DIO2 27            //display pin
#define CLK2 26            // diplay pin
#define RXp2 16
#define TXp2 17
#define debounce_delay 100
TM1637TinyDisplay display1(CLK1, DIO1);
TM1637TinyDisplay6 display2(CLK2, DIO2);
int irPin = 25;
int count = 0;
bool bPress = false;
const int IncbuttonPin = 14;
const int DecbuttonPin = 33;
const int ResetbuttonPin = 32;
boolean prev_state = true;
int current_number = 0;
int buttonPushCounter = 0;  // counter for the number of button presses

int IncbuttonState = 0;      // current state of the button
int lastIncbuttonState = 0;  // previous state of the button
int ResetbuttonState = 0;
int lastResetbuttonstate = 0;
int DecbuttonState = 0;  // current state of the button
int lastDecbuttonState = 0;
unsigned int a;
unsigned int b;
unsigned int c;
unsigned int d;
unsigned int e;
int weight = 0;
int counter = 0;

unsigned long lastDisplayUpdate, lastIRUpdate, lastDecreaseUpdate, lastIncreaseUpdate;

char *subStr(char *input_string, char *separator, int segment_number) {
  char *act, *sub, *ptr;
  static char copy[MAX_STRING_LEN];
  int i;
  strcpy(copy, input_string);
  for (i = 1, act = copy; i <= segment_number; i++, act = NULL) {
    sub = strtok_r(act, separator, &ptr);
    if (sub == NULL) break;
  }
  return sub;
}

const char *SSID = "Bisma 123";
const char *PWD = "KeripikKentang";

WebServer server(80);


StaticJsonDocument<250> jsonDocument;
char buffer[250];



void setup_routing() {
  server.on("/weight", getScale);
  server.on("/counter", getCounter);
  server.on("/data", getData);

  server.begin();
}

void create_json(char *tag, float value, char *unit) {
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit;
}

void read_sensor_data(void *parameter) {

  for (;;) {
    weight = (d * 1000) + (e * 1000) + (c * 100) + (b * 10) + a;
    // read serial data loop
    count;
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // delay untuk baca data
  }
}


void getScale() {
  Serial.println("Get weight");
  create_json("weight", weight, "gr");
  server.send(200, "application/json", buffer);
}

void getCounter() {
  Serial.println("Get counter");
  create_json("counter", count, "pcs");
  server.send(200, "application/json", buffer);
}



void getData() {
  Serial.println("Get All Data");
  jsonDocument.clear();
  add_json_object("weight",  weight , "gr");
  add_json_object("counter", count, "pcs");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}





void setup() {
  Serial.begin(115200);

  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  WiFi.softAP(SSID, PWD);
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());

  //delay(2000);

  Serial2.begin(1200, SERIAL_8N1, RXp2, TXp2);  // serial 1 digital pin (tx 18,rx 19)
  display1.setBrightness(BRIGHT_HIGH);
  display2.setBrightness(BRIGHT_HIGH);
  display1.clear();
  display2.clear();
  pinMode(irPin, INPUT);
  pinMode(IncbuttonPin, INPUT_PULLUP);
  pinMode(DecbuttonPin, INPUT_PULLUP);
  pinMode(ResetbuttonPin, INPUT_PULLUP);
  Serial.print("Connecting to Wi-Fi");

  //setup_task();
  setup_routing();
  lastDisplayUpdate = millis();
  lastIRUpdate = millis();
  lastIncreaseUpdate = millis();
  lastDecreaseUpdate = millis();
}

void loop() {
  server.handleClient();
  IRcounter();
  checkAllButton();
  readSerialC();

  if ((millis() - lastDisplayUpdate) >= 500) {
    display2.showNumber((float)weight / 1000.000, 3);
    display1.showNumberDec(count, true);
    lastDisplayUpdate = millis();
  }
}
void readSerialC() {               // 3 spaces 1 dots between values
  if (Serial2.available() > 0) {   //makes sure something is ready to be read
    static int bufpos = 0;         //starts the buffer back at the first position in the incoming serial.read
    char inchar = Serial2.read();  //assigns one byte (as serial.read()'s only input one byte at a time
    if (inchar != EOPmarker) {     //if the incoming character is not the byte that is the incoming package ender
      serialbuf[bufpos] = inchar;  //the buffer position in the array get assigned to the current read
      bufpos++;                    //once that has happend the buffer advances, doing this over and over again until the end of package marker is read.
    } else {                       //once the end of package marker has been read
      serialbuf[bufpos] = '\0';    //restart the buff// end of string
      bufpos = 0;                  //restart the position of the buff


      int p = atoi(subStr(serialbuf, "", 1));
      //millis();
      int x = atoi(subStr(serialbuf, "", 1));
      //millis();
      int y = atoi(subStr(serialbuf, "", 1));
      //millis();
      int z = atoi(subStr(serialbuf, ".", 2));
      a = (p % 1000 / 100);
      b = (x % 100 / 10);
      c = (y % 10);
      d = (z % 100);
      e = (z / 100);
      weight = (d * 1000) + (e * 1000) + (c * 100) + (b * 10) + a;  // this is to make the array from the character
      // Serial.print(a);
      //Serial.print(b);
      //Serial.println(c);
      //Serial.println(d);
      // Serial.println(e);
      //Serial.println(z);
      //Serial.println(serialbuf);
    }
  }
}

void IRcounter() {
  bool state = digitalRead(irPin);

  if (state != prev_state) {
    if (state) {
      if ((millis() - lastIRUpdate) >= 500) {
        count++;
        lastIRUpdate = millis();
      }
    }
    prev_state = state;
  }
}

void checkAllButton() {
  IncbuttonState = digitalRead(IncbuttonPin);
  if (IncbuttonState != lastIncbuttonState) {
    if (!IncbuttonState) {
      if ((millis() - lastIncreaseUpdate) >= 300) {
        count++;
        lastIncreaseUpdate = millis();
      }
    }
    lastIncbuttonState = IncbuttonState;
  }

  DecbuttonState = digitalRead(DecbuttonPin);
  if (DecbuttonState != lastDecbuttonState) {
    if (!DecbuttonState) {
      if ((millis() - lastDecreaseUpdate) >= 300) {
        count--;
        lastDecreaseUpdate = millis();
      }
    }
    lastDecbuttonState = DecbuttonState;
  }

  ResetbuttonState = digitalRead(ResetbuttonPin);
  //checkIncButtonPress();
  //checkDecButtonPress();
  ResetButtonPress();
  if (bPress) {
    bPress = false;
    //delay(50);
  }
}


void checkIncButtonPress() {
  // compare the IncbuttonState to its previous state
  if (IncbuttonState != lastIncbuttonState) {
    // if the state has changed, increment the counter
    if (IncbuttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      //  bPress = true;
      count++;
      if (buttonPushCounter > 9999) buttonPushCounter = 0;
      Serial.println("on");

    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    //delay(10);
  }
  // save the current state as the last state, for next time through the loop
  lastIncbuttonState = IncbuttonState;
}
void checkDecButtonPress() {
  // compare the IncbuttonState to its previous state
  if (DecbuttonState != lastDecbuttonState) {
    // if the state has changed, increment the counter
    if (DecbuttonState == LOW) {
      // if the current state is HIGH then the button went from off to on:
      bPress = true;
      count--;
      if (count < 0) count = 9999;
      Serial.println("on");

    } else {
      // if the current state is LOW then the button went from on to off:
      //  Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    //delay(10);
  }
  // save the current state as the last state, for next time through the loop
  lastDecbuttonState = DecbuttonState;
}
void ResetButtonPress() {
  if (ResetbuttonState != lastResetbuttonstate) {
    bPress = true;
    if (count >= 1) {
      count = 0;
    }
    //delay(10);
  }
  lastResetbuttonstate = ResetbuttonState;
}
