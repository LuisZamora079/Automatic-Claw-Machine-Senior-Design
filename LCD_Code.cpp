#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


const int rs = 6, en = 7, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

typedef struct _task{
	signed char state; 		    //Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

#define NUM_TASKS 2 //TODO: Change to the number of tasks being used
task tasks[NUM_TASKS];

int message[2]; //Message from other transmitter
int joy[2]; // [0] = joystick |||| [1] = button
int status[2]; //Flag for Manual or Automatic

RF24 radio(9, 8); // CE, CSN

byte pipenum;
const byte JoyPipe[5] = {'J','O','Y','T','X'}; //|||||ADDRESSES FOR TRANSMITTING AND RECEIVING|||||||||||
const byte LCDpipe[5] = {'L','C','D','T','X'};

int Home = 1, Auto =0, LCDMode=0,dir = 0, Manual = 0;
int pos,line, press, menu, row, wait,Prize;

enum LCD_States {LCD_INIT,LCD_wait, LCD_Home, LCD_Manual, LCD_Auto,LCD_Cursor, LCD_Auto_Wait, LCD_Manual_Wait} LCD_state;
enum JoyStick_States{J_INIT, J_wait, J_release}J_state;
int Y =0;
int temp[2];

int LCD_States(int state){
  switch(state){ //transitions
    case LCD_INIT:
      // Serial.println("In LCD_INIT");
      lcd.clear();
      lcd.noCursor();
      pos = 7;  //position of the cursor on line
      Home = 1; //flag for home displaying
      line = 0; // what line the cursor is on
      menu = 0; // Automatic menu screen is displayed
      row = 0;  // what row the selection cursor is on on the menu screen
      Auto = 0;
      Manual = 0;
      Prize = 0;
      // press = 0;
      status[0] = 0;
      wait = 0;
      if(press == 0 && dir == 0){
        press = 0;
        state = LCD_wait;
      }
      break;

    case LCD_wait:
      if(Home == 1){ //Home == 1 Tells system to display the home screen
        Auto = 0;   // Flag that Auto was Selected
        Manual = 0; //Flag that Manual was selected
        Home = 2; // flag for home is being displayed (Manual and Automatic screen)
        state = LCD_Home;
      }
      else if(Auto == 1 && Home == 2 && menu == 0){ 
        menu = 1; // Options for Auto is displayed (FLAG)
        pos = 4;  
        line = 0;
        state = LCD_Auto;
      }
      else if(Home == 2 && Manual ==1 && Auto == 0 && menu == 0){
        // Serial.println("In wait to manual");
        Home = 0; // deactives cursor control
        state = LCD_Manual;
      }
      else if (Auto ==1 && menu ==1 && Home == 2 && Prize == 1){
        Home = 0; // no more selection ... deactivates cursor control
        state = LCD_Auto_Wait;
      }
      if((dir != 0 || press !=0) && Home == 2 ){
        state = LCD_Cursor;
      }
      break;

    case LCD_Home:
      state = LCD_wait;
      break;

    case LCD_Manual:
      // Serial.println("in Manual for Press");
      status[0] = 1; //Manual
      status[1] = 455;
      press = 0;
      // delay(5);
      state = LCD_Manual_Wait;
      break;
    
    case LCD_Auto:
      state = LCD_wait;
      break;

    case LCD_Auto_Wait:
      
      status[1] = 455;
      state = LCD_Manual_Wait;
      break;
    case LCD_Manual_Wait:
      if(press == 1  && wait ==1){
        state = LCD_INIT;
      }
      break;
    
    case LCD_Cursor:
      state = LCD_wait;
    default:
    break;
  }
  switch(state){ // actions
    case LCD_INIT:
      break;
    case LCD_wait:
        
      break;
    case LCD_Home:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Manual");
      lcd.setCursor(0, 1);
      lcd.print("Automatic");
      lcd.setCursor(pos, 0);
      lcd.print("<");
      break;
    
    case LCD_Manual:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Good Luck!");
      wait = 1;
      break;
    
    case LCD_Auto:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Red");
      lcd.setCursor(0, 1);
      lcd.print("Blue");
      lcd.setCursor(8, 0);
      lcd.print("Orange");
      lcd.setCursor(8, 1);
      lcd.print("Purple");
      lcd.setCursor(pos, 0);
      lcd.print("<");
      break;
    case LCD_Auto_Wait:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Waiting for");
      lcd.setCursor(6, 1);
      lcd.print("Prize");
      break;
    case LCD_Manual_Wait:

      break;
    case LCD_Cursor:
      if(dir == 2 && line == 1){ // up
        lcd.setCursor(pos,line);
        lcd.print(" ");
        line = 0;
        if(Home == 2 && menu == 0){
          pos = 7;
          lcd.setCursor(pos,line);
          lcd.print("<");
        }
        else if(menu ==1){
          if(row == 1){ // orange
            pos = 15;
          }
          else if(row == 0){ // red
            pos =4;
          }
          lcd.setCursor(pos,line);
          lcd.print("<");
        }
        
      }
      if(dir == 3 && line == 0 ){ //down
        // Serial.println("In Dir 2");
        lcd.setCursor(pos,0);
        lcd.print(" ");
        line = 1;
        if(Home == 2 && menu == 0){
          pos = 10;
          lcd.setCursor(pos,line);
          lcd.print("<");
        }
        else if(menu ==1){
          if(row == 1){ //Purple
            pos = 15;
          }
          else if(row == 0){ //blue
            pos =5;
          }
          lcd.setCursor(pos,line);
          lcd.print("<");
        }
      }
      if(dir == 1 && row ==0){ //left
        if(menu ==1){
          lcd.setCursor(pos,line);
          lcd.print(" ");
          row = 1;
          pos = 15;
          lcd.setCursor(pos,line);
          lcd.print("<");
        }
      }
      if(dir == 4 && row == 1){ //right
        if(menu ==1){
          lcd.setCursor(pos,line);
          lcd.print(" ");
          row = 0;
          if(line == 0){
            pos = 4;
          }
          else if(line == 1){
            pos = 5;
          }
          lcd.setCursor(pos,line);
          lcd.print("<");
        }
        row = 0;
      }
      if(press == 1 && menu == 0){
        if(line ==1 && Home == 2){
          Auto = 1;
          status[0] = 3;
        }
        if(line == 0 && Home == 2){
          Manual = 1;
        }
        press = 0;
      }
      if(press == 1 && menu == 1){
        // Serial.println("in Auto Selection press");
        if(line == 0 && row == 0){
          status[0] = 10; // Red
        }
        if(line == 0 && row == 1){
          status[0] = 15; // Oranfge
        }
        if(line == 1 && row == 0){
          status[0] = 20; // Blue
        }
        if(line == 1 && row == 1){
          status[0] = 25; // Purple
        }
        press = 0;
        Prize = 1;
        wait = 1;
      }
      dir = 0;
      
    break;
    default:
    break;
  } 
  return state;
}
int JoyStick_States(int state){
  switch(state){ //transitions
    case J_INIT:
      state = J_wait;
      break;
    case J_wait:

      if(joy[0] == 1){  //up
        dir = 1;
        state = J_release;
      }
      else if(joy[0] == 2){ //down
        dir = 2; //down
        state = J_release;
      }
      else if(joy[0] == 3){ //left
        dir = 3;
        state = J_release;
      }
      else if(joy[0] == 4){ //right
        dir = 4;
        state = J_release;
      }
      else if(joy[0] == 5){ //Button is pressed
        // Serial.println("Pressed");
        press =1;
        state = J_release;
      }
      else{
        state = J_wait;
      }
      break;
    case J_release:
      if(joy[0] == 0){
        dir = 0;
        press =0;
        state = J_wait;
      }
      break;

    default:
    break;
  }
  switch(state){ // action
    case J_wait:
      break;
    case J_release:
      break;
    default:
    break;
  }
  return state;
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  radio.begin();
  radio.setAutoAck(true);
  radio.openWritingPipe(LCDpipe);    //To motor
  radio.openReadingPipe(0, JoyPipe); //from joystick
  radio.setPALevel(RF24_PA_LOW);
  radio.setRetries(5,15);
  tasks[0].state = LCD_INIT;
  tasks[0].TickFct = &LCD_States;
  tasks[1].state = J_INIT;
  tasks[1].TickFct = &JoyStick_States;
}

void loop() {
  radio.startListening();
  if(radio.available(&pipenum)){
    Serial.println(message[1]);
    radio.read(&message, sizeof(message));
    if(message[1] == 123){ //123 id for Joystick
      joy[0] = message[0];
    }
    // Serial.print("Joy[0] = ");
    // Serial.println(joy[0]);
    // Serial.print("Joy[1] = ");
    // Serial.println(joy[1]);
  }

  tasks[1].state = tasks[1].TickFct(tasks[1].state);
  tasks[0].state = tasks[0].TickFct(tasks[0].state);

  if(status[0] != 0){
    // Serial.print("Status: ");
    // Serial.println(status[0]);
    status[1] = 455;
    radio.stopListening();
    radio.write(&status, sizeof(status));
    if(status[0] == 3){
      status[0] = 0;
      delay(3000);
    }
    status[0] = 0;
  }
}

