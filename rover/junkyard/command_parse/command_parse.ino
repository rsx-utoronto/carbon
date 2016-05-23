String inData;
boolean keep_reading = false;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {

  while (Serial1.available() > 0)
  {
    char received = Serial1.read();
    //Serial.print(received);

    if (received == '<') {
      keep_reading = true;
      inData = "";
      continue;
    }

    if (received == '>') {
      //Serial.println(getValue(inData, ',', 1).toInt());
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
    0, -1  };
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



