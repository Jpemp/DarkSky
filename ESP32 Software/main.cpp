#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wifi.h>
#include <time.h>
#include <stdlib.h>

using namespace std;

const char *wifi_ID = "GuysHouse"; //set up with NETGEAR router
const char *password = "Proverbs910";
const char *ntpServer = "pool.ntp.org";

int schedule_time;

double utc = -6*3600; //time displayed as universal time. adjusted from greenwich mean time
double daylight_savings = 3600; //account for daylight savings. Figure out how to change this when daylight savings is over 

struct tm universal_time; //time struct to keep track of utc time on ESP32

static int tempSensor_input = 26;
static int fan_power = 27;
static int SQM_power = 15;
static int miniPC_power = 32;
static int dewHeater_power = 14;


// put function declarations here:
void temp_read(void); //record temperature. If it is too hot, fan will turn on
void time_read(void);
void communication();
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
  //socket_setup();
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
  delay(500);
}

// put function definitions here:
void temp_read(void) {
  float temp;
  tempSensor.requestTemperatures();
  temp = tempSensor.getTempFByIndex(0);

  Serial.print("Temperature: ");
  Serial.println(temp);

  if(temp>=90){ //subject to change. This is to turn on fan if its too hot
    digitalWrite(27, LOW);
  }
  else{
    digitalWrite(27, HIGH);
  }
}

void time_read(void){
  getLocalTime(&universal_time);
  Serial.print(universal_time.tm_hour);
  Serial.print(":");
  Serial.print(universal_time.tm_min);
  Serial.print(":");
  Serial.println(universal_time.tm_sec);
  if(universal_time.tm_hour == (22||0||2)){

  }
}

void WiFi_initializing(void){
  int fail_count = 0;

  WiFi.begin(wifi_ID, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
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
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);
  digitalWrite(LED_BUILTIN, LOW);
}

void socket_setup(void){

}
