

#include "OneWire.h"
#include "DallasTemperature.h"

//States
#define AUTOMATIC 0
#define MANUAL 1

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
#define PIN_AKT_FEEDER 9
#define PIN_AKT_FAN 11

//Others definitions
#define INTV_LIGHT_SIZE 10
#define INTV_FAN_SIZE 10
#define INTV_FEEDER_SIZE 2
#define TIMEVAR_SIZE 3
#define FEEDER_DEBOUNCE_MS 200 



// ####################  Variables  ####################
boolean status_mode = false;
boolean status_light = false;
boolean status_feeder = false;
boolean status_fan = false;
boolean status_temp_warning = false;
boolean allowNextFeeding = true;
char inputCommand[20] = {0};
char inputData[13][10] = {0};
//int timeVar[TIMEVAR_SIZE]; // [HH, MM, SS]
//boolean stringComplete = false;
char buffer[100];
char buffer2[50];
float criticalTemperature = 23;
float currentTemperature = 0;
float interval_light[INTV_LIGHT_SIZE] = {6.0, 7.0, 10.0, 12.0, 14.0, 15.5, 17.5, 23.0, -1.0, -1.0};
float interval_fan[INTV_FAN_SIZE] = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
float interval_feeder[INTV_FEEDER_SIZE] = {18.0, -1.0};

//time
unsigned long currentMillis;
unsigned long previousMillis = millis();
long deltaTime;  // in s
long timeSampling = 180000; // 3min*60=180s
float currentTime = 0.0;
 
// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(PIN_TEMP_SENS);

// Pass OneWire reference to Dallas Temperature
DallasTemperature TempSensor(&oneWire);

// ####################  Functions  ####################
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

void once_feeding(int durationLimit){
  Serial.println("feeder");
  int highDuration = 1485;  //us
  int PWM_cycle = 20000;  //us  
  // create limited PWM signal
  for (int t=0; t < durationLimit; t = t + PWM_cycle){
    digitalWrite(PIN_AKT_FEEDER, HIGH);
    delayMicroseconds(highDuration);
    digitalWrite(PIN_AKT_FEEDER, LOW);
    delayMicroseconds(PWM_cycle - highDuration);
  }
}


void setCurrentTime(int HH, int MM, int SS){
  // send current values per serial
  sprintf(buffer, "time %2d:%2d:%2d", HH, MM, SS);
  Serial.println(buffer);
  // set new time
  currentTime = ((float)HH + (float)(MM*60+SS)/3600);
}

void readValuesFromSerial(int numberOfVariables){
  // read values from Serial seperated by # (also # at the end)
  /*int i = 0;
  while (1){
    Serial.readBytesUntil('#', inputData[i], 10);
    i++;
    if (i == numberOfVariables + 1){
      break;
    }
  }*/
  for(int i=0; i<=numberOfVariables; i++){
    Serial.readBytesUntil('#', inputData[i], 10);
  }
}

float getCurrentTemperature(){
  TempSensor.requestTemperatures();
  return TempSensor.getTempCByIndex(0);
}

// ####################  Arduino-Routine  ####################
void setup() {
  Serial.begin(9600);
  TempSensor.begin(); // Start up the library
  
  //Outputs
  pinMode(PIN_LED_MODE_M, OUTPUT);
  pinMode(PIN_LED_MODE_A, OUTPUT);
  pinMode(PIN_AKT_LIGHT, OUTPUT);
  pinMode(PIN_AKT_FEEDER, OUTPUT);
  pinMode(PIN_AKT_FAN, OUTPUT);
  pinMode(PIN_LED_TEMP_WARNING, OUTPUT);

  //Inputs
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_LIGHT, INPUT_PULLUP);
  pinMode(PIN_BTN_FEEDER, INPUT_PULLUP);
  pinMode(PIN_BTN_FAN, INPUT_PULLUP);

  //Preconditions
  currentTemperature = getCurrentTemperature();
  setMode(status_mode);
  setStatus(PIN_AKT_LIGHT, status_light);
  setStatus(PIN_AKT_FEEDER, status_feeder);
  setStatus(PIN_AKT_FAN, status_fan);
  setStatus(PIN_LED_TEMP_WARNING, status_temp_warning);

}

void loop() {
  char floatStr[10];
  int i = 0;

  // update time
  currentMillis = millis();
  deltaTime = (currentMillis - previousMillis);
  //Serial.println(deltaTime);
  if(deltaTime >= timeSampling){  // every 3min
    // correction of time variable
    currentTime = currentTime + ((float)timeSampling/3600.0)/1000.0;
    previousMillis = currentMillis;
    Serial.println(currentTime);
    if (abs(currentTime - 24.0) < 0.01){
      //currentTime = currentTime - 24.0;
      currentTime = 0.0;
    }
    // check temperature
    currentTemperature = getCurrentTemperature();
    Serial.println(currentTemperature);
    if (currentTemperature > criticalTemperature){
      status_temp_warning = 1;
    }
    else{
      status_temp_warning = 0;
    }
    // after time update allow next feeding
    allowNextFeeding = true;
  }

  // check request from RPi
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

      //sprintf(buffer, "interval_light %s %s", "0.00", "0.00");
      strcpy(buffer, "interval_light ");
      for (int i = 0; i<INTV_LIGHT_SIZE; i++){
        dtostrf(interval_light[i], 5, 2, floatStr);
        sprintf(buffer2, "%s ", floatStr);
        strcat(buffer, buffer2);
      }
      Serial.println(buffer);

      //sprintf(buffer, "interval_fan %s %s", "0.00", "0.00");
      strcpy(buffer, "interval_fan ");
      for (int i = 0; i<INTV_FAN_SIZE; i++){
        dtostrf(interval_fan[i], 5, 2, floatStr);
        sprintf(buffer2, "%s ", floatStr);
        strcat(buffer, buffer2);
      }
      Serial.println(buffer);

      //sprintf(buffer, "interval_feeder %s %s", "0.00", "0.00");
      strcpy(buffer, "interval_feeder ");
      for (int i = 0; i<INTV_FEEDER_SIZE; i++){
        dtostrf(interval_feeder[i], 5, 2, floatStr);
        sprintf(buffer2, "%s ", floatStr);
        strcat(buffer, buffer2);
      }
      Serial.println(buffer); 
    }
    //-------------------- Set ------------------
    else if(strcmp(inputCommand, "set") == 0){
      // read values from Serial (7 variables)
      readValuesFromSerial(7);
      
      buffer[0] = (char)0;
      sprintf(buffer, "recieved %*s", inputData);
      Serial.println(buffer);
      
      setCurrentTime(atoi(inputData[0]), atoi(inputData[1]), atoi(inputData[2]));
      status_mode = atoi(inputData[3]);
      status_light = atoi(inputData[4]);
      status_fan = atoi(inputData[5]);
      status_feeder = atoi(inputData[6]);
      criticalTemperature = atof(inputData[7]);
      
      Serial.println(status_mode);
      Serial.println(status_light);
      Serial.println(status_fan);
      Serial.println(status_feeder);
      Serial.println(criticalTemperature);
    }
    //-------------------- set light interval ------------------
    else if(strcmp(inputCommand, "set_light_interval") == 0){
      // read values from Serial
      readValuesFromSerial(INTV_LIGHT_SIZE + TIMEVAR_SIZE);
      
      setCurrentTime(atoi(inputData[0]), atoi(inputData[1]), atoi(inputData[2]));

      for(i = 0; i < INTV_LIGHT_SIZE; i++){
          interval_light[i] = atof(inputData[i+3]);
          Serial.println(interval_light[i]);
      }
    }
    //-------------------- set fan interval ------------------
    else if(strcmp(inputCommand, "set_fan_interval") == 0){
      // read values from Serial
      readValuesFromSerial(INTV_FAN_SIZE + TIMEVAR_SIZE);
      
      setCurrentTime(atoi(inputData[0]), atoi(inputData[1]), atoi(inputData[2]));

      for(i = 0; i < INTV_FAN_SIZE; i++){
          interval_fan[i] = atof(inputData[i+3]);
          Serial.println(interval_fan[i]);
      }
    }
    //-------------------- set feeder interval ------------------
    else if(strcmp(inputCommand, "set_feeder_interval") == 0){
      // read values from Serial
      readValuesFromSerial(INTV_FEEDER_SIZE + TIMEVAR_SIZE);

      setCurrentTime(atoi(inputData[0]), atoi(inputData[1]), atoi(inputData[2]));

      for(i = 0; i < INTV_FEEDER_SIZE; i++){
          interval_feeder[i] = atof(inputData[i+3]);
          Serial.println(interval_feeder[i]);
      }
    }
    //-------------------- sync_time ------------------
    else if(strcmp(inputCommand, "sync_time") == 0){
      // read values from Serial
      readValuesFromSerial(TIMEVAR_SIZE);
      
      setCurrentTime(atoi(inputData[0]), atoi(inputData[1]), atoi(inputData[2]));
    }
    //-------------------- get_time ------------------
    else if(strcmp(inputCommand, "get_time") == 0){
      // show current time
      Serial.println(currentTime);
    }
  }

  
  

  //check mode
  status_mode = btn_onRelease(PIN_BTN_MODE, status_mode);
  setMode(status_mode);

  // check button state
  if (status_mode == MANUAL){
    status_light = btn_onRelease(PIN_BTN_LIGHT, status_light);
    status_feeder = btn_onRelease(PIN_BTN_FEEDER, status_feeder);
    status_fan = btn_onRelease(PIN_BTN_FAN, status_fan);
  }

  else if (status_mode == AUTOMATIC){
    // ------------------  light interval  ------------------
    status_light = false;
    for(i=0;i<INTV_LIGHT_SIZE;i=i+2){
      // check if current time is in interval
      if(currentTime >= interval_light[i] && currentTime <= interval_light[i+1]){
        status_light = true;
        break;
      }
    }
    // ------------------  fan interval  ------------------
    if(status_temp_warning){
      status_fan = true;  // fan always active if temperature is critical 
    }
    else{
      status_fan = false;
      for(i=0;i<INTV_FAN_SIZE;i=i+2){
        // check if current time is in interval
        if(currentTime >= interval_fan[i] && currentTime <= interval_fan[i+1]){
          status_fan = true;
          break;
      }
    }
    }
    
    // ------------------  feeder interval  ------------------
    status_feeder = false;
    for(i=0;i<INTV_FEEDER_SIZE;i++){
      // check if current time is in interval (becouse of float the difference will be compared)
      if(abs(currentTime - interval_feeder[i]) < 0.01 && allowNextFeeding == true){
        status_feeder = true;
        allowNextFeeding = false;
        break;
      }
    }



  }
  
  // set status to actuatoor
  setStatus(PIN_AKT_LIGHT, status_light);
  setStatus(PIN_AKT_FAN, status_fan);
  setStatus(PIN_LED_TEMP_WARNING, status_temp_warning);
  //setStatus(PIN_AKT_FEEDER, status_feeder);
  if(status_feeder){
    once_feeding(FEEDER_DEBOUNCE_MS);
    // Feeder activation should hold the value for FEEDER_DEBOUNCE_MS
    //delay(FEEDER_DEBOUNCE_MS);
    status_feeder = false;
  }
  

  //reset string
  //inputCommand[0] = (char)0;
  memset(inputCommand, 0, sizeof(inputCommand));

  // sampling of 50ms
  delay(50);
}
