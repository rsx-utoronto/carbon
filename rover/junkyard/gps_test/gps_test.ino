#include <TinyGPS.h>
#include <SoftwareSerial.h>

unsigned long fix_age;
TinyGPS gps;
void gpsdump(TinyGPS &gps);
bool feedgps();
void getGPS();
long lat, lon;
float LAT, LON;
void setup(){
  Serial1.begin(9600); // GPS module
  Serial.begin(100);
}
void loop(){
  long lat, lon;
  unsigned long fix_age, time, date, speed, course;
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  // retrieves +/- lat/long in 100000ths of a degree
  gps.get_position(&lat, &lon, &fix_age);
  // time in hh:mm:ss, date in dd/mm/yy
  /*gps.get_datetime(&date, &time, &fix_age);
   year = date % 100;
   month = (date / 100) % 100;
   day = date / 10000;
   hour = time / 1000000;
   minute = (time / 10000) % 100;
   second = (time / 100) % 100;
   Serial.print("Date: ");
   Serial.print(year); Serial.print("/");
   Serial.print(month); Serial.print("/");
   Serial.print(day);
   Serial.print(" :: Time: ");
   Serial.print(hour); Serial.print(":");
   Serial.print(minute); Serial.print(":");
   Serial.println(second);
   */
  getGPS();
  Serial.print(LAT/100000,7);
  Serial.print(",");
  Serial.println(LON/100000,7);
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
  while (Serial1.available())
  {
    if (gps.encode(Serial1.read()))
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

