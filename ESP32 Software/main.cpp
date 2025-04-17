#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wifi.h>
#include <time.h>

#include <string>
#include <cstdlib>

#define SCHEDULE_SIZE 5


using namespace std;

const char *wifi_ID = "GuysHouse";
const char *password = "Proverbs910";
//const char *wifi_ID = "NTGR_26A4_5G"; //set up with the NETGEAR router
//const char *password = "wp2aVA7s";
const char *ntpServer = "pool.ntp.org"; //time server to connect to in order to get local time.
const char *compServerIP = "192.168.1.180";

static bool connectFlag = false; //indicate if the ESP32 client socket is connected to server socket
static char serverCommand[256] = ""; //used to store messages from server

WiFiClient client; //creates a client socket

TaskHandle_t connection_task; //allows for more effecient code by doing socket checking and socket handling by core 0, while the rest of the code is handled by core 1. ESP32 is a dual core system

static struct tm schedule_times[SCHEDULE_SIZE];
static int number_of_schedules = sizeof(schedule_times)/sizeof(schedule_times[0]);
static int onTimeMin = 15;

static double utc_offset = -6*3600; //time displayed as military time. adjusted from greenwich mean time(utc) to central time(Texas time). Maybe include a function where this is adjustable?
static double daylight_savings = 3600; //account for daylight savings. Figure out how to change this when daylight savings is over. Include a feature where its adjustable?

static struct tm time_ESP32; //time struct to keep track of utc time on ESP32

static float fanOnTemp = 90.0; //temperature at which the fan turns on at
static float temp; //temperature read by sensor

//names of pins
const int tempSensor_input = 26; //A0 pin. Used for DS18B20 temp sensor data line input
const int fanPWM = 21; //21 pin. Used to control fan speed with PWM signal
const int fanSpeed_input = 34; //A2 pin. Used to monitor fan speed
const int fan_power = 27; //27 pin. Used for fan power signal to relay switch
const int SQM_power = 15; //15 pin. Used for SQM power signal to relay switch
const int miniPC_power = 32; //32 pin. Used for mini-PC power signal to relay switch
const int dewHeater_power = 14; //14 pin. Used for dew heater power signal to relay switch 


// put function declarations here:
void temp_read(); //record internal temperature of enclosure using DS18B20 temp sensor.
void time_read(); //take time from NTP server.
void fan_read(); //reads the speed of the fan
void power_boolean_read(); //reads if the power is supplied to recording device and/or fan
void WiFi_initializing(); //connects ESP32 to the NETGEAR WiFi
void socket_connection(void*); //socket connection from ESP32 to a remote computer. Need to figure out how to connect between two networks.
void communication();
void control_menu(char*);
void power_menu();
void fanSpeedChange();
void timeMenu();
void tempChange();
void time_check(); //turn on recording device at the scheduled time
void temp_check(); //turn on fan if it is too hot
void record_on();
void fan_on();
void record_off();
void fan_off();
void time_add();
void time_remove();
void time_change();
void remove_array_entry(int);
void tm_initialization();

//flags to keep fan and recording device on regardless of conditional statement
static bool fanFlag = false;
static bool recordFlag = false;
//
static bool fanOn = false;
static bool recordOn = false;


//fan speed modes
static int offSpd = 0;
static int lowSpd = 63;
static int mediumSpd = 127; 
static int highSpd = 191;
static int maxSpd = 255;

OneWire oneWire(tempSensor_input); //GPIO 26/A0 is input for digital temp reader. Argument tells OneWire(DS18B20) which pin the temp sensor data line is going to 
DallasTemperature tempSensor(&oneWire); //passes GPIO address to DallasTemperature. Pointer parameter requires this. Address points to GPIO pin 34

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200); //serial communication to terminal

  //pin configuration
  pinMode(LED_BUILTIN, OUTPUT); //LED on the board. We can use this to indicate that the board is on
  pinMode(fan_power, OUTPUT); //Connect to fan relay (GPIO 27/A10 input on ADC2)
  pinMode(SQM_power, OUTPUT); //Connect to SQM relay (GPIO 15/A8 input on ADC2)
  pinMode(miniPC_power, OUTPUT); //Connect to MiniPC relay (GPIO 32/A7 on ADC1/32KHz crystal)
  pinMode(dewHeater_power, OUTPUT); //Connect to dew heater (GPIO 14/A6 on ADC2)
  pinMode(fanPWM, OUTPUT); //Connects fan PWM (fan speed output) to GPIO 21
  pinMode(fanSpeed_input, INPUT); //Connects fan speed data read to A2/GPIO 34

  //pin initialization
  digitalWrite(LED_BUILTIN, 0); 
  digitalWrite(fan_power, 1); 
  digitalWrite(SQM_power, 1); 
  digitalWrite(miniPC_power, 1); 
  digitalWrite(dewHeater_power, 1); 
  analogWrite(fanPWM, mediumSpd);
  
  WiFi_initializing(); //Connects ESP32 to WiFi

  configTime(utc_offset, daylight_savings, ntpServer); //Configuring ESP32 time module to npt_server

  tm_initialization();
  
  tempSensor.begin(); //intializes the DS18B20 sensor

  xTaskCreatePinnedToCore(
    socket_connection,      //code for task
    "SocketConnectionTask", //name of task
    10000,                 //stack size of task
    NULL,                   //put input paramaters here
    1,                      //task priority
    &connection_task,       //the struct which the xTaskCreatePinnedToCore information is passed to
    0                       //Which core the task will run in
  );//Sets up the task assignment to the core

}

void loop() { //NOTE: everything else besides the task is being ran on Core 1 I think
  // put your main code here, to run repeatedly:

  digitalWrite(LED_BUILTIN, HIGH);
  
  time_check(); //read the time
  temp_check(); //read internal temperature


  //digitalWrite(27, LOW);
  //digitalWrite(15, LOW);
  //digitalWrite(32, LOW);
  //digitalWrite(14, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);

  //digitalWrite(27, HIGH);
  //digitalWrite(15, HIGH);
  //digitalWrite(32, HIGH);
  //digitalWrite(14, HIGH);
  delay(500); //delays translate to a 1 second clock update (500ms + 500ms = 1s )
}

// put function definitions here:
void tm_initialization(void){
  schedule_times[0].tm_hour = 22;
  schedule_times[0].tm_min = 0;
  schedule_times[0].tm_sec = 0;

  schedule_times[1].tm_hour = 23;
  schedule_times[1].tm_min = 0;
  schedule_times[1].tm_sec = 0;

  schedule_times[2].tm_hour = 0;
  schedule_times[2].tm_min = 0;
  schedule_times[2].tm_sec = 0;

  schedule_times[3].tm_hour = 1;
  schedule_times[3].tm_min = 0;
  schedule_times[3].tm_sec = 0;

  schedule_times[4].tm_hour = 2;
  schedule_times[4].tm_min = 0;
  schedule_times[4].tm_sec = 0;
}

void temp_check(void) {
  tempSensor.requestTemperatures();
  temp = tempSensor.getTempFByIndex(0);

  Serial.print("Temperature: ");
  Serial.println(temp);

  if((temp>=fanOnTemp) || (fanFlag)){ //subject to change. This is to turn on fan if its too hot in the enclosure as determined by the DS18B20 sensor
    digitalWrite(fan_power, LOW);
    fanOn = true;
  }
  else{
    digitalWrite(fan_power, HIGH);
    fanOn = false;
  }
}

void time_check(void){
  int i;

  Serial.print("Time: ");
  getLocalTime(&time_ESP32);
  Serial.print(time_ESP32.tm_hour);
  Serial.print(":");
  Serial.print(time_ESP32.tm_min);
  Serial.print(":");
  Serial.println(time_ESP32.tm_sec);
  
  for(i=0; i<SCHEDULE_SIZE; i++){ //this method for turning on at a schedule can be improved
    if(((time_ESP32.tm_hour == schedule_times[i]) && (time_ESP32.tm_min < onTime)) || (recordFlag)){ //change this to account for minutes
      digitalWrite(SQM_power, LOW);
      digitalWrite(dewHeater_power, LOW);
      digitalWrite(miniPC_power, LOW);
      recordOn = true;
      break;
    }
    else{
      digitalWrite(SQM_power, LOW);
      digitalWrite(dewHeater_power, LOW);
      digitalWrite(miniPC_power, LOW);
      recordOn = false;
    }
  }
}

void WiFi_initializing(void){ //ensure esp32 is a client/station mode
  int fail_count = 0;

  WiFi.begin(wifi_ID, password);
  Serial.print("Connecting to ");
  Serial.print(wifi_ID);
  Serial.print("...");
  while (WiFi.status()!=WL_CONNECTED){
    if(fail_count >= 20){
      Serial.println("");
      Serial.print("WiFi connection is taking too long! Please look at WiFi connection! Retrying program...");
      delay(10000);
      abort(); //if it's taking too long to connect to WiFi, end program
    }
    else{
      fail_count++;
    }

    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }

  Serial.println();
  Serial.println("Connected!");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(3000);
}

void socket_connection(void *taskParamaters){
  //bool functionCall = false;
  int i = 0;
  while(true){ //infinite loop to run task in
    delay(1000); //1 second delay to prvent watchdog trigger
    while(!connectFlag){ //while ESP32 is disconnected from control center
      connectFlag = client.connect(compServerIP, 8080);
      if(!connectFlag){
        Serial.println("No server connection!");
      }

      else{
        Serial.println("ESP32 client has connected to a server!");
      }
    }

    connectFlag = client.connected();
    if(!connectFlag){
      Serial.println("Computer has disconnected from ESP32!");
      client.stop(); //ends socket connection
    }

    else{
      Serial.println("Communicating with Server...");
      communication();
    }

  }
}

void communication(void){
  int i = 0;
  if (client.available()){
    while(client.available()){
      //Serial.println(i);
      serverCommand[i] = client.read();
      i++;
    }
    Serial.println(serverCommand);
    control_menu(serverCommand);
    memset(serverCommand, 0, sizeof(serverCommand)); //clears the serverCommand character array for the next use

  }

}

void control_menu(char* command){
  Serial.println("control_menu called");
  switch(command[0]){
    case '0':
      while(true){
        if((client.read()=='0') || (!client.connected())){
          break;
        }
        power_boolean_read();
        temp_read();
        time_read();
        fan_read();
        delay(1000);
      }
      break;
    case '1':
      fanSpeedChange();
      //change fan speed
      break;
    case '2':
      tempChange();
      //change temp condition
      break;
    case '3':
      timeChange();
      //change time schedule
      break;
    case '4':
      power_menu();
      //go to power system menu
      break;
    default:
      Serial.println("Invalid Command. Please Try Again");
      break;
 
  }
}

void power_menu(void){
  Serial.println("power_menu called");
  bool exitLoop = false;
  char power_command;
  while(!exitLoop){
    power_command = client.read();
    switch(power_command){
      case '0':
        exitLoop = true;
        break;
      case '1':
        record_on();
        break;
      case '2':
        record_off();
        break;
      case '3':
        fan_on();
        break;
      case '4':
        fan_off();
        break;
      default:
        break;
  }
}
}

void temp_read(void){
  Serial.println("temp_read called");
  //Serial.println(to_string(temp).c_str());
  client.write(to_string(temp).c_str());
}

void time_read(void){
  Serial.println("time_read called");
  //Serial.println(asctime(&time_ESP32));
  client.write(asctime(&time_ESP32));
  
}

void fan_read(void){
  Serial.println("fan_read called");
  //Serial.println(to_string(analogRead(fanSpeed_input)).c_str());
  client.write(to_string(analogRead(fanSpeed_input)).c_str());
} 

void power_boolean_read(void){
  Serial.println("power_boolean_read called");
  client.write(recordOn);
  client.write(fanOn);
}

void fanSpeedChange(void){
  Serial.println("fanSpeedChange called");
  int i=0;
  bool exitFlag = false;
  char speedChange;
  while (!exitFlag){
    speedChange = client.read();
    switch(speedChange){
      case '0':
        analogWrite(fanPWM, offSpd);
        break;
      case '1':
        analogWrite(fanPWM, lowSpd);
        break;
      case '2':
        analogWrite(fanPWM, mediumSpd);
        break;
      case '3':
        analogWrite(fanPWM, highSpd);
        break;
      case '4':
        analogWrite(fanPWM, maxSpd);
        break;
      default:
        boolFlag = true;
        Serial.println("Exiting Window");
        break;
  }
  //memset(serverCommand, 0, sizeof(serverCommand));
}

void timeMenu(void){ //still needs to be done
  Serial.println("timeChange called");
  char timeCommand;
  timeCommand = client.read();
  switch(timeCommand){
    case '0':
      time_add();
    case '1':
      time_remove();
    case '2':
      time_change();
    default:
      break;
  }

}

void time_add(void){
  Serial.println("time_add called");
  int i = 0;
  while(client.available()){
    serverCommand = client.read();
    i++;
  }
  
  memset(serverCommand, 0, sizeof(serverCommand));
  timeMenu();
}

void time_remove(void){
  Serial.println("time_remove called");
  char timeCommand;
  timeCommand = client.read();

  if(){
    remove_array_entry(atoi(timeCommand));
    client.write("Entry ");
    client.write(timeCommand);
    client.write(" was successfully removed!");
  }
  else{
    client.write("Entry does not exist! Please try again!");
  }
  
  memset(serverCommand, 0, sizeof(serverCommand));
  timeMenu();
}

void time_change(void){
  Serial.println("time_change called");
  char timeCommand;
  int i = 0;
  timeCommand = client.read();
  
  while(client.available()){
  
  }
  memset(serverCommand, 0, sizeof(serverCommand));
  timeMenu();
}

void remove_array_entry(int element){
  Serial.println("remove_array_entry is called");
  int arraySize = sizeof(schedule_times)/sizeof(schedule_times[0]);
  for(int i=element; i<arraySize; i++){
    schedule_times[i] = schedule_times[i+1];
  }
}

void tempChange(void){
  Serial.println("tempChange called");
  char tempChange[20];
  int i=0;
  while(client.available()){
    tempChange[i]=client.read();
    i++;
  }
  fanOnTemp = atof(tempChange);
  Serial.println(fanOnTemp);
  //memset(tempChange, 0, sizeof(tempChange));
}

void record_on(void){
  Serial.println("record_on called");
  recordFlag = true;
}

void fan_on(void){
  Serial.println("fan_on called");
  fanFlag = true;
}

void record_off(void){
  Serial.println("record_off called");
  recordFlag = false;
}

void fan_off(void){
  Serial.println("fan_off called");
  fanFlag = false;
}
