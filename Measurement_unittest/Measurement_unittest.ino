#include <Wire.h>
#include <ADS1115_WE.h> 
#include <ArduinoLowPower.h>
#include <WiFiNINA.h>
#include <WiFiSSLClient.h>
#include "ThingSpeak.h"


ADS1115_WE adc = ADS1115_WE();

int del = 500;
int snr_threshold = -1;
int time_scale = 20;
float STD = 0.0;

int greenLEDmosfet = 5;
int redLEDmosfet = 4;

void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);
  Wire.begin();
  pinMode(greenLEDmosfet, OUTPUT);
  pinMode(redLEDmosfet, OUTPUT);
  Serial.println("Getting single-ended readings from AIN0");
  Serial.println("ADC Range: +/- 3.3V (1 bit = 3mV)");
}

void loop() {
  // put your main code here, to run repeatedly:
  measure_data();
}

float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage = adc.getResult_mV();
  return voltage; 
}

float avg_ambient(){
  // averages 30 readings in ambient light
  float voltage = 0.0;
  float total = 0.0;
  for(int i=0; i<30; i++){
    voltage = readChannel(ADS1115_COMP_0_GND);
    total += voltage;
    delay(5);
  }
  Serial.print("Average voltage: "); Serial.println(total/30);
  return total/30;
}







float thirty_LED(float ambient){
  float voltage = 0.0;
  float total_voltage = 0.0;
  float snr = 0.0;
  float total_snr = 0.0;
  float total_sqr = 0.0;
  float average_snr = 0.0;

  for(int i = 0; i < 30; i++){
    voltage = readChannel(ADS1115_COMP_0_GND);
    total_voltage += voltage;
    snr = 20*log10(voltage/ambient);

    total_snr += snr;
    total_sqr += snr*snr;
    delay(20);
  }
  delay(100);

  average_snr = total_snr/30;
  STD = sqrt(abs((total_sqr/30) - average_snr*average_snr));
  Serial.print("Average voltage: "); Serial.println(total_voltage/30);
  Serial.print("Average SNR: "); Serial.println(average_snr);
  Serial.print("STD: "); Serial.println(STD);
  Serial.println();
  return average_snr;
}



void measure_data() { 
  float ambient = 0.0;
  float green_avg_snr = 0.0;
  float green_STD = 0.0;
  float red_avg_snr = 0.0;
  float red_STD = 0.0;

  // ambient reading
  Serial.println("ambient");
  ambient = avg_ambient();

  // green reading
  digitalWrite(greenLEDmosfet, HIGH);
  delay(del);
  Serial.println("green");
  green_avg_snr = thirty_LED(ambient);
  green_STD = STD; 
  delay(del);
  digitalWrite(greenLEDmosfet, LOW);
  Serial.println("average SNR: ");
  Serial.print(green_avg_snr);

  // red reading
  digitalWrite(redLEDmosfet, HIGH);
  delay(del);
  Serial.println("red");
  red_avg_snr = thirty_LED(ambient);
  red_STD = STD; 
  delay(del);
  digitalWrite(redLEDmosfet, LOW);
  Serial.println("average SNR: ");
  Serial.print(red_avg_snr);

  }
