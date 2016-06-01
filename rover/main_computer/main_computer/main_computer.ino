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
int sent_counter = 0;
unsigned int time_at_last_GPS_update = 0;

// Motor pins
Servo wheel_fl;
Servo wheel_fr;
Servo wheel_cl;
Servo wheel_cr;
Servo wheel_bl;
Servo wheel_br;

// Arm pins
Servo arm_1; // Elbow
Servo arm_2; // Shoulder
Servo arm_3; // End effector
Servo arm_4; // Scroller
Servo arm_5; // DC accessory

Servo hand_1;
Servo hand_2;
Servo hand_3;

int hand_lower_limit = 30;
int hand_upper_limit = 150;

// Drive system variables
int NEUTRAL = 93;
int spd, rt_spd, lt_spd, dir, twst_dir, twst_spd, prev_spd, prev_dir = 0;

// Arm control variables
int arm_1_val, arm_2_val, arm_3_val, arm_4_val, arm_5_val, hand_1_val, hand_2_val, hand_3_val;
int hand_1_pos, hand_2_pos, hand_3_pos = 0;

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

  hand_1.attach(44);
  hand_2.attach(45);
  hand_3.attach(46);

  // Compass setup
  while (!compass.begin()) {
    Serial.println("No compass detected");
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

      if (sent_counter >= 5) {
        sent_counter = 0;
        sendData();
      }
      else {
        sent_counter++;
      }

      continue;
    }

    if (received == '>') {

      prev_spd = spd;
      prev_dir = dir;

      dir = getValue(inData, ',', 0).toInt();
      rt_spd = getValue(inData, ',', 1).toInt();
      lt_spd = getValue(inData, ',', 2).toInt();
      spd = getValue(inData, ',', 3).toInt();
      twst_dir = getValue(inData, ',', 4).toInt();
      twst_spd = getValue(inData, ',', 5).toInt();

      arm_1_val = getValue(inData, ',', 6).toInt();
      arm_2_val = getValue(inData, ',', 7).toInt();
      arm_3_val = getValue(inData, ',', 8).toInt();
      arm_4_val = getValue(inData, ',', 9).toInt();
      arm_5_val = getValue(inData, ',', 10).toInt();

      hand_1_val = getValue(inData, ',', 11).toInt();
      hand_2_val = getValue(inData, ',', 12).toInt();
      hand_3_val = getValue(inData, ',', 13).toInt();

      /*
      if ((prev_spd - spd) > 4) {
       smoothStop(prev_dir, prev_spd);
       }
       */

      if (dir != 0) {
        driveRover(dir, rt_spd, lt_spd);
      }
      else {
        twistRover(twst_dir, twst_spd);
      }

      moveJoint(1, arm_1_val);
      moveJoint(2, arm_2_val);
      moveJoint(3, arm_3_val);
      moveJoint(4, arm_4_val);
      moveJoint(5, arm_5_val);

      moveHand(1, hand_1_val);
      moveHand(2, hand_2_val);
      moveHand(3, hand_3_val);

      Serial.println(inData);

      keep_reading = false;
      inData = "";

    }

    if (keep_reading == true) {
      inData += received;
    }

  }

}

void sendData() {

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

  Serial.print(LAT/100000,7);
  Serial.print(",");
  Serial.print(LON/100000,7);
  Serial.print(",");
  Serial.println(headingDegrees);

}

// Pass in a string, seperator, and index you want, and get that value
// http://stackoverflow.com/a/14824108

String getValue(String data, char separator, int index) {

  int found = 0;
  int strIndex[] = {
    0, -1                };
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

void twistRover(int dir, int spd) {

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

    wheel_fl.write(NEUTRAL - spd);
    wheel_cl.write(NEUTRAL);
    wheel_bl.write(NEUTRAL - spd);

    wheel_fr.write(NEUTRAL - spd);
    wheel_cr.write(NEUTRAL);
    wheel_br.write(NEUTRAL - spd);

    break;

  case 2:

    wheel_fl.write(NEUTRAL + spd);
    wheel_cl.write(NEUTRAL);
    wheel_bl.write(NEUTRAL + spd);

    wheel_fr.write(NEUTRAL + spd);
    wheel_cr.write(NEUTRAL);
    wheel_br.write(NEUTRAL + spd);

    break;

  }

}

void driveRover(int dir, int rt_spd, int lt_spd) {

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

void moveJoint(int joint_num, int dir) {

 int increment = 0;
 int joint_speed = 0;

 if (dir == 1) {
   joint_speed = -10;
 }
 else if (dir == 2) {
   joint_speed = 10;
 }

 switch(joint_num) {
  case 1:
    arm_1.write(NEUTRAL + joint_speed);
    break;
  case 2:
    arm_2.write(NEUTRAL + joint_speed);
    break;
  case 3:
    arm_3.write(NEUTRAL + joint_speed);
    break;
  case 4:
    arm_4.write(NEUTRAL + joint_speed);
    break;
  case 5:
    arm_5.write(NEUTRAL + joint_speed);
    break;
}

void moveHand(int hand_num, int dir) {

  int increment = 0;

  if (dir == 1) {
    increment = -2;
  }
  else if (dir == 2) {
    increment = 2;
  }

  switch(hand_num) {

  case 1:
    hand_1_pos += increment;
    if (hand_1_pos > hand_upper_limit) { hand_1_pos = hand_upper_limit; }
    else if (hand_1_pos < hand_lower_limit) { hand_1_pos = hand_lower_limit; }
    hand_1.write(hand_1_pos);

  case 2:
    hand_2_pos += increment;
    if (hand_2_pos > hand_upper_limit) { hand_2_pos = hand_upper_limit; }
    else if (hand_2_pos < hand_lower_limit) { hand_2_pos = hand_lower_limit; }
    hand_2.write(hand_2_pos);

  case 3:
    hand_3_pos += increment;
    if (hand_3_pos > hand_upper_limit) { hand_3_pos = hand_upper_limit; }
    else if (hand_3_pos < hand_lower_limit) { hand_3_pos = hand_lower_limit; }
    hand_3.write(hand_3_pos);

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
  // Every 1 seconds we print an update
  if(millis() - time_at_last_GPS_update > 1000) // only update once per second
  {
    Serial.println("gps update");
    if (feedgps ()){
      Serial.println("feed is true");
      newdata = true;
    }
  }
  if (newdata)
  {
    gpsdump(gps);
    time_at_last_GPS_update = millis();
    Serial.println("D");
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



