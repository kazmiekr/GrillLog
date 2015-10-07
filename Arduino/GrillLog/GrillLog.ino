/************************************************************
GrillLog
Kevin Kazmierczak

This sketch will read temperature data from two probes using 
A0 and A1 and will sent that data up to a proxy server using an 
ESP8266 via serial commands. The current temperatures are also 
displayed on a 2 line LCD.

************************************************************/

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// External lib config
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial esp8266(8,9);

// Generic config
#define TEMP_READ_DELAY 10000 // 10 seconds between temp reading
#define BAUD_RATE 9600 // Baudrate for serial monitor and ESP

// Proxy Server configs
const char destServer[] = "192.168.1.9";
const uint16_t destPort = 8050;
const String postEndpoint = "/temp";

// Temperature Probes 
#define TEMPERATURENOMINAL 25
#define NUMSAMPLES 10 // Number of samples to smooth input

#define FOOD_PIN 0
#define FOOD_THERMISTORNOMINAL 192000 // resistance at 25 degrees C 
#define FOOD_BCOEFFICIENT 4250 // The beta coefficient of the thermistor
#define FOOD_SERIESRESISTOR 220000 // the value of the 'other' resistor
 
#define GRILL_PIN 1
#define GRILL_THERMISTORNOMINAL 1000000 // resistance at 25 degrees C 
#define GRILL_BCOEFFICIENT 4800 // The beta coefficient of the thermistor
#define GRILL_SERIESRESISTOR 1000000 // the value of the 'other' resistor

// ESP8266 Config
// Lots of this was pulled from https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library

#define ESP8266_RX_BUFFER_LEN 128 // Number of bytes in the serial receive buffer
char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
unsigned int bufferHead; // Holds position of latest byte placed in buffer.

// Define out the AT commands we're going to use to setup the wifi connection and
// send the data
const char ESP8266_OPEN_CONNECTION[] = "AT+CIPSTART";
const char ESP8266_SEND_CONNECTION[] = "AT+CIPSEND";
const char ESP8266_CLOSE_CONNECTION[] = "AT+CIPCLOSE";
const char ESP8266_CHANGE_MUX[] = "AT+CIPMUX";
// What to look for to know the previous command was sent correctly
const char RESPONSE_OK[] = "OK\r\n";

const int DEFAULT_TIMEOUT = 5000; // Timeout for an AT command before we call it a failure
const int SEND_COMMAND_DELAY = 100; // Just a delay between AT commands to help avoid issues

const boolean debugMode = false; // Enable this to see all the AT commands and responses happening in the monitor

float samples[NUMSAMPLES];

void setup() {
  Serial.begin(BAUD_RATE);
  esp8266.begin(BAUD_RATE);
  lcd.begin(16, 2);
}

void loop() {

  // ** General flow ** 
  // 1) Get analog reading from pin
  // 2) Convert the analog input to resistance
  // 3) Calculate degrees C from the resistance
  // 4) Convert that to F
  // 5) Write those values to lcd
  // 6) Send them to a server
  // 7) Wait x seconds and repeat

  float foodAverage = sampleTempData(FOOD_PIN, NUMSAMPLES);
  float foodResistance = convertAnalogToResistance(foodAverage, FOOD_SERIESRESISTOR);
  float foodDegC = calculateCFromResistance(foodResistance, FOOD_THERMISTORNOMINAL, FOOD_BCOEFFICIENT, TEMPERATURENOMINAL);
  float foodDegF = convertCtoF(foodDegC);

  float grillAverage = sampleTempData(GRILL_PIN, NUMSAMPLES);
  float grillResistance = convertAnalogToResistance(grillAverage, GRILL_SERIESRESISTOR);
  float grillDegC = calculateCFromResistance(grillResistance, GRILL_THERMISTORNOMINAL, GRILL_BCOEFFICIENT, TEMPERATURENOMINAL);
  float grillDegF = convertCtoF(grillDegC);

  // Update temps on display
  writeTempDatatoSerial(grillDegF, foodDegF);
  writeTempToDisplay(grillDegF, foodDegF);

  // Send those two temps to your server of choice
  sendDataToServer(grillDegF, foodDegF);
  
  Serial.println("--------------");
  
  delay(TEMP_READ_DELAY);
}

/**
 * Samples the analog input numSamples number of times to smooth out the final value
 */
float sampleTempData(int pin, int numSamples){
  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< numSamples; i++) {
   samples[i] = analogRead(pin);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< numSamples; i++) {
     average += samples[i];
  }
  average /= numSamples;

  Serial.print("Average analog reading "); 
  Serial.println(average);

  return average;
}

/**
 * Helper method to convert analog data to resistance. Taken from https://learn.adafruit.com/thermistor/using-a-thermistor
 */
float convertAnalogToResistance(float avg, long resistor){
  avg = 1023 / avg - 1;
  avg = resistor / avg;
  Serial.print("Thermistor resistance "); 
  Serial.println(avg);
  return avg;
}

/**
 * Helper method to calculate C from a resistance. This taken from https://learn.adafruit.com/thermistor/using-a-thermistor
 */
float calculateCFromResistance(float resistance, long thermNominal, int coefficient, int tempNominal){
  float steinhart;
  steinhart = resistance / thermNominal;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= coefficient;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (tempNominal + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  return steinhart;
}

/**
 * Converts C to F
 */
float convertCtoF(float degC){
  return degC * (9.0/5.0) + 32.0;
}

/**
 * Log the temps to the serial monitor
 */
void writeTempDatatoSerial(float temp1, float temp2){
  Serial.print("Grill: "); 
  Serial.println(temp1, 2);
  Serial.print("Food: ");
  Serial.println(temp2, 2);
}

/**
 * Log the temps to the LCD Display
 */
void writeTempToDisplay(float temp1, float temp2){
  lcd.setCursor(0, 0);
  lcd.print("Grill: ");
  lcd.print(temp1, 2);
  lcd.print(" ");
  lcd.setCursor(0, 1);
  lcd.print("Food: ");
  lcd.print(temp2, 2);
  lcd.print(" ");
}

/**
 * Sends the two temps to the server you've defined.  They get
 * POST'd as:
 *  'foodTemp=100.0&grillTemp=250.0
 */
void sendDataToServer(float grillTemp, float foodTemp){
  
  // Construct POST headers
  String httpPost = "POST " + postEndpoint + " HTTP/1.1\r\n";
  httpPost += "Host: " + String(destServer) + "\r\n";
  httpPost += "Content-Type: application/x-www-form-urlencoded\r\n";
  httpPost += "Connection: close\r\n";
  
  // Construct POST Data
  String params = "foodTemp=" + String(foodTemp) + "&grillTemp=" + String(grillTemp);
  String contentLength = "Content-Length: " + String(params.length()) + "\r\n\r\n";
  // Combine them
  httpPost += contentLength + params;

  // Ensure we're in single connection mode.  The ESP supports multi connections, but for simplicity's sake
  // we're going to stay serial here and only do one connection at a time
  sendCommand(ESP8266_CHANGE_MUX, "0");
  int resp = readForResponse(RESPONSE_OK, DEFAULT_TIMEOUT);

  delay(SEND_COMMAND_DELAY); // Pause between commands

  // Open connection to proxy server
  String connection = "\"TCP\",\"" + String(destServer) + "\"," + String(destPort);
  char buf[connection.length()+1];
  connection.toCharArray(buf, connection.length() + 1);
  sendCommand(ESP8266_OPEN_CONNECTION, buf);
  resp = readForResponse(RESPONSE_OK, DEFAULT_TIMEOUT);
  
  delay(SEND_COMMAND_DELAY);

  // Tell the ESP how much data we're about to send it
  String postLength = String(httpPost.length());
  char charBuf[postLength.length()+2];
  postLength.toCharArray(charBuf, postLength.length()+2);
  
  sendCommand(ESP8266_SEND_CONNECTION, charBuf);
  resp = readForResponse(">", DEFAULT_TIMEOUT);
  
  delay(SEND_COMMAND_DELAY);

  // Once we get the '>', we're ready to send out character buffer to the ESP
  esp8266.print(httpPost);

  // Wait for an OK and we're good!
  resp = readForResponse("200 OK", DEFAULT_TIMEOUT);
  
  if (resp > 0){
    Serial.println("Saved temp to server");
  }
}

/**
 * Wrapper method to support sendCommand without params
 */
void sendCommand(const char * cmd){
  sendCommand(cmd, NULL);
}

/**
 * Sends an AT command with params over the software serial
 */
void sendCommand(const char * cmd, const char * params){
  if (debugMode){
    Serial.println("SENDING:" + String(cmd) + ", PARAMS: " + String(params));  
  }
  esp8266.print(cmd);
  if (params){
    esp8266.print("=");
    esp8266.print(params);
  }
  esp8266.print("\r\n");
}

/**
 * Waits to receive a response on the software serial.  If the response is found, it'll
 * return the count of received data.  Heavily borrowed from https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library
 */
int16_t readForResponse(const char * rsp, unsigned int timeout){
  unsigned long timeIn = millis();  // Timestamp coming into function
  unsigned int received = 0; // received keeps track of number of chars read
  
  clearBuffer();  // Clear the class receive buffer (esp8266RxBuffer)
  while (timeIn + timeout > millis()){
    // If data is available on UART RX
    if (esp8266.available()) {
      received += readByteToBuffer();
      if (searchBuffer(rsp)){
        if (debugMode){
          logATResponse(received);
        }
        return received;  // Return how number of chars read 
      }
    }
  }

  int16_t errorCode;
  if (received > 0) // If we received any characters
    errorCode = -2; // Return unkown response error code
  else // If we haven't received any characters
    errorCode = -1; // Return the timeout error code
    
  logATResponse(errorCode);
  return errorCode;
}

/**
 * Logs detail around the AT command to the serial monitor
 */
void logATResponse(int resp){
  if (resp > 0){
    Serial.println("AT COMMAND OK");
    Serial.println("****");
    Serial.println(esp8266RxBuffer);
    Serial.println("****");
  } else {
    Serial.println("AT COMMAND FAILED WITH CODE: " + String(resp));
    Serial.println("****");
    Serial.println(esp8266RxBuffer);
    Serial.println("****");
  }
}

/**
 * Helper method to convert an int to a char *
 */
char * convertToString(int val){
  String s = String(val);
  Serial.println("S:" + s);
  int strlen = s.length();
  Serial.println("l:" + String(strlen));
  char buf[strlen+1]; // Include null thing
  s.toCharArray(buf, strlen+1);
  Serial.println("buf:" + String(buf));
  return buf;
}

/**
 * Clears the ESP buffer. Taken from https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library
 */
void clearBuffer(){
  memset(esp8266RxBuffer, '\0', ESP8266_RX_BUFFER_LEN);
  bufferHead = 0;
}

/**
 * Reads the ESP data. Taken from https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library
 */
unsigned int readByteToBuffer(){
  // Read the data in
  char c = esp8266.read();
  
  // Store the data in the buffer
  esp8266RxBuffer[bufferHead] = c;
  //! TODO: Don't care if we overflow. Should we? Set a flag or something?
  bufferHead = (bufferHead + 1) % ESP8266_RX_BUFFER_LEN;
  
  return 1;
}

/**
 * Searches the string buffer for a specific string to be found. Typically so we know the response
 * was OK
 */
char * searchBuffer(const char * test){
  int bufferLen = strlen((const char *)esp8266RxBuffer);
  // If our buffer isn't full, just do an strstr
  if (bufferLen < ESP8266_RX_BUFFER_LEN){
    return strstr((const char *)esp8266RxBuffer, test);
  } else { //! TODO
    // If the buffer is full, we need to search from the end of the 
    // buffer back to the beginning.
    int testLen = strlen(test);
    for (int i=0; i<ESP8266_RX_BUFFER_LEN; i++){
      
    }
  }
}
