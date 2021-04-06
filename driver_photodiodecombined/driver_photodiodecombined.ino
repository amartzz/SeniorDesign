#include <Wire.h>
#include <ADS1115_WE.h> 

 int sensorPin = A1;
int sensorValue = 0;
ADS1115_WE adc = ADS1115_WE();

void setup(void)
{
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  Serial.println("Getting single-ended readings from AIN0");
  Serial.println("ADC Range: +/- 3.3V (1 bit = 3mV)");
  
  read_pd();
}
 
void loop(void)
{
  // ambient reading
  
  Serial.println("ambient");
  read_pd();
  delay(2000);

  // green reading
  digitalWrite(5, HIGH);
  delay(2000);
  Serial.println("green");
  read_pd();
  delay(2000);
  digitalWrite(5, LOW);

  // red reading
  digitalWrite(4, HIGH);
  delay(1000);
  Serial.println("red");
  read_pd();
  delay(1000);
  digitalWrite(4, LOW);


  // pause
  Serial.println("------------------------");
  delay(2000);
}

float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage = adc.getResult_mV();
  return voltage; 
}

void read_pd(void)
{
  float voltage = 0.0;
  float total = 0.0;
  for(int i=0; i<30; i++){
    Serial.print("made it here");
    voltage = readChannel(ADS1115_COMP_0_GND);
    total += voltage;
    delay(5);
  }
  Serial.print("Average voltage: "); Serial.println(total/30);
  
}





  
