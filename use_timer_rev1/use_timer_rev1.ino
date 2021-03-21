      ////--- SHELL Basic Interface  ---///
//  0.1 - based on shell_intervace_1.3
//  0.4 - production test 

int software1 = 0; // Software Series
int software2 = 4; // Software Rev

      //-- Libraries  --//
#include <Wire.h>  // (7% 140k) I2C communication
#include <LiquidCrystal_I2C.h>  // (7% 140k) LCD display (A4 SDA-Black A5 SCL-white)


  //////--  DESIGNATE CONFIGURATIONS  --////////

    //---INITIALIZE---//
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

    // Outputs     
  //#define LEDpin 13

   // Sensor Inputs
   #define switchPin 9

        
   // Button Pins and Variables
  int displayButton1Pin = 3;

    //-- Display Managment Variables --//
byte displayWait = 20; //loops (*10) set displays waits for input

boolean backlightStatus = 1;  // tells backlight to be off if zero (backlight is turned on in button debounce function and off in main loop)
byte backlightDelay = 240;  // backlight off delay in seconds
int backlightCount = backlightDelay;

      // Loop timing, button debounce variables
byte buttonSelected = 0;  // which button is pushed, 0 is no button selected
byte buttonCheck = LOW;  // low indicates all buttons released
byte lastSecond = 0;  // used for loop timing functions


byte upArrow[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
};

byte downArrow[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};


  //-- Timer Parameters --//
long loopMillis = 0; //used for loop timing
long checkLoop = 0;  // makes sure delay isn't long in reset
long elapsedMillis = 0;
long elapsedSeconds = 0;
long startMillis = 0;
long nowSeconds = 0;
#define LOOPTIME 500 // time for main loop
//int lastMinute = 0; // used for display refresh vs full update



  //////--  SETUP  --////////
  
void setup(){

  // start serial port to computer  
  Serial.begin(9600);
  Serial.println("wait to start");

  //--- initialize LCD
  lcd.init(); //initialize the lcd
  lcd.backlight(); //turns backlight on 
  lcd.createChar(1, upArrow);  //create special characters
  lcd.createChar(2, downArrow);


  //initial input/output pins  
    //pinMode(LEDpin, OUTPUT);
    pinMode(switchPin, INPUT);


  //initial inputs
    pinMode(displayButton1Pin,INPUT);  
    //pinMode(displayButton2Pin,INPUT);
    //pinMode(displayButton3Pin,INPUT); 

  //Dead Pin  
  //  pinMode(deadPin1,INPUT_PULLUP);


  ///--Initialize Processes--//
  Wire.begin(); 

 
 
//--- Startup Display (software rev)

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Use Timer");
  lcd.setCursor(1,1);
  lcd.print("S/W rev:  ");
  lcd.print(software1);
  lcd.print(".");
  lcd.print(software2);
      
  delay(2000);


  zeroDisplay();


} //-- end of pre start loop


////////-----  MAIN LOOP  -------///////

void loop(void){
  // ---- Check Switch ----
  //increment elapsed millis if switch closed
  if (digitalRead(switchPin) == 1){
    elapsedMillis = elapsedMillis + (millis() - loopMillis);
    elapsedSeconds = elapsedMillis / 1000;
    }

  // nowSeconds is total seconds 
  nowSeconds = (millis() - startMillis) / 1000;

  // reset loop timer, must be after swith check
  loopMillis = millis();

  // update display
  refreshDisplay(elapsedSeconds, nowSeconds, false);

  // --- button reset ---
  buttonSelected = 0;
  readButton1();
  if(buttonSelected == 1){  
      buttonSelected = 0;
      lcd.clear(); 
      lcd.setCursor(0,0);   
      lcd.print("HOLD TO RESET");
      lcd.setCursor(0,1);   
      lcd.print("ALL TIMERS");
      
      delay(2000);
      
      lcd.clear(); 
      lcd.setCursor(0,0);   
      lcd.print("you will lose");
      lcd.setCursor(0,1);   
      lcd.print("EVERYTHING");
      
      delay(2000); 
      readButton1();  // Second button debounce to avoid reset
      if(buttonSelected == 1){
        // check button for reset
        buttonSelected = 0;
        startMillis = millis();
        elapsedMillis = 0;
        elapsedSeconds = 0;
        buttonSelected = 0;
        zeroDisplay();
        // this is here because of delays in reset
        loopMillis = millis();
      }else{
        Serial.print("no reset");
        refreshDisplay(elapsedSeconds, nowSeconds, true);
      }// end button select if
      buttonSelected = 0;
      delay(2000);
  }


  // manage loop to about 1 second
  // conditional is due to issues during reset
  checkLoop = LOOPTIME - (millis() - loopMillis);
  if (checkLoop > LOOPTIME || checkLoop < 0){
    checkLoop = 0;
  }
  delay(checkLoop); // adjust for 1 second loop

}//end main loop





//////////////----- FUNCTIONS ------------/////////////

  ///--- Button Read and Debounce Functions ---////  

void readButton1(){
  boolean displayButton1State;
  displayButton1State = digitalRead(displayButton1Pin);   
  if(displayButton1State == HIGH){
    delay(50);
    if(displayButton1State == HIGH){
      buttonSelected = 1;
    }
  } 
}  // end readButton1


void zeroDisplay(){
  lcd.clear();  
  lcd.setCursor(0,0);   
  lcd.print(" 0:00:00");
  
  lcd.setCursor(5,1);   
  lcd.print(" 0:00:00");
}


// ---- Main Display Function ----
void refreshDisplay(int elapsedSeconds, int nowSeconds, bool full){
  // calculate elapsed times
  int hhE = elapsedSeconds / 3600;
  int mmE = (elapsedSeconds / 60) - (hhE * 60);
  int ssE = elapsedSeconds - (mmE * 60) - (hhE * 3600);

  // calculate percent used
  int percentUsed = ((float)elapsedSeconds / (float)nowSeconds) * 100.0;
  if (percentUsed < 0){percentUsed = 0;}
  if (percentUsed > 100){percentUsed = 100;}

  // calculate Total times
  int hhT = nowSeconds / 3600;
  int mmT = (nowSeconds / 60) - (hhT * 60);
  int ssT = nowSeconds - (mmT * 60) - (hhT * 3600);

  // Every minute fully refresh the display
  if (ssT == 1){full = true;}

  // clear display only if doing full
  if (full == true){
    lcd.clear(); 
  }
  
  // Display Elapsed Time Timer 
  if (mmE == 0 || full == true){ 
    lcd.setCursor(0,0);
    if (hhE < 10){lcd.print(" ");}
    lcd.print(hhT); 
    lcd.print(":");
    }
  if (ssE == 0 || full == true){
    lcd.setCursor(3,0);
    if (mmE < 10){lcd.print("0");}
    lcd.print(mmE);
    lcd.print(":");
    }
  lcd.setCursor(6,0);
  if (ssE < 10){lcd.print("0");}
  lcd.print(ssE);

  // Display Percent Used
  lcd.setCursor(11,0);
  if (percentUsed < 10){lcd.print(" ");}
  if (percentUsed < 100){lcd.print(" ");}
  lcd.print(percentUsed);
  lcd.print("%");

  // Display Total Time
  if (mmT == 0 || full == true){ 
    lcd.setCursor(5,1);
    if (hhT < 10){lcd.print(" ");}
    lcd.print(hhT); 
    lcd.print(":");
    }
  if (ssT == 0 || full == true){
    lcd.setCursor(8,1);
    if (mmT < 10){lcd.print("0");}
    lcd.print(mmT);
    lcd.print(":");
    }
  lcd.setCursor(11,1);
  if (ssT < 10){lcd.print("0");}
  lcd.print(ssT);
}


 
