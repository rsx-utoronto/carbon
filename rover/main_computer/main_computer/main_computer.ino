#include <Servo.h>

// ascii character map http://www.asciitable.com/index/asciifull.gif

// General purpose variables
String inData;
boolean keep_reading = false;

// Drive system variables
Servo drive_signal_1;
Servo drive_signal_2;
int NEUTRAL = 93;
int spd, rt_spd, lt_spd, dir, prev_spd, prev_dir = 0;

void setup() {

  // Starting serial connections
  Serial.begin(9600); // Local debugging
  Serial1.begin(9600); // Main telemetry IO

  // Drive signal setup
  drive_signal_1.attach(12);
  drive_signal_2.attach(13);

}

void loop() {

  while (Serial1.available() > 0)
  {
    char received = Serial1.read();

    if (received == '<') {
      keep_reading = true;
      inData = "";
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
    drive_signal_1.write(NEUTRAL);
    drive_signal_2.write(NEUTRAL);  
    break;

  case 1:
    drive_signal_1.write(NEUTRAL + rt_spd);
    drive_signal_2.write(NEUTRAL - lt_spd);
    break;

  case 2:
    drive_signal_1.write(NEUTRAL - rt_spd);
    drive_signal_2.write(NEUTRAL + lt_spd);
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


