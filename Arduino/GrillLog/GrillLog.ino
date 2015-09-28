#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// External lib config

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial esp8266(8,9);

// Server configs

const char destServer[] = "192.168.1.9";
const uint16_t destPort = 8050;

// Temp stuff

// which analog pin to connect
#define FOODPIN 0
#define GRILLPIN 1         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 192000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 220000   

float samples[NUMSAMPLES];

// ESP8266 Junk

#define ESP8266_RX_BUFFER_LEN 128 // Number of bytes in the serial receive buffer
char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
unsigned int bufferHead; // Holds position of latest byte placed in buffer.

const char ESP8266_OPEN_CONNECTION[] = "AT+CIPSTART";
const char ESP8266_SEND_CONNECTION[] = "AT+CIPSEND";
const char ESP8266_CLOSE_CONNECTION[] = "AT+CIPCLOSE";
const char ESP8266_CHANGE_MUX[] = "AT+CIPMUX";
const char RESPONSE_OK[] = "OK\r\n";

const int BAUD_RATE = 9600;
const int DEFAULT_TIMEOUT = 5000;
const int SEND_COMMAND_DELAY = 100;
const int TEMP_READ_DELAY = 10000;

const boolean debugMode = false;

typedef enum esp8266_cmd_rsp {
  ESP8266_CMD_BAD = -5,
  ESP8266_RSP_MEMORY_ERR = -4,
  ESP8266_RSP_FAIL = -3,
  ESP8266_RSP_UNKNOWN = -2,
  ESP8266_RSP_TIMEOUT = -1,
  ESP8266_RSP_SUCCESS = 0
};

void setup() {
  Serial.begin(BAUD_RATE);
  esp8266.begin(BAUD_RATE);
  lcd.begin(16, 2);
}

void loop() {

  float foodAverage = sampleTempData(FOODPIN, NUMSAMPLES);
  // convert the value to resistance
  float foodResistance = convertAnalogToResistance(foodAverage, SERIESRESISTOR);
  float foodDegC = calculateCFromResistance(foodResistance, THERMISTORNOMINAL, BCOEFFICIENT, TEMPERATURENOMINAL);
  float foodDegF = convertCtoF(foodDegC);

  float grillAverage = sampleTempData(GRILLPIN, NUMSAMPLES);
  // convert the value to resistance
  float grillResistance = convertAnalogToResistance(grillAverage, 1000000);
  float grillDegC = calculateCFromResistance(grillResistance, 1000000, BCOEFFICIENT, TEMPERATURENOMINAL);
  float grillDegF = convertCtoF(grillDegC);

  writeTempDatatoSerial(grillDegF, foodDegF);
  writeTempToDisplay(grillDegF, foodDegF);
  
  logData(grillDegF, foodDegF);
  Serial.println("--------------");
  delay(TEMP_READ_DELAY);
}

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

float convertAnalogToResistance(float avg, long resistor){
  avg = 1023 / avg - 1;
  avg = resistor / avg;
  Serial.print("Thermistor resistance "); 
  Serial.println(avg);
  return avg;
}

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

void writeTempDatatoSerial(float temp1, float temp2){
  Serial.print("Grill: "); 
  Serial.println(temp1, 2);
  Serial.print("Food: ");
  Serial.println(temp2, 2);
}

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

float convertVoltagetoC(float voltage){
  return (voltage - 0.5) * 100.0;
}

float convertCtoF(float degC){
  return degC * (9.0/5.0) + 32.0;
}

float getVoltage(int pin){
  // This equation converts the 0 to 1023 value that analogRead()
  // returns, into a 0.0 to 5.0 value that is the true voltage
  // being read at that pin.
  return (analogRead(pin) * 0.004882814);
}

void logData(float grillTemp, float foodTemp){
  // Construct POST headers
  String httpPost = "POST /temp HTTP/1.1\r\n";
  httpPost += "Host: " + String(destServer) + "\r\n";
  httpPost += "Content-Type: application/x-www-form-urlencoded\r\n";
  httpPost += "Connection: close\r\n";
  // Construct POST Data
  String params = "foodTemp=" + String(foodTemp) + "&grillTemp=" + String(grillTemp);
  String contentLength = "Content-Length: " + String(params.length()) + "\r\n\r\n";
  // Combine them
  httpPost += contentLength + params;

  // Ensure we're in single connection mode
  sendCommand(ESP8266_CHANGE_MUX, "0");
  int resp = readForResponse(RESPONSE_OK, DEFAULT_TIMEOUT);

  delay(SEND_COMMAND_DELAY);

  // Open connection
  String connection = "\"TCP\",\"" + String(destServer) + "\"," + String(destPort);
  char buf[connection.length()+1];
  connection.toCharArray(buf, connection.length() + 1);
  sendCommand(ESP8266_OPEN_CONNECTION, buf);
  resp = readForResponse(RESPONSE_OK, DEFAULT_TIMEOUT);
  
  delay(SEND_COMMAND_DELAY);

  String postLength = String(httpPost.length());
  char charBuf[postLength.length()+2];
  postLength.toCharArray(charBuf, postLength.length()+2);
  
  sendCommand(ESP8266_SEND_CONNECTION, charBuf);
  resp = readForResponse(">", DEFAULT_TIMEOUT);
  
  delay(SEND_COMMAND_DELAY);

  esp8266.print(httpPost);

  resp = readForResponse("200 OK", DEFAULT_TIMEOUT);
  if (resp > 0){
    Serial.println("Saved temp to server");
  }
}


void sendCommand(const char * cmd){
  sendCommand(cmd, NULL);
}

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
    errorCode = ESP8266_RSP_UNKNOWN; // Return unkown response error code
  else // If we haven't received any characters
    errorCode = ESP8266_RSP_TIMEOUT; // Return the timeout error code
    
  logATResponse(errorCode);
  return errorCode;
}

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

void clearBuffer(){
  memset(esp8266RxBuffer, '\0', ESP8266_RX_BUFFER_LEN);
  bufferHead = 0;
}

unsigned int readByteToBuffer(){
  // Read the data in
  char c = esp8266.read();
  
  // Store the data in the buffer
  esp8266RxBuffer[bufferHead] = c;
  //! TODO: Don't care if we overflow. Should we? Set a flag or something?
  bufferHead = (bufferHead + 1) % ESP8266_RX_BUFFER_LEN;
  
  return 1;
}

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
