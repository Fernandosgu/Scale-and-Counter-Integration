#include <Arduino.h>
#include <WiFi.h> // this is the wifi setup library
#include <WebServer.h> // this is to startthe web server
#include <ArduinoJson.h>
const char EOPmarker = '=';  //This is the end of packet marker
char serialbuf[8];           //This gives
#include <EEPROM.h>
#include <TM1637TinyDisplay6.h> // library for the 6 digit scale diplay
#include <TM1637TinyDisplay.h> // library for the 4 digit counter display
#include <string.h>        // we'll need this for subString
#define MAX_STRING_LEN 20  // like 3 lines above, change as needed.
#define DIO1 22            // display 4 digit pin 22
#define CLK1 19            // display 4 digit pin 19
#define DIO2 27            //display 6 digit pin 27
#define CLK2 26            // diplay 6digit pin 26
#define RXp2 16             // Reciever pin from rs232 to ttl
#define TXp2 17             // transmiter pin from rs232 to ttl
#define debounce_delay 100  // sensor delay value
TM1637TinyDisplay display1(CLK1, DIO1); // display pin declearation 4 digit
TM1637TinyDisplay6 display2(CLK2, DIO2); // display pin decleatation 6 digit
int irPin = 25; // sensor pin 
int count = 0; // sensor starting count
bool bPress = false; // button pressed status
const int IncbuttonPin = 14; // increase push button pin
const int DecbuttonPin = 33; // decrease pusuh button pin
const int ResetbuttonPin = 32;// reset button pin
boolean prev_state = true; // push buttons previous state
int current_number = 0; // count  starting number
int buttonPushCounter = 0;  // counter for the number of button presses

int IncbuttonState = 0;      // current state of the button
int lastIncbuttonState = 0;  // previous state of the button
int ResetbuttonState = 0;  // current state of the button
int lastResetbuttonstate = 0;// previous state of the button
int DecbuttonState = 0;  // current state of the button
int lastDecbuttonState = 0;// previous state of the button
unsigned int a; // Scale unsigned int for gramm digit 
unsigned int b; // scale unsigned int for 10 gram digit
unsigned int c; //scale unsigned int for 100 gramm digit
unsigned int d; // scale unsigned int for 1kg digit
unsigned int e; // scale unsigned int for 10kg digit
int weight = 0; // inital display for weight
int counter = 0; // initial display for scale

unsigned long lastDisplayUpdate, lastIRUpdate, lastDecreaseUpdate, lastIncreaseUpdate; //  displayscreen update 

// sub string initiallizer for the scale reading 
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

const char *SSID = "Bisma 123"; // wifi name
const char *PWD = "KeripikKentang";// wifi password
 
WebServer server(80); // web server slot 


StaticJsonDocument<250> jsonDocument; // json character buffer
char buffer[250];// number of json character buffer


// setup for HTTP routing
void setup_routing() {
  server.on("/weight", getScale);
  server.on("/counter", getCounter);
  server.on("/data", getData);

  server.begin();
}
// setup for JSON within the server
void create_json(char *tag, float value, char *unit) {
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}
//setup to add json object within the server
void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit;
}
// pulling data to be sent to the Server
void read_sensor_data(void *parameter) {

  for (;;) {
    weight = (d * 1000) + (e * 1000) + (c * 100) + (b * 10) + a;
    // read serial data loop
    count;
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // delay untuk baca data
  }
}

// server scale reading
void getScale() {
  Serial.println("Get weight");
  create_json("weight", weight, "gr");
  server.send(200, "application/json", buffer);
}
// server counter reading
void getCounter() {
  Serial.println("Get counter");
  create_json("counter", count, "pcs");
  server.send(200, "application/json", buffer);
}


// server or all data reading
void getData() {
  Serial.println("Get All Data");
  jsonDocument.clear();
  add_json_object("weight",  weight , "gr");
  add_json_object("counter", count, "pcs");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}





void setup() {
  Serial.begin(115200); // starting serial monitor for trouble shooting
// connecting to wifi
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  WiFi.softAP(SSID, PWD);
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());

  //delay(2000);

  Serial2.begin(1200, SERIAL_8N1, RXp2, TXp2);  //  starting the serial port for rs232
  display1.setBrightness(BRIGHT_HIGH); // initial display starting (brightness)
  display2.setBrightness(BRIGHT_HIGH);// initial display starting (brightness)
  display1.clear();// initial display
  display2.clear();// initial display
  pinMode(irPin, INPUT); // ir sensor pin mode
  pinMode(IncbuttonPin, INPUT_PULLUP);// increase button pin mode
  pinMode(DecbuttonPin, INPUT_PULLUP);// decrease button pin mode
  pinMode(ResetbuttonPin, INPUT_PULLUP);// reset button pin mode
  Serial.print("Connecting to Wi-Fi"); 

  //setup_task();
  setup_routing();// starting the server
  lastDisplayUpdate = millis(); // display update delay decleration
  lastIRUpdate = millis();// infrared sensor debounce delay initallizer
  lastIncreaseUpdate = millis();// button delay
  lastDecreaseUpdate = millis();// button delay
}

void loop() {
  server.handleClient(); // server handle
  IRcounter(); // callin counter program
  checkAllButton(); // calling button program
  readSerialC(); // calling scale reading program

// display update and delay
  if ((millis() - lastDisplayUpdate) >= 500) {
    display2.showNumber((float)weight / 1000.000, 3);
    display1.showNumberDec(count, true);
    lastDisplayUpdate = millis();
  }
}
// this void is the program to read the scale data send via rs232
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


      int p = atoi(subStr(serialbuf, "", 1)); // extract 1gram
      //millis();
      int x = atoi(subStr(serialbuf, "", 1)); //extract 10 gram
      //millis();
      int y = atoi(subStr(serialbuf, "", 1)); // extract 100 gramm
      //millis();
      int z = atoi(subStr(serialbuf, ".", 2)); // extract 1kg & // extratct 10 kg
      a = (p % 1000 / 100); // The modulus is use to do the error or known as check digit algorithm 
      b = (x % 100 / 10); // A check digit algorithm calculates a check digit 
      c = (y % 10);      // based on an original character string
      d = (z % 100);  // If the recalculated character string contains the correct check digit, 
      e = (z / 100); // the data is error-free and may be used 
      //if the character string that does not include the correct check digit indicates a transfer error,
      //which signals that data must be re-entered and/or reverified
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
// 
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
