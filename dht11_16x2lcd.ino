#include <WiFi.h>
#include <LiquidCrystal.h>
#include "DHT.h"

uint16_t Humi;
uint16_t Temp;

const char* ssid     = "xxx";
const char* password = "xxx";
String header;

WiFiServer server(80);
// IPAddress local_IP(192, 168, 1, 184);
// IPAddress subnet(255, 255, 255, 0);

// LCD module connections (RS, E, D4, D5, D6, D7)
//stm32
// LiquidCrystal lcd(PA0, PA1, PA2, PA3, PA4, PA5);
//arduino
// LiquidCrystal lcd(40, 41, 42, 43, 44, 45);
//esp32
LiquidCrystal lcd(13, 12, 14, 27, 26, 25);
 
 //stm32
// #define DHTPIN  PA6           // DHT11 data pin is connected to Blue Pill PA6 pin
// #define LEDPIN PC13
//arduino
// #define DHTPIN  46
// #define LEDPIN 13
//esp32
#define DHTPIN  33
#define LEDPIN 2

#define DHTTYPE DHT11         // DHT11 sensor is used
DHT dht11(DHTPIN, DHTTYPE);   // initialize DHT library

uint16_t LedDelay = 100;
 
void setup() {

  pinMode(LEDPIN,OUTPUT);
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  //Serial.println(WiFi.localIP());
  server.begin();
 
  // set up the LCD's number of columns and rows
  //lcdSetup();
 
  // initialize DHT11 sensor
  dht11.begin();
 
}
 
// data display buffer
char dis_buf[7];
 
void loop() {
  Humi = dht11.readHumidity();
  Temp = dht11.readTemperature();

  if(WiFi.status() != WL_CONNECTED){
    LedDelay=10;
  } else if(Temp>0 && Temp<100) {
    LedDelay=Temp*10;
  } else {
    LedDelay=10;
  }
  delay(1000-LedDelay);
  digitalWrite(LEDPIN,HIGH);
  delay(LedDelay);           // wait 1 second between readings
  digitalWrite(LEDPIN,LOW);


 
  
 
  webserver();

  
}


void webserver(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();
            
            
            // Web Page Heading
            client.print("{\"temp\":");
            client.print(Temp);
            client.print(",\"humid\":");
            client.print(Humi);
            client.println("}");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}


void lcdSetup(){
  lcd.begin(16, 2);
 
  lcd.setCursor(0, 0);           // move cursor to position (0, 0) -- 1st column & 1st row
  lcd.print("STM32 Blue Pill");  // print text on the LCD
  lcd.setCursor(0, 1);           // move cursor to position (0, 1) -- 1st column & 2nd row
  lcd.print("& DHT11 Sensor");   // print text on the LCD
  
  delay(5000);    // wait 5 seconds
  lcd.clear();    // clear the display
  lcd.setCursor(0, 0);   // move cursor to position (0, 0) -- 1st column & 1st row
  lcd.print("Temp:");    // print text on the LCD
  lcd.setCursor(0, 1);   // move cursor to position (0, 1) -- 1st column & 2nd row
  lcd.print("Humi:");    // print text on the LCD
}

void lcdDrive(){
   if (isnan(Humi) || isnan(Temp)) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Error");
    return;
  }
 
  // print temperature (in °C)
  sprintf( dis_buf, "%02u.%1u%cC", (Temp / 10) % 100, Temp % 10, 223 );
  lcd.setCursor(6, 0);
  lcd.print(dis_buf);
 
  // print humidity (in %)
  sprintf( dis_buf, "%02u.%1u %%", (Humi / 10) % 100, Humi % 10 );
  lcd.setCursor(6, 1);
  lcd.print(dis_buf);
}
 