#include <WiFiNINA.h>
#include<WiFiSSLClient.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "ThingSpeak.h"

Adafruit_ADS1015 ads1015;

int snr_threshold=10;
int32_t ambient=0;
int sensorPin = A1;
int sensorValue = 0;
int time_scale=20; // run the clock 20 times faster so that testing can be done quickly
float adc_voltage=0.0;

const char* ssid = "InThisHouse";    //  your network SSID (name)
const char* password = "dorneyville";  // your network password
String httpsRequest = "https://hooks.zapier.com/hooks/catch/9802542/o7jb5nc"; // your Zapier URL
const char* host = "hooks.zapier.com";

// data to send to the server

 String Date = "March 25, 2021";
 String Time="9:30";
 String warning_message = "This is a warning message";
 
// wifi connection 
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket
bool fetch_data=false;
int ledPin = 2; // on when measuring SNR
int ledServerConnected=3; // on when device is started remotely

WiFiClient client = server.available();
WiFiSSLClient email_client;
WiFiClient  ThingSpeakClient;
unsigned long myChannelNumber = 1338345;

const char * myWriteAPIKey = "0X41J157YNCX2MFL";

void setup() {
 Serial.begin(9600);
 pinMode(ledPin, OUTPUT);
 pinMode(ledServerConnected, OUTPUT);

 while (!Serial);
  enable_WiFi();
  connect_WiFi();
  server.begin();
  printWifiStatus();
  ads1015.begin();
  ThingSpeak.begin(ThingSpeakClient);


}

void loop() {
 
 client = server.available();
  if (client) {
    printWEB();
  }

if(fetch_data) {
  measure_data();
  delay((1800*10^3)/time_scale);// delay it by 30 minutes
}
}

void measure_data() {
 Serial.println("Getting single-ended readings from AIN0..3");
 Serial.println("ADC Range: +/- 3.3V (1 bit = 3mV)");
  int32_t adc0=0;
  
  float total_snr=0.0;
  float total_sqr=0.0;
  float Average_SNR = 0.0;
  float STD =0.0;
 float snr=0.0;
   delay(500);



  Serial.print("ambient voltage before adjusted: "); Serial.println(ambient*3);Serial.println(" mV");
  digitalWrite(ledPin, HIGH);
  if(ambient==0) {
    ambient=1;
  }
  Serial.print("ambient voltage: "); Serial.println(ambient*3);Serial.println(" mV");

  for(int i=0; i<30; i++){
    adc0 = ads1015.readADC_SingleEnded(0);
    delay(1000);
    snr=20*log10(adc0/ambient);
    delay(1000);
    total_snr=total_snr+snr;
    total_sqr=total_sqr+snr*snr;
    Serial.print("Total: "); Serial.println(total_snr);
    Serial.print("total_sqr: "); Serial.println(total_sqr);
    Serial.print("Voltage: "); Serial.println(adc0*3); Serial.println(" mV");
    Serial.print("snr: ");Serial.print(snr);
    Serial.println();
  }
  total_snr=total_snr;
  total_sqr=total_sqr; // from std equation
  Average_SNR=total_snr/30;
  STD=sqrt((total_sqr/30) - Average_SNR*Average_SNR);
  Serial.print("___________________________________________ \n");
  Serial.print("Average SNR: "); Serial.println(Average_SNR);
  Serial.print("STD: "); Serial.println(STD);
  Serial.println(" ");

 ThingSpeak.setField(1, Average_SNR);
 ThingSpeak.setField(2, STD);
 ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
 Serial.println("Data uploaded to ThingS");
 
  if(Average_SNR<snr_threshold) {
    Serial.println(" Started sendig emails.");
    String message="Average SNR value is below expected ";
    message.replace(" ", "%20");// encode space with %20
    send_email(Average_SNR,STD, message);
    Serial.println(" Finished sendig emails.");

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


void connect_WiFi() {

 Serial.print("Connecting Wifi: ");
 Serial.println(ssid);
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, password);

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


void printWEB() {

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          Serial.write(c); 
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
          ambient = ads1015.readADC_SingleEnded(0);
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
