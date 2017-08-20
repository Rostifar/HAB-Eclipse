#include <TinyGPS.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "HardwareSerial.h"
#include "SdFat.h"
#include "Adafruit_TSL2591.h"
#include "Adafruit_MCP9808.h"
#include <Wire.h>
#include <stdio.h>
//#include "Buffer.h"


#define LED_PIN 13
#define LIGHT_ERR -520392.0f

//chip for sd io
#define CHIP_SELECT 10
#define SD_SPEED SD_SCK_MHZ(50)
#define DEBUG
#define SD_MISSING_ERR 2
#define FILE_CREATION_ERR 3
//#define BUFFER_SIZE 1000

TinyGPS gps;
//Buffer buffer(BUFFER_SIZE);
Adafruit_TSL2591 tsl;
Adafruit_MCP9808 tempsensor;
File sd;
SdFile logfile;

char name[] = "LOGGER00.CSV";

void initTSL2591();
void initMCP9808();
float tsl2591UnifiedRead();
float mcp9808Read();

void error();
static void smartdelay(unsigned long ms);
static void log_float(float val, float invalid);
static void log_int(unsigned long val, unsigned long invalid);
static void log_date(TinyGPS &gps);
static void log_str(const char *str);

void setup() {
 Serial.println(F("Starting eclipse logger."));
  
 initTSL2591();
 initMCP9808();
 
 Serial.begin(115200);
 Serial1.begin(9600);

 pinMode( LED_PIN, OUTPUT );
 pinMode( 10, OUTPUT );

 Serial.println(F("Starting SD card."));

 if (!sd.begin(CHIP_SELECT, SD_SPEED)) {
    sd.initErrorHalt();
 }
   
for (uint8_t i = 0; i < 100; i++) {
   name[6] = i/10 + '0';
   name[7] = i%10 + '0';
   if (sd.exists(name)) continue;
   break;
 }
 
 logfile = sd.open(name, FILE_WRITE);

 if (!logfile) {
  Serial.print(F("couldn't create "));
  Serial.println(name);
  error(FILE_CREATION_ERR);
 }
 Serial.print(F("Writing to "));
 Serial.println(name);    
}

void error( uint8_t errno ) {
  while (1) {
    uint8_t i;
    for ( i=0;i < errno;i++ ) {
      digitalWrite( LED_PIN, HIGH );  
      delay( 100 );
      digitalWrite( LED_PIN, LOW );
      delay( 100 );  
    }
    for ( i=errno; i<10; i++ ) {
      delay( 200 );  
    }
  }
}

void initTSL2591() {
  Serial.println(F("Starting Adafruit TSL2591"));
  
  if (tsl.begin()) {
    Serial.println(F("Found a TSL2591 sensor"));
  }
  else {
    Serial.println(F("No sensor found ... check your wiring?"));
    while(1);
  }

  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */  

  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
}

void initMCP9808() {
  Serial.println(F("Starting MCP9808"));

  if (!tempsensor.begin()) {
    Serial.println(F("Couldn't find MCP9808"));
    while (1);
  }
}

//recommended reader for tsl2591
float tsl2591UnifiedRead() {
  Serial.println(F("Reading from TSL2591."));
  
  sensors_event_t event;
  tsl.getEvent( &event );
  if ((event.light == 0) | (event.light > 4294966000.0) | (event.light <-4294966000.0))
  {
      Serial.println(F("Invalid data (adjust gain or timing)"));
  }
  else
  {
      return event.light;
  }
  return LIGHT_ERR;
}

float mcp9808Read() {
  return tempsensor.readTempC();
}

int i = 0;
void loop()
{
  unsigned long age, date, time, chars = 0;
  unsigned short sentences, failed;
  float lan, lon;

  log_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES);
  log_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP);
  gps.f_get_position(&lan, &lon, &age);
  log_float(lat, TinyGPS::GPS_INVALID_F_ANGLE);
  log_float(lon, TinyGPS::GPS_INVALID_F_ANGLE);
  log_int(age, TinyGPS::GPS_INVALID_AGE);
  log_date(gps);
  log_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE);
  log_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE);
  log_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED);
  log_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "***" : TinyGPS::cardinal(gps.f_course()));
  log(tsl2591UnifiedRead(), LIGHT_ERR);
  logfile.print(mcp9808Read());
  logfile.println();

  if (i % 60) {
    logfile.close();  logfile = sd.open(name, FILE_WRITE);
  }
  i++;
  smartdelay(1000);
}

static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while(Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}

static void log_float(float val, float invalid) {
  if (val == invalid) logfile.print('*'); 
  else logfile.print(val); 
  logfile.print(',');
  smartdelay(0);
}

static void log_int(unsigned long val, unsigned long invalid) {
  if (val == invalid) logfile.print('*');
  else logfile.print(val);
  logfile.print(',');
  smartdelay(0);
}

static void log_date(TinyGPS &gps) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE) logfile.print('*');
  else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    logfile.print(sz);
  }
  logfile.print(',');
  log_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  logfile.print(',');
  smartdelay(0);
}

static void log_str(const char *str) {
  logfile.print(str); logfile.print(',');
  smartdelay(0);
}