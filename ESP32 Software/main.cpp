#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wifi.h>
#include <SPI.h>
#include <time.h>
#include <stdlib.h>

using namespace std;

const char *wifi_ID = "GuysHouse"; //set up with the NETGEAR router
const char *password = "Proverbs910";
const char *ntpServer = "pool.ntp.org";

static int schedule_times[5] = {12, 23, 0, 1, 2};
static int number_of_schedules = sizeof(schedule_times)/sizeof(schedule_times[0]);

double utc = -6*3600; //time displayed as military time. adjusted from greenwich mean time to central time. Maybe include a function where this is adjustable?
double daylight_savings = 3600; //account for daylight savings. Figure out how to change this when daylight savings is over. Include a feature where its adjustable?

static struct tm universal_time; //time struct to keep track of utc time on ESP32

WiFiClient client;

const int tempSensor_input = 26; //names for pins 
const int fan_power = 27;
const int SQM_power = 15;
const int miniPC_power = 32;
const int dewHeater_power = 14;

// put function declarations here:
void temp_read(void); //record temperature. If it is too hot, fan will turn on
void time_read(void);
void communication(void);
void WiFi_initializing(void);
void socket_setup(void);

OneWire oneWire(tempSensor_input); //GPIO 26/A0 is input for digital temp reader. Argument tells OneWire(DS18B20) which pin the reader info is going to 
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
  socket_setup(); //Connect ESP32 to remote WiFi
  configTime(utc, daylight_savings, ntpServer); //Configuring time module to npt_server

  tempSensor.begin(); //intializes the DS18B20 sensor 

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  //temp_read();
  time_read();
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

  if(temp>=90){ //subject to change. This is to turn on fan if its too hot in the enclosure as determined by the DS18B20 sensor
    digitalWrite(fan_power, LOW);
  }
  else{
    digitalWrite(fan_power, HIGH);
  }
}

void time_read(void){
  int i;

  getLocalTime(&universal_time);
  Serial.print(universal_time.tm_hour);
  Serial.print(":");
  Serial.print(universal_time.tm_min);
  Serial.print(":");
  Serial.println(universal_time.tm_sec);
  
  for(i=0; i<number_of_schedules; i++){ //this method for turning on at a schedule can be improved
    if((universal_time.tm_hour == schedule_times[i]) && (universal_time.tm_min < 25)){
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

void WiFi_initializing(void){
  int fail_count = 0;

  WiFi.begin(wifi_ID, password);
  Serial.print("Connecting to ");
  Serial.print(wifi_ID);
  Serial.print("...");
  while (WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    delay(250);
    if(fail_count >= 20){
      Serial.print("WiFi connection is taking too long! Exiting program.");
      exit(1); //if it's taking too long to connect to WiFi, end program
    }
    else{
      fail_count++;
    }
  }
  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);
  digitalWrite(LED_BUILTIN, LOW);
}

void socket_setup(void){
  Serial.println("Connecting client to server...");
  if(client.connect("google.com", 80)){
    Serial.println("Client has connected!");
  }
  else{
    Serial.println("Client connection has failed!");
    exit(1);
  }

}
