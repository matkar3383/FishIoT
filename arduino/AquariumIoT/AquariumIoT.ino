

#include "OneWire.h"
#include "DallasTemperature.h"

//States
#define AUTOMATIC 0
#define MANUAL 1
//#define CRITICAL_TEMPERATURE 23

//LED-Pins
#define PIN_LED_MODE_M 5
#define PIN_LED_MODE_A 3
#define PIN_LED_TEMP_WARNING 6

//Buttons-Pins
#define PIN_BTN_MODE 8
#define PIN_BTN_LIGHT 2
#define PIN_BTN_FEEDER 4
#define PIN_BTN_FAN 7

//Other-Pins
#define PIN_TEMP_SENS A0
#define PIN_AKT_LIGHT 10
#define PIN_AKT_FEEDER 12
#define PIN_AKT_FAN 11

//variables
boolean status_mode = false;
boolean status_light = false;
boolean status_feeder = false;
boolean status_fan = false;
boolean status_temp_warning = false;
unsigned long previousMillis = 0;
const long temperature_interval = 10000;  // 10s
String inputString = "";
boolean stringComplete = false;
char buffer[50];
float criticalTemperature = 23;

//Temperatur-Sensor
 
// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(PIN_TEMP_SENS);

// Pass OneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);
 
const float No_Val = 999.99;

//Functions
void setStatus(int pin_led, boolean status){
  if (status){
    digitalWrite(pin_led, HIGH);
  }
  else{
    digitalWrite(pin_led, LOW);
  }
}

void setMode(boolean status){
  setStatus(PIN_LED_MODE_M, status);
  setStatus(PIN_LED_MODE_A, !status);
}

boolean btn_onRelease(int pin_btn, boolean status){
  if (digitalRead(pin_btn) == LOW) {
    status = !status;
    delay(50);
    // wait for release button
    while(1){
      if(digitalRead(pin_btn) == HIGH){
        delay(50);
        break;
      }
    }
  }
  return status;
}

//Routine
void setup() {
  Serial.begin(9600);
  sensors.begin(); // Start up the library
  
  //LEDs
  pinMode(PIN_LED_MODE_M, OUTPUT);
  pinMode(PIN_LED_MODE_A, OUTPUT);
  pinMode(PIN_AKT_LIGHT, OUTPUT);
  pinMode(PIN_AKT_FEEDER, OUTPUT);
  pinMode(PIN_AKT_FAN, OUTPUT);
  pinMode(PIN_LED_TEMP_WARNING, OUTPUT);

  //Inputs
  //Buttons
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_LIGHT, INPUT_PULLUP);
  pinMode(PIN_BTN_FEEDER, INPUT_PULLUP);
  pinMode(PIN_BTN_FAN, INPUT_PULLUP);

  

  //Preconditions
  setMode(status_mode);
  setStatus(PIN_AKT_LIGHT, status_light);
  setStatus(PIN_AKT_FEEDER, status_feeder);
  setStatus(PIN_AKT_FAN, status_fan);
  setStatus(PIN_LED_TEMP_WARNING, status_temp_warning);

}

void loop() {
  unsigned long currentMillis = millis();
  char floatStr[10];
  // check request from RPi
  /*
  while (Serial.available()) {
        char inChar = (char)Serial.read();
        inputString += inChar;
        
        if(inChar == '\n'){
          stringComplete = true;
          //Serial.println(inputString);
          if (inputString.startsWith("check")){
            status_mode = !status_mode;
            sprintf(buffer, "status_mode %d, status_light %d", status_mode, status_light);
            Serial.println(buffer);
            
          }
        }
        */
   if (Serial.available()>0){
    inputString = Serial.readStringUntil('\n');
    if(inputString == "check"){
        
        sprintf(buffer, "status_mode %d", status_mode);
        Serial.println(buffer);
        sprintf(buffer, "status_light %d", status_light);
        Serial.println(buffer);
        sprintf(buffer, "status_fan %d", status_fan);
        Serial.println(buffer);
        sprintf(buffer, "status_feeder %d", status_feeder);
        Serial.println(buffer);
        dtostrf(criticalTemperature, 5, 2, floatStr); // turn float into string
        sprintf(buffer, "criticalTemperature %s", floatStr);
        Serial.println(buffer);
      }
    if(inputString == "set"){
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("mode")){
            //status_mode = !status_mode;
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("criticalTemperature")){
            //criticalTemperature = -128;
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("light")){
            status_light = !status_light;
          }
      }
    
    }

        /*
        Serial.println("Temperature is: " + String(sensors.getTempCByIndex(0)) + "°C");
        Serial.print("Folgender char wurde empfangen: ");
        Serial.println(nr, DEC);*/
  


  //check mode
  status_mode = btn_onRelease(PIN_BTN_MODE, status_mode);
  setMode(status_mode);


  if (status_mode == MANUAL){
    status_light = btn_onRelease(PIN_BTN_LIGHT, status_light);
    status_feeder = btn_onRelease(PIN_BTN_FEEDER, status_feeder);
    status_fan = btn_onRelease(PIN_BTN_FAN, status_fan);
  }
  /*
  
  */

  
  if (currentMillis - previousMillis >= temperature_interval)
  {
    previousMillis = currentMillis;
    sensors.requestTemperatures();
    if (sensors.getTempCByIndex(0) > criticalTemperature){
      status_temp_warning = 1;
    }
    else{
      status_temp_warning = 0;
    }
    //Serial.println("Temperature is: " + String(sensors.getTempCByIndex(0)) + "°C");
    //Serial.println(analogRead(PIN_TEMP_SENS));
  }
  
  setStatus(PIN_AKT_LIGHT, status_light);
  setStatus(PIN_AKT_FEEDER, status_feeder);
  setStatus(PIN_AKT_FAN, status_fan);
  setStatus(PIN_LED_TEMP_WARNING, status_temp_warning);

  //reset string
  stringComplete = false;
  inputString = "";


  //
  delay(50);
}
