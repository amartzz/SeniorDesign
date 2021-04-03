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

const char* ssid = "MySpectrumWiFi11-2G";    //  your network SSID (name)
const char* password = "bridgekite964";  // your network password
String httpsRequest = "https://hooks.zapier.com/hooks/catch/9802542/o7jb5nc"; // your Zapier URL
const char* host = "hooks.zapier.com";

// data to send to the server

String Date = "March 21, 2021";
String Time="9:30";
String warning_message = "This is a warning message";

// wifi connection 

int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket
bool fetch_data=false;
int ledPin = 2; // on when measuring SNR
int ledServerConnected=3; // on when device is started remotely

int greenLEDmosfet = 5;
int redLEDmosfet = 4;

WiFiClient client = server.available();
WiFiSSLClient email_client;
WiFiClient  ThingSpeakClient;
unsigned long myChannelNumber = 1338345;

const char * myWriteAPIKey = "0X41J157YNCX2MFL";

void setup(void)
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(greenLEDmosfet, OUTPUT);
  pinMode(redLEDmosfet, OUTPUT);
  while (!Serial);
  
  enable_WiFi();
  connect_WiFi();
  server.begin();
  printWifiStatus();
  ThingSpeak.begin(ThingSpeakClient);

  if(!adc.init()){
    Serial.println("ADS1115 not connected!");
  }
  Serial.println("Getting single-ended readings from AIN0");
  Serial.println("ADC Range: +/- 3.3V (1 bit = 3mV)");
}
 
void loop(void)
{
  client = server.available();
  if (client) {
    printWEB();
  }

  if (fetch_data) {
    Serial.println("------------------------wake");
    measure_data();
    // pause
    Serial.println("------------------------sleep");
    delay(5000);
//    delay((1800*10^3)/time_scale);// delay it by 30 minutes
  }
}

float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage = adc.getResult_mV();
  return voltage; 
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

  // red reading
  digitalWrite(redLEDmosfet, HIGH);
  delay(del);
  Serial.println("red");
  red_avg_snr = thirty_LED(ambient);
  red_STD = STD; 
  delay(del);
  digitalWrite(redLEDmosfet, LOW);

  // send to thingspeak
  ThingSpeak.setField(1, green_avg_snr);
  ThingSpeak.setField(2, red_avg_snr);
  ThingSpeak.setField(3, green_STD);
  ThingSpeak.setField(4, red_STD);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  Serial.println("Data uploaded to ThingS");

  // send email alerts if necessary
  if(red_avg_snr < snr_threshold){
    Serial.println("Started sending emails.");
    String message="Average red SNR value is below expected ";
    message.replace(" ", "%20");// encode space with %20
    send_email(red_avg_snr, red_STD, message);
    Serial.println("Finished sending emails.");
  }
  else if (green_avg_snr < snr_threshold) {
    Serial.println("Started sending emails.");
    String message="Average green SNR value is below expected ";
    message.replace(" ", "%20");// encode space with %20
    send_email(green_avg_snr, green_STD, message);
    Serial.println("Finished sending emails.");
  }
  
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

void connect_WiFi() {
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, password);

    // skip wait if connected
    if (status == WL_CONNECTED) {
      break;
    }

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void printWEB() {
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
//        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
//          Serial.write(c); 
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            client.println();
            // the content of the HTTP response follows the header:
            client.println("<!DOCTYPE HTML>");
            client.print("<head>");
            client.print("<link rel=");
            client.print("stylesheet");
            client.print("href=");
            client.print (">");
            client.print("</head>");
            client.print("<body>");
            client.println("<center><br><br><div class='container'><h1>Horse Monitoring System<h1/></div></center>");
            client.println("<center><div class='container'><left><button style='color:white;background-color:green;width:80px;height:80px;' class='on' type='submit' value='ON' onmousedown=location.href='/H\'>ON</button>");
            client.println("<button style='color:white;background-color:red;width:80px;height:80px;' class='off' type='submit' value='OFF' onmousedown=location.href='/L\'>OFF</button></div><br>");
            client.println("</body>");
            client.println("</html>");
            
            // The HTTP response ends with another blank line:
            client.println();
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        if (currentLine.endsWith("GET /H")) {
          digitalWrite(ledServerConnected, HIGH);  
          fetch_data=true;
          delay(500);      
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(ledServerConnected, LOW); 
          fetch_data=false;      
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void send_email(float Average_SNR, float STD, String message) {
 // convert values to String
 String _Average_SNR = String(Average_SNR);
 String _STD = String(STD);
 String _message = message;

 if (email_client.connect(host, 443)) {
   email_client.println("POST " + httpsRequest + "?Average_SNR=" + _Average_SNR + "&STD=" + _STD + "&Error_message=" + _message+ " HTTP/1.1");
   email_client.println("Host: " + String(host));
   email_client.println("Connection: close");
   email_client.println();
   delay(1000);
   while (email_client.available()) { // Print on the console the answer of the server
     char c = email_client.read();
     Serial.write(c);
   }
   email_client.stop();  // Disconnect from the server
 }
 else {
   Serial.println("Failed to connect to client");
 }
}
