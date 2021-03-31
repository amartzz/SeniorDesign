#include <ArduinoLowPower.h>
int n=0;
void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(2, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
 Serial.begin(9600);
  digitalWrite(4, HIGH);
  delay(1000);
  digitalWrite(4, LOW);
  delay(1000);
  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(5, LOW);
  delay(1000);
  Serial.print("Asleep"); Serial.println(n);
   n=n+1;
   digitalWrite(2, HIGH);
  Serial.end();
  LowPower.sleep(5000);
  digitalWrite(2, LOW);
  //Serial.println("awake");
  
  
  
}
