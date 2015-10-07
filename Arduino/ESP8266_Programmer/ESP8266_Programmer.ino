/************************************************************
ESP8266_Programmer
Kevin Kazmierczak

This is a simple sketch to supply some defaults for network configuration
into the ESP8266 on the SparkFun Wifi Shield using the Arduino as a serial passthrough.  
Anything you type into the serial monitor will get sent to the ESP8266 and any response it 
has will get received in the monitor.

MAKE SURE YOU SET THE SERIAL MONITOR TO USE 'BOTH NL&CR' LINE ENDINGS or the stuff you enter here
will not get sent!

To configure your ESP8266, you can enter some AT commands.  Details can be found online or somewhere 
like - https://cdn.sparkfun.com/assets/learn_tutorials/4/0/3/4A-ESP8266__AT_Instruction_Set__EN_v0.30.pdf 
depending on the version of the firmware installed on the chip

Examples
AT - Status check
AT+GMR - Show version information
AT+CWLAP - List access points
AT+CIFSR - Get IP address
AT+CWJAP_DEF="SSID","PASSWORD" - Connects to access point and saves as default
AT+CWQAP - Disconnect from AP
AT+PING="www.kevinkaz.com" - Ping an address

I typically use to check on the running config and ensure the chip is 
default connecting to my wifi so I don't need to program around that portion.

You may need to mess with the baud_rates depending on what your ESP is running at.  By
default the SparkFun shield is using 9600.  Also make sure you set your Serial Monitor to 
use the number you set here

************************************************************/

#include <SoftwareSerial.h>

SoftwareSerial esp8266(8,9);

void setup() 
{
  Serial.begin(9600);
  esp8266.begin(9600);
}

void loop() 
{
  serialPassthrough();
}

void serialPassthrough()
{
  while (Serial.available())
    esp8266.write(Serial.read());
  while (esp8266.available())
    Serial.write(esp8266.read());
}
