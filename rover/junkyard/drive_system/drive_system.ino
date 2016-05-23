#include <Servo.h>

// ascii character map http://www.asciitable.com/index/asciifull.gif

Servo myservo1;  //Front left wheel
Servo myservo2;  //Back left wheel

int TOPSPEED = 10;

int forwardSpeed = 96;
int zeroSpeed = 93;
int backSpeed = 90;
int inByte;
String state;

String FORWARD = "state:forward"; 
String BACKWARD = "state:backward"; 
String LEFT = "state:left"; 
String RIGHT = "state:right"; 
String STOP = "state:stop"; 

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);

  myservo1.attach(12); //Left wheel
  myservo2.attach(13); //Right wheel
}

void loop(){
  if(Serial1.available()){
    inByte = Serial1.read();
    Serial.print(inByte);
  }

  if (inByte == 115){
    state = FORWARD;
  }else if(inByte == 119){
    state = BACKWARD; 
  }else if(inByte == 97){
    state = LEFT; 
  }else if(inByte == 100){
    state = RIGHT;
  }else if (0 < inByte && inByte < 255){
    state = STOP; 
  }

  if (state == FORWARD){ //Both forward
    actuate_motor(myservo1, forwardSpeed); //Front left wheel moves faster than right wheels
    actuate_motor(myservo2, backSpeed); //Back left wheel moves faster than right wheels
  }else if(state == BACKWARD){ //Both backward
    actuate_motor(myservo1, backSpeed); //Front left wheel moves faster than right wheels
    actuate_motor(myservo2, forwardSpeed); //Back left wheel moves faster than right wheels
  }else if(state == LEFT){ //Turn left
    actuate_motor(myservo1, backSpeed); //Front left wheel moves faster than right wheels
    actuate_motor(myservo2, backSpeed); //Back left wheel moves faster than right wheels
  }else if(state == RIGHT){ //Turn Right
    actuate_motor(myservo1, forwardSpeed); //Front left wheel moves faster than right wheels
    actuate_motor(myservo2, forwardSpeed); //Back left wheel moves faster than right wheels
  }else if (state == STOP){
    actuate_motor(myservo1, zeroSpeed); //Front left wheel moves faster than right wheels
    actuate_motor(myservo2, zeroSpeed); //Back left wheel moves faster than right wheels
  }
}

void actuate_motor(Servo servo, float velocity){
  servo.write(velocity);
}
