#include <Arduino.h>
#include <Stepper.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <Servo.h>

Servo myservo;
Servo servo_2;
int pos = 0;
int lastx, lasty = 0;

int clawposx,clawposy = 0, cnty=0, cntx = 0, Found = 0;

const int stepPin = 3; 
const int dirPin = 2; 
const int stepPin1 = 4; 
const int dirPin1 = 5; 
const int stepPinClaw = 10;
const int dirPinClaw = 11;
const int Servo1 = 22;
const int Servo2 = 24;
int prev = 0;
byte pipenum;
int message[3];
int restart[2];
int Manual,reset,Automatic, Middle, Return = 0;
int coordx, coordy = 0;

RF24 radio(7, 8); // CE, CSN

const byte JoyPipe[5] = {'J','O','Y','T','X'};
const byte LCDpipe[5] = {'L','C','D','T','X'};
const byte Motorpipe[5] = {'M','O','T','T','X'};
const byte Campipe[5] = {'C','A','M','E','X'};


int joy[2];
int Status[2] = {0,0}; //[0] = Flags for manual and automatic selections
int Y = 0;

void setup() {
  Serial.begin(9600);
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  pinMode(stepPin1,OUTPUT); 
  pinMode(dirPin1,OUTPUT);
  pinMode(Servo1,OUTPUT); 
  pinMode(Servo2,OUTPUT);
  pinMode(stepPinClaw,OUTPUT); 
  pinMode(dirPinClaw,OUTPUT);
  myservo.attach(Servo1);
  servo_2.attach(Servo2);
  radio.begin();
  radio.setAutoAck(true);
  radio.openReadingPipe(0, JoyPipe); //from Joystick
  radio.openReadingPipe(1, LCDpipe); //from LCD
  radio.setRetries(5,15);
  radio.setPALevel(RF24_PA_LOW);
  Status[0] = 0;
  reset = 0;
  Middle = 0;
  myservo.write(0);
  servo_2.write(180);
}

void loop() { 
  radio.startListening();
  if(radio.available(&pipenum)){
    radio.read(&message, sizeof(message));
    // Serial.print("Message: ");
    // Serial.println(message[1]);
    if(message[1] == 123){
      joy[0] = message[0];
      if(joy[0] != 5){
        prev = 0;
      }
    }
    else if(message[1] == 455){
      Status[0] = message[0];

    }
    else if(message[1] == 789){ 
      coordx = message[0]; // x coord
      coordy = message[2]; // y coord
      Found = 1;
      radio.closeReadingPipe(0);
    }
    // Serial.print("Joy: ");
    // Serial.println(joy[0]);
    // Serial.print("Status: ");
    // Serial.println(Status[0]); 
    // Serial.print("coordx: ");
    // Serial.println(coordx);
    // Serial.print("coordy: ");
    // Serial.println(coordy);
  }
  if(Status[0] == 1){//Manual
    // Serial.println("In manual =1");
    Manual = 1;
    if(joy[0] == 5){
      prev = 5;
    }
    Status[0] = 0;
  }
  else if(Status[0] == 3){//Auto
    radio.closeReadingPipe(0);
    radio.openReadingPipe(0, Campipe); //from Camera
    Automatic = 1;
    Middle = 1;
    reset = 0;
    Status[0] = 0;
  }
  
  if(Manual == 1){
    digitalWrite(dirPin,HIGH);
    if(joy[0] == 2 && clawposy < 15){ //up
      cnty++;
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(500);
      if(cnty == 140){
        clawposy += 1;
        cnty = 1;
      }
    }
  
    digitalWrite(dirPin,LOW);
    if(joy[0] == 3 && clawposy > 0){ //down
      cnty--;
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(500);
      if(cnty == 0){
        clawposy -= 1;
        cnty = 139;
      }
    }
  
    digitalWrite(dirPin1,HIGH);
    if(joy[0] == 4 && clawposx < 15){ //Left
      cntx++;
      digitalWrite(stepPin1,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin1,LOW); 
      delayMicroseconds(500);
      if(cntx == 140){
        clawposx += 1;
        cntx = 1;
      }
    }
  
    digitalWrite(dirPin1,LOW);
    if(joy[0] == 1 && clawposx > 0){ // Right
      cntx--;
      digitalWrite(stepPin1,HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin1,LOW);
      delayMicroseconds(500);
      if(cntx == 0){
        clawposx -= 1;
        cntx = 139;
      }
    }
    if(joy[0] == 5 && prev != 5){ //Claw picks up Prize
        reset = 1;
      }
    }

    else if(Middle == 1){
      digitalWrite(dirPin,HIGH); //up
      for(int x = 0; x < (140*7); ++x){
        cnty++;
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500);
        if(cnty == 140){
          clawposy += 1;
          cnty = 1;
        }
      }
      delay(500);
      digitalWrite(dirPin1,HIGH);
      for(int x = 0; x < (140*7); ++x){ //Left
        cntx++;
        digitalWrite(stepPin1,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin1,LOW); 
        delayMicroseconds(500);
        if(cntx == 140){
          clawposx += 1;
          cntx = 1;
        }
      }
      Middle = 0;
    }

    if(Automatic == 1 && Found == 1){
      delay(1000);
      if(coordx != -1 && coordy != -1){
        if(clawposx < coordx){
          int temp1 = coordx - clawposx;
          digitalWrite(dirPin1,HIGH);
          for(int x = 0; x < (140*(temp1)); ++x){ //Left
            cntx++;
            digitalWrite(stepPin1,HIGH); 
            delayMicroseconds(500); 
            digitalWrite(stepPin1,LOW); 
            delayMicroseconds(500);
            if(cntx == 140){
              clawposx += 1;
              cntx = 1;
            }
          }
        }
        else if(clawposx > coordx){
          int temp2 = clawposx - coordx;
          digitalWrite(dirPin1,LOW);
          for(int x = 0; x < (140*(temp2)); ++x){ //Left
            cntx--;
            digitalWrite(stepPin1,HIGH); 
            delayMicroseconds(500); 
            digitalWrite(stepPin1,LOW); 
            delayMicroseconds(500);
            if(cntx == 0){
              clawposx -= 1;
              cntx = 139;
            }
          }
        }
        delay(500);
        if(clawposy < coordy){
          int temp3 = coordy - clawposy;
          digitalWrite(dirPin,HIGH);
          for(int x = 0; x < (140*(temp3)); ++x){ //up
            cnty++;
            digitalWrite(stepPin,HIGH); 
            delayMicroseconds(500); 
            digitalWrite(stepPin,LOW); 
            delayMicroseconds(500);
            if(cnty == 140){
              clawposy += 1;
              cnty = 1;
            }
          }
        }
        else if(clawposy > coordy){
          int temp4 = clawposy - coordy;
          digitalWrite(dirPin,LOW);
          for(int x = 0; x < (140*(temp4)); ++x){ //down
            cnty--;
            digitalWrite(stepPin,HIGH); 
            delayMicroseconds(500); 
            digitalWrite(stepPin,LOW); 
            delayMicroseconds(500);
            if(cnty == 0){
              clawposy -= 1;
              cnty = 139;
            }
          }
        }
        delay(500);
        reset = 1;
        Found = 0;
    }
    else{
      Return = 1;
      Found = 0;
    }
  }

  if(Return == 1){
    delay(700);
    digitalWrite(dirPin1,LOW);
    for(int x = (140*clawposx) + cntx; x > 0; x--){ //X axis motor returns to origin
      digitalWrite(stepPin1,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin1,LOW); 
      delayMicroseconds(500);
    }
    delay(700);
    digitalWrite(dirPin,LOW);
    for(int x = (140*clawposy) + cntx; x > 0; x--){ // Y axis motor returns to origin
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(500);
    }
    Return = 0;
    Status[0] = 0;
    Manual = 0;
    Automatic = 0;
    //claw coordinates variables
    clawposx = 0; 
    clawposy = 0;
    cntx = 0;
    cnty = 0;
    // Automatic coordiantes 
    radio.closeReadingPipe(0);
    radio.openReadingPipe(0, JoyPipe); //from Joystick
    coordx = 0;
    coordy = 0;
  }
  
  if(reset == 1){
    // Serial.print("In reset");
    digitalWrite(dirPinClaw,LOW);
    for(int x = 0; x < 3800; x++) { //Claw moves down
      digitalWrite(stepPinClaw,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPinClaw,LOW); 
      delayMicroseconds(500);
    }
    delay(1500);
    for (pos = 0; pos <= 280; pos += 10){ //Servos close claw
      myservo.write(pos); 
      // Serial.println(pos);
      servo_2.write(180-pos);
    }
    delay(500);
    digitalWrite(dirPinClaw,HIGH);
    for(int x = 0; x < 3800; x++) { //Claw moves back up
      digitalWrite(stepPinClaw,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPinClaw,LOW); 
      delayMicroseconds(500);
    }
    delay(700);
    digitalWrite(dirPin1,LOW);
    for(int x = (140*clawposx) + cntx; x > 0; x--){ //X axis motor returns to origin
      digitalWrite(stepPin1,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin1,LOW); 
      delayMicroseconds(500);
    }
    delay(700);
    digitalWrite(dirPin,LOW);
    for(int x = (140*clawposy) + cntx; x > 0; x--){ // Y axis motor returns to origin
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(500);
    }

    delay(1000); // Servos open claw
    myservo.write(0);
    servo_2.write(180);
  
    // Serial.println("In reastart");
    Status[0] = 0;
    Manual = 0;
    Automatic = 0;
    reset = 0;
    //claw coordinates variables
    clawposx = 0; 
    clawposy = 0;
    cntx = 0;
    cnty = 0;
    // Automatic coordiantes 
    radio.closeReadingPipe(0);
    radio.openReadingPipe(0, JoyPipe); //from Joystick
    coordx = 0;
    coordy = 0;
  }

}
