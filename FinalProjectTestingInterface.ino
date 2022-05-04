//////////////////////////////////////////////////////////////////////////////////////
//  Smart Chair Project - Wearable and Ubiquitous Computing - UARK 2022
//  Joshua Davis and Trenton Bryant
//////////////////////////////////////////////////////////////////////////////////////
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Adafruit_DRV2605.h"
#include "pitches.h"
#include <String.h>

//////////////////////////////////////////////////////////////////////////////////////
//  GLOBALS
//////////////////////////////////////////////////////////////////////////////////////
//_________________________________
// Default Values /////////////////
const int STANDING_TIME = 2;
const int WEIGHT_BUFFER = 100;
const int POSTURE_FLAG_TIME = 5;
const int INACTIVITY_TIME = 5;

//_________________________________
// Buttons ////////////////////////
const int btnUp = 23;
const int btnConfirm = 24;
const int btnDown = 22;
int cBS1;
int pBS1 = LOW;
int cBS2;
int pBS2 = LOW;
int cBS3;
int pBS3 = LOW;
unsigned long DT1 = 0;
unsigned long DT2 = 0;
unsigned long DT3 = 0;
unsigned long debounceDelay = 20;

//_________________________________
// LCD ////////////////////////////
LiquidCrystal_I2C lcd(0x27,20,4);
uint8_t blockChar[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t upArrowChar[8] = {0x04,0x0E,0x1F,0x0E,0x0E,0x0E,0x0E,0x0E};
uint8_t downArrowChar[8] = {0x0E,0x0E,0x0E,0x0E,0x0E,0x1F,0x0E,0x04};
uint8_t sideArrowChar[8] = {0x00,0x04,0x1E,0x1F,0x1F,0x1E,0x04,0x00};
boolean screenChange = true;
boolean alarmActive = false;
String alarmMessage = "";

//_________________________________
// Force Sensors //////////////////
const int FS1 = A0;
const int FS2 = A1;
const int FS3 = A2;
const int FS4 = A4;
const int FS5 = A3;
const int MIN_THRESHOLD = 25;  //Was 10
const int MID_THRESHOLD = 100; //Was 200
const int MAX_THRESHOLD = 300; //Was 500
int currBar[5] = {0, 0, 0, 0, 0};
String BAR_HEADER = "FLT BLT MID BRT FRT ";
boolean updateScreen = true;

//_________________________________
// Haptic Motor ///////////////////
Adafruit_DRV2605 drv;
const int motorPin = 25;
const int haptic_strong_pulse = 58;
const int haptic_medium_pulse = 74;
const int haptic_tap = 78;
const int haptic_nudge = 90;

//_________________________________
// Timers /////////////////////////
static long sittingPresentTime = 0;
static long sittingPastTime = 0;
int sittingTimer = 10;
static long standingPresentTime = 0;
static long standingPastTime = 0;
int standingTimer = STANDING_TIME;
static long posturePresentTime = 0;
static long posturePastTime = 0;
int postureTimer = 0;
static long menuActivityPresentTime = 0;
static long menuActivityPastTime = 0;
int menuActivityTimer = 0;

//_________________________________
// Menu ///////////////////
String menu[6] = {
  " Vibration:     ",
  " Sound:         ",
  " Track Sitting: ",
  " Track Posture: ",
  " Sit Time:      ",
  " Stand Time:    ",
};
int settings[6] = {1, 1, 1, 1, 10, 3};
bool activeMenu = false;
int buttonInterface = 0;

//_________________________________
// Buzzer /////////////////////////
const int buzzer = 53;
const int startUpNumNotes = 5;
const int notificationNumNotes = 3;
const int alarmNumNotes = 3;
int startUpMelody[] = {
  NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_DS5, NOTE_AS4
};
int notificationMelody[] = {
  NOTE_GS4, NOTE_DS5, NOTE_AS4
};
int alarmMelody[] = {
  NOTE_AS5, NOTE_DS5, NOTE_AS5
};
//note durations: 4 = quarter note, 8 = eighth note, etc.
int startUpDuration[] = {4, 4, 2, 4, 2};
int notificationDuration[] = {8, 12, 12};
int alarmDuration[] = {12, 8, 16};

//////////////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  printMenu()
//    Prints the menu text based on the currently selected menu option.
//    Checks rows below the current row and their contents. 
//    Calls menuOptionSetting to print the settings for each of the menu options as
//    well.
//====================================================================================
void printMenu(){
  lcd.clear();
  
  int oneBelow = buttonInterface + 1;
  int twoBelow = buttonInterface + 2;
  int threeBelow = buttonInterface + 3;

  if(buttonInterface == 3){
    threeBelow = 0;
  }
  else if(buttonInterface == 4){
    twoBelow = 0;
    threeBelow = 1;
  }
  else if(buttonInterface == 5){
    oneBelow = 0;
    twoBelow = 1;
    threeBelow = 2;
  }
  
  writeAtRow(0, menu[buttonInterface]);
  writeAtRow(1, menu[oneBelow]);
  writeAtRow(2, menu[twoBelow]);
  writeAtRow(3, menu[threeBelow]);
  menuOptionSetting(oneBelow, twoBelow, threeBelow);
  printArrows();
}

//====================================================================================
//  printArrows()
//    Prints stationary arrows at specific points on the screen to help with user 
//    navigation.
//====================================================================================
void printArrows(){
  lcd.setCursor(19, 0);
  lcd.write((char)1);
  lcd.setCursor(19, 3);
  lcd.write((char)2);
  lcd.setCursor(0, 0);
  lcd.write((char)3);
}

//====================================================================================
//  menuOptionSetting()
//    Shows the saved values below the current selection on the menu.
//    Essentially prints rows 2, 3, and 4 on the LCD screen, but only the status of
//    them. 
//====================================================================================
void menuOptionSetting(int oneBelow, int twoBelow, int threeBelow){
  lcd.setCursor(16, 0);
  if(buttonInterface == 4 || buttonInterface == 5){
    lcd.print(settings[buttonInterface]);
  }
  else{
    printOffOn(settings[buttonInterface]);
  }
  lcd.setCursor(16, 1);
  if(oneBelow == 4 || oneBelow == 5){
    lcd.print(settings[oneBelow]);
  }
  else{
    printOffOn(settings[oneBelow]);
  }
  lcd.setCursor(16, 2);
  if(twoBelow == 4 || twoBelow == 5){
    lcd.print(settings[twoBelow]);
  }
  else{
    printOffOn(settings[twoBelow]);
  }
  lcd.setCursor(16, 3);
  if(threeBelow == 4 || threeBelow == 5){
    lcd.print(settings[threeBelow]);
  }
  else{
    printOffOn(settings[threeBelow]);
  }
}

//====================================================================================
//  printOffOn()
//    Prints off or on to the LCD menu based on the passed integer. 
//====================================================================================
void printOffOn(int num){
  if(num == 1){
      lcd.print("ON ");
    }
    else{
      lcd.print("OFF");
    }
}

//====================================================================================
//  playMelody()
//    Plays a melody based on passed arrays. 
//    Note frequencies are pulled from pitches.h.
//====================================================================================
void playMelody(int notes[], int durations[], int numNotes){
  for (int note = 0; note < numNotes; note++) {
    int noteDuration = 1000 / durations[note];
    tone(buzzer, notes[note], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(buzzer);
  }
}

//====================================================================================
//  setup()
//====================================================================================
void setup() {
  Serial.begin(9600);
  
  //Initialize Pins
  pinMode(motorPin, OUTPUT);
  pinMode(btnUp, INPUT);
  pinMode(btnConfirm, INPUT);
  pinMode(btnDown, INPUT);

  //Initialize Haptic Library
  drv.begin();
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG); 

  //Initialize LCD
  lcd.backlight();
  lcd.begin(20, 4);
  lcd.createChar(0, blockChar);
  lcd.createChar(1, upArrowChar);
  lcd.createChar(2, downArrowChar);
  lcd.createChar(3, sideArrowChar);
  writeAtRow(0, "- - - - - - - - - - ");
  writeAtRow(1, "Wearable/Ubiquitous ");
  writeAtRow(2, "Smart Chair Project ");
  writeAtRow(3, " - - - - - - - - - -");
  
  //Play the startup melody, then clear screen
  playMelody(startUpMelody, startUpDuration, startUpNumNotes);
  delay(1000);
  lcd.clear();
}

//====================================================================================
//  writeAtRow()
//    Writes the passed message at the passed row, starting at 0. 
//====================================================================================
void writeAtRow(int row, String message){
  lcd.setCursor(0, row);
  if(message.length() > 20){
    char messageArray[message.length() + 1];
    strcpy(messageArray, message.c_str());
    for(int i = 0; i < message.length(); i++){
      if(i < 20){
        lcd.print(messageArray[i]);
        if(i == 19){
          lcd.setCursor(0, row+1);
        }
      }
      else if(i < 40){
        lcd.print(messageArray[i]);
        if(i == 39){
          lcd.setCursor(0, row+2);
        }
      }
      else if(i < 60){
        lcd.print(messageArray[i]);
        if(i == 59){
          lcd.setCursor(0, row+2);
        }
      }
      else if(i < 80){
        lcd.print(messageArray[i]);
      }
      else{
        continue; //Message overflows
      }
    }
  }
  else{
    lcd.print(message);
  }
}

//====================================================================================
//  writeBlockAtPos()
//    Writes a block character at a specfic position on the LCD
//====================================================================================
void writeBlockAtPos(int row, int col){
  lcd.setCursor(col, row);
  lcd.write((char)0);
}

//====================================================================================
//  getAverageFSR()
//    Returns the average value of each force sensor
//====================================================================================
int getAverageFSR(){
  int average = (analogRead(FS1) + analogRead(FS2) + analogRead(FS3) 
                + analogRead(FS4) + analogRead(FS5))/5;
  return average;
}

//====================================================================================
//  chartFSR()
//    Updates the force sensor chart based on the analog reading of each sensor.
//====================================================================================
void chartFSR(){
  int FSR[] = {
    analogRead(FS1), analogRead(FS2), analogRead(FS3), analogRead(FS4), analogRead(FS5)
  };
  checkScreenState(FSR);
  if(updateScreen){
    lcd.clear();
    writeAtRow(0, BAR_HEADER);
    updateScreen = false;
    for(int i = 0; i < 5; i++){
      int pos = i*4;  //This marks the column where the bar graph starts
      if (FSR[i] < MIN_THRESHOLD){
        continue;
      }
      else if (FSR[i] < MID_THRESHOLD){
        writeBlockAtPos(3, pos);
        writeBlockAtPos(3, pos+1);
        writeBlockAtPos(3, pos+2);
      }
      else if (FSR[i] < MAX_THRESHOLD){
        writeBlockAtPos(3, pos);
        writeBlockAtPos(3, pos+1);
        writeBlockAtPos(3, pos+2);
        writeBlockAtPos(2, pos);
        writeBlockAtPos(2, pos+1);
        writeBlockAtPos(2, pos+2);
      }
      else {
        writeBlockAtPos(1, pos);
        writeBlockAtPos(1, pos+1);
        writeBlockAtPos(1, pos+2);
        writeBlockAtPos(2, pos);
        writeBlockAtPos(2, pos+1);
        writeBlockAtPos(2, pos+2);
        writeBlockAtPos(3, pos);
        writeBlockAtPos(3, pos+1);
        writeBlockAtPos(3, pos+2);
      }
    }
    delay(100);
  }
}

//====================================================================================
//  checkScreenState()
//    Looks to see if a bar should be printed again on the force sensor chart.
//    This is to avoid printing too many times, which leads to flickering
//====================================================================================
void checkScreenState(int FSR[]){
  for(int i = 0; i < 5; i++){
    if (FSR[i] < MIN_THRESHOLD){
      if(currBar[i] == 0){
        continue;
      }
      else{
        currBar[i] = 0;
        updateScreen = true;
      }
    }
    else if (FSR[i] < MID_THRESHOLD){
      if(currBar[i] == 1){
        continue;
      }
      else{
        currBar[i] = 1;
        updateScreen = true;
      }
    }
    else if (FSR[i] < MAX_THRESHOLD){
      if(currBar[i] == 2){
        continue;
      }
      else{
        currBar[i] = 2;
        updateScreen = true;
      }
    }
    else {
      if(currBar[i] == 3){
        continue;
      }
      else{
        currBar[i] = 3;
        updateScreen = true;
      }
    }
  }
}

//====================================================================================
//  checkUserState()
//    Checks the current state of the user based on an average reading from the 
//    force sensors. Only tracks sitting and posture if the user has set up the menu
//    to do so. Turns off the motor and resets posture timer if the force sensors
//    read 0, then starts tracking stand time.
//====================================================================================
void checkUserState(){
  int forceSensorReading = getAverageFSR();
  
  if(forceSensorReading > 0){
    if(settings[2] == 1){
      trackSitting();
    }
    if(settings[3] == 1){
      trackPosture();
    }
  }
  if(forceSensorReading <= 0){
    digitalWrite(motorPin, LOW);
    postureTimer = 0;
    trackStanding();
  }
}

//====================================================================================
//  trackSitting()
//    Updates sitting timer if a second has passed. 
//    Triggers alarm if user has sit for too long
//====================================================================================
void trackSitting(){
  sittingPresentTime = millis();
  standingTimer = 0;
  if(sittingPresentTime - sittingPastTime > 1000){
    sittingPastTime = millis();
    sittingTimer += 1;
  }
  if(sittingTimer > settings[4]){
    alarm();  
  }
}

//====================================================================================
//  trackStanding()
//    Updates standing timer if a second has passed. Sets sittingTimer to 0 and 
//    deactivates alarm when the appropriate time has passed.
//====================================================================================
void trackStanding(){
  standingPresentTime = millis();
  if(standingPresentTime - standingPastTime > 1000){
    standingPastTime = millis();
    standingTimer += 1;
  }
  if(standingTimer > settings[5]){
    sittingTimer = 0;
    alarmActive = false;
  }
}

//====================================================================================
//  trackPosture()
//    Averages the amount of force applied to the left and right force sensors, then
//    weighs the difference of those averages with a global buffer to determine
//    if the user is leaning too far to one side or not.
//====================================================================================
void trackPosture(){
  posturePresentTime = millis();

  //If user is leaning too far to the left
  if((analogRead(FS1) + analogRead(FS2))/2 - (analogRead(FS4) + analogRead(FS5))/2 > WEIGHT_BUFFER){
    if(posturePresentTime - posturePastTime > 1000){
      posturePastTime = millis();
      postureTimer += 1;
    }
    if(postureTimer > POSTURE_FLAG_TIME){
      digitalWrite(motorPin, HIGH);
      alarmMessage = "Adjust Right!";
      alarmActive = true;
    }
  }
  //If user is leaning too far to the right
  else if((analogRead(FS4) + analogRead(FS5))/2 - (analogRead(FS1) + analogRead(FS2))/2 > WEIGHT_BUFFER){
    if(posturePresentTime - posturePastTime > 1000){
      posturePastTime = millis();
      postureTimer += 1;
    }
    if(postureTimer > POSTURE_FLAG_TIME){
      drv.setWaveform(0, haptic_strong_pulse);
      drv.setWaveform(1, 0);
      drv.go();
      alarmMessage = "Adjust Left!";
      alarmActive = true;
    }
  }
  //If user is balanced
  else{
    postureTimer = 0;
    alarmActive = false;
    digitalWrite(motorPin, LOW);
  }
}

//====================================================================================
//  printSerialData()
//    Prints a list of variables to the serial for debugging
//====================================================================================
void printSerialData(){
  Serial.println("====================================================");
  Serial.print("standingTimer: ");
  Serial.println(standingTimer);
  Serial.print("sittingTimer: ");
  Serial.println(sittingTimer);
  Serial.print("postureTimer: ");
  Serial.println(postureTimer);
}

//====================================================================================
//  trackMenuActivity()
//    Keeps track of last time the menu was navigated (last button pressed)
//====================================================================================
void trackMenuActivity(){
  menuActivityPresentTime = millis();
  if(menuActivityPresentTime - menuActivityPastTime > 1000){
    menuActivityPastTime = millis();
    menuActivityTimer -= 1;
  }
}

//====================================================================================
//  alarm()
//    Triggers speakers/motors depending on menu settings
//====================================================================================
void alarm(){
  //If sensors are being pressed and both haptic and speakers are on
  if(getAverageFSR() > 0 && settings[0] && settings[1]){
    digitalWrite(motorPin, HIGH);
    drv.setWaveform(0, haptic_strong_pulse);
    drv.setWaveform(1, 0);
    drv.go();
    playMelody(alarmMelody, alarmDuration, alarmNumNotes);
    alarmMessage = "Stand up!";
    alarmActive = true;
  }
  //If sensors are being pressed and only haptic is on
  else if(getAverageFSR() > 0 && settings[0] && !settings[1]){
    digitalWrite(motorPin, HIGH);
    drv.setWaveform(0, haptic_strong_pulse);
    drv.setWaveform(1, 0);
    drv.go();
    alarmMessage = "Stand up!";
    alarmActive = true;
  }
  //If sensors are being pressed and only speakers are on
  else if(getAverageFSR() > 0 && !settings[0] && settings[1]){
    playMelody(alarmMelody, alarmDuration, alarmNumNotes);
    alarmMessage = "Stand up!";
    alarmActive = true;
  }
}

//====================================================================================
//  printAlarm()
//    Simple alarm alert, prints message and timer based on alarm triggered
//====================================================================================
boolean printAlarm(){

  //if(updateScreen){
    lcd.clear();
    writeAtRow(0, alarmMessage);
    if(alarmMessage == "Stand up!"){
      writeAtRow(1, String(settings[5] - standingTimer));
      lcd.setCursor(3, 1);
      lcd.print(" Seconds");
    }  
}

//====================================================================================
//  buttonPressed()
//    Handles button debouncing
//====================================================================================
boolean buttonPressed(int btn){
  static int buttonState = 0;
  static int debounce = 0;
  static long lastTime = millis();
  
  if(buttonState == 0 && digitalRead(btn)){
    menuActivityTimer = INACTIVITY_TIME;
    buttonState = 1;
    debounce = 1;
    lastTime = millis();
    return true;
  }
  else if(debounce == 1 && ((millis() - lastTime) > 100)){
    debounce = 0;
  }
  else{
    buttonState = digitalRead(btn);
    lastTime = millis();
  }
  return false;
}

//====================================================================================
//  trackButtons()
//    Reads each button and handles debouncing and menu navigation elements
//====================================================================================
void trackButtons(){
  int read1 = digitalRead(btnUp);
  int read2 = digitalRead(btnConfirm);
  int read3 = digitalRead(btnDown);
  
  //Up Button 
  if(read1 != pBS1){
    DT1 = millis();
  }
  if ((millis() - pBS1) > debounceDelay) {
    if (read1 != cBS1) {
      cBS1 = read1;
      if (cBS1 == HIGH) {
        menuActivityTimer = INACTIVITY_TIME;
        updateScreen = true;
        screenChange = true;
        if(buttonInterface > 0){
          buttonInterface -= 1;
        }
        else{
          buttonInterface = 5;
        }
      }
    }
  }

  //Down Button
  if ((millis() - pBS2) > debounceDelay) {
    if (read2 != cBS2) {
      cBS2 = read2;
      if (cBS2 == HIGH) {
        menuActivityTimer = INACTIVITY_TIME;
        updateScreen = true;
        screenChange = true;
        
        if(buttonInterface < 5){
          buttonInterface += 1;
        }
        else{
          buttonInterface = 0;
        }
      }
    }
  }
  
  //Confirm Button
  if ((millis() - pBS3) > debounceDelay) {
    if (read3 != cBS3) {
      cBS3 = read3;
      if (cBS3 == HIGH) {
        menuActivityTimer = INACTIVITY_TIME;
        updateScreen = true;
        screenChange = true;
        if(buttonInterface <= 3 ){
          if(settings[buttonInterface] == 0){
            settings[buttonInterface] = 1;
          }
          else{
            settings[buttonInterface] = 0;
          }
        }
        if(buttonInterface == 4){
          if(settings[buttonInterface] < 120)
          {
            settings[buttonInterface] += 10;
          }
          else
          {
            settings[buttonInterface] = 10;
          }
        }
        if(buttonInterface == 5){
          if(settings[buttonInterface] < 10)
          {
            settings[buttonInterface] += 1;
          }
          else
          {
            settings[buttonInterface] = 1;
          }
        }
      }
    }
  }
  pBS1 = read1;
  pBS2 = read2;
  pBS3 = read3;
}

//====================================================================================
//  loop()
//====================================================================================
void loop() {
  //Menu Overlay Swap/////////////////////////////////////////////////////////////////
  //If the alarm isn't going off, print displays
  if(!alarmActive){
    //Switch to Force Sensor Reader when menu isn't active
    if(menuActivityTimer <= 0){
      chartFSR();
    }
    //Otherwise switch to menu if a button was pressed
    else{
      if(screenChange){
        printMenu();
        screenChange = false;
      }
    }
  }
  //Otherwise, switch to the alarm screen
  else{
    printAlarm();
  }
  
  //Button Logic//////////////////////////////////////////////////////////////////////
  trackButtons();
  trackMenuActivity();
}
