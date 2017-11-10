/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <WebSocketsServer.h> 
//#include <ESP8266WiFiMulti.h> 
#include <ESP8266WebServer.h>
#include <Hash.h>
#include <TimerObject.h>

#include "HC_SR04.h"

#include <Adafruit_AMG88xx.h>
#include <Wire.h>

#include "html_file.h"

/* Set these to your desired credentials. */
const char *ssid = "ESPap";
const char *password = "thereisnospoon";

/* Pin definition */
const int LED_PIN = 2;//LED_BUILTIN;

/* Sensor definition */
const int TRIGGER_PIN  = 13;  // Arduino pin tied to trigger pin on ping sensor.
const int ECHO_PIN     = 15; // Arduino pin tied to echo pin on ping sensor.
const int MAX_DISTANCE = 200; // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

HC_SR04 sonar(TRIGGER_PIN, ECHO_PIN, ECHO_PIN);
int sonar_range;
int sonar_counter = 0;

// Thermal stuff
Adafruit_AMG88xx amg;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

// Callback timer function
TimerObject *timer10ms = new TimerObject(100); //will call the callback in the interval of 100 ms

// Data buffer objects
int rand_data[64+3];
char rand_string[300];

#define LENGTH_ASYNC    1   // Number of Async pins
// Pins to drive async
int async_pins[LENGTH_ASYNC] = {LED_PIN};
// State of those pins
int async_state[LENGTH_ASYNC];
// Time for those pins to be in that state
int async_timeoff[LENGTH_ASYNC];

void asynclikeDrive(int desired_state[LENGTH_ASYNC], int desired_time[LENGTH_ASYNC])
/*
    Drive a pin asynchronously. Call to set the status of a pin
*/
{
    unsigned long curr_time = millis();
    // Loop over all pins
    for (int i=0; i < LENGTH_ASYNC; i++)
    {
        // Set new state and set timer
        async_state[i] = desired_state[i];
        async_timeoff[i] = desired_time[i] + curr_time;
        digitalWrite(async_pins[i], async_state[i]);
    }
}

void asynclikeUpdate()
/*
    Update the asynchronous driven pins.
*/
{
    unsigned long curr_time = millis();
    // Loop over all pins
    for (int i=0; i < LENGTH_ASYNC; i++)
    {
        // Check state and check timer
        if (async_state[i] == 0 && curr_time >= async_timeoff[i])
        {
            async_state[i] = 1;
            digitalWrite(async_pins[i], async_state[i]);
        }
    }

}

ESP8266WebServer server(80);
/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
	server.send(200, "text/html", html_data);
}

WebSocketsServer webSocket = WebSocketsServer(81);
void webSocketHandler(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            {
                Serial.printf("[%u] Disconnected!\n", num);
            }
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                int led_state[1] = {0};
                int led_time[1] = {10};
                asynclikeDrive(led_state, led_time); 
            }
            break;
        case WStype_TEXT:
            {
                Serial.printf("[%u] get Text: %s\n", num, payload);
                int led_state[1] = {0};
                int led_time[1] = {100};
                asynclikeDrive(led_state, led_time); 
            }
            break;
        case WStype_BIN:
            {
                Serial.printf("[%u] get binary length: %u\n", num, length);
                hexdump(payload, length);
            }
            break;
    }

}

void SampleData() {
/*  
    Eventually will make calls to read thermal camera and other sensors, but for
    now this function is used purely to generate random data to broadcast to 
    clients
*/
    amg.readPixels(pixels);


    if (sonar.isFinished())
    {
        sonar_range = sonar.getRange(); //Returns range in cm
        sonar.start();
    }

    int length = 0;

    for (int i=0;i<64+3;i++) {
        if (i == 1) 
            rand_data[i] = sonar_range;
        else if (i > 2) 
            rand_data[i] = pixels[i-3];
        else
        {
            rand_data[i] = random(99);
        }
        length += sprintf(rand_string+length, "%d,", rand_data[i]);
    }

    webSocket.broadcastTXT(rand_string);
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN,LOW);
	delay(250);
    digitalWrite(LED_PIN,HIGH);
	delay(250);
    digitalWrite(LED_PIN,LOW);
	delay(250);
    digitalWrite(LED_PIN,HIGH);
	delay(250);
    digitalWrite(LED_PIN,LOW);
	delay(250);
    digitalWrite(LED_PIN,HIGH);
	Serial.begin(9600);
	Serial.println();
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
    
    webSocket.begin();
    webSocket.onEvent(webSocketHandler);
	Serial.println("Websocket Started");

	server.on("/", handleRoot);
	server.begin();
	Serial.println("HTTP server started");

    timer10ms->setOnTimer(&SampleData);
    timer10ms->Start(); //start the thread.

    sonar.begin(); //start ultrasonic 
	Serial.println("Sonar started");

    amg.begin();  //start thermal
	Serial.println("Thermal started");
}

void loop() {
	server.handleClient();
    webSocket.loop();
    timer10ms->Update();
    asynclikeUpdate();
}
