#include <MobaTools.h>
#include "Perfusion.h"
#include "ReadTemperatureHumidity.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// —– Configuration of sensor —–
#define DHTPIN     2     // Digital pin connected to the DATA line
#define DHTTYPE    DHT22 // DHT 22 (AM2302), AM2321


// Hardware pins
const byte stepPin = 8;
const byte dirPin = 9;
const byte enablePin = 10;
const byte valvePin = 5;

// Create perfusion system
Perfusion perfusion(stepPin, dirPin, enablePin, valvePin, 
                    1.5, 120, 200, 64); // Target pressure 1.5, initial speed 120


// commands for arduino to operate. The structure is as follows: 1. Stepper motor, 2. Relay,
//String commands[] = {"STOP_MOTOR", "OFF"};
String inputString = "";
const char delimiter = ',';
const int maxCommands = 3;
String Commands[maxCommands] = {"IDLE", "5", "1.736"}; // {perfusion_state, pressure, flow_rate}


String result;

void setup() {

   Serial.begin(115200);
   //Serial.setTimeout(500);
   perfusion.set_end_position(5000); // Set syringe end position
   Serial.println("<OK>");

   
}

void loop() {
   if (Serial.available()) {
    inputString = Serial.readStringUntil('\n'); // Read until newline
    inputString.trim();
    /*if (inputString == Commands[0]+","+Commands[1]){
      Status();
    }
    else if */
   if (inputString != Commands[0]+","+Commands[1]+","+Commands[2]) {
      CommandParser(inputString, Commands);
      //Status();
      //Serial.println("<"+Commands[0]+","+Commands[1]+">");
    }
    else{
      //Status();
    }
   }

// In next step, these all if conditions will be put in a separate file in a function and only the function will be called here.
   if (Commands[0] == "START_PERFUSION") {
      perfusion.start_perfusion();
        
   } else if (Commands[0] == "PAUSE_PERFUSION") {
      perfusion.pause_perfusion();

   } else if (Commands[0] == "CONTINUE_PERFUSION"){
      perfusion.continue_perfusion();
         
   } else if (Commands[0] == "END_PERFUSION") {
      perfusion.end_perfusion();
   } else {

   }

   // Secondary commands
   if (Commands[1] != "5"){
      perfusion.set_pressure(Commands[1].toFloat());
   //} else if (Commands[1] == "1.74"){
      //perfusion.set_speed(2);
   } else if(Commands[2] == "1.74"){
      perfusion.set_flow_rate(Commands[2].toFloat());
   } else {

   }

   
   

   result = ReadTempHumidity(DHTPIN, DHTTYPE);
   Status();
   //perfusion.update_data(data);
   delay(500);
}


void CommandParser(String inputString, String *Commands) {
   int index = 0;

   while (inputString.length() > 0 && index < maxCommands) {
      int delimiterIndex = inputString.indexOf(delimiter);
      if (delimiterIndex == -1) {
        Commands[index++] = inputString;
        break;
      } else {
        Commands[index++] = inputString.substring(0, delimiterIndex);
        inputString = inputString.substring(delimiterIndex + 1);
      }
    }
}


void Status(){
   Serial.println("<"+Commands[0]+", " +Commands[1]+", " + result+">");
}
