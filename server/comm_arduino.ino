/*
 *  NOTE: modified by David P Janes / IOTDB for iotdb-serial
 *
Arduino Serial Protocol 1.6
created November 18th 2012 by Roberto Valgolio
last modified March 2014
this example code is in the public domain
any feedback will be appreciated, please write to roberto.valgolio@gmail.com

The  program builds an ASCII protocol useful to activate the Arduino basic functions from a remote system or from a terminal like Putty or HyperTerminal.
Arduino executes the requested function and returns the result that can be displayed or treated by the sender.
Input and output messages are NL terminated.

What'new:
1.3 - April 2013
•  Arduino answer message ends with NL instead CR+NL
1.4 - October 2013
•  code optimization useful for integration with other projects
•  tested serial speed up to 115200
1.5 - December 2013
•  data sending in binary format, see Oscilloscope functions
•  for other info search comments "oscilloscope addon"
1.6 - March 2014
•  csv data sending
•  for other info search comments "csv addon"

The protocol built in functions are:
•  pinMode
•  digitalRead
•  digitalWrite
•  analogRead
•  analogWrite
•  pulseIn
•  tone
•  noTone
•  millis
•  delay
•  myFunction
•  oscilloscopeOn
•  oscilloscopeGo
•  oscilloscopeOff
•  csvOn
•  csvOff
Developers can add new functions to interact with their own sketch.

The solution can be a ready to go software for many Arduino applications or a module to improve every sketch.
 */

// Serial Protocol
String  inString = "";  // input string

// oscilloscope addon
bool           oscSwitch = false;
int            oscPin = 0;
unsigned long  oscResolution = 1000;
int            oscTrigger = 0;
bool           oscGo = false;
bool           oscRepeat = false;

// csv addon
bool           csvSwitch = false;
int            csvIndex = 0;
unsigned long  csvFrequency = 1;

const int baud_rate = 9600;  // also good: 57600; 115200

const int max_pins = 48;
char pin_setup[max_pins];

void _clear_pin_setup() {
  for (int pi = 0; pi < max_pins; pi++) {
    pin_setup[pi] = 0;
  }
}

void _serial_setup() {
  Serial.begin(baud_rate);
  
  char c;
  while (Serial.available()) {
    c = (char)Serial.read();
  }
}

void _serial_hello() {
  Serial.print("\n\n\n\n!hello,Arduino\n");
}

void _set_pin_mode(int pin, int mode) {
  if (pin_setup[pin]) {
    return;
  }
  
  pin_setup[pin] = 1;
  pinMode(pin, mode);

}

int listen_max = 0;
int listen_digital[max_pins];  // notification <seq>
int listen_value[max_pins];    // current value 
int listen_debounce[max_pins]; // value for debounce
long listen_debounce_millis;
long listen_debounce_delta = 50;

void _listen_setup() {
  for (int li = 0; li < max_pins; li++) {
    listen_digital[li] = 0;
    listen_value[li] = -1;
  }
}

#define MAX(a,b) ((a) > (b) ? (a) : (b))

void _listen_digital(int seq, int pin) {
  _set_pin_mode(pin, INPUT);
  
  listen_max = MAX(listen_max, pin + 1);
  listen_digital[pin] = seq;
  listen_value[pin] = -1;
  listen_debounce[pin] = -1;
}

void _listen_sweep() {
  if (!listen_max) {
    return;
  }
  
//  Serial.print("!sweep,\n");
  
  for (int li = 0; li < listen_max; li++) {
    int dseq = listen_digital[li];
    if (dseq) {
      int dvalue = digitalRead(li);
      if (dvalue != listen_debounce[li]) {
        listen_debounce_millis = millis();
        listen_debounce[li] = dvalue;
      }
      if ((dvalue != listen_value[li]) && ((millis() - listen_debounce_millis) > listen_debounce_delta)) {
        listen_value[li] = dvalue;
        
        Serial.print(dseq);
        Serial.print(",");
        Serial.print(dvalue);
        Serial.print("\n");
      }
    }
  }
}

// ---------------------------------------- setup() ----------------------------------------

void setup(){
  _serial_setup();
  _serial_hello();

  _clear_pin_setup(); 
  _listen_setup();
}

// ---------------------------------------- begin of loop() ----------------------------------------
void loop() {
  
static unsigned long  loopCount = 0;
static unsigned long  setupTime = micros();
static unsigned long  loopTime = 0;
static unsigned long  Time1 = 0;
static unsigned long  Time2 = 0;
static unsigned long  Time3 = 0;
static unsigned long  Time4 = 0;
static unsigned long  Time5 = 0;
static unsigned long  Time6 = 0;

static int  i,j = 0;

// oscilloscope addon
static int  value;
static int  valueLast;
static int  valueCount;
static bool window = false;
static byte hiByte;
static byte loByte;
static int  wave1 = 0; // oscilloscope addon for sqare wave generating
static int  wave2 = 0; // oscilloscope addon for sqare wave generating

  if (!Serial.available()) { 
    _listen_sweep();
  }


  loopCount++;
  loopTime = micros() - setupTime; // an empty loop with minimal testing code runs about at 90 khz
  
  if ((loopTime - Time1) >= oscResolution) { // measure timing
    Time1 = loopTime;
    // oscilloscope addon
    // you can delete the following code if you aren't interested on Oscilloscope Function working with Excel
    if (oscSwitch) { // if oscilloscope switch is on
      value = analogRead(oscPin);
      if (oscGo) { // if oscilloscope go is on
        if (oscTrigger > 0 && window == false) { // if trigger is defined and window is off
          if ((value - valueLast) >= oscTrigger) { // if a positive edge is detected
            window = true;
          }
        }
        if ((oscTrigger > 0 && window == true) || (oscTrigger == 0)) { // if (trigger id defined and window is on) or (trigger is undefined)
          hiByte = (value >> 8) & 0xFF;
          loByte = value & 0xFF;
          Serial.write(loByte);
          Serial.write(hiByte);
          valueCount++;
        }
        if (valueCount > 99) { // if 100 values are sent
          if (oscRepeat == false) { // if no repeating option
            oscGo = false;
          }
          valueCount = 0;
          window = false;
        }
      }
      else { // oscilloscope go is off
        valueCount = 0;
        window = false;
      }
      valueLast = value;
    }
    else { // switch off
      valueCount = 0;
      window = false;
    }
  }

/*    
  // istructions executed each 8,333ms to perform a 60hz sqare wave
  if ((loopTime - Time2) >= 8333) {
    Time2 = loopTime;
    if (wave1 == 0) {
      wave1 = 1;
    }
    else {
      wave1 = 0;
    }
    digitalWrite(6,wave1);
  }

  // istructions executed each 5ms to perform a 100hz sqare wave
  if ((loopTime - Time3) >= 5000) {
    Time3 = loopTime;
    if (wave2 == 0) {
      wave2 = 1;
    }
    else {
      wave2 = 0;
    }
    digitalWrite(7,wave2);
  }
*/

  // csv addon
  // you can delete the following code if you aren't interested on csv data sending
  if ((loopTime - Time4) >= (csvFrequency * 1000000)) {
    Time4 = loopTime;
    if (csvSwitch) {
      // the output format to Excel is:
      // XLS,CellName,Index,Value
      // where:
      //   'XLS' is a constant keyword for messages to Excel
      //   'CellName' is a keyword that specifies the cells(s) name where info will be allocated in
      //   'Index specifies the item (forced to zero for simple variables)
      //   'Value' is the info value 
      // messages are \n terminated
      // use Arduino Excel Commander of the same author for messages reading and info real time representing
      // example 1
      // the following info will be placed in a cell VAR1 named
      Serial.print("XLS,VAR1,");
      Serial.print("0"); // while we are sending a simple variable index is forced to zero
      Serial.print(",");
      Serial.print(analogRead(0)); // only for example, put here your variable
      Serial.print("\n");
      // example 2
      // the following info will be placed in a cell VAR2 named
      Serial.print("XLS,VAR2,");
      Serial.print("0"); // while we are sending a simple variable index is forced to zero
      Serial.print(",");
      Serial.print(analogRead(1)); // only for example, put here your variable
      Serial.print("\n");
      // example 3
      // the following info will be placed in a cell VAR3 named
      Serial.print("XLS,VAR3,");
      Serial.print("0"); // while we are sending a simple variable index is forced to zero
      Serial.print(",");
      Serial.print(analogRead(2)); // only for example, put here your variable
      Serial.print("\n");
      // example 4
      // the following info will be placed at i position of a cell group named ARR1
      Serial.print("XLS,ARR1,");
      Serial.print(csvIndex); // only for example, put here your index
      Serial.print(",");
      Serial.print(random(0, 100)); // only for example, put here your variable
      Serial.print("\n");
      csvIndex++;
    }
  }
/*    
  // istructions executed each 1 s
  if ((loopTime - Time6) >= 1000000) {
    Time6 = loopTime;
    // loop performance check
    // about 90 khz with a empty loop
    // note that due architecture with timers loop performances are quite stable
    Serial.print("LoopRate");
    Serial.print(",");
    Serial.print(((float)loopCount / (float)loopTime * (float)1000000),0);
    Serial.print(",");
    Serial.print(((float)loopTime / (float)loopCount / 1000),3);
    Serial.print(",");
    Serial.print(diff);
    Serial.print("\n");
  }
*/    
  
} // ---------------------------------------- end of loop() ----------------------------------------

// this function is automatically called each loop if somewhat is present in the RX buffer
void serialEvent() {

char           inChar;           // single read char
String         inLine = "";      // line extracted from inString
int            i,j;              // indexes
String         inId = "";        // id from inString
String         inFunction = "";  // function from inString
char           cArray[10];       // character array for string to long conversion
String         inArgStr[4];      // up to 4 args from inString
unsigned long  inArgLng[4];      // up to 4 args from inString
float          inArgFlo[4];      // up to 4 args from inString
int            arg = 0;          // args index
int            intValue;         // return value
unsigned long  ulongValue;       // return value
boolean        out = true;       // output flag

 
  // line extraction
  //       Serial.print("!scan,\n");
  while (Serial.available()) { // RX buffer holds up to 64 bytes
    // note that char type and String type (array of chars) can manage only codes from 0 to 127
    inChar = (char)Serial.read();
    inString += inChar;
    if (inChar == 10){ // LF ends the command line
        inLine = inString;
        inString = "";
        break; // other bytes if present are retainded in the RX buffer
    }
  }
  
  // id, function and arg extraction
  // the following code is executed only if an input line is complete
  if (inLine != "") {
    for (i = 0; i < inLine.length(); i++) {
      if (inLine.charAt(i) == 44 || inLine.charAt(i) == 10) { // comma separates id, function and args, LF ends the command line
        if (inId == "") { // Id
          inId = inLine.substring(0, i);
        }
        else if (inFunction == "") { // function
          inFunction = inLine.substring(j, i);
        }
        else { // args
          inArgStr[arg] = inLine.substring(j, i); // string type argument
          inArgStr[arg].toCharArray(cArray,10);
          inArgLng[arg] = atol(cArray); // long type argument
          inArgFlo[arg] = atof(cArray); // float type argument
          arg++; // arg index
        }
        j = i + 1;
      }
    }
    if (inFunction == "") {
      Serial.println(F("command format: id,function[,arg1][,arg2][,arg3] (id: sender id string)\n"));
    }
 
    // commands processing    
    // you can delete the code about unused functions
    if (inFunction.equalsIgnoreCase("pinMode")) {
      _set_pin_mode(inArgLng[0], inArgLng[1]);
    }
    else if (inFunction.equalsIgnoreCase("digitalListen")) {
      inId.toCharArray(cArray,10);
      _listen_digital(atol(cArray), inArgLng[0]);
    }
    else if (inFunction.equalsIgnoreCase("digitalRead")) {
      _set_pin_mode(inArgLng[0], INPUT);
      intValue = digitalRead(inArgLng[0]); // pin
    }
    else if (inFunction.equalsIgnoreCase("digitalWrite")) {
      _set_pin_mode(inArgLng[0], OUTPUT);
      digitalWrite(inArgLng[0], inArgLng[1] ? HIGH : LOW); // pin,value
    }
    else if (inFunction.equalsIgnoreCase("analogRead")) {
      _set_pin_mode(inArgLng[0], INPUT);
      intValue = analogRead(inArgLng[0]); // pin
    }
    else if (inFunction.equalsIgnoreCase("analogWrite")) {
      _set_pin_mode(inArgLng[0], OUTPUT);
      analogWrite(inArgLng[0], inArgLng[1]); // pin,dutycycle
    }
    else if (inFunction.equalsIgnoreCase("pulseIn")) {
      ulongValue = pulseIn(inArgLng[0], inArgLng[1], inArgLng[2]); // pin,mode,timeout
    }
    else if (inFunction.equalsIgnoreCase("tone")) {
      tone(inArgLng[0], (int)inArgLng[1]); // pin,frequency
    }
    else if (inFunction.equalsIgnoreCase("noTone")) {
      noTone(inArgLng[0]); // pin
    }
    else if (inFunction.equalsIgnoreCase("millis")) {
      ulongValue = millis();
    }
    else if (inFunction.equalsIgnoreCase("delay")) {
      delay(inArgLng[0]); // milliseconds
    }
    else if (inFunction.equalsIgnoreCase("myFunction")) {
      // while personalizing be careful on argument type
    }
    else if (inFunction.equalsIgnoreCase("oscilloscopeOn")) {    // oscilloscope addon
      oscPin = inArgLng[0];
      oscResolution = inArgLng[1] * 1000; // millisecond value converted in microsecond
      oscTrigger = inArgLng[2];
      oscSwitch = true;
      oscGo = false;
      out = false;
      csvSwitch = false;
   }
    else if (inFunction.equalsIgnoreCase("oscilloscopeGo")) {    // oscilloscope addon
      if (inArgLng[0] == 0) {
        oscRepeat = false;
      }
      else {
        oscRepeat = true;
      }
      oscGo = true;
      out = false;
    }
    else if (inFunction.equalsIgnoreCase("oscilloscopeOff")) {    // oscilloscope addon
      oscSwitch = false;
      oscGo = false;
      out = false;
    }
    else if (inFunction.equalsIgnoreCase("csvOn")) {    // csv addon
      csvSwitch = true;
      csvFrequency = inArgLng[0]; // sending frequency
      csvIndex = 0;
    }
    else if (inFunction.equalsIgnoreCase("csvOff")) {    // csv addon
      csvSwitch = false;
    }
    else if (inId == "?" || inFunction == "?") {
      Serial.print(F("\n"));
      Serial.print(F("Arduino Serial Protocol 1.6\n"));
      Serial.print(F("command format: id,function[,arg1][,arg2][,arg3] (id: sender id string)\n"));
      Serial.print(F("functions are:\n"));
      Serial.print(F("  pinMode,pin,mode (mode: 0 for INPUT, 2 for INPUT_PULLUP, 1 for OUTPUT)\n"));
      Serial.print(F("  digitalRead,pin\n"));
      Serial.print(F("  digitalWrite,pin,value (value: 0 for LOW, 1 for HIGH)\n"));
      Serial.print(F("  analogRead,pin\n"));
      Serial.print(F("  analogWrite,pin,dutycycle (dutycycle: between 0 always off and 255 always on)\n"));
      Serial.print(F("  pulseIn,pin,mode,timeout (mode: 0 for LOW, 1 for HIGH) (timeout unit: microseconds)\n"));
      Serial.print(F("  tone,pin,frequency (frequency: between 31 to 65535)\n"));
      Serial.print(F("  noTone,pin\n"));
      Serial.print(F("  millis\n"));
      Serial.print(F("  delay,milliseconds\n"));
      Serial.print(F("  myFunction\n"));
      Serial.print(F("  oscilloscopeOn,pin,resolution,trigger (resolution: between 1 and 4294966) (resolution unit: milliseconds) (trigger:  between 0 no trigger to 1023)\n"));
      Serial.print(F("  oscilloscopeGo,repeat (repeat: 0 for OFF 1 for ON\n"));
      Serial.print(F("  oscilloscopeOff\n"));
      Serial.print(F("  csvOn,frequency (frequency unit: seconds)\n"));
      Serial.print(F("  csvOff\n"));
      Serial.print(F("function use: see http://arduino.cc/en/Reference/HomePage\n"));
      Serial.print(F("\n"));
      out = false;
    }
    else if (inFunction == "") {
      out = false;
    }
    else {
      Serial.print("error,unknown command\n");
      out = false;
    }
    if (out) { // output management
      // the format of the answer is: id,- or id,value
      Serial.print(inId); // returns the sender id
      Serial.print(",");
      // you can delete the code about unused functions
      if (inFunction.equalsIgnoreCase("digitalRead")) { // digitalRead value
        Serial.print(intValue);
        Serial.print("\n");
      }
      else if (inFunction.equalsIgnoreCase("analogRead")) { // analogRead value
        Serial.print(intValue);
        Serial.print("\n");
      }
      else if (inFunction.equalsIgnoreCase("pulseIn")) { // pulseIn value
        Serial.print(ulongValue);
        Serial.print("\n");
      }
      else if (inFunction.equalsIgnoreCase("millis")) { // millis value
        Serial.print(ulongValue);
        Serial.print("\n");
      }
      else if (inFunction.equalsIgnoreCase("myFunction")) { // myFunction
        Serial.print("my result is ...");
        Serial.print("\n");
      } else if (inFunction.equalsIgnoreCase("digitalListen")) { // digitalRead value
        Serial.print("\n"); 
      } else {
        Serial.print("-\n"); // if no value returns a -
      }
    }
  }
}
