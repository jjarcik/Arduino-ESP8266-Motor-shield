#include "WiFiEsp.h"
#include "SoftwareSerial.h"
SoftwareSerial Serial1(2, 3); // RX, TX
char ssid[] = "SSID";            // your network SSID (name)
char pass[] = "password";        // your network password

int MOTOR_A_SPEED = 10; // ovladanie rychlosti
int MOTOR_A_LEFT = 11; // smer
int MOTOR_A_RIGHT = 12; // smer

int MOTOR_B_SPEED = 7; // ovladanie rychlosti
int MOTOR_B_LEFT = 8; // smer
int MOTOR_B_RIGHT = 9; // smer

int buzzPin = 5; //Define buzzerPin


int status = WL_IDLE_STATUS;     // the Wifi radio's status

int ledStatus = LOW;
int step = 1500;
int motor_isruning = 0;
long int time = millis();

WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup() {
  Serial.begin(9600);   // initialize serial for debugging      
  buzConfig();
  tonOn();
  wifiConfig();  
  tonOn();
  motorConfig();
    
}


void loop()
{
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    Serial.println("New client");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        char c = client.read();               // read a byte, then
        buf.push(c);                          // push it to the ring buffer

        // printing the stream to the serial monitor will slow down
        // the receiving of data from the ESP filling the serial buffer
        //Serial.write(c);
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n\r\n")) {
          sendHttpResponse(client);
          break;
        }
        
        if (buf.endsWith("GET /L")) {          
          Serial.println("Turn left");
          tonOn();          
          motor(1, 255, 1);
          sendHttpResponse(client);
          
          client.stop();
        }
        
        if (buf.endsWith("GET /R")) {          
          Serial.println("Turn Right");          
          tonOn();
          motor(0,255, 1);
          sendHttpResponse(client);          
          client.stop();
        }

        if (buf.endsWith("GET /F")) {          
          Serial.println("Turn Forward");
          tonOn();
          motor(0,255, 1);
          motor(1,255, 1);
          sendHttpResponse(client);          
          client.stop();
        }

        if (buf.endsWith("GET /B")) {          
          Serial.println("Turn Back"); 
          tonOn();         
          motor(0,255, 0);
          motor(1,255, 0);
          sendHttpResponse(client);          
          client.stop();
        }/**/
        
      }
    }
    
    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }

/**/
  if (millis() > time + step && motor_isruning == 1){    
    motorStop();
  }/**/

}


void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:  
  client.print("<link rel='stylesheet' href='http://www.designuj.cz/dev/styles.css' />");          
  client.println("<a href=\"/F\">F</a>");
  client.println("<a href=\"/R\">R</a>");
  client.println("<a href=\"/B\">B</a>");
  client.println("<a href=\"/L\">L</a>");
  
  
  // The HTTP response ends with another blank line:
  client.println();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}

void wifiConfig(){   
  Serial1.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}

void motorConfig(){
  pinMode(MOTOR_A_SPEED, OUTPUT);
  pinMode(MOTOR_A_LEFT, OUTPUT);
  pinMode(MOTOR_A_RIGHT, OUTPUT);

  pinMode(MOTOR_B_SPEED, OUTPUT);
  pinMode(MOTOR_B_LEFT, OUTPUT);
  pinMode(MOTOR_A_RIGHT, OUTPUT);
}


// motor:     0 is motor A,   1 is motor B
// speed:     0 - 255
// direction: 0 is back, 1 is forward

void motor(int motor, int motor_speed, int motor_direction){
  
    Serial.print("MOTOR ");
    Serial.println(motor);    
 
    boolean inPin1 = LOW;
    boolean inPin2 = HIGH;
    motor_isruning = 1;
    time = millis();  
    
    /**/
    if(motor_direction == 1){
      inPin1 = HIGH;
      inPin2 = LOW;
    }

  if(motor == 0){
    Serial.println("Motor A Start " + motor_speed);
    digitalWrite(MOTOR_A_LEFT, inPin1);
    digitalWrite(MOTOR_A_RIGHT, inPin2);
    analogWrite(MOTOR_A_SPEED, motor_speed);
  } else if(motor == 1){
    Serial.println("Motor B Start " + motor_speed);
    digitalWrite(MOTOR_B_LEFT, inPin2);
    digitalWrite(MOTOR_B_RIGHT, inPin1);
    analogWrite(MOTOR_B_SPEED, motor_speed);
  } 

//  digitalWrite(buzzPin, LOW); // Tone OFF
   
   
 /**/
}

void motorStop(){
  Serial.println("Motor Stop");
   motor_isruning = 0;    
   //analogWrite(MOTOR_A_SPEED, 0);
   //analogWrite(MOTOR_B_SPEED, 0);
}

void buzConfig(){
  pinMode(buzzPin, OUTPUT); // Set buzzer-pin as output
}

void tonOn(){
  
  digitalWrite(buzzPin, LOW); // Tone ON
  delay(50); // Tone length
  digitalWrite(buzzPin, HIGH); // Tone OFF
  return;
}


