#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SolarCalculator.h>
#include <Wifi.h>
#include <time.h>
#include <stdlib.h>

#include <string>

using namespace std;

const char *wifi_ID = "GuysHouse";
const char *password = "Proverbs910";
const char *ntpServer = "pool.ntp.org";

double gmt = 0; //keep in universal time
double daylight_savings = 3600;

struct tm universal_time;


// put function declarations here:
void temp_read(void); //record temperature. If it is too hot, fan will turn on
void time_read(void);
void communication();
void WiFi_initializing(void);

OneWire oneWire(26); //GPIO 26/A0 is input for digital temp reader. Argument tells OneWire(DS18B20) which pin the reader info is going to 
DallasTemperature tempSensor(&oneWire); //passes GPIO address to DallasTemperature. Pointer parameter requires this. Address points to GPIO pin 34

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //serial communication to terminal

  //pin configuration
  pinMode(LED_BUILTIN, OUTPUT); //LED on the board. We can use this to indicate that the board is on
  pinMode(27, OUTPUT); //Connect to fan relay (GPIO 27/A10 input on ADC2)
  pinMode(15, OUTPUT); //Connect to SQM relay (GPIO 15/A8 input on ADC2)
  pinMode(32, OUTPUT); //Connect to MiniPC relay (GPIO 32/A7 on ADC1/32KHz crystal)
  pinMode(14, OUTPUT); //Connect to dew heater (GPIO 14/A6 on ADC2)

  WiFi_initializing();
  configTime(gmt, daylight_savings, ntpServer); //Configuring time

  //Serial.println(universal_time->tm_hour);

  tempSensor.begin(); //intializes the DS18B20 sensor 

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  //temp_read();
  //time_read();
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
  time_read();
  //std::time_t  universal_time = now();
  //if(universal_time == midnight) then call calcSunriseSunset funciton
  /*calcSunriseSunset(universal_time, lattitude, longitude, transit, sunrise, sunset);

  Serial.print("Sunrise: ");
  Serial.println(sunrise);

  Serial.print("Sunset: ");
  Serial.println(sunset);*/

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
      Serial.print("Connection is taking too long! Exiting program.");
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
