/* This is the Lighting control program for the body of Blue Crew's mascot R2-Blue2.  This program is based on the original 
 *  Dataport/CBI program by Michael Erwin with additions from CuriousMarc, VAShadow, and S. Sloan. 
 * 
 * R2-Blue2 uses Michael Erwin's CBI and Dataport boards which utilize the MAX7219 chip to control LEDs.  In addition, R2 has three
 * team created light features using NeoPixels, the Large Dataport, the Coin Slots, and the Lightbar (on the DPL). These will be accessed
 * by using the FastLED.h and pixeltypes.h, its helper library
 * 
 */
#include <LedControl.h> //Needed for MAX7219 driven leds
#include <FastLED.h>  // Needed for Neopixels
#include <pixeltypes.h>  //Helper Library for FastLED


/*************************************************************************
 * ********************** MACRO DEFINITIONS ******************************
 * ***********************************************************************/
#define CBIDoorPin 2 //  Used for the CBI door open/close - Switch should be wired for Normally Closed
#define DPLDoorPin 3 //  Used for DPL door open/close - Switch should be wired for Normally close
#define DPL_LOAD 4   //  DPL load pin for the MAX7219 chip.
#define DPL_CLOCK 5  //  DPL clock pin for the MAX7219 chip
#define DPL_DATA 6   //  DPL Data pin for the MAX7219 chip
#define CBI_LOAD   7 //  CBI load pin for the MAX7219 chip.
#define CBI_CLOCK  8 //  CBI clock pin for the MAX7219 chip
#define CBI_DATA 9 // CBI data pin for the MAX7219 chip
#define CS_PIN 11  // Coin slots FastLED signal pin
#define LDPL_PIN 10 // Large Data port FastLED signal pin
#define LB_PIN 12 // sets the data pin for the LightBar Fast LED
#define LIGHTBARSPEED  150  // speed for DPL Neopixel Light bar
#define TOPBLOCKSPEED   70  // speed for top left blocks on DPL
#define BOTTOMLEDSPEED  175  // speed for bottom white leds on DPL
#define REDLEDSPEED     250  // speed for two red leds on DPL
#define BLUELEDSPEED    200  // speed for blue leds on DPL
#define BARGRAPHSPEED   100  // speed for Right blocks on DPL
#define CBISPEED_RANDOM 50 // 50 for Curious Marc version
#define TESTDELAY 30 // Test routine for DPL and CBI delay
#define MAXGRAPH 2  // Used for Bar Graph display
#define LB_LEDS 16  // Num of leds in DPL Light Bar 
#define LDPL_LEDS 15  // Number of leds in Large Data Port
#define CS_LEDS 18  // Number of leds in Coin Slots
#define DATAPORT 0 // device 1 is second in chain
#define CBI      0 // device 0 is first in chain
#define NUMDEV 1   // One for the dataport, one for the battery indicator/CBI
#define DATAPORTINTENSITY 15  // 15 is max
#define CBIINTENSITY 15  // 15 is max
// Currently, our droid uses two unconnected MAX7219 driven light systems - the DPL and the CBI



//Set this to which Analog Pin you use for the voltage in.
#define analoginput 0

#define greenVCC 12.5    // Green LED on if above this voltage
#define yellowVCC 12.0   // Yellow LED on if above this voltage
#define redVCC 11.5      // Red LED on if above this voltage

// For 15volts: R1=47k, R2=24k
// For 30volts: R1=47k, R2=9.4k
#define R1 47000.0     // >> resistance of R1 in ohms << the more accurate these values are
#define R2 24000.0     // >> resistance of R2 in ohms << the more accurate the measurement will be

// uncomment this to test the LEDS one after the other at startup
//#define TEST

// If you are using the voltage monitor uncomment this
//#define monitorVCC





// Uncomment this if you want an alternate effect for the blue LEDs, where it moves
// in sync with the bar graph
//#define BLUELEDTRACKGRAPH

//========================End Macro Definitions ====================================================


/*************************************************************************
 * ********************** GLOBAL VARIABLES *******************************
 * ***********************************************************************/

float vout = 0.0;       // for voltage out measured analog input
int value = 0;          // used to hold the analog value coming out of the voltage divider
float vin = 0.0;        // voltage calulcated... since the divider allows for 15 volts
     
// Instantiate LedControl driver
LedControl cc=LedControl(CBI_DATA, CBI_CLOCK, CBI_LOAD,NUMDEV);   // CBI
LedControl lc=LedControl(DPL_DATA,DPL_CLOCK, DPL_LOAD,NUMDEV);   // Dataport 
int displayEffect = 100; // 100=no change, 4=whistle/heart sequence
int dev_address, dev_option;                      // Create variables for device address and device option
char dev_command;                                 // Create variable for the device command
int cs_State = 0;                                 // Sets default Coin Slot State to off
int cs_Speed = 425;                               // Sets default Coin Slot speed to Medium
int cs_Tspeed = 10;                               // Sets default Coin Slot throb speed (0-99)
int ldpl_State = 0;                                 // Sets default Large Data Port Logics State to off
int ldpl_Speed = 375;                               // Sets default Large Data Port Logics speed to Medium
int ldpl_Tspeed = 10;                               // Sets default Large Data Port Logics throb speed (0-99)
CRGB lb[LB_LEDS];
CRGB ldpl[LDPL_LEDS];
CRGB cs[CS_LEDS];
//========================End Global Variables ====================================================

/*************************************************************************
 * ********************** FUNCTION DECLARATIONS ******************************
 * ***********************************************************************/

void bargraphDisplay(byte disp);  //Updates the blocks on the right of the DPL
int cal_Speed(int num); //returns a calculated speed based on passed num
void checkSerial();  //Checks for serial commands
int coinslot(int num); //Controls the neo-pixel based coinslot 
void cs_updown(int num); //Displays up/down routine on Coin slots
void cs_singleUpDown(int num); //Displays single Up/Down routine on coin slots
void fillBar(byte disp, byte data, byte value, byte maxcol); //Utility to light up a bar of leds based on a value
void getVCC(); //Updates the current voltage
void ldpl_single(int num); //Displays single dot sequence on LDPL
void ldpl_double(int num); // Display double dot sequence on LDPL
void proc_command(int address, char command, int option);  //Processes received serial command
byte randomRow(byte randomMode);  //Utility to generate random LED patterns
void singleTest();  //Tests every led in the CBI and DPL
int std_color(int num); //Returns a standard color based on passed num
byte updatebar(byte disp, byte* bargraphdata, byte maxcol); //Utility to make bargraph look more realistic
void updatebottomLEDs();  //Updates the bottom White leds in the DPL
void updateCBILEDs();  //Updates the CBI leds
void updateBlueLEDs(); //Updates the blue LEDs on the DPL
int updateLDPL(int num); //Controls the neo-pixel based LDPL
void updateLightBar();  //Updates the DPL Light Bar
void updateRedLEDs(); //Updates the two red LEDs on the DPL
void updateTopBlocks();  //Updates the Top Green and Yellow Blocks on the DPL


//========================End Function Declarations ====================================================



void setup() 
{
  Serial.begin(115200); //Set up Serial Communication                   
  FastLED.addLeds<WS2811, LB_PIN, GRB>(lb, LB_LEDS); //Adds LEDs to FastLED array
  FastLED.addLeds<WS2811, CS_PIN, GRB>(cs, CS_LEDS); //Adds LEDs to FastLED array
  FastLED.addLeds<WS2811, LDPL_PIN, GRB>(ldpl, LDPL_LEDS); //Adds LEDs to FastLED array
  // initialize Maxim driver chips
  lc.shutdown(DATAPORT,false);                  // take out of shutdown
  lc.clearDisplay(DATAPORT);                    // clear
  lc.setIntensity(DATAPORT,DATAPORTINTENSITY);  // set intensity
  cc.shutdown(CBI,false);                       // take out of shutdown
  cc.clearDisplay(CBI);                         // clear
  cc.setIntensity(CBI,CBIINTENSITY);            // set intensity
  
#ifdef  TEST// test LEDs
  singleTest(); 
  delay(2000);
#endif

  pinMode(DPLDoorPin, INPUT_PULLUP);  //Pin on the Arduino Mini Breakout Board connected to left door switch HIGH=Door closed (NC when door closed) - S.Sloan
  pinMode(CBIDoorPin, INPUT_PULLUP);  //Pin on the Arduino Mini Breakout Board connected to right door switch HIGH=Door closed (NC when door closed) - S.Sloan


#ifndef monitorVCC
  pinMode(analoginput, INPUT);
#endif

}


void loop() 
{ 

int DPLDoorStatus = digitalRead(DPLDoorPin);        // Open(Low)/Closed(High) Status of Dataport Door - S.Sloan
int CBIDoorStatus = digitalRead(CBIDoorPin);      // Open(Low)/Closed(High) Status of Charge Bay Indicator Door - S.Sloan

// this is the legacy algorythm. Super simple, but very blocky.
#ifdef LEGACY

 for (int row=0; row<6; row++) lc.setRow(DATAPORT,row,random(0,256));
 #ifdef monitorVCC
   for (int row=0; row<4; row++) cc.setRow(CBI,row,random(0,256));
   getVCC();
 #else
   for (int row=0; row<7; row++) cc.setRow(CBI,row,random(0,256));
 #endif
 delay(1000);
 
#else

  // Dataport LED sequence
  if (DPLDoorStatus == LOW) {                      //If the Dataport door is open then set the LEDs - S.Sloan
    // this is the new code. Every block of LEDs is handled independently
   // Serial.print("\nDataPort");
    updateTopBlocks();
    bargraphDisplay(0);
    updatebottomLEDs();
    updateRedLEDs();
    #ifndef BLUELEDTRACKGRAPH
      updateBlueLEDs();
    #endif
    updateLightBar();
    
  }
  else {                                            //If the Dataport door is closed then switch off the LEDs - S.Sloan
    //switch off all data leds
    lc.setRow(DATAPORT,1,0); // top yellow blocks
    lc.setRow(DATAPORT,2,0); // top yellow blocks
    lc.setRow(DATAPORT,3,0); // top yellow blocks
    lc.setRow(DATAPORT,4,0); // top yellow blocks
    lc.setRow(DATAPORT,5,0); // top green blocks
    lc.setRow(DATAPORT,0,0); // blue LEDs
    for(int x=0; x<16; x++) lb[x]=CRGB::Black;
    FastLED.show();
  }


  //Charge Bay Indicator LED sequence
  if (CBIDoorStatus == LOW) {                     //If the CBI door is open then Set the LEDs - S.Sloan
     updateCBILEDs();
     #ifdef monitorVCC
       getVCC();
     #endif
   }
   else {                                           //If the CBI door is closed then switch off the LEDs - S.Sloan
    //switch off all CBI leds
    for (int row=0; row<7; row++) cc.setRow(CBI,row,0);
   }
   
#endif
    checkSerial();
    coinslot(cs_State);
    updateLDPL(ldpl_State);
}


///////////////////////////////////////////////////
// Test LEDs, each Maxim driver row in turn
// Each LED blinks according to the col number
// Col 0 is just on
// Col 1 blinks twice
// col 2 blinks 3 times, etc...
//


void singleTest() 
{
  for(int row=0;row<6;row++){
    for(int col=0;col<7;col++){
      delay(TESTDELAY);
      lc.setLed(DATAPORT,row,col,true);
      delay(TESTDELAY);
      for(int i=0;i<col;i++){
        lc.setLed(DATAPORT,row,col,false);
        delay(TESTDELAY);
        lc.setLed(DATAPORT,row,col,true);
        delay(TESTDELAY);
      }
    }
  }
  for(int row=0;row<4;row++){
    for(int col=0;col<5;col++){
      delay(TESTDELAY);
      cc.setLed(CBI,row,col,true);
      delay(TESTDELAY);
      for(int i=0;i<col;i++){
        cc.setLed(CBI,row,col,false);
        delay(TESTDELAY);
        cc.setLed(CBI,row,col,true);
        delay(TESTDELAY);
      }
    }
   }
   cc.setLed(CBI,4,5,true);
   delay(TESTDELAY);
   cc.setLed(CBI,5,5,true);
   delay(TESTDELAY);
   cc.setLed(CBI,6,5,true);
   delay(TESTDELAY);
   return;
}



///////////////////////////////////
// animates the two top left blocks
// (green and yellow blocks)
void updateTopBlocks()
{
  static unsigned long timeLast=0;
  unsigned long elapsed;
  elapsed=millis();
  if ((elapsed - timeLast) < TOPBLOCKSPEED) return;
  timeLast = elapsed; 

  lc.setRow(DATAPORT,4,randomRow(4)); // top yellow blocks
  lc.setRow(DATAPORT,5,randomRow(4)); // top green blocks
  return;
}

///////////////////////////////////
// animates the CBI
//
void updateCBILEDs()
{
  static unsigned long timeLast=0;
  unsigned long elapsed;
  elapsed=millis();

  if ((elapsed - timeLast) < CBISPEED_RANDOM) return;
  timeLast = elapsed; 
  cc.setRow(CBI,random(4),randomRow(random(4)));
  return;
}



////////////////////////////////////
// Utility to generate random LED patterns
// Mode goes from 0 to 6. The lower the mode
// the less the LED density that's on.
// Modes 4 and 5 give the most organic feel
byte randomRow(byte randomMode)
{
  switch(randomMode)
  {
    case 0:  // stage -3
      return (random(256)&random(256)&random(256)&random(256));
      break;
    case 1:  // stage -2
      return (random(256)&random(256)&random(256));
      break;
    case 2:  // stage -1
      return (random(256)&random(256));
      break;
    case 3: // legacy "blocky" mode
      return random(256);
      break;
    case 4:  // stage 1
      return (random(256)|random(256));
      break;
    case 5:  // stage 2
      return (random(256)|random(256)|random(256));
      break;
    case 6:  // stage 3
      return (random(256)|random(256)|random(256)|random(256));
      break;
    default:
      return random(256);
      break;
  }
}

//////////////////////
// bargraph for the right column
// disp 0: Row 2 Col 5 to 0 (left bar) - 6 to 0 if including lower red LED, 
// disp 1: Row 3 Col 5 to 0 (right bar)



void bargraphDisplay(byte disp)
{ 
  static byte bargraphdata[MAXGRAPH]; // status of bars
  
  if(disp>=MAXGRAPH) return;
  
  // speed control
  static unsigned long previousDisplayUpdate[MAXGRAPH]={0,0};

  unsigned long currentMillis = millis();
  if(currentMillis - previousDisplayUpdate[disp] < BARGRAPHSPEED) return;
  previousDisplayUpdate[disp] = currentMillis;
  
  // adjust to max numbers of LED available per bargraph
  byte maxcol;
  if(disp==0 || disp==1) maxcol=6;
  else maxcol=3;  // for smaller graph bars, not defined yet
  
  // use utility to update the value of the bargraph  from it's previous value
  byte value = updatebar(disp, &bargraphdata[disp], maxcol);
  byte data=0;
  // transform value into byte representing of illuminated LEDs
  // start at 1 so it can go all the way to no illuminated LED
  for(int i=1; i<=value; i++) 
  {
    data |= 0x01<<i-1;
  }
  // transfer the byte column wise to the video grid
  fillBar(disp, data, value, maxcol); 
  return;  
}

/////////////////////////////////
// helper for updating bargraph values, to imitate bargraph movement
byte updatebar(byte disp, byte* bargraphdata, byte maxcol)
{
  // bargraph values go up or down one pixel at a time
  int variation = random(0,3);            // 0= move down, 1= stay, 2= move up
  int value=(int)(*bargraphdata);         // get the previous value
  //if (value==maxcol) value=maxcol-2; else      // special case, staying stuck at maximum does not look realistic, knock it down
  value += (variation-1);                 // grow or shring it by one step
#ifndef BLUELEDTRACKGRAPH
  if (value<=0) value=0;                  // can't be lower than 0
#else
  if (value<=1) value=1;                  // if blue LED tracks, OK to keep lower LED always on
#endif
  if (value>maxcol) value=maxcol;         // can't be higher than max
  (*bargraphdata)=(byte)value;            // store new value, use byte type to save RAM
  return (byte)value;                     // return new value
}

/////////////////////////////////////////
// helper for lighting up a bar of LEDs based on a value
void fillBar(byte disp, byte data, byte value, byte maxcol)
{
  byte row;
  
  // find the row of the bargraph
  switch(disp)
  {
    case 0:
      row = 2;
      break;
    case 1:
      row = 3;
      break;
    default:
      return;
      break;
  }
  
  for(byte col=0; col<maxcol; col++)
  {
    // test state of LED
    byte LEDon=(data & 1<<col);
    if(LEDon)
    {
      //lc.setLed(DATAPORT,row,maxcol-col-1,true);  // set column bit
      lc.setLed(DATAPORT,2,maxcol-col-1,true);      // set column bit
      lc.setLed(DATAPORT,3,maxcol-col-1,true);      // set column bit
      //lc.setLed(DATAPORT,0,maxcol-col-1,true);      // set blue column bit
    }
    else
    {
      //lc.setLed(DATAPORT,row,maxcol-col-1,false); // reset column bit
      lc.setLed(DATAPORT,2,maxcol-col-1,false);     // reset column bit
      lc.setLed(DATAPORT,3,maxcol-col-1,false);     // reset column bit
      //lc.setLed(DATAPORT,0,maxcol-col-1,false);     // set blue column bit
    }
  }
#ifdef BLUELEDTRACKGRAPH
  // do blue tracking LED
  byte blueLEDrow=B00000010;
  blueLEDrow=blueLEDrow<<value;
  lc.setRow(DATAPORT,0,blueLEDrow);
#endif
return;
}

/////////////////////////////////
// This animates the bottom white LEDs
void updatebottomLEDs()
{
  static unsigned long timeLast=0;
  unsigned long elapsed=millis();
  if ((elapsed - timeLast) < BOTTOMLEDSPEED) return;
  timeLast = elapsed;  
  
  // bottom LEDs are row 1, 
  lc.setRow(DATAPORT,1,randomRow(4));
  return;
}

////////////////////////////////
// This is for the two red LEDs
void updateRedLEDs()
{
  static unsigned long timeLast=0;
  unsigned long elapsed=millis();
  if ((elapsed - timeLast) < REDLEDSPEED) return;
  timeLast = elapsed;  
  
  // red LEDs are row 2 and 3, col 6, 
  lc.setLed(DATAPORT,2,6,random(0,2));
  lc.setLed(DATAPORT,3,6,random(0,2));
  return;
}

//////////////////////////////////
// This animates the blue LEDs
// Uses a random delay, which never exceeds BLUELEDSPEED 
void updateBlueLEDs()
{
  static unsigned long timeLast=0;
  static unsigned long variabledelay=BLUELEDSPEED;
  unsigned long elapsed=millis();
  if ((elapsed - timeLast) < variabledelay) return;
  timeLast = elapsed;  
  variabledelay=random(10, BLUELEDSPEED);
  
  /*********experimental, moving dots animation
  static byte stage=0;
  stage++;
  if (stage>7) stage=0;
  byte LEDstate=B00000011;
  // blue LEDs are row 0 col 0-5 
  lc.setRow(DATAPORT,0,LEDstate<<stage);
  *********************/
  
  // random
  lc.setRow(DATAPORT,0,randomRow(4));
  return;   
}

void getVCC()
{
  value = analogRead(analoginput); // this must be between 0.0 and 5.0 - otherwise you'll let the blue smoke out of your arduino
  vout= (value * 5.0)/1024.0;  //voltage coming out of the voltage divider
  vin = vout / (R2/(R1+R2)); //voltage to display

  cc.setLed(CBI,6,5,(vin >= greenVCC));
  cc.setLed(CBI,5,5,(vin >= yellowVCC));
  cc.setLed(CBI,4,5,(vin >= redVCC));
  //Serial.print("Volt Out = ");                                  // DEBUG CODE
  //Serial.print(vout, 1);   //Print float "vin" with 1 decimal   // DEBUG CODE
  //Serial.print("\tVolts Calc = ");                             // DEBUG CODE
  //Serial.println(vin, 1);   //Print float "vin" with 1 decimal   // DEBUG CODE
  return;
}

void updateLightBar(){
  
  static unsigned long timeLast=0;
  unsigned long elapsed=millis();
  if ((elapsed - timeLast) < LIGHTBARSPEED) return;
  timeLast = elapsed;
  
  
  int leftBar=random(0,7);
  int rightBar=random(0,7);
  for(int x=0; x<16; x++)lb[x]=CRGB::Black;  
  //First do the left bar
  for(int x=0; x<=leftBar; x++){
      if(x<4)lb[x]=CRGB::Green;
      if(x<6 && x>=4) lb[x]=CRGB::Yellow;
      if(x>5) lb[x]=CRGB::Red;
  }
  
  //now do the right light bar
  for(int x=0; x<=rightBar; x++){
      int x2 = map(x,0,7,15,8);
      if(x<4)lb[x2]=CRGB::Green;
      if(x<6 && x>=4) lb[x2]=CRGB::Yellow;
      if(x>5) lb[x2]=CRGB::Red;
    }
    
  
  
  FastLED.show();
  return;
}

void checkSerial(){
  char message[64];                               //Create a character array to hold our incoming message 
                                                  //    (max length of serial buffer is 64)
  int msgLength = Serial.available();             // Set msgLength to the number of bytes available in the 
                                                  //    Serial buffer
  if(msgLength>=7){                                  // If there is something in the serial buffer, start the loop
    for(int x=0; x<msgLength; x++){
      message[x]=Serial.read();                   // Read a character from the serial buffer,
    }
    
    if(message[0]!='%') return;             // If not for this arduino - quit the parsing
    dev_address = (message[1]-48)*10 + (message[2]-48);  
    dev_command = message[3];                   // determine the device command  
    dev_option=(message[4]-48)*100 + (message[5]-48)*10 + (message[6]-48);  
    Serial.flush();                                 // Clear the Serial0 buffer
    proc_command(dev_address, dev_command, dev_option); 
    
  }
  return;
}
//the proc_Command or process command function takes the parsed command from the Serial interface and executes it
 
void proc_command(int address, char command, int option){
                                                    //First determine the command type
  if(command=='T'||command=='S'){                   //This program recognizes two commands - T and S
    if(command=='T'){                              //   The T command changes the routine settings 
       switch(address){                             // Determine which device to change
        case 81:                                    // Address of Coin Slots is 81
          cs_State=option;                          // Set the Coin Slot state to the option
          break;
        case 82:                                    // Address of Dataport is 82
          ldpl_State=option;                          // Set the Dataport state to the option
          break;
        case 83:                                    // Address of Charging port is 83
          //Code for Charging Port    
          //cp_State=option;                          // set the Charging port to the option
          break;
      }
    } 
    if(command=='S'){                               //   The S command changes the speed settings 
       switch(address){                             // Determine which device to update
        case 81:                                    // Address of Coin Slots is 81
          if(option<100){
            cs_Speed=cal_Speed(option);             // Calculate the Speed and set it
          }else{
            if(option<200)cs_Tspeed=cal_Speed(option);
          }
          break;
        case 82:                                    // Address of Dataport is 82
          //dp_Speed=option;                          // Set the Dataport state to the option
          break;
        case 83:                                    // Address of Charging port is 83    
          //cp_Speed=option;                           // set the Charging port to the option
          break;
      }
    }
  }
  
}
//  cal_Speed returns the time in milliseconds for routine delays from the passed option 
int cal_Speed(int num){
  switch(num){
    case 1:  return 100;                            // Fastest  
    case 2:  return 250;
    case 3:  return 500;
    case 4:  return 750;
    case 5:  return 1000;                           // Slowest
    default:
      if(num>100&&num<200) return num-100;
  }
}


  
int coinslot(int num){
  //Shut off coin slots
  if(num == 0){
      for(int x=0; x<=17; x++) cs[x]=0x000000;
      FastLED.show();
      return 0;
  }
  //
  static unsigned long timeLast=0;
  unsigned long elapsed=millis();
  if ((elapsed - timeLast) < cs_Speed) return num;
  timeLast = elapsed;  
  if(num<11){
      cs_updown(num);
      return num;
  }
  if(num<21){
      cs_singleUpDown(num-10);
      return num-10;
  }
  if(num<31){
      int hue = std_color(num-20);
      for(int x=0; x<=17; x++) cs[x]=CHSV(hue, 255, 255);
      FastLED.show();
      return num-20;
  }
     
      
  
  return 0;
  
}

int updateLDPL(int num){
  //Shut off Large Data Port
  if(num == 0){
      for(int x=0; x<=14; x++) ldpl[x]=0x000000;
      FastLED.show();
      return 0;
  }
  //
  
  if(num<11){
      ldpl_single(num);
      return num;
  }
  if(num<21){
      ldpl_double(num-10);
      return num-10;
  }
  if(num<31){
      int hue = std_color(num-20);
      for(int x=0; x<=17; x++) cs[x]=CHSV(hue, 255, 255);
      FastLED.show();
      return num-20;
  }
     
      
  
  return 0;
  
}


void ldpl_single(int num){
  static unsigned long timeLast=0;
  unsigned long elapsed=millis();
  if ((elapsed - timeLast) < ldpl_Speed) return num;
  timeLast = elapsed;  
  int hue = std_color(num);
  static int turn = 0;
  static int dir = 0;
  if(turn<=0 && dir==1){ 
    dir=0;
    turn=0; 
  }
  if(turn>=14 && dir ==0){
    dir=1;
    turn=14;
  }
  for(int x=0; x<15; x++) ldpl[x]=CHSV(0,0,0);
  ldpl[turn]=CHSV(hue,255,255);
  if(dir==0){
      turn++;
  }else{
      turn--;  
  }
  return num;
  
}

void ldpl_double(int num){
  static unsigned long timeLast=0;
  unsigned long elapsed=millis();
  if ((elapsed - timeLast) < ldpl_Speed) return num;
  timeLast = elapsed;  
  int hue = std_color(num);
  static int turn = 0;
  static int dir = 0;
  if(turn<=0 && dir==1){ 
    dir=0;
    turn=0; 
  }
  if(turn>=14 && dir ==0){
    dir=1;
    turn=14;
  }
  for(int x=0; x<15; x++) ldpl[x]=CHSV(0,0,0);
  ldpl[turn]=CHSV(hue,255,255);
  ldpl[turn-1]=CHSV(hue,255,255);
  if(dir==0){
      turn++;
  }else{
      turn--;  
  }
  return num;
  
}


int std_color(int num){
  int curr_Hue;  // a double integer to hold routine color value up to 256 cubed (256 for red, 256 for green, 256 for blue)
  switch(num){  
   
    case 1:
      curr_Hue=0; //Color is Red
      break;
    case 2:
      curr_Hue=8; //Color is Orange
      break;
    case 3:
      curr_Hue=32; //Color is Yellow
      break;
    case 4:
      curr_Hue=96; //Color is Green
      break;
    case 5:
      curr_Hue=128; //Color is Cyan
      break;
    case 6:
      curr_Hue=160; //Color is Blue
      break;
    case 7:
      curr_Hue=192; //Color is Violet
      break;
    case 8:
      curr_Hue=224; //Color is Pink
      break;
    case 9:
      curr_Hue=255; //Color is White
      break;
    case 10: 
      curr_Hue=random(0,255); //Color is Random
      break;
  }
  return curr_Hue;
  
}




/*  cs_updown displays the passed color one coin slot at a time starting with the top coin slot. Once all are filled,
 *   they are cleared from the bottom up.
 * 
 */



void cs_updown(int num){
    static int turn=0; //tracks which coinslot is being addressed
    static int dir=0;  // 0 is up 1 is down
    int hue=std_color(num);
    if(turn<=0 && dir==1){
      dir=0;
      turn=0;
    }
    if(turn>16 && dir==0){
      dir=1;
      turn=15;
    }
    if(dir==0){
      cs[turn]= CHSV(hue,255,255);
      cs[turn+1] = CHSV(hue,255,255);
      cs[turn+2] = CHSV(hue,255,255);
    }else{
      cs[turn]= CHSV(0,0,0);
      cs[turn+1] = CHSV(0,0,0);
      cs[turn+2] = CHSV(0,0,0);
    }
    
    if(dir==0){
      turn+=3;
    }else{
      turn-=3;
    }
    FastLED.show();
    return num;
}





void cs_singleUpDown(int num){
    static int turn=0; //tracks which coinslot is being addressed
    static int dir=0;  // 0 is up 1 is down
    int hue=std_color(num);
    if(turn<=0 && dir==1){
      dir=0;
      turn=0;
    }
    if(turn>16 && dir==0){
      dir=1;
      turn=15;
    }
    for(int x=0; x<18; x++) cs[x]=CHSV(0,0,0);
    cs[turn]= CHSV(hue,255,255);
    cs[turn+1] = CHSV(hue,255,255);
    cs[turn+2] = CHSV(hue,255,255);
    if(dir==0){
      turn+=3;
    }else{
      turn-=3;
    }
    FastLED.show();
    return num;
}


  
