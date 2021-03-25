
int sensorPin = A1;
int sensorValue = 0;
float voltageValue= 0;


void setup(void) {
   Serial.begin(9600);
   pinMode(sensorPin, INPUT); 
}

float conversion(int aVal) {
  voltageValue= aVal* (3.3/1024) ;
  return voltageValue;
}

void loop(void) {
   sensorValue = analogRead(sensorPin);
   Serial.println("---");
   Serial.println(sensorValue);   
   delay(1000);   
   Serial.print(conversion(sensorValue));
   Serial.println(" volts");
   Serial.println("---");
   
}
