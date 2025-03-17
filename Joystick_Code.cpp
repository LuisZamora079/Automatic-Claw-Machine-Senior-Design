#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define SwitchPin1 3
#define SwitchPin2 4
#define SwitchPin3 5
#define SwitchPin4 6
#define button A3

int SwitchState1 = 0;
int SwitchState2 = 0;
int SwitchState3 = 0;
int SwitchState4 = 0;
int ButtonState = 0;
int prev;

int state[2]; // [0] = joystick |||| [1] = id

RF24 radio(9, 8); // CE, CSN

// const byte Joypipe[6] = "00002";
const byte JoyPipe[5] = {'J','O','Y','T','X'};

void setup() {
  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  pinMode(SwitchPin4, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
  Serial.begin(9600);
  radio.begin();
  radio.setAutoAck(true);
  radio.openWritingPipe(JoyPipe);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
  radio.setRetries(5,15);
}
void loop() {
    SwitchState1 = digitalRead(SwitchPin1); //up
    SwitchState2 = digitalRead(SwitchPin2); //down
    SwitchState3 = digitalRead(SwitchPin3); //left
    SwitchState4 = digitalRead(SwitchPin4); //right
    ButtonState = analogRead(button);

    if( SwitchState1 & SwitchState2 & SwitchState3 & SwitchState4 ){
      if(ButtonState < 50){ //press when no other button is pressed
        state[0] = 5;
      }
      else{
        state[0] = 0; //does nothing
      }
    }
    else if(~SwitchState1 & SwitchState2 & SwitchState3 & SwitchState4 ){ // up
      state[0] = 1;
    }
    else if(~SwitchState2 & SwitchState1 & SwitchState3 & SwitchState4 ){ //down
      state[0] = 2;
    }
    else if(~SwitchState3 & SwitchState2 & SwitchState1 & SwitchState4 ){ //left
      state[0] = 3;
    }
    else if(~SwitchState4 & SwitchState2 & SwitchState3 & SwitchState1 ){ //right
      state[0] = 4;
    }

  // Serial.print("prev: ");
  // Serial.println(prev);
    
  if(prev != state[0]){
    prev = state[0]; state[1] = 123; // 123 = joystick id
    radio.write(&state, sizeof(state));
    // Serial.println("Change 00");
    // Serial.print("state0: ");
    // Serial.println(state[0]);
    // Serial.print("state1: ");
    // Serial.println(state[1]);
    delay(100);
  }
}
