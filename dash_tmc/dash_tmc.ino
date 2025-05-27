#include <MobaTools.h>

const byte stepPin = 8;
const byte dirPin = 9;
const int ENABLE_PIN = 10;

const int motorStepsPerRev = 200; // Full steps per revolution
const int microsteps = 64;        // Microstepping setting (e.g., 16 for 1/16 microstepping)
const int stepsPerRev = motorStepsPerRev * microsteps;

MoToStepper stepper(stepsPerRev, STEPDIR);

#define Relay 5

// commands for arduino to operate. The structure is as follows: 1. Stepper motor, 2. Relay,
//String commands[] = {"STOP_MOTOR", "OFF"};
String inputString = "";
const char delimiter = ',';
const int maxCommands = 2;
String Commands[maxCommands] = {"STOP_MOTOR", "OFF"};

bool connected = 1;

void setup() {

    stepper.attach(stepPin, dirPin);
    //stepper.attachEnable(enablePin, 10, LOW); // 10ms delay, LOW to enable
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW); // Enable the driver (active-low)
    stepper.setSpeed(1);                     // Speed in RPM
    stepper.setRampLen(50);                   // Ramp length in steps
    stepper.setZero();                        // Set current position as zero

    pinMode(Relay, OUTPUT);

    Serial.begin(115200);
    //Serial.setTimeout(500);

    Serial.println("<OK>");
}

void loop() {
   if (Serial.available()) {
    inputString = Serial.readStringUntil('\n'); // Read until newline
    inputString.trim();
    if (inputString == "STATUS"){
      Status();
    }
    else if (inputString != Commands[0]+","+Commands[1]) {
      CommandParser(inputString, Commands);
      Serial.println("<"+Commands[0]+","+Commands[1]+">");
    }
    else{
    }
   }

    if (Commands[0] == "RUN_CLOCKWISE") {
        //Serial.println("Motor running clockwise.");
        RunClockwise();
        
      } else if (Commands[0] == "RUN_COUNTERCLOCKWISE") {
        //Serial.println("Motor running counterclockwise.");
        RunCounterClockwise();

      } else if (Commands[0] == "STOP_MOTOR"){
         //Serial.println("Motor stopped.");
         StopMotor();
         
      } else {
         //Serial.println("Unknown command.");
      }
   RelayControl(Commands[1]);

   
}


void RunClockwise(){
   digitalWrite(ENABLE_PIN, LOW);
   stepper.doSteps(40);
  //stepper.rotate(1); // Rotate clockwise indefinitely
   delay(500);
}

void RunCounterClockwise(){
   digitalWrite(ENABLE_PIN, LOW);
   stepper.doSteps(-40);
  //stepper.rotate(-1); // Rotate counter clockwise indefinitely
   delay(500);
}

void StopMotor(){
   digitalWrite(ENABLE_PIN, HIGH);
}


void RelayControl(String state){
   if (state=="ON"){
      digitalWrite(Relay, HIGH);
      //Serial.println("Relay is turned ON.\n");
   }

   else if (state=="OFF") {
      digitalWrite(Relay, LOW);
      //Serial.println("Relay is turned OFF.\n");
   }

   else {
      //Serial.println("Wrong parameter!\n");
   }
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
   Serial.println("<"+Commands[0]+","+Commands[1]+">");
}