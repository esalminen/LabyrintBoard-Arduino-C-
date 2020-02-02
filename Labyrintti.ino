#include <Servo.h>
#include <ctype.h>
#include <LiquidCrystal.h>

#define ServoXPin 9
#define ServoYPin 10

// Arduino variables
Servo servoX;
Servo servoY;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
String ctrlString ="";
char ctrlStringLCDrow1[16];
char ctrlStringLCDrow2[16];
String servoXnameString = "";
String servoYnameString = "";
String servoXcmdString = "";
String servoYcmdString = "";
bool ctrlStringRead = false;
int timer = 0;
int prevTimer = 0;
int timeDelta = 0;
int LcdScreenNo = 0;

// Commands from .NET application
bool bServoX_EnableCmd = false;     // Enable command
bool bServoY_EnableCmd = false;
int nServoX_Ns = 0;                 // Setpoint for servo position (0-180)
int nServoY_Ns = 0;
int nServoX_OperRangeNs = 0;        // Servo operating range e.g. 100 -> possible servo position 40-140 (90-(100/2) - 90+(100/2))
int nServoY_OperRangeNs = 0;
int nServoX_CorrNs = 0;             // Correction value. This is used to get servo axis straight in midposition
int nServoY_CorrNs = 0;

// Feedback to .NET application
float fX_AxisAngleMe = 0;           // Angle measurement from GY-521 sensor
float fY_AxisAngleMe = 0;
float fZ_AxisAngleMe = 0;

void setup() {
  lcd.begin(16, 2);
  servoX.attach(ServoXPin);
  servoY.attach(ServoYPin);
  Serial.begin(115200);
}

void loop() {    

    // Read data from .NET application
    if (Serial.available() > 0) {
      ReadSerialData();    
    }
    
    // Handle control string if flag is raised
    if (ctrlStringRead) {
      HandleCtrlString();
    }
    
     // Control servo motors
     ServoControl(servoX, bServoX_EnableCmd, nServoX_Ns, nServoX_OperRangeNs, nServoX_CorrNs);
     ServoControl(servoY, bServoY_EnableCmd, nServoY_Ns, nServoY_OperRangeNs, nServoY_CorrNs);

     // Write data to .NET application

     // Write control data to LCD
     if (Timer(1000)){
         UpdateLCD(LcdScreenNo);
     }
}

void ServoControl(Servo& servo, bool enable, int servoNs, int servoOpeRangeNs, int servoCorrNs){
  
  
    // Calculate servo midpoint with given corretion value
    int servoMidPoint = 90 + servoCorrNs;
    
    // Calculate max operating range limits
    int minLim = servoMidPoint - servoOpeRangeNs / 2;
    int maxLim = servoMidPoint + servoOpeRangeNs / 2;    

    // If Enable is on, follow setpoints and corrections from .NET application, else put servo on mid position
    if (enable) {
        servo.write(map(servoNs, 0, 180, minLim, maxLim));
      }
    else servo.write(servoMidPoint);
  }
  
void ReadSerialData(){
  
  ctrlString = "";
  while(true) {
    
    // Peek if anything to read. Jump in to loop if there is
    if(Serial.peek() > 0) {
    char inChar = (char)Serial.read(); 
     
    // Jump out of loop if char is newline
    if(inChar == '\n') break; 
    
    ctrlString += inChar;
    }  
  }  
  // Raise flag to indicate there is new data in ctrl string
  ctrlStringRead = true;
}

void HandleCtrlString(){
  
  // Reset flag
  ctrlStringRead = false;
   
  // Ctrl string is following format: ServoX_1_90_180_0#ServoY_1_90_180_0# where -> 
  // "COMPONENTNAME1_ENABLECMD_NS_OPERANGENS_CORRNS+ENDOFMESSAGE+COMPONENTNAME2_ENABLECMD_NS_OPERANGENS_CORRNS+ENDOFMESSAGE"
  int indexOfSeparator = ctrlString.indexOf('#');
  servoXcmdString = ctrlString.substring(0, indexOfSeparator);
  servoXcmdString.substring(5).toCharArray(ctrlStringLCDrow1, 16);  
  ctrlString.remove(0,servoXcmdString.length() + 1);
  indexOfSeparator = ctrlString.indexOf('#');
  servoYcmdString = ctrlString.substring(0, indexOfSeparator);
  servoYcmdString.substring(5).toCharArray(ctrlStringLCDrow2, 16);  
  
  
  HandleServoString(servoXcmdString, servoXnameString, bServoX_EnableCmd, nServoX_Ns, nServoX_OperRangeNs, nServoX_CorrNs);
  HandleServoString(servoYcmdString, servoYnameString, bServoY_EnableCmd, nServoY_Ns, nServoY_OperRangeNs, nServoY_CorrNs);
  ctrlString = ""; 
  }

void HandleServoString(String cmdString, String &servoName, bool &enableCmd, int &Ns, int &operRangeNs, int &corrNs)  {

    // Search first separator and copy first part to servo variable
    int indexOfSeparator = cmdString.indexOf('_');
    servoName = cmdString.substring(0, indexOfSeparator); 
    cmdString.remove(0, servoName.length() + 1);
    
    // Search second separator and copy second part to servo variable
    indexOfSeparator = cmdString.indexOf('_');
    String temp = cmdString.substring(0, indexOfSeparator);
    enableCmd = temp.toInt();
    cmdString.remove(0, temp.length() + 1);

    // Search third separator and copy third part to servo variable
    indexOfSeparator = cmdString.indexOf('_');
    temp = cmdString.substring(0, indexOfSeparator);
    Ns = temp.toInt();
    cmdString.remove(0, temp.length() + 1);

    // Search fourth separator and copy fourth part to servo variable
    indexOfSeparator = cmdString.indexOf('_');
    temp = cmdString.substring(0, indexOfSeparator);
    operRangeNs = temp.toInt();
    cmdString.remove(0, temp.length() + 1);

    // Search fourth separator and copy fourth part to servo variable
    indexOfSeparator = cmdString.indexOf('_');
    temp = cmdString.substring(0, indexOfSeparator);
    corrNs = temp.toInt();
    cmdString.remove(0, temp.length() + 1);

//      Serial.println(servoName);
//      Serial.println(enableCmd);
//      Serial.println(Ns);
//      Serial.println(operRangeNs);
//      Serial.println(corrNs);
    }

void UpdateLCD(int screenNo) {
     lcd.clear();
     lcd.setCursor(0, 0);
     String firstRow = "X: ";
     String secondRow = "Y: ";
     lcd.print(firstRow + bServoX_EnableCmd + ' ' + nServoX_Ns + ' ' + nServoX_OperRangeNs + ' ' + nServoX_CorrNs + "      ");
     lcd.setCursor(0, 1);
     lcd.print(secondRow + bServoY_EnableCmd + ' ' + nServoY_Ns + ' ' + nServoY_OperRangeNs + ' ' + nServoY_CorrNs + "      ");
     //lcd.print(ctrlStringLCDrow1);
     lcd.setCursor(0, 1);
     //lcd.print(ctrlStringLCDrow2);
  }

bool Timer(int intervalMs) {
  // Record time and compare it to interval parameter. Return true is time recorded is greater or equal to interval parameter.
  prevTimer = timer;
  timer = millis();
  timeDelta += (timer - prevTimer);
  if(timeDelta >= intervalMs) {
      timeDelta = 0;
      return true;
    }
  return false;
  }
