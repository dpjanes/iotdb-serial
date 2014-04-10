/*
 *  comm_arduino.ino
 *
 *  David Janes
 *  IOTDB.org
 *  2014-04-09
 *
 *  See COMMENTS.md for the origin of this file!
 *  Originally by Roberto Valgolio / roberto.valgolio@gmail.com
 */

// define this if you want to use the DHT temperature / sensor library
#define USE_DHT

#if defined(USE_DHT)
#include <DHT.h>
#endif

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

// --- setup section

const int max_digital_pins = 24;
const int max_analog_pins = 12;
char pin_setup[max_digital_pins];

void _clear_pin_setup() {
  for (int pi = 0; pi < max_digital_pins; pi++) {
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
  Serial.print("\n\n\n\n!hello,Arduino");
#if defined(USE_DHT)
  Serial.print("+DHT");
#endif
  Serial.print("\n");
}

void _set_pin_mode(int pin, int mode) {
  if (pin_setup[pin]) {
    return;
  }
  
  pin_setup[pin] = 1;
  pinMode(pin, mode);

}

#define MAX(a,b) ((a) > (b) ? (a) : (b))

long listen_debounce_millis;
long listen_debounce_delta = 50;    // changes have to be stable this long

// --- digital listen section

int listen_digital_max = 0;
int listen_digital_seq[max_digital_pins];  // notification <seq>
int listen_digital_value[max_digital_pins];    // current value 
int listen_digital_debounce[max_digital_pins]; // value for debounce

void _listen_digital_setup() {
  for (int pin = 0; pin < max_digital_pins; pin++) {
    listen_digital_seq[pin] = 0;
    listen_digital_value[pin] = -1;
  }
}

void _listen_digital(int seq, int pin) {
  _set_pin_mode(pin, INPUT);
  
  listen_digital_max = MAX(listen_digital_max, pin + 1);
  listen_digital_seq[pin] = seq;
  listen_digital_value[pin] = -1;
  listen_digital_debounce[pin] = -1;
}

void _listen_digital_sweep() {
  if (!listen_digital_max) {
    return;
  }
  
  for (int pin = 0; pin < listen_digital_max; pin++) {
    int dseq = listen_digital_seq[pin];
    if (dseq) {
      int dvalue = digitalRead(pin);
      if (dvalue != listen_digital_debounce[pin]) {
        listen_debounce_millis = millis();
        listen_digital_debounce[pin] = dvalue;
      }
      if ((dvalue != listen_digital_value[pin]) && ((millis() - listen_debounce_millis) > listen_debounce_delta)) {
        listen_digital_value[pin] = dvalue;
        
        Serial.print(dseq);
        Serial.print(",");
        Serial.print(dvalue);
        Serial.print("\n");
      }
    }
  }
}

// --- analog listen section

long listen_analog_blur = 5;       // ignore analog changes smaller than this

int listen_analog_max = 0;
int listen_analog_seq[max_analog_pins];  // notification <seq>
int listen_analog_value[max_analog_pins];    // current value 
int listen_analog_debounce[max_analog_pins]; // value for debounce

void _listen_analog_setup() {
  for (int pin = 0; pin < max_analog_pins; pin++) {
    listen_analog_seq[pin] = 0;
    listen_analog_value[pin] = -1;
  }
}

void _listen_analog(int seq, int pin) {
  _set_pin_mode(pin, INPUT);
  
  listen_analog_max = MAX(listen_analog_max, pin + 1);
  listen_analog_seq[pin] = seq;
  listen_analog_value[pin] = -1;
  listen_analog_debounce[pin] = -1;
}

void _listen_analog_sweep() {
  if (!listen_analog_max) {
    return;
  }
  
  for (int pin = 0; pin < listen_analog_max; pin++) {
    int aseq = listen_analog_seq[pin];
    if (aseq) {
      int avalue = analogRead(pin);
      int adelta = abs(avalue - listen_analog_debounce[pin]);
      if (adelta >= listen_analog_blur) {
        listen_debounce_millis = millis();
        listen_analog_debounce[pin] = avalue;
      }
      adelta = abs(avalue - listen_analog_value[pin]);
      if ((adelta >= listen_analog_blur) && ((millis() - listen_debounce_millis) > listen_debounce_delta)) {
        listen_analog_value[pin] = avalue;
        
        Serial.print(aseq);
        Serial.print(",");
        Serial.print(avalue);
        Serial.print("\n");
      }
    }
  }
}

// --- temperature & humidity section
#if defined(USE_DHT)
DHT* dht = 0;
int dht_type = DHT11;
int dht_pin = 0;
int dht_seq_temperature = -1;
float dht_last_temperature = -999;
int dht_seq_humidity = -1;
float dht_last_humidity = -999;

void _listen_dht_setup() {
}

void _listen_dht_sweep() {
  if (!dht) {
    return;
  }
  
  if (dht_seq_humidity > -1) {
    float humidity = dht->readHumidity();
    if (humidity != dht_last_humidity) {
      dht_last_humidity = humidity;
      
      Serial.print(dht_seq_humidity);
      Serial.print(",");
      Serial.print(dht_last_humidity);
      Serial.print("\n");
    }
  }

  if (dht_seq_temperature > -1) {
    float temperature = dht->readTemperature();
    if (temperature != dht_last_temperature) {
      dht_last_temperature = temperature;
      
      Serial.print(dht_seq_temperature);
      Serial.print(",");
      Serial.print(dht_last_temperature);
      Serial.print("\n");
    }
  }
}

DHT* _dht(int _dht_pin) {
  if (dht == 0) {
    dht_pin = A0;
    dht = new DHT(dht_pin, dht_type);
    dht->begin();
  }
  
  return dht;
}

void _set_dht_type(int _dht_type) {
  dht_type = _dht_type;
}

void _listen_dht_temperature(int seq, int _dht_pin) {
  _dht(_dht_pin);
  dht_seq_temperature = seq;
}

void _listen_dht_humidity(int seq, int _dht_pin) {
  _dht(_dht_pin);
  dht_seq_humidity = seq;
}

float _get_dht_temperature(int _dht_pin) {
  return _dht(_dht_pin)->readTemperature();
}

float _get_dht_humidity(int _dht_pin) {
  return _dht(_dht_pin)->readHumidity();
}
#endif


// ---------------------------------------- setup() ----------------------------------------

void setup(){
  _serial_setup();
  _serial_hello();

  _clear_pin_setup(); 
  _listen_digital_setup();
  _listen_analog_setup();
#if defined(USE_DHT)
  _listen_dht_setup();
#endif
}

// ---------------------------------------- begin of loop() ----------------------------------------
unsigned long mc = 0;
int heartbeat_count = 0;

void loop() {
  if (!Serial.available()) { 
    _listen_digital_sweep();
    _listen_analog_sweep();
#if defined(USE_DHT)
    _listen_dht_sweep();
#endif
  }
  
  unsigned long m = millis();
  unsigned long d = m - mc;
  if (d > (30 * 1000)) {
    mc = m;
    Serial.print("!,Heartbeat,");
    Serial.print(heartbeat_count++);
    Serial.print("\n");
  }
} 

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
float          floatValue;
boolean        out = true;       // output flag
boolean        out_dash = false;  // output flag
boolean        out_intValue = false;  // output flag
boolean        out_ulongValue = false;  // output flag
boolean        out_floatValue = false;  // output flag

 
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
      out_dash = false;
    }
    else if (inFunction.equalsIgnoreCase("analogListen")) {
      inId.toCharArray(cArray,10);
      _listen_analog(atol(cArray), inArgLng[0]);
      out_dash = false;
    }
    else if (inFunction.equalsIgnoreCase("digitalRead")) {
      _set_pin_mode(inArgLng[0], INPUT);
      intValue = digitalRead(inArgLng[0]); // pin
      out_intValue = true;
    }
    else if (inFunction.equalsIgnoreCase("digitalWrite")) {
      _set_pin_mode(inArgLng[0], OUTPUT);
      digitalWrite(inArgLng[0], inArgLng[1] ? HIGH : LOW); // pin,value
    }
#if defined(USE_DHT)
    else if (inFunction.equalsIgnoreCase("DHT11")) {
      inId.toCharArray(cArray,10);
      _set_dht_type(DHT11);
    }
    else if (inFunction.equalsIgnoreCase("DHT21")) {
      inId.toCharArray(cArray,10);
      _set_dht_type(DHT21);
    }
    else if (inFunction.equalsIgnoreCase("DHT22")) {
      inId.toCharArray(cArray,10);
      _set_dht_type(DHT22);
    }
    else if (inFunction.equalsIgnoreCase("temperatureListen")) {
      inId.toCharArray(cArray,10);
      _listen_dht_temperature(atol(cArray), inArgLng[0]);
      out_dash = false;
    }
    else if (inFunction.equalsIgnoreCase("humidityListen")) {
      inId.toCharArray(cArray,10);
      _listen_dht_humidity(atol(cArray), inArgLng[0]);
      out_dash = false;
    }
    else if (inFunction.equalsIgnoreCase("temperatureRead")) {
      floatValue = _get_dht_temperature(inArgLng[0]);
      out_floatValue = true;
    }
    else if (inFunction.equalsIgnoreCase("humidityRead")) {
      floatValue = _get_dht_humidity(inArgLng[0]);
      out_floatValue = true;
    }
#endif
    else if (inFunction.equalsIgnoreCase("analogRead")) {
      _set_pin_mode(inArgLng[0], INPUT);
      intValue = analogRead(inArgLng[0]); // pin
      out_intValue = true;
    }
    else if (inFunction.equalsIgnoreCase("analogWrite")) {
      _set_pin_mode(inArgLng[0], OUTPUT);
      analogWrite(inArgLng[0], inArgLng[1]); // pin,dutycycle
    }
    else if (inFunction.equalsIgnoreCase("pulseIn")) {
      ulongValue = pulseIn(inArgLng[0], inArgLng[1], inArgLng[2]); // pin,mode,timeout
      out_ulongValue = true;
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
      out_ulongValue = true;
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
      if (out_intValue) {
        Serial.print(intValue);
      } else if (out_ulongValue) {
        Serial.print(ulongValue);
      } else if (out_floatValue) {
        Serial.print(floatValue);
      } else if (out_dash) {
        Serial.print("-");
      }
      Serial.print("\n"); 
    }
  }
}
