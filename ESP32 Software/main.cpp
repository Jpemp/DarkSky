#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wifi.h>
#include <time.h>

#include <string>

using namespace std;

const char *wifi_ID = "GuysHouse";
const char *password = "Proverbs910";
//const char *wifi_ID = "NTGR_26A4_5G"; //set up with the NETGEAR router
//const char *password = "wp2aVA7s";
const char *ntpServer = "pool.ntp.org"; //time server to connect to in order to get local time.
const char *compServerIP = "192.168.1.180";

static bool connectFlag = false; //indicate if the ESP32 client socket is connected to server socket
static char serverCommand[250] = ""; //used to store messages from server

WiFiClient client; //creates a client socket

TaskHandle_t connection_task; //allows for more effecient code by doing socket checking and socket handling by core 0, while the rest of the code is handled by core 1. ESP32 is a dual core system

static int schedule_times[5] = {12, 23, 0, 1, 2};
static int number_of_schedules = sizeof(schedule_times)/sizeof(schedule_times[0]);

static double utc_offset = -6*3600; //time displayed as military time. adjusted from greenwich mean(utc) time to central time(Texas time). Maybe include a function where this is adjustable?
static double daylight_savings = 3600; //account for daylight savings. Figure out how to change this when daylight savings is over. Include a feature where its adjustable?

static struct tm time_ESP32; //time struct to keep track of utc time on ESP32

static float fanOnTemp = 90.0;

//names of pins
const int tempSensor_input = 26; //A0 pin. Used for DS18B20 temp sensor data line input
const int fanPWM = 21; //21 pin. Used to control fan speed with PWM signal
const int fanSpeed_input= 25; //A1 pin. Used to monitor fan speed
const int fan_power = 27; //27 pin. Used for fan power signal to relay switch
const int SQM_power = 15; //15 pin. Used for SQM power signal to relay switch
const int miniPC_power = 32; //32 pin. Used for mini-PC power signal to relay switch
const int dewHeater_power = 14; //14 pin. Used for dew heater power signal to relay switch 


// put function declarations here:
void temp_read(); //record internal temperature of enclosure using DS18B20 temp sensor. If it is too hot, fan will turn on
void time_read(); //take time from NTP server and turn on some devices according to a time schedule
void fan_read(); //reads the speed of the fan
void WiFi_initializing(); //connects ESP32 to the NETGEAR WiFi
void socket_connection(void*); //socket connection from ESP32 to a remote computer. Need to figure out how to connect between two networks.
void communication();
void fanSpeedChange();
void timeChange();
void tempChange();

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

  WiFi_initializing(); //Connects ESP32 to WiFi

  configTime(utc_offset, daylight_savings, ntpServer); //Configuring ESP32 time module to npt_server
  
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

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(LED_BUILTIN, HIGH);
  
  if(!connectFlag){
    time_read(); //read the time
    temp_read(); //read internal temperature
  }
  else{

  }

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
void temp_read(void) {
  float temp;
  tempSensor.requestTemperatures();
  temp = tempSensor.getTempFByIndex(0);

  Serial.print("Temperature: ");
  Serial.println(temp);

  if(temp>=fanOnTemp){ //subject to change. This is to turn on fan if its too hot in the enclosure as determined by the DS18B20 sensor
    digitalWrite(fan_power, LOW);
  }
  else{
    digitalWrite(fan_power, HIGH);
  }
}

void time_read(void){
  int i;

  Serial.print("Time: ");
  getLocalTime(&time_ESP32);
  Serial.print(time_ESP32.tm_hour);
  Serial.print(":");
  Serial.print(time_ESP32.tm_min);
  Serial.print(":");
  Serial.println(time_ESP32.tm_sec);
  
  for(i=0; i<number_of_schedules; i++){ //this method for turning on at a schedule can be improved
    if((time_ESP32.tm_hour == schedule_times[i]) && (time_ESP32.tm_min < 30)){
      digitalWrite(SQM_power, HIGH);
      digitalWrite(dewHeater_power, HIGH);
      digitalWrite(miniPC_power, HIGH);
      break;
    }
    else{
      digitalWrite(SQM_power, LOW);
      digitalWrite(dewHeater_power, LOW);
      digitalWrite(miniPC_power, LOW);
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
  bool functionCall = false;
  for(;;){ //infinite loop to run task in
    delay(1000); //1 second delay to prvent watchdog trigger
    while(!connectFlag){ //while ESP32 is disconnected from control center
      connectFlag = client.connect(compServerIP, 8080);
      if(!connectFlag){
        Serial.println("No connection!");
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
    
    /*while(connectFlag){ //while ESP32 is connected to control center
      connectFlag = client.connected();
      //Serial.println(functionCall);
      if(!connectFlag){
        Serial.println("Computer has disconnected from ESP32!");
        client.stop(); //ends socket connection
        //functionCall=false;
      }
      else if (connectFlag && !functionCall){
        //communication(); //issue, make this function call only once. Task keeps going even though its in the middle of a function call, causing repeat calls
        functionCall=true;
      }
      else{
        //functionCall=false;
      }
    }*/
  }
}

void communication(){
  int i = 0;
  while(client.available()){
    //Serial.println(i);
    serverCommand[i] = client.read();
    i++;
  }
  //Serial.println(serverCommand);
  //client.write("Hello from Client");

}

