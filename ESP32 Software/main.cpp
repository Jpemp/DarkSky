#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wifi.h>
#include <time.h>

#include <string>
#include <cstdlib>

#define SCHEDULE_SIZE 5

using namespace std;

const char *wifi_ID = "NTGR_26A4_5G"; //set up with the NETGEAR router
const char *password = "wp2aVA7s";
const char *ntpServer = "pool.ntp.org"; //time server to connect to in order to get local time.
const char *compServerIP = "198.168.1.1";

static bool connectFlag = false; //indicate if the ESP32 client socket is connected to server socket
static char serverCommand[256] = ""; //used to store messages from server

WiFiClient client; //creates a client socket

TaskHandle_t connection_task; //allows for more effecient code by doing socket checking and socket handling by core 0, while the rest of the code is handled by core 1. ESP32 is a dual core system

static struct tm schedule_times[SCHEDULE_SIZE];
static int number_of_schedules = 5;
static int onTimeMin = 15;

static double utc_offset = -6*3600; //time displayed as military time. adjusted from greenwich mean time(utc) to central time(Texas time). Maybe include a function where this is adjustable?
static double daylight_savings = 3600; //account for daylight savings. Figure out how to change this when daylight savings is over. Include a feature where its adjustable?

static struct tm time_ESP32; //time struct to keep track of utc time on ESP32

static float fanOnTemp = 90.0; //temperature at which the fan turns on at
static float temp; //temperature read by sensor

//names of pins
const int tempSensor_input = 39; //A3 pin. Used for DS18B20 temp sensor data line input
const int fanPWM = 4; //A5 pin. Used to control fan speed with PWM signal
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
void duration_change();
void tm_print(int);

//flags to keep fan and recording device on regardless of conditional statement
static bool fanFlag = false;
static bool recordFlag = false;

static bool fanOn = false;
static bool recordOn = false;

//fan speed modes
static int offSpd = 0;
static int lowSpd = 63;
static int mediumSpd = 127; 
static int highSpd = 191;
static int maxSpd = 255;

OneWire oneWire(tempSensor_input); //GPIO 39/A3 is input for digital temp reader. Argument tells OneWire(DS18B20) which pin the temp sensor data line is going to 
DallasTemperature tempSensor(&oneWire); //passes GPIO address to DallasTemperature. Pointer parameter requires this. Address points to GPIO pin 34

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200); //serial communication to terminal

  tempSensor.begin(); //intializes the DS18B20 sensor
  
  //pin configuration
  pinMode(LED_BUILTIN, OUTPUT); //LED on the board. We can use this to indicate that the board is on
  pinMode(fan_power, OUTPUT); //Connect to fan relay (GPIO 27/A10 input on ADC2)
  pinMode(SQM_power, OUTPUT); //Connect to SQM relay (GPIO 15/A8 input on ADC2)
  pinMode(miniPC_power, OUTPUT); //Connect to MiniPC relay (GPIO 32/A7 on ADC1/32KHz crystal)
  pinMode(dewHeater_power, OUTPUT); //Connect to dew heater (GPIO 14/A6 on ADC2)
  pinMode(fanPWM, OUTPUT); //Connects fan PWM (fan speed output) to GPIO 4/A5
  pinMode(fanSpeed_input, INPUT); //Connects fan speed data read to A2/GPIO 34

  //pin initialization
  digitalWrite(LED_BUILTIN, 0); 
  digitalWrite(fan_power, 0); 
  digitalWrite(SQM_power, 0); 
  digitalWrite(miniPC_power, 0); 
  digitalWrite(dewHeater_power, 0); 
  analogWrite(fanPWM, lowSpd);


  
  WiFi_initializing(); //Connects ESP32 to WiFi

  configTime(utc_offset, daylight_savings, ntpServer); //Configuring ESP32 time module to npt_server

  tm_initialization();
  

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
void tm_initialization(void){ //finished
  schedule_times[0].tm_hour = 18;
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

void temp_check(void) { //finished
  tempSensor.requestTemperatures();
  temp = tempSensor.getTempFByIndex(0);

  Serial.print("Temperature: ");
  Serial.println(temp);

  if((temp>=fanOnTemp) || (fanFlag)){ //subject to change. This is to turn on fan if its too hot in the enclosure as determined by the DS18B20 sensor
    digitalWrite(fan_power, HIGH);
    fanOn = true;
  }
  else{
    digitalWrite(fan_power, LOW);
    fanOn = false;
  }
}

void time_check(void){ //finished
  int i;

  Serial.print("Time: ");
  getLocalTime(&time_ESP32);
  Serial.print(time_ESP32.tm_hour);
  Serial.print(":");
  Serial.print(time_ESP32.tm_min);
  Serial.print(":");
  Serial.println(time_ESP32.tm_sec);
  
  for(i=0; i<SCHEDULE_SIZE; i++){ //this method for turning on at a schedule can be improved
    if(((time_ESP32.tm_hour == schedule_times[i].tm_hour) && (time_ESP32.tm_min < onTimeMin)) || (recordFlag)){ //change this to account for minutes spilling over
      digitalWrite(SQM_power, HIGH);
      digitalWrite(dewHeater_power, HIGH);
      digitalWrite(miniPC_power, HIGH);
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

void WiFi_initializing(void){ //finished
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

void socket_connection(void *taskParamaters){ //finished
  //bool functionCall = false;
  int i = 0;
  while(true){ //infinite loop to run task in
    delay(1000); //1 second delay to prvent watchdog trigger
    while(!connectFlag){ //while ESP32 is disconnected from control center
      connectFlag = client.connect(compServerIP, 80);
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
      while(client.connected()){
        delay(1000);
        communication();
      }
    }

  }
}

void communication(void){ //finished
  Serial.println("COMMUNICATION");
  if (client.available()){
    serverCommand[0] = client.read();
    //Serial.println(serverCommand);
    control_menu(serverCommand);
  }
    
   
    memset(serverCommand, 0, sizeof(serverCommand)); //clears the serverCommand character array for the next use

  }

void control_menu(char* command){
  bool exitMenu = false;
  char exitCommand = ' ';
  Serial.println("control_menu called");
  Serial.println(command);
  switch(command[0]){
    case '0': //case 0 finished
      while(true){
        delay(1000);
        if(client.available()){
          exitCommand = client.read();
          client.flush();
        }
        if((exitCommand == '0') || (!client.connected())){
          break;
        }
        power_boolean_read();
        delay(1000);
        temp_read();
        delay(1000);
        time_read();
        fan_read();
        delay(1000);
      }
      break;
    case '1': //case 1 finished
      fanSpeedChange();
      //change fan speed
      break;
    case '2': //case 2 finished
      client.write(to_string(fanOnTemp).c_str());
      client.flush();
      while(true){
        delay(1000);
        if(client.available()){
          exitCommand = client.read();
          client.flush();
        }
        if((exitCommand =='0') || (!client.connected())){
          break;
        }
        if(exitCommand == '1'){
          tempChange();
        }
        exitCommand = ' ';
      }
      //change temp condition
      break;
    case '3':
      timeMenu();
      //change time schedule
      break;
    case '4': //case 4 finished
      power_menu();
      //go to power system menu
      break;
    default: //default case done
      Serial.println("Invalid Command. Please Try Again");
      break;
 
  }
  exitCommand = ' ';
  client.flush();
}

void temp_read(void){ //finished
  Serial.println("temp_read called");
  //Serial.println(to_string(temp).c_str());
  client.write(to_string(temp).c_str());
}

void time_read(void){ //finished
  Serial.println("time_read called");
  //Serial.println(asctime(&time_ESP32));
  client.write(asctime(&time_ESP32));
  
}

void fan_read(void){
  Serial.println("fan_read called");
  //Serial.println(to_string(analogRead(fanSpeed_input)).c_str());
  client.write(to_string(analogRead(fanSpeed_input)).c_str());
} 

void power_boolean_read(void){ //finished
  Serial.println("power_boolean_read called");
  client.write(to_string(recordOn).c_str());
  delay(1000);
  client.write(to_string(fanOn).c_str());
}

void tempChange(void){ //finished
  Serial.println("tempChange called");
  char tempChange[256] = "";
  int i=0;
  char changeCommand;
  bool exitMenu = false;
  //changeCommand = client.read();
  //Serial.println(changeCommand);

  //if(changeCommand == '1'){ //issue here
    Serial.println("ITS CALLED");
    client.flush();
    while(true){
      delay(1000);
      if(client.available()){
        while(client.available()){
            Serial.println(i);
            tempChange[i]=client.read();
            i++;
        }
        break;
      }
    }
    Serial.println(tempChange);
    fanOnTemp = atof(tempChange);
    client.write(to_string(fanOnTemp).c_str());
    memset(tempChange, 0, sizeof(tempChange));
    //}

  }

void fanSpeedChange(void){ //finished
  Serial.println("fanSpeedChange called");
  int i=0;
  bool exitLoop = false;
  char speedChange[1] = "";
  while (true){
    delay(1000); //prevents watchdog trigger
    
    if(client.connected()){
      speedChange[0] = client.read();
    }
    
    if(!client.connected() || (speedChange[0] == '0')){
      break;
    }
   
    else if (speedChange[0]=='1'){
      speedChange[0] = ' ';
      while(!exitLoop){
        delay(1000);
        speedChange[0] = client.read();
        Serial.println(speedChange);
        switch(speedChange[0]){
          case '0':
            exitLoop = true;
            Serial.println("Exiting Window");
            break;
          case '1':
            analogWrite(fanPWM, offSpd);
            break;
          case '2':
            analogWrite(fanPWM, lowSpd);
            break;
          case '3':
            analogWrite(fanPWM, mediumSpd);
            break;
          case '4':
            analogWrite(fanPWM, highSpd);
            break;
          case '5':
            analogWrite(fanPWM, maxSpd);
            break;
          default:
            break;
        }
        client.flush();
      }
    }
    else{
    }
    client.flush();
  }
  memset(speedChange, 0, sizeof(speedChange));
}

void timeMenu(void){ //still needs to be done
  Serial.println("timeMenu called");
  client.flush();
  char timeCommand;
  bool exitLoop = false;
  while(!exitLoop){
    delay(1000); //prevents watchdog trigger
    if(!client.connected()){
      break;
    }
    timeCommand = client.read();
    switch(timeCommand){
      case '0':
        exitLoop = true;
        break;
      case '1':
      if(number_of_schedules == 0){
      }
      else{
        time_remove();
      }
      break;
      case '2':
        if(number_of_schedules == 5){
          time_add();
        }
        else{
          time_add();
          
        }
        break;
      case '3':
        time_change();
        break;
      case '4':
        duration_change();
        break;
      default:
        break;
    }
    client.flush();
  }
}

void duration_change(void){ //finished
  Serial.println("duration_change called");
  client.flush();
  client.write(to_string(onTimeMin).c_str());
  int i=0;
  while(true){
    delay(1000);
    if(client.available()){
      while(client.available()){
        serverCommand[i]=client.read();
        i++;
      }
      break;
    }
  }
  onTimeMin = atoi(serverCommand);
  Serial.print("New Duration: ");
  Serial.println(onTimeMin);
  memset(serverCommand, 0, sizeof(serverCommand));
}

void time_add(void){
  Serial.println("time_add called");
  client.flush();
  tm_print(number_of_schedules);
  //client.write(asctime(schedule_times));
  char timeCommand[1] = ""; 
  int i = 0;
  while(true){
    delay(1000);
    if(client.available()){
      timeCommand[i] = client.read();
      break;
    }
  }
  
  while(true){
    delay(1000);
    if(client.available()){
      while(client.available()){
        serverCommand[i] = client.read();
        i++;
      }
      break;
    }
  }
  
  char *token_string = strtok(serverCommand, ":");
  schedule_times[atoi(timeCommand)].tm_hour = atoi(token_string); //change this to be adjusted for the element size
  
  token_string = strtok(NULL, ":"); 
  schedule_times[atoi(timeCommand)].tm_min = atoi(token_string);
  
  token_string = strtok(NULL, ":");
  schedule_times[atoi(timeCommand)].tm_sec = atoi(token_string);
  
  memset(serverCommand, 0, sizeof(serverCommand));
  number_of_schedules++;
}

void time_remove(void){
  Serial.println("time_remove called");
  char timeCommand[1];
  timeCommand[0] = client.read();

  int x=0;
  if(x==1){
    remove_array_entry(atoi(timeCommand));
    client.write("Entry ");
    client.write(timeCommand);
    client.write(" was successfully removed!");
  }
  else{
    client.write("Entry does not exist! Please try again!");
  }
  
  memset(serverCommand, 0, sizeof(serverCommand));
  number_of_schedules--;
}

void time_change(void){
  Serial.println("time_change called");
  char timeCommand[1];
  int i = 0;
  timeCommand[0] = client.read();
  while(client.available()){
    serverCommand[i] = client.read();
    i++;
  }

  char *token_string = strtok(serverCommand, ":");
  schedule_times[atoi(timeCommand)].tm_hour = atoi(token_string);
  
  token_string = strtok(NULL, ":"); 
  schedule_times[atoi(timeCommand)].tm_min = atoi(token_string);
  
  token_string = strtok(NULL, ":");
  schedule_times[atoi(timeCommand)].tm_sec = atoi(token_string);
  
  memset(serverCommand, 0, sizeof(serverCommand));
  timeMenu();
}

void tm_print(int i){
  
}

void remove_array_entry(int element){
  Serial.println("remove_array_entry is called");
  int arraySize = sizeof(schedule_times)/sizeof(schedule_times[0]);
  for(int i=element; i<arraySize; i++){
    schedule_times[i] = schedule_times[i+1];
  }
}

void power_menu(void){ //finished
  Serial.println("power_menu called");
  bool exitLoop = false;
  char power_command[1] = "";
  while(!exitLoop){
    delay(1000); //prevents watchdog trigger
    if(!client.connected()){
      break;
    }
    power_command[0] = client.read();
    Serial.println(power_command);
    switch(power_command[0]){
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
    client.flush();
  }
  memset(power_command, 0, sizeof(power_command));
}

void record_on(void){ //finished
  Serial.println("record_on called");
  recordFlag = true;
}

void fan_on(void){ //finished
  Serial.println("fan_on called");
  fanFlag = true;
}

void record_off(void){ //finished
  Serial.println("record_off called");
  recordFlag = false;
}

void fan_off(void){ //finished
  Serial.println("fan_off called");
  fanFlag = false;
}
