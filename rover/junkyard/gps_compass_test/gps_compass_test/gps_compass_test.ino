#include <Wire.h>
#include <HMC5883L.h>

HMC5883L compass;

#include <TinyGPS.h>
#include <SoftwareSerial.h>

unsigned long fix_age;
TinyGPS gps;
void gpsdump(TinyGPS &gps);
bool feedgps();
void getGPS();
long lat, lon;
float LAT, LON;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // GPS module

  while (!compass.begin())
  {
    Serial.println("Could not find a valid HMC5883L sensor, check wiring!");
    delay(500);
  }

  // Set measurement range
  compass.setRange(HMC5883L_RANGE_1_3GA);

  // Set measurement mode
  compass.setMeasurementMode(HMC5883L_CONTINOUS);

  // Set data rate
  compass.setDataRate(HMC5883L_DATARATE_30HZ);

  // Set number of samples averaged
  compass.setSamples(HMC5883L_SAMPLES_8);

  // Set calibration offset. See HMC5883L_calibration.ino
  compass.setOffset(0, 0);
}

void loop()
{

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
  
  Serial.print(LAT/100000,7);
  Serial.print(",");
  Serial.print(LON/100000,7);  
  Serial.print(",");  
  Serial.println(headingDegrees);
  
  delay(100);
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

