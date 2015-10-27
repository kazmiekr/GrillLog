![Diagram](https://raw.githubusercontent.com/kazmiekr/GrillLog/master/Design/icon_120.png) 
#GrillLog

This is a project that uses an Arduino UNO to monitor two temperature probes used for a BBQ smoker setup. One probe for food and another for the internal grill temp.  The temperature data is collected by the Arduino, then sent to a proxy server via the SparkFun wifi shield, which is based on the ESP8266.  The proxy server then forwards the temperature data to parse.com as parse.com required HTTPS and the ESP8266 doesn't.  Before the data is saved on Parse, a `beforeSave` trigger fires, and if the temp goes over a certain threshold, it will send a push notification to any subscribed clients.  There is an iOS project in here that allows for simple reading of the parse.com data.

##Parts
* Breadboard 
* Misc Jumper wires
* [Arduino Uno R3](http://www.amazon.com/Arduino-Ultimate-Starter-page-Instruction/dp/B00BT0NDB8/ref=sr_1_5?ie=UTF8&qid=1444243576&sr=8-5&keywords=arduino+uno)
* [Sparkfun Wifi Shield](https://www.sparkfun.com/products/13287)
* [Maverick ET732 Smoker Probe](http://www.amazon.com/gp/product/B006XLWL7K?psc=1&redirect=true&ref_=oh_aui_detailpage_o04_s00)
* Generic Food Probe - ( I’m using one from [this](http://www.amazon.com/Oregon-Scientific-AW129-Wireless-Thermometer/dp/B0006G2WYK/ref=sr_1_6?ie=UTF8&qid=1444242131&sr=8-6&keywords=oregon+scientific+grill+thermometer))
* 1X 10K Trimpot
* 1X 16x2 LCD Module With Presoldered pinheaders ( Using the one from [this](http://www.amazon.com/gp/product/B00HI0RYJK?psc=1&redirect=true&ref_=oh_aui_detailpage_o06_s00) )
* 1X 220K resistor
* 1X 1M resistor
* 2X [2.5mm jacks](http://www.amazon.com/gp/product/B00AKWR59M?psc=1&redirect=true&ref_=oh_aui_detailpage_o06_s00) soldered to wires

##Diagram
![Diagram](https://raw.githubusercontent.com/kazmiekr/GrillLog/master/Exports/GrillLog_bb.png)

##Pictures
![Grill](https://raw.githubusercontent.com/kazmiekr/GrillLog/master/Exports/pics/grill.jpg)
![Complete](https://raw.githubusercontent.com/kazmiekr/GrillLog/master/Exports/pics/complete.jpg)
