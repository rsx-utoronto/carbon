#include <Servo.h>
#include <Wire.h>
#include <HMC5883L.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

// ascii character map http://www.asciitable.com/index/asciifull.gif

// Rover drive pin map

/*
            ****
******   **********  ******
* 13 * ************* * 12 *
******   **********  ******
         ********** 
******   **********  ******
* 11 * ************* * 10 *
******   **********  ******
         **********
******   **********  ******
* 09 * ************* * 08 *
******   **********  ******
            ****
*/

// General purpose variables
String inData;
boolean keep_reading = false;

// Motor pins
Servo wheel_fl;
Servo wheel_fr;
Servo wheel_cl;
Servo wheel_cr;
Servo wheel_bl;
Servo wheel_br;

// Arm pins
Servo arm_1;
Servo arm_2;
Servo arm_3;
Servo arm_4;
Servo arm_5;
Servo arm_6;
Servo arm_7;

// Drive system variables
int NEUTRAL = 93;
int spd, rt_spd, lt_spd, dir, prev_spd, prev_dir = 0;

// Sensor variables

HMC5883L compass;
unsigned long fix_age;
TinyGPS gps;
void gpsdump(TinyGPS &gps);
bool feedgps();
void getGPS();
long lat, lon;
float LAT, LON;

void setup() {

  // Starting serial connections
  Serial.begin(9600); // Local debugging
  Serial1.begin(9600); // Main telemetry IO
  Serial2.begin(9600); // GPS module  

  // Drive signal setup
  wheel_fl.attach(13);
  wheel_fr.attach(12);
  wheel_cl.attach(11);
  wheel_cr.attach(10);
  wheel_bl.attach(9);
  wheel_br.attach(8);
  
  // Arm signal setup
  arm_1.attach(7);
  arm_2.attach(6);
  arm_3.attach(5);
  arm_4.attach(4);
  arm_5.attach(3);
  arm_6.attach(2);
  arm_7.attach(1)
  
  // Compass setup
  while (!compass.begin()) {
    Serial.println("Could not find a valid HMC5883L sensor, check wiring!");
    delay(500);
  }
  
  compass.setRange(HMC5883L_RANGE_1_3GA);
  compass.setMeasurementMode(HMC5883L_CONTINOUS);
  compass.setDataRate(HMC5883L_DATARATE_30HZ);
  compass.setSamples(HMC5883L_SAMPLES_8);
  compass.setOffset(0, 0);  
}

void loop() {
  
  ////////////////////
  ////// INPUT ///////
  ////////////////////
  
  while (Serial1.available() > 0)
  {
    char received = Serial1.read();

    if (received == '<') {
      keep_reading = true;
      inData = "";
      send_data();
      continue;
    }

    if (received == '>') {

      prev_spd = spd;
      prev_dir = dir;

      dir = getValue(inData, ',', 0).toInt();
      rt_spd = getValue(inData, ',', 1).toInt();
      lt_spd = getValue(inData, ',', 2).toInt();
      spd = getValue(inData, ',', 3).toInt();
      
      if ((prev_spd - spd) > 4) {
        smoothStop(prev_dir, prev_spd); 
      }

      driveRover(dir, rt_spd, lt_spd);       

      Serial.println(inData);
      keep_reading = false;
      inData = "";

    }

    if (keep_reading == true) {
      inData += received; 
    }
    
  }
  
}

void send_data() {

  /////////////////////
  ////// OUTPUT ///////
  /////////////////////
  
  // COMPASS CODE

  Vector norm = compass.readNormalize();
  float heading = atan2(norm.YAxis, norm.XAxis);
  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI);
  heading += declinationAngle;

  if (heading < 0) {
    heading += 2 * PI;
  }

  if (heading > 2 * PI) {
    heading -= 2 * PI;
  }

  float headingDegrees = heading * 180/M_PI; 
  
  // GPS CODE

  long lat, lon;
  unsigned long fix_age, time, date, speed, course;
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  gps.get_position(&lat, &lon, &fix_age);
  getGPS();
  
  // DATA PACKET
  
  Serial1.print(LAT/100000,7);
  Serial1.print(",");
  Serial1.print(LON/100000,7);  
  Serial1.print(",");  
  Serial1.println(headingDegrees);
  
}

// Pass in a string, seperator, and index you want, and get that value
// http://stackoverflow.com/a/14824108

String getValue(String data, char separator, int index) {

  int found = 0;
  int strIndex[] = {
    0, -1            };
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";

}

void driveRover(int dir, int rt_spd, int lt_speed) {

  switch(dir) {

  case 0:
  
    wheel_fl.write(NEUTRAL);
    wheel_cl.write(NEUTRAL);
    wheel_bl.write(NEUTRAL);
    
    wheel_fr.write(NEUTRAL);
    wheel_cr.write(NEUTRAL);    
    wheel_br.write(NEUTRAL);
    
    break;

  case 1:
  
    wheel_fl.write(NEUTRAL - lt_spd);
    wheel_cl.write(NEUTRAL - lt_spd);
    wheel_bl.write(NEUTRAL - lt_spd);
    
    wheel_fr.write(NEUTRAL + rt_spd);
    wheel_cr.write(NEUTRAL + rt_spd);    
    wheel_br.write(NEUTRAL + rt_spd);
    
    break;

  case 2:
  
    wheel_fl.write(NEUTRAL + lt_spd);
    wheel_cl.write(NEUTRAL + lt_spd);
    wheel_bl.write(NEUTRAL + lt_spd);
    
    wheel_fr.write(NEUTRAL - rt_spd);
    wheel_cr.write(NEUTRAL - rt_spd);    
    wheel_br.write(NEUTRAL - rt_spd);
      
    break;

  }

}

void smoothStop(int prev_dir, int prev_spd) {

  for(int i = prev_spd; i > 0; i--) {
    driveRover(prev_dir, i, i);
    delay(25);
  }
  
  while (Serial.available()) Serial.read();

}

void smoothStart(int dir, int spd, int prev_spd) {

  for(int i = prev_spd; i <= spd; i++) {
    driveRover(dir, i, i);
    delay(20);
  }

}

void getGPS(){
  bool newdata = false;
  unsigned long start = millis();
  // Every 1 seconds we print an update
  while (millis() - start < 1000)
  {
    if (feedgps ()){
      newdata = true; 
    }
  }
  if (newdata)
  {
    gpsdump(gps);
  }
}

bool feedgps(){
  while (Serial2.available())
  {
    if (gps.encode(Serial2.read()))
      return true;
  }
  return 0;
}

void gpsdump(TinyGPS &gps)
{
  //byte month, day, hour, minute, second, hundredths;
  gps.get_position(&lat, &lon);
  LAT = lat;
  LON = lon;
  {
    feedgps(); // If we don't feed the gps during this long routine, we may drop characters and get checksum errors
  }
}

