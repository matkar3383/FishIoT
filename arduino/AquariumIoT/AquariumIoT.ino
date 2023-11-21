

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

//Others
#define INTV_LIGHT_SIZE 10
#define INTV_FAN_SIZE 10
#define INTV_FEEDER_SIZE 2
#define TIMEVAR_SIZE 3

//variables
boolean status_mode = false;
boolean status_light = false;
boolean status_feeder = false;
boolean status_fan = false;
boolean status_temp_warning = false;
unsigned long previousMillis = 0;
const long temperature_interval = 10000;  // 10s
//String inputString = "";
char inputCommand[20] = {0};
char inputData[13][10] = {0};
int timeVar[TIMEVAR_SIZE]; // [HH, MM, SS]
//boolean stringComplete = false;
char buffer[50];
float criticalTemperature = 23;
float currentTemperature = 0;
float interval_light[INTV_LIGHT_SIZE] = {0};
float interval_fan[INTV_FAN_SIZE] = {0};
float interval_feeder[INTV_FEEDER_SIZE] = {0};

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
  int i = 0;
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
    //inputString = Serial.readStringUntil('\n');
    Serial.readBytesUntil('\n', inputCommand, 20);
    //Serial.readBytesUntil('\n', inputString, 20);
    //-------------------- Check ------------------
    if(strcmp(inputCommand, "check") == 0){
        sprintf(buffer, "status_mode %d", status_mode);
        Serial.println(buffer);
        sprintf(buffer, "status_light %d", status_light);
        Serial.println(buffer);
        sprintf(buffer, "status_fan %d", status_fan);
        Serial.println(buffer);
        sprintf(buffer, "status_feeder %d", status_feeder);
        Serial.println(buffer);
        dtostrf(currentTemperature, 5, 2, floatStr); // turn float into string
        sprintf(buffer, "currentTemperature %s", floatStr);
        Serial.println(buffer);
        dtostrf(criticalTemperature, 5, 2, floatStr); // turn float into string
        sprintf(buffer, "criticalTemperature %s", floatStr);
        Serial.println(buffer);
        sprintf(buffer, "interval_light %s %s", "0.00", "0.00");
        Serial.println(buffer);
        sprintf(buffer, "interval_fan %s %s", "0.00", "0.00");
        Serial.println(buffer);
        sprintf(buffer, "interval_feeder %s %s", "0.00", "0.00");
        Serial.println(buffer);
        
      }/*
    if(inputString == "set"){
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("sync_time")){
            //status_mode = !status_mode;
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_mode")){
            sscanf(inputString.c_str(), "set_mode %d", &status_mode);
            //status_mode = !status_mode;
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_light")){
            sscanf(inputString.c_str(), "set_light %d", &status_light);
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_fan")){
            //sscanf(inputString.c_str(), "set_fan %d", &status_fan);
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_feeder")){
            //sscanf(inputString.c_str(), "set_feeder %d", &status_feeder);
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_critical_temperature")){
            //status_light = !status_light;
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_interval_light")){
            //status_light = !status_light;
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_interval_fan")){
            //status_light = !status_light;
          }
        inputString = Serial.readStringUntil('\n');
        if(inputString.startsWith("set_interval_feeder")){
            //status_light = !status_light;
          }
      }*/
      /*
      if(inputString == "set"){
        
        inputString = Serial.readStringUntil('\n');

        sscanf(inputString.c_str(), "%d,%d,%d,%d,%d,%d,", &timeVar, &status_mode, &status_light, &status_fan, &status_feeder, &criticalTemperature);

        Serial.println(inputString);
        Serial.println(timeVar);
        Serial.println(status_mode);
        Serial.println(status_light);
        Serial.println(status_fan);
        Serial.println(status_feeder);
        Serial.println(criticalTemperature);
      }*/
      //-------------------- Set ------------------
      else if(strcmp(inputCommand, "set") == 0){
        // read values from Serial
        i = 0;
        while (1){
          Serial.readBytesUntil('#', inputData[i], 10);
          i++;
          if (i == 8){
            break;
          }
        }
        //inputString = Serial.readStringUntil('\n');

        //sscanf(inputString.c_str(), "%d,%d,%d,%d,%d,%2.2f,", &timeVar, &status_mode, &status_light, &status_fan, &status_feeder, &criticalTemperature);
        buffer[0] = (char)0;
        sprintf(buffer, "recieved %*s", inputData);
        Serial.println(buffer);
        
        timeVar[0] = atoi(inputData[0]);
        timeVar[1] = atoi(inputData[1]);
        timeVar[2] = atoi(inputData[2]);
        status_mode = atoi(inputData[3]);
        status_light = atoi(inputData[4]);
        status_fan = atoi(inputData[5]);
        status_feeder = atoi(inputData[6]);
        criticalTemperature = atof(inputData[7]);
        
        sprintf(buffer, "time %2d:%2d:%2d", timeVar[0], timeVar[1], timeVar[2]);
        Serial.println(buffer);
        Serial.println(status_mode);
        Serial.println(status_light);
        Serial.println(status_fan);
        Serial.println(status_feeder);
        Serial.println(criticalTemperature);
      }
      //-------------------- set light interval ------------------
      else if(strcmp(inputCommand, "set_light_interval") == 0){
        // read values from Serial
        i = 0;
        while (1){
          Serial.readBytesUntil('#', inputData[i], 10);
          i++;
          if (i == INTV_LIGHT_SIZE + TIMEVAR_SIZE + 1){//sizeof(interval_light)+sizeof(timeVar)+1){
            break;
          }
        }
        
        timeVar[0] = atoi(inputData[0]);
        timeVar[1] = atoi(inputData[1]);
        timeVar[2] = atoi(inputData[2]);

        sprintf(buffer, "time %2d:%2d:%2d", timeVar[0], timeVar[1], timeVar[2]);
        Serial.println(buffer);
        
        for(i = 0; i < INTV_LIGHT_SIZE; i++){
           interval_light[i] = atof(inputData[i+3]);
           Serial.println(interval_light[i]);
        }
      }
      //-------------------- set fan interval ------------------
      else if(strcmp(inputCommand, "set_fan_interval") == 0){
        // read values from Serial
        i = 0;
        while (1){
          Serial.readBytesUntil('#', inputData[i], 10);
          i++;
          if (i == INTV_FAN_SIZE + TIMEVAR_SIZE + 1){//sizeof(interval_fan)+sizeof(timeVar)+1){
            break;
          }
        }
        
        timeVar[0] = atoi(inputData[0]);
        timeVar[1] = atoi(inputData[1]);
        timeVar[2] = atoi(inputData[2]);

        sprintf(buffer, "time %2d:%2d:%2d", timeVar[0], timeVar[1], timeVar[2]);
        Serial.println(buffer);
        
        for(i = 0; i < INTV_FAN_SIZE; i++){
           interval_fan[i] = atof(inputData[i+3]);
           Serial.println(interval_fan[i]);
        }
      }
      //-------------------- set feeder interval ------------------
      else if(strcmp(inputCommand, "set_feeder_interval") == 0){
        // read values from Serial
        i = 0;
        while (1){
          Serial.readBytesUntil('#', inputData[i], 10);
          i++;
          if (i == INTV_FEEDER_SIZE + TIMEVAR_SIZE + 1){
            break;
          }
        }
        
        timeVar[0] = atoi(inputData[0]);
        timeVar[1] = atoi(inputData[1]);
        timeVar[2] = atoi(inputData[2]);

        sprintf(buffer, "%2d:%2d:%2d", timeVar[0], timeVar[1], timeVar[2]);
        Serial.println(buffer);
        
        for(i = 0; i < INTV_FEEDER_SIZE; i++){
           interval_feeder[i] = atof(inputData[i+3]);
           Serial.println(interval_feeder[i]);
        }
      }
      //-------------------- sync_time ------------------
      else if(strcmp(inputCommand, "sync_time") == 0){
        // read values from Serial
        i = 0;
        while (1){
          Serial.readBytesUntil('#', inputData[i], 10);
          i++;
          if (i == TIMEVAR_SIZE + 1){
            break;
          }
        }
        
        timeVar[0] = atoi(inputData[0]);
        timeVar[1] = atoi(inputData[1]);
        timeVar[2] = atoi(inputData[2]);

        sprintf(buffer, "time %2d:%2d:%2d", timeVar[0], timeVar[1], timeVar[2]);
        Serial.println(buffer);
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
    currentTemperature = sensors.getTempCByIndex(0);
    if (currentTemperature > criticalTemperature){
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
  //stringComplete = false;
  //inputString = "";
  //inputCommand[0] = (char)0;
  memset(inputCommand, 0, sizeof(inputCommand));


  //
  delay(50);
}
