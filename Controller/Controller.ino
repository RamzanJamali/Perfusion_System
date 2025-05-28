#include <MobaTools.h>
#include <Perfusion.h>

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
const int maxCommands = 2;
String Commands[maxCommands] = {"STOP_MOTOR", "OFF",};

// Create sensor object
DHT dht(DHTPIN, DHTTYPE);
String TempHumid[3];

void setup() {

   Serial.begin(115200);
   //Serial.setTimeout(500);
   perfusion.set_end_position(5000); // Set syringe end position
   Serial.println("<OK>");

   dht.begin();
}

void loop() {
   if (Serial.available()) {
    inputString = Serial.readStringUntil('\n'); // Read until newline
    inputString.trim();
    /*if (inputString == Commands[0]+","+Commands[1]){
      Status();
    }
    else if */
   if (inputString != Commands[0]+","+Commands[1]) {
      CommandParser(inputString, Commands);
      Status();
      //Serial.println("<"+Commands[0]+","+Commands[1]+">");
    }
    else{
      //Status();
    }
   }

// In next step, these all if conditions will be put in a separate file in a function and only the function will be called here.
   if (Commands[0] == "START_PERFUSION") {
      //Serial.println("Motor running clockwise.");
      //RunClockwise();
      perfusion.start_perfusion();
        
   } else if (Commands[0] == "PAUSE_PERFUSION") {
      //Serial.println("Motor running counterclockwise.");
      //RunCounterClockwise();
      perfusion.pause_perfusion();

   } else if (Commands[0] == "CONTINUE_PERFUSION"){
      //Serial.println("Motor stopped.");
      //StopMotor();
      perfusino.continue_perfusion();
         
   } else if (Commands[0] == "END_PERFUSION"{
      //Serial.println("Unknown command.");
      perfusion.end_perfusion();
   } else {

   }

   // Secondary commands
   if (Commands[1]== "SET_END_POSITION"){
      perfusion.set_end_position(4000);
   } else if (Commands[1] == "SET_PRESSURE"){
      perfusion.set_pressure(100);
   } else if (Commands[1] == "SET_SPEED"){
      perfusion.set_speed(2);
   } else if(Commands[1] == "SET_FLOW_RATE"){
      perfusion.set_flow_rate(5);
   } else {

   }

   
   

   ReadTemperatureHumidity(TempHumid);
   Status();
   perfusion.update_data(data);
   delay(500);
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
   Serial.println("<"+Commands[0]+", " +Commands[1]+", Humidity:" + TempHumid[0]+"%"+", Temperature:" +TempHumid[1]+" °C / "+"," +TempHumid[2]+" °F" +">");
}

void ReadTemperatureHumidity(String *TempHumid){
   // Read humidity (percent)
  float humidity = dht.readHumidity();
  // Read temperature in Celsius
  float temperature = dht.readTemperature();
  // Optionally read temperature in Fahrenheit
  float temperatureF = dht.readTemperature(true);

  // Check for failed reads
  if (isnan(humidity) || isnan(temperature)) {
   humidity =0; temperature=0; temperatureF=0;
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
   TempHumid[0] = humidity; TempHumid[1]=temperature; TempHumid[2]=temperatureF;

}