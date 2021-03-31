#include <Wire.h>
#include <Adafruit_ADS1015.h>
 
Adafruit_ADS1015 ads1015;

int sensorPin = A1;
int sensorValue = 0;

void setup(void)
{
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  Serial.println("Getting single-ended readings from AIN0");
  Serial.println("ADC Range: +/- 3.3V (1 bit = 3mV)");
  ads1015.begin();
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

void read_pd(void)
{
  int16_t adc0, adc_voltage;
 
  adc0 = ads1015.readADC_SingleEnded(0);
  adc_voltage = 3 * adc0;

  Serial.print("AIN0: "); Serial.println(adc0);
  Serial.print(adc_voltage); Serial.println(" mV");
  Serial.println(" ");
}
