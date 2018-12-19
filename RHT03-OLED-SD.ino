/*	RHT03-Example-Serial.cpp
	Jim Lindblom <jim@sparkfun.com>
	August 31, 2015
    
    Ported to Arduino by Shawn Hymel
    October 28, 2016
	https://github.com/sparkfun/SparkFun_RHT03_Arduino_Library
	
	This a simple example sketch for the SparkFunRHT03 Ardiuno
	library.
	
	Looking at the front (grated side) of the RHT03, the pinout is as follows:
	 1     2        3       4
	VCC  DATA  No-Connect  GND
	
	Connect the data pin to Arduino pin D4. Power the RHT03 off the 3.3V bus.
	
	A 10k pullup resistor can be added to the data pin, though it seems to
	work without it.
	
    Development environment specifics:
	Arduino IDE v1.6.5
	Distributed as-is; no warranty is given.  
*/

// Include the library:
#include <SparkFun_RHT03.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

/////////////////////
// Pin Definitions //
/////////////////////
const int RHT03_DATA_PIN = 7; // RHT03 data pin
// If using software SPI (the default case):
#define OLED_MOSI  11   //D1
#define OLED_CLK   12   //D0
#define OLED_DC    9
#define OLED_CS    8
#define OLED_RESET 10
// Second SPI
#define SD_CS    53

///////////////////////////
// RHT03 Object Creation //
///////////////////////////
RHT03 rht; // This creates a RTH03 object, which we'll use to interact with the sensor
float latestHumidity = 0;
float latestTempC = 0;
float latestTempF = 0;
long lastUpdated = 0;
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
File myFile;
boolean noSD = true;
int filename;

void setup()
{
	Serial.begin(9600); // Serial is used to print sensor readings.
  
	// Call rht.begin() to initialize the sensor and our data pin
	rht.begin(RHT03_DATA_PIN);

  display.begin(SSD1306_SWITCHCAPVCC);
  display.display();
  delay(100);
  display.clearDisplay();

  Serial.println("RHT03-OLED");

  SD.begin(SD_CS);
  filename = getNextName();
  myFile = SD.open(String(filename)+".csv", FILE_WRITE);
  if (myFile) {
    noSD = false;
    myFile.println(F("Millis(ms),Temperature(C),Humidity(%)"));
    myFile.close();
  }
  delay(100);
}

void output(){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(String(latestTempC, 1) + " C");
  //display.setCursor(68, 0);
  display.println(String(latestHumidity, 1) + " %");
  display.setTextSize(1);
  display.setCursor(95, 18);
  if (noSD) { 
    display.println("No SD");
  }
  else display.println(filename);
  display.display();  
  myFile = SD.open(String(filename)+".csv", FILE_WRITE);
  if (!noSD && myFile) {
    myFile.print(millis());
    myFile.print(","+String(latestTempC, 1));
    myFile.println(","+String(latestHumidity, 1));
    myFile.close();
    noSD = false;
  }
  else noSD = true;
  delay(100);
}

int getNextName() { // check filenames on SD and increment by one
  File root = SD.open("/");
  int out = 10000;
  while (true) {
    File entry =  root.openNextFile();
    if (!entry) break;
    String temp = entry.name();
    temp = temp.substring(0 ,5);
    int num = temp.toInt();
    if (num < 99999 && num >= out) out = num + 1;
    entry.close();
  }
  return out;
}

void loop()
{
	// Call rht.update() to get new humidity and temperature values from the sensor.
	int updateRet = rht.update();
	
	// If successful, the update() function will return 1.
	// If update fails, it will return a value <0
	if (updateRet == 1)
	{
    lastUpdated = millis();
		// The humidity(), tempC(), and tempF() functions can be called -- after 
		// a successful update() -- to get the last humidity and temperature
		// value 
		latestHumidity = rht.humidity();
		latestTempC = rht.tempC();
		latestTempF = rht.tempF();
		
		// Now print the values:
		Serial.println("Humidity: " + String(latestHumidity, 1) + " %");
		Serial.println("Temp (F): " + String(latestTempF, 1) + " deg F");
		Serial.println("Temp (C): " + String(latestTempC, 1) + " deg C");
    display.clearDisplay();
    output();
	}
	else
	{
		// If the update failed, try delaying for RHT_READ_INTERVAL_MS ms before
		// trying again.
    Serial.println("No update");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(95, 10);
    display.println((String)((millis()-lastUpdated)/1000)+"s");
    output();    
		delay(RHT_READ_INTERVAL_MS);
	}
	
	delay(1000);
}
