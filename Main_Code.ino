// MOTOR SETUP

#include "AccelStepper.h" 
#include <Servo.h>
#define OPEN 170
#define CLOSED 120
#define REFILL 80
AccelStepper stepperA(1, 38, 40); 
AccelStepper stepperB(1, 30, 32);
AccelStepper stepperC(1, 39, 41);
AccelStepper stepperD(1, 31, 33); 
#define home_switchA 42
#define home_switchB 34
#define home_switchC 43
#define home_switchD 35
long serial_input; 
long steps; 
int move_finished=1;  
long initial_homing=1;  //just a counter
int i = 1;
long t_homing = 3000;
long t_start;
bool refill_on_off = 0;   //set to 1 if you want to refill, when homing is complete
Servo servo;  
bool homingYN = true;
bool emptyYN = false;
long PIVOT = -500;
long syringelength = -116000; //steps from empty to full

//TOUCHSCREEN SETUP

#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
MCUFRIEND_kbv tft;

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define TS_MINX 208   //  210
#define TS_MINY 214   //  186
#define TS_MAXX 960   //  926
#define TS_MAXY 916   //  902

#define VD_Y 140
#define VD_X 10
#define ReWt 45
#define SqWt 15

const int XP=6,XM=A2,YP=A1,YM=7; //ID=0x6809
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;

int vxcor[] = {8,60,112,164,216,268};
int vx15cor[] = {23,75,127,179,231,283};

void setup()
{
      //MOTOR CODE
  pinMode(home_switchA, INPUT_PULLUP);
  pinMode(home_switchB, INPUT_PULLUP);
  pinMode(home_switchC, INPUT_PULLUP);
  pinMode(home_switchD, INPUT_PULLUP);
  servo.attach(44);
  servo.write(CLOSED);
  Serial.begin(9600);
  stepperA.setMaxSpeed(2500);
  stepperA.setAcceleration(1000);
  stepperB.setMaxSpeed(2500);
  stepperB.setAcceleration(1000);
  stepperC.setMaxSpeed(2500);
  stepperC.setAcceleration(1000);
  stepperD.setMaxSpeed(2500);
  stepperD.setAcceleration(1000);

  
    //TOUCHSCREEN CODE
    tft.reset();
    uint16_t identifier = tft.readID();
    Serial.print("ID = 0x");
    Serial.println(identifier, HEX);
    if (identifier == 0xEFEF) identifier = 0x9486;
    tft.begin(identifier); // ID= 0x6809  // ID=0x6001 with shield//tft.begin(6809);  //Manual test

    //HOMING
   
    homing();
    drawHomeScreen();
}
uint8_t aspect;

void loop()
{
    // put your main code here, to run repeatedly:
  TSPoint p = ts.getPoint();  //Get touch point

  if (p.z > ts.pressureThreshhold) {

   Serial.print("X = "); Serial.print(p.x);
   Serial.print("\tY = "); Serial.print(p.y);
   Serial.print("\n");
   
   p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
   p.y = map(p.y, TS_MAXY, TS_MINY, 0, 240);
       
   if(p.x>23 && p.x<23+130 && p.y>65 && p.y<65+130) 
   // The user has pressed PROGRAM 1 button
   {
    // This is important, because the libraries are sharing pins 
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    program2();
   }
   if(p.x>23+150 && p.x<23+130+150 && p.y>65 && p.y<65+130) 
   // The user has pressed MANUAL button
   {
    // This is important, because the libraries are sharing pins 
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    manual();
   }
   delay(10);
  }

//SERIAL MONITOR CODE
    char c;
  while (Serial.available()){
    c = Serial.read();
     if(c=='A'){
      serial_input = Serial.parseInt();
      steps = serial_input*1;
      stepperA.move(steps);
      Serial.print("Moving A by "); Serial.print(steps) ; Serial.print(" steps or ML "); Serial.println(serial_input);
      }
     if(c=='B'){
      serial_input = Serial.parseInt();
      steps = serial_input*1;//1966;
      stepperB.move(steps);
      Serial.print("Moving B by "); Serial.print(steps) ; Serial.print(" steps or ML "); Serial.println(serial_input);
      }
      if(c=='C'){
      serial_input = Serial.parseInt();
      steps = serial_input*1;//1966;
      stepperC.move(steps);
      Serial.print("Moving C by "); Serial.print(steps) ; Serial.print(" steps or ML "); Serial.println(serial_input);
      }
      if(c=='D'){
      serial_input = Serial.parseInt();
      steps = serial_input*1;//1966;
      stepperD.move(steps);
      Serial.print("Moving D by "); Serial.print(steps) ; Serial.print(" steps or ML "); Serial.println(serial_input);
      }
      if(c=='Q'){//Speed Test
      serial_input = Serial.parseInt();
      //servo.write(180);
      stepperA.setMaxSpeed(serial_input);stepperB.setMaxSpeed(serial_input);stepperC.setMaxSpeed(serial_input);stepperD.setMaxSpeed(serial_input); //stepperB.move(5000);
      Serial.print("Moving by 10.000 steps at speed: "); Serial.print(serial_input) ;
      }
      
     if(c=='I'){
      impulse(3000);
     }
      if(c=='R'){
      stepperA.setMaxSpeed(150);
      stepperA.setAcceleration(100);
      stepperB.setMaxSpeed(500);
      stepperB.setAcceleration(1000);
      Serial.println("Reset speed");
     }
     if(c=='S'){
        serial_input = Serial.parseInt();
        servo.write(serial_input);      
     }
      if(c=='P'){
      serial_input = Serial.parseInt();
      if(serial_input = 1){program1();}
      if(serial_input = 2){program2();}
      if(serial_input = 3){program3();}
      if(serial_input = 4){program4();}
     }
      if(c=='H'){
      homing();
     }
     if(c=='E'){
      Serial.println("Cleaning with alcohol");
      serial_input = Serial.parseInt();
      clean(serial_input);
      }
    }
 stepperA.run();
 stepperB.run();
 stepperC.run();
 stepperD.run();
}


void program1(){

  
// HOW TO USE:
// wait(x): wait x seconds
// medium(STEPS,SPEED), pbs(STEPS,SPEED), trypsin(STEPS,SPEED), air(STEPS,SPEED), 
t_start = millis();

    tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
    tft.fillRect(23,65,150+130,130,BLACK);
    tft.setCursor(23+20,65+20);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    //tft.print("Merci Robin!");
    
      /*stepperA.setMaxSpeed(500);
      stepperA.setAcceleration(6000);
      stepperB.setMaxSpeed(500);
      stepperB.setAcceleration(6000);
      stepperC.setMaxSpeed(500);
      stepperC.setAcceleration(6000);
      stepperD.setMaxSpeed(500);
      stepperD.setAcceleration(6000);   */
      Serial.println("Running Program");
      
      delay(500);
      stepperA.setAcceleration(10000);
      stepperB.setAcceleration(10000);
      stepperC.setAcceleration(10000);
      stepperD.setAcceleration(10000);

      air(15000,8000);if(emptyYN){return;}
      wait(5);
      pbs(15000,8000);if(emptyYN){return;}
      wait(5);
      air(1000,3000);if(emptyYN){return;}
      wait(5);
      trypsin(15000,5000);if(emptyYN){return;}
      wait(5);
      air(1000,3000);if(emptyYN){return;}
      wait(5);
      pbs(15000,5000);if(emptyYN){return;}
      wait(5);
      air(1000,3000);if(emptyYN){return;}
      wait(5);
      trypsin(15000,5000);if(emptyYN){return;}
      wait(5);
      air(1000,3000);if(emptyYN){return;}
      wait(5);
      pbs(15000,5000);if(emptyYN){return;}
      wait(5);
      air(1000,3000);if(emptyYN){return;}
      wait(5);
      trypsin(15000,5000);if(emptyYN){return;}
      wait(5);
      air(1000,3000);if(emptyYN){return;}
      wait(5);
      pbs(15000,5000);if(emptyYN){return;}
      wait(5);
      /*
      air(15000,3000);if(emptyYN){return;}
      wait(5);if(emptyYN){return;}
      medium(15000,5000);if(emptyYN){return;}
      //pivot('D',500);
      wait(5);if(emptyYN){return;}
      air(15000,5000);if(emptyYN){return;}
      //trypsin(2000,2000); if(emptyYN){return;}
      medium(15000,5000);if(emptyYN){return;}
      wait(5);if(emptyYN){return;}
      air(15000,5000);if(emptyYN){return;}
      pbs(15000,5000);if(emptyYN){return;}
      wait(5);if(emptyYN){return;}
      air(15000,5000);if(emptyYN){return;}
      trypsin(10000,3000);if(emptyYN){return;}
      wait(5);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      medium(10000,3000);if(emptyYN){return;}
      wait(5);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      trypsin(10000,3000);if(emptyYN){return;}
*/
      
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
      Serial.println("FINISHED");
      delay(10000);
      drawHomeScreen();
      tft.fillRect(20+1,5+1,280-2,30-2,BLACK);
      tft.setCursor(35,12);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
  
}

void homing(){

  //Ask if you want to home the steppers
     stepperA.setMaxSpeed(4000);
     stepperB.setMaxSpeed(4000);
     stepperC.setMaxSpeed(4000);
     stepperD.setMaxSpeed(4000);
    tft.setRotation(1); // Landscape Mode
    tft.fillScreen(0x0000); // Black Screen

    tft.drawRect(20,5,280,30,GREEN);
    
    tft.setCursor(35,12);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Home the steppers?");

    tft.fillRect(23,VD_Y-60-15, 130, 130, GREEN);
    tft.setCursor(23, VD_Y-55);
    tft.setTextSize(2);
    tft.println("   YES");

    tft.fillRect(23+150,VD_Y-60-15, 130, 130, RED);
    tft.setCursor(23+150, VD_Y-55);
    tft.setTextSize(2);
    tft.println("    NO");
    
  while(homingYN){
    TSPoint p = ts.getPoint();  //Get touch point
  if (p.z > ts.pressureThreshhold) {

   Serial.print("X = "); Serial.print(p.x);
   Serial.print("\tY = "); Serial.print(p.y);
   Serial.print("\n");
   
   p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
   p.y = map(p.y, TS_MAXY, TS_MINY, 0, 240);
       
   if(p.x>23 && p.x<23+130 && p.y>65 && p.y<65+130) 
   // The user has pressed inside button YES
   {
    // This is important, because the libraries are sharing pins 
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    homingYN = false;
   }
   delay(10);
   if(p.x>23+150 && p.x<23+130+130 && p.y>65 && p.y<65+130) 
   // The user has pressed inside button NO
   {
    // This is important, because the libraries are sharing pins 
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    return;
   }
  }
  }

 tft.fillRect(0,0,750,750,BLACK);
 tft.drawRect(20,5,280,30,GREEN);
 char *msg[] = {"MEDIUM homed", "PBS homed", "TRYPSIN homed", "AIR homed",};
  int vxcor[] = {120,120,120,120};
    
    tft.setCursor(35,12);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("HOMING");
    Serial.println("HOMING, homie");

    char *ABCD[] = {"A", "B", "C", "D",};
  int vxcor1[] = {20,80,140,200};
  int vycor1[] = {20,50,130,200};
  int pluswidth = 40;
  int plusheight = 60;
  int minuswidth = 40;
  int minusheight = 40;
  int DELAY = 200;
  int x = 0;
  int y = 0;
  tft.setTextSize(1);
  
  tft.setTextSize(4);
  while(x<4){
    tft.fillRect(vxcor1[x],vycor1[1],pluswidth,plusheight,RED);
    tft.setCursor(vxcor1[x]+10,vycor1[1]+18);
    tft.println(ABCD[x]);
    x++;
  }
  x = 0;
  tft.setTextSize(2);
   tft.fillRect(0,vycor1[3],400,40,RED);
 tft.setCursor(vxcor1[0]+120,vycor1[3]+5);
 tft.println("EXIT");
   tft.fillRect(265,vycor1[1]-10,40,90,RED);
 tft.setTextSize(2);
 tft.setCursor(270+10,vycor1[1]);
 tft.println("F");
 tft.setCursor(270+10,vycor1[1]+20);
 tft.println("I");
 tft.setCursor(270+10,vycor1[1]+40);
 tft.println("L");
 tft.setCursor(270+10,vycor1[1]+60);
 tft.println("L");
    servo.write(OPEN);


   //Touch code
   
    bool YN = true;
while(YN){
    TSPoint p = ts.getPoint();  //Get touch point
  if (p.z > ts.pressureThreshhold) {
  Serial.print("X = "); Serial.print(p.x);
   Serial.print("\tY = "); Serial.print(p.y);
   Serial.print("\n");
   
   p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
   p.y = map(p.y, TS_MAXY, TS_MINY, 0, 240);
   if(p.x>vxcor1[0] && p.x<vxcor1[0]+pluswidth && p.y>vycor1[1] && p.y<vycor1[1]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    Serial.println("A homing");
    tft.fillCircle(285,vycor1[1]-30,20,GREEN);
    delay(DELAY);
    while(digitalRead(home_switchA)){
          stepperA.moveTo(initial_homing);
          initial_homing++;
          stepperA.run();stepperB.run();stepperC.run();stepperD.run();
        }
        stepperA.setCurrentPosition(0);stepperA.moveTo(0);
        tft.fillCircle(285,vycor1[1]-30,20,BLACK);
        initial_homing = 1;
    tft.fillRect(vxcor1[0],vycor1[1],pluswidth,plusheight,GREEN);
    if(refill_on_off){
      stepperA.move(syringelength);
   }
   }
   if(p.x>vxcor1[1] && p.x<vxcor1[1]+pluswidth && p.y>vycor1[1] && p.y<vycor1[1]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    Serial.println("B homing");
    tft.fillCircle(285,vycor1[1]-30,20,GREEN);
    delay(DELAY);
    while(digitalRead(home_switchB)){
          stepperB.moveTo(initial_homing);
          initial_homing++;
          stepperA.run();stepperB.run();stepperC.run();stepperD.run();
        }
        stepperB.setCurrentPosition(0);stepperB.moveTo(0);
        initial_homing = 1;
        tft.fillCircle(285,vycor1[1]-30,20,BLACK);
    tft.fillRect(vxcor1[1],vycor1[1],pluswidth,plusheight,GREEN);
    if(refill_on_off){
      stepperB.move(syringelength);
   }
   }

   if(p.x>vxcor1[2] && p.x<vxcor1[2]+pluswidth && p.y>vycor1[1] && p.y<vycor1[1]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    Serial.println("C homing");
    tft.fillCircle(285,vycor1[1]-30,20,GREEN);
    delay(DELAY);
    while(digitalRead(home_switchC)){
          stepperC.moveTo(initial_homing);
          initial_homing++;
          stepperA.run();stepperB.run();stepperC.run();stepperD.run();
        }
        initial_homing = 1;
        tft.fillCircle(285,vycor1[1]-30,20,BLACK);
        stepperC.setCurrentPosition(0);stepperC.moveTo(0);
    tft.fillRect(vxcor1[2],vycor1[1],pluswidth,plusheight,GREEN);
    if(refill_on_off){
      stepperC.move(syringelength);
   }
   }
   if(p.x>vxcor1[3] && p.x<vxcor1[3]+pluswidth && p.y>vycor1[1] && p.y<vycor1[1]+plusheight) 
   { 
   pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    Serial.println("D homing");
    tft.fillCircle(285,vycor1[1]-30,20,GREEN);
    delay(DELAY);
    while(digitalRead(home_switchD)){
          stepperD.moveTo(initial_homing);
          initial_homing++;
          stepperA.run();stepperB.run();stepperC.run();stepperD.run();
        }
        initial_homing = 1;
        tft.fillCircle(285,vycor1[1]-30,20,BLACK);
        stepperD.setCurrentPosition(0);stepperD.moveTo(0);
    tft.fillRect(vxcor1[3],vycor1[1],pluswidth,plusheight,GREEN);
    if(refill_on_off){
      stepperD.move(syringelength);
   }
   }
   if(p.y>200) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperA.move(0);stepperB.move(0);stepperC.move(0);stepperD.move(0);stepperA.stop();stepperB.stop();stepperC.stop();stepperD.stop();stepperA.move(0);stepperB.move(0);stepperC.move(0);stepperD.move(0);
    Serial.println("EXIT");
    YN = false;
    //drawHomeScreen();
    //return;
   }
   delay(10);
   if(p.x>265 && p.x<265+40+pluswidth && p.y>vycor1[1]-10 && p.y<vycor1[1]+90) 
   {  //265,vycor[1]-10,40,150
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    tft.fillCircle(285,vycor1[1]-30,20,GREEN);
    refill_on_off = true;
    delay(DELAY);
    tft.fillCircle(285,vycor1[1]-30,20,BLACK);
    tft.fillRect(265,vycor1[1]-10,40,90,GREEN);
 tft.setTextSize(2);
 tft.setCursor(270+10,vycor1[1]);
 tft.println("F");
 tft.setCursor(270+10,vycor1[1]+20);
 tft.println("I");
 tft.setCursor(270+10,vycor1[1]+40);
 tft.println("L");
 tft.setCursor(270+10,vycor1[1]+60);
 tft.println("L");
   }
  }
  stepperA.run();stepperB.run();stepperC.run();stepperD.run();
  }
    /*
         while(digitalRead(home_switchA)){
          stepperA.moveTo(initial_homing);
          initial_homing++;
          stepperA.run();
        }
        Serial.println("A Homed"); 
        stepperA.setCurrentPosition(0);
        initial_homing = 1;
        //servo.write(REFILL);
        //stepperA.moveTo(-11790);
        tft.setCursor(vxcor[0],50);
        tft.setTextSize(2);
        tft.println(msg[0]);
        
        pinMode(home_switchB, INPUT_PULLUP);
        while(digitalRead(home_switchB)){
          stepperB.moveTo(initial_homing);
          initial_homing++;
          stepperB.run();stepperA.run();
        }
        Serial.println("B Homed");
        stepperB.setCurrentPosition(0);
        initial_homing = 1;
        //stepperB.moveTo(-117960);
        tft.setCursor(vxcor[1],70);
        tft.setTextSize(2);
        tft.println(msg[1]);
        
        pinMode(home_switchC, INPUT_PULLUP);
        while(digitalRead(home_switchC)){
          stepperC.moveTo(initial_homing);
          initial_homing++;
          stepperC.run();stepperB.run();stepperA.run();
        }
        Serial.println("C Homed");
        stepperC.setCurrentPosition(0);
        initial_homing = 1;
        tft.setCursor(vxcor[2],90);
        tft.setTextSize(2);
        tft.println(msg[2]);
        //stepperC.moveTo(-117960);
      
        pinMode(home_switchD, INPUT_PULLUP);
        while(digitalRead(home_switchD)){
          stepperD.moveTo(initial_homing);
          initial_homing++;
          stepperD.run();stepperC.run();stepperB.run();stepperA.run();
        }
        Serial.println("D Homed");
        tft.setCursor(vxcor[3],110);
        tft.setTextSize(2);
        tft.println(msg[3]);
        stepperD.setCurrentPosition(0);
        initial_homing = 1;
        servo.write(OPEN);
        //stepperD.moveTo(-117960);
      
        while (stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0){stepperA.run();stepperB.run();stepperC.run();stepperD.run();};
        */
       tft.fillRect(0,200,400,40,RED);
       tft.setCursor(vxcor[0]+120,200+5);
       tft.println("EXIT");
        t_homing = millis();
        /*
        stepperA.setMaxSpeed(500);
        stepperA.setAcceleration(5000);
        stepperB.setMaxSpeed(500);
        stepperB.setAcceleration(5000);
        stepperC.setMaxSpeed(500);
        stepperC.setAcceleration(5000);
        stepperD.setMaxSpeed(500);
        stepperD.setAcceleration(5000);
        Serial.println("HOMED, homie");
        */
  }




void manual(){
  //Called when you press manual
  tft.fillRect(0,0,750,750,BLACK);
  char *msg[] = {"A", "B", "C", "D",};
  int vxcor[] = {20,80,140,200};
  int vycor[] = {20,50,130,200};
  int pluswidth = 40;
  int plusheight = 60;
  int minuswidth = 40;
  int minusheight = 40;
  int x = 0;
  int y = 0;
  tft.setTextSize(1);
  while(x<4){
    tft.setCursor(vxcor[x]+1,vycor[0]);
    tft.println(msg[x]);
    tft.drawRect(vxcor[x],vycor[0]-7,40,20,GREEN);
    x++;
  }
  x = 0;
  tft.setTextSize(4);
  while(x<4){
    tft.fillRect(vxcor[x],vycor[1],pluswidth,plusheight,GREEN);
    tft.setCursor(vxcor[x]+10,vycor[1]+18);
    tft.println("+");
    x++;
  }
  x = 0;
    while(x<4){
    tft.fillRect(vxcor[x],vycor[2],minuswidth,minusheight,BLUE);
    tft.setCursor(vxcor[x]+10,vycor[2]+6);
    tft.println("-");
    x++;
  }
  x = 0;
 tft.fillRect(265,vycor[1]-10,40,70,MAGENTA);
 tft.setTextSize(2);
 tft.setCursor(270+10,vycor[1]);
 tft.println("R");
 tft.setCursor(270+10,vycor[1]+20);
 tft.println("E");
 tft.setCursor(270+10,vycor[1]+40);
 tft.println("F.");
// tft.setCursor(270+10,vycor[1]+60);
// tft.println("U");
tft.fillRect(265,vycor[2]-10,40,70,MAGENTA);
 tft.setCursor(270+10,vycor[1]+80);
 tft.println("I");
 tft.setCursor(270+10,vycor[1]+100);
 tft.println("M");
 tft.setCursor(270+10,vycor[1]+120);
 tft.println("P.");
 tft.fillRect(0,vycor[3],400,40,RED);
 tft.setCursor(vxcor[0]+120,vycor[3]+5);
 tft.println("EXIT");

bool capture =true;
long stepsPos = 1000;
long stepsNeg = -1000;
int DELAY = 200;

 while(capture){
    TSPoint p = ts.getPoint();  //Get touch point
  if (p.z > ts.pressureThreshhold) {
   
   Serial.print("X = "); Serial.print(p.x);
   Serial.print("\tY = "); Serial.print(p.y);
   Serial.print("\n");
   
   p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
   p.y = map(p.y, TS_MAXY, TS_MINY, 0, 240);
//EXIT    
   if(p.y>200) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperA.move(0);stepperB.move(0);stepperC.move(0);stepperD.move(0);stepperA.stop();stepperB.stop();stepperC.stop();stepperD.stop();stepperA.move(0);stepperB.move(0);stepperC.move(0);stepperD.move(0);
    Serial.println("EXIT");
    capture = false;
    drawHomeScreen();
   }
   delay(10);
//POS
   if(p.x>vxcor[0] && p.x<vxcor[0]+pluswidth && p.y>vycor[1] && p.y<vycor[1]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperA.move(stepsPos+stepperA.distanceToGo());
    Serial.println("A POS");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }
   if(p.x>vxcor[1] && p.x<vxcor[1]+pluswidth && p.y>vycor[1] && p.y<vycor[1]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperB.move(stepsPos+stepperB.distanceToGo());
    Serial.println("B POS");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }

   if(p.x>vxcor[2] && p.x<vxcor[2]+pluswidth && p.y>vycor[1] && p.y<vycor[1]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperC.move(stepsPos+stepperC.distanceToGo());
    Serial.println("C POS");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }
   if(p.x>vxcor[3] && p.x<vxcor[3]+pluswidth && p.y>vycor[1] && p.y<vycor[1]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperD.move(stepsPos+stepperD.distanceToGo());
    Serial.println("D POS");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }

//NEG
    if(p.x>vxcor[0] && p.x<vxcor[0]+pluswidth && p.y>vycor[2] && p.y<vycor[2]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperA.move(stepsNeg+stepperA.distanceToGo());
    Serial.println("A NEG");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }
   if(p.x>vxcor[1] && p.x<vxcor[1]+pluswidth && p.y>vycor[2] && p.y<vycor[2]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperB.move(stepsNeg+stepperB.distanceToGo());
    Serial.println("B NEG");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }
   if(p.x>vxcor[2] && p.x<vxcor[2]+pluswidth && p.y>vycor[2] && p.y<vycor[2]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperC.move(stepsNeg+stepperC.distanceToGo());
    Serial.println("C NEG");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }
   if(p.x>vxcor[3] && p.x<vxcor[3]+pluswidth && p.y>vycor[2] && p.y<vycor[2]+plusheight) 
   { 
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    stepperD.move(stepsNeg+stepperD.distanceToGo());
    Serial.println("D NEG");
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }
//refill
   if(p.x>265 && p.x<265+40+pluswidth && p.y>vycor[1]-10 && p.y<vycor[1]+70) 
   {  //265,vycor[1]-10,40,150
    
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    tft.fillRect(265,vycor[2]-10,40,70,RED);
    stepsNeg = syringelength;
    //impulse();
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);
   }
  
  //impulse
   if(p.x>265 && p.x<265+40+pluswidth && p.y>vycor[2]-10 && p.y<vycor[2]+70) 
   {  //265,vycor[1]-10,40,150
    pinMode(XM, OUTPUT);pinMode(YP, OUTPUT);
    tft.fillCircle(285,vycor[1]-30,20,GREEN);
    impulse(3000);
    delay(DELAY);
    tft.fillCircle(285,vycor[1]-30,20,BLACK);

   }
  }
  stepperA.run();
  stepperB.run();
  stepperC.run();
  stepperD.run();
 } 
}

void impulse(int STEPS){
  Serial.println("IMPULSE");
  servo.write(CLOSED);
  stepperA.move(STEPS);
  while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
  servo.write(OPEN);
  delay(1000);
  servo.write(CLOSED);
  return;
}


void drawHomeScreen(){

    tft.setRotation(1); // Landscape Mode
    tft.fillScreen(0x0000); // Black Screen
    tft.drawRect(20,5,280,30,GREEN);
    tft.setCursor(35,12);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("SyringeMaster 3000 MM");
    
    tft.fillRect(23,VD_Y-60-15, 130, 130, RED);
    tft.setCursor(23, VD_Y-55);
    tft.setTextSize(2);
    tft.println(" PROGRAM ");

    tft.fillRect(23+150,VD_Y-60-15, 130, 130, RED);
    tft.setCursor(23+150, VD_Y-55);
    tft.setTextSize(2);
    tft.println("  MANUAL");
}


void wait(int waittime){
      long minutes = 0; long hours = 0; long seconds = 0;
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      //if(waittime<=60){
        seconds = waittime;
      tft.setCursor(23+20,65+20);tft.print("Waiting "); tft.print(waittime); tft.print(" seconds");
      Serial.print("Waiting "); Serial.print(seconds); Serial.println(" seconds");
      //}
      /*
      if(waittime>60 && waittime <=3600){
        minutes = waittime/60 -hours*3600 ; seconds = waittime - minutes*60 - hours*3600;
        tft.setCursor(23+20,65+20);tft.print("Waiting "); tft.print(minutes); tft.print("min."); tft.print(seconds); tft.print("sec.");
        Serial.print("Waiting "); Serial.print(minutes); Serial.print("min.");Serial.print(seconds); Serial.println("sec.");
      }
      if(waittime>3600){
        hours = waittime/3600 ;
        minutes = waittime%60 ;
        seconds = waittime - hours*3600 -minutes*60;
        tft.setCursor(23+20,65+20);tft.print("Waiting "); tft.print(hours); tft.print("h.");tft.print(minutes); tft.print("min."); tft.print(seconds); tft.print("sec.");
        Serial.print("Waiting "); Serial.print(hours); Serial.print("h.");Serial.print(minutes); Serial.println("min.");Serial.print(seconds); Serial.println("sec.");
      }
      */
      delay(waittime*1000);
      
      /*
      //BROKEN
      long minutes = 0; long hours = 0; long seconds = 0;
      tft.setCursor(23+20,65+20);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
   while(waittime>0){
      Serial.println(waittime);
        tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      
        
        
      }
      if(waittime>3600){
        hours = waittime/60 ;
        minutes = waittime/60 -hours*3600 ;
        seconds = waittime - minutes*60 - hours*3600;
        tft.setCursor(23+20,65+20);tft.print("Waiting "); tft.print(hours); tft.print("h.");tft.print(minutes); tft.print("min."); tft.print(seconds); tft.print("sec.");
        Serial.print("Waiting "); Serial.print(hours); Serial.print("h.");Serial.print(minutes); Serial.println("min.");Serial.print(seconds); Serial.println("sec.");
        waittime = waittime-3600;delay(3600*10000);
      }
      if(waittime<60){
        seconds = waittime;
      tft.setCursor(23+20,65+20);tft.print("Waiting "); tft.print(seconds); tft.print(" seconds");
      Serial.print("Waiting "); Serial.print(seconds); Serial.println(" seconds");
        waittime = waittime -1; delay(1000);
      }
       
        
      }*/
}

void medium(int STEPS, int SPEED){
  
  stepperD.setMaxSpeed(SPEED);
  stepperD.move(STEPS);
  int SECONDS = STEPS/SPEED;
  Serial.print("Moving MEDIUM by "); Serial.print(STEPS) ; Serial.println(" steps");
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(RED);
      tft.setTextSize(2);
      tft.print("Moving Medium by "); tft.setCursor(23+20,65+40);tft.print(STEPS); tft.print(" steps for ");tft.setCursor(23+20,65+60); tft.print(SECONDS); tft.print(" seconds");
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      //servo.write(0);
      //stepperD.setMaxSpeed(100);
      //stepperD.move(PIVOT);
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      //servo.write(CLOSED);
}
void pbs(int STEPS, int SPEED){
  stepperB.setMaxSpeed(SPEED);
  stepperB.move(STEPS);
  int SECONDS = STEPS/SPEED;
  Serial.print("Moving PBS by "); Serial.print(STEPS) ; Serial.println(" steps");
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(BLUE);
      tft.setTextSize(2);
      tft.print("Moving PBS by "); tft.setCursor(23+20,65+40);tft.print(STEPS); tft.print(" steps for ");tft.setCursor(23+20,65+60); tft.print(SECONDS); tft.print(" seconds");
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      //servo.write(0);
      //stepperB.setMaxSpeed(100);
      //stepperB.move(PIVOT);
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      //servo.write(CLOSED);
}
void trypsin(int STEPS, int SPEED){
  stepperC.setMaxSpeed(SPEED);
  stepperC.move(STEPS);
  int SECONDS = STEPS/SPEED;
  Serial.print("Moving Trypsin by "); Serial.print(STEPS) ; Serial.println(" steps");
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(RED);
      tft.setTextSize(2);
      tft.print("Moving Trypsin by "); tft.setCursor(23+20,65+40);tft.print(STEPS); tft.print(" steps for ");tft.setCursor(23+20,65+60); tft.print(SECONDS); tft.print(" seconds");
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      //servo.write(0);
      //stepperC.setMaxSpeed(100);
      //stepperC.move(PIVOT);
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      //servo.write(CLOSED);
}
void air(int STEPS, int SPEED){
  stepperA.setMaxSpeed(SPEED);
  stepperA.move(STEPS);
  servo.write(OPEN);
  int SECONDS = STEPS/SPEED;
  Serial.print("Moving Air by "); Serial.print(STEPS) ; Serial.println(" steps");
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(RED);
      tft.setTextSize(2);
      tft.print("Moving Air by "); tft.setCursor(23+20,65+40);tft.print(STEPS); tft.print(" steps for ");tft.setCursor(23+20,65+60); tft.print(SECONDS); tft.print(" seconds");
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0 )&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
    delay(500);
    servo.write(CLOSED);
}


bool empty(){
   if (!digitalRead(home_switchA) || !digitalRead(home_switchB) ||!digitalRead(home_switchC) ||!digitalRead(home_switchD)){
    Serial.println("EMPTY!!!");
     stepperA.stop();stepperB.stop();stepperC.stop();stepperD.stop();
    drawHomeScreen();
    tft.drawRect(20-1,5-1,280+2,30+2,GREEN);
    tft.fillRect(20,5,280,30,BLACK);
    tft.setCursor(35,12);
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.print("EMPTY");
    tft.setTextColor(WHITE);

      emptyYN = true;
    return true;
   }
  //Serial.println("FALSE");
  return false;
  
}

void pivot(char S,long steps){
  servo.write(0);
  delay(200);
  if(S == 'A'){stepperA.moveTo(-steps);}
  if(S == 'B'){stepperB.moveTo(-steps);}
  if(S == 'C'){stepperC.moveTo(-steps);}
  if(S == 'D'){stepperD.moveTo(-steps);}
  while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0 )&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();};
  servo.write(CLOSED);
}

void program2(){   //  REFERENCE PROGRAM
     // HOW TO USE:
    // wait(x): wait x seconds
    // medium(STEPS,SPEED), pbs(STEPS,SPEED), trypsin(STEPS,SPEED), air(STEPS,SPEED), 

    t_start = millis();

    tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
    tft.fillRect(23,65,150+130,130,BLACK);
    tft.setCursor(23+20,65+20);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    Serial.println("Running Program");
    delay(500);
      stepperA.setAcceleration(10000);
      stepperB.setAcceleration(10000);
      stepperC.setAcceleration(10000);
      stepperD.setAcceleration(10000);
      
      int fullCartrige = 17000;   //in steps
      int bubble = 1300;          //in steps
      int trypsin_time = 120000;    // in milliseconds
      int medium_speed = 200;     // in steps/sec. 
      int time_now = millis();
      int t = 52;              //TRYPSIN TIME in SECONDS
      
      int i = 0;
      servo.write(CLOSED);
      medium(fullCartrige,medium_speed);if(emptyYN){return;}
      //delay(6000);
      air(bubble,1000);if(emptyYN){return;}
      //delay(5000);
      pbs(fullCartrige,3000);if(emptyYN){return;}
      air(bubble,1000);if(emptyYN){return;}
      trypsin(fullCartrige,3000);if(emptyYN){return;}
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);tft.fillRect(23,65,150+130,130,BLACK);tft.setCursor(23+20,65+20);tft.setTextColor(RED);tft.setTextSize(2);
      tft.print("Waiting for  "); tft.setCursor(23+20,65+40);tft.print(t); tft.print(" seconds ");
      while(i<t){delay(1000);i++;}i=0;
      air(bubble,1000);if(emptyYN){return;}
      medium(fullCartrige,1000);if(emptyYN){return;}
      //air(bubble,1000);if(emptyYN){return;}
      //medium(fullCartrige/3,1000);if(emptyYN){return;}
      //air(bubble,1000);if(emptyYN){return;}
      //medium(fullCartrige/2,1000);if(emptyYN){return;}

      //  Here: Bubble

      
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);tft.fillRect(23,65,150+130,130,BLACK);tft.setCursor(23+20,65+20);tft.setTextColor(RED);tft.setTextSize(2);
      tft.print("Change to Cell Collection  "); tft.setCursor(23+20,65+40);tft.print("30"); tft.print(" seconds ");
      while(i<30){delay(1000);i++;}i=0;                                   //wait 30 s
      impulse(2000);if(emptyYN){return;}
      impulse(2000);if(emptyYN){return;}
      medium(16000,1000);if(emptyYN){return;}
      air(23000,3000);if(emptyYN){return;}
      
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
      Serial.println("FINISHED");
      delay(10000);
      drawHomeScreen();
      tft.fillRect(20+1,5+1,280-2,30-2,BLACK);
      tft.setCursor(35,12);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
  
}

void program3(){   //  REFERENCE PROGRAM  with longer trypsin time
     // HOW TO USE:
    // wait(x): wait x seconds
    // medium(STEPS,SPEED), pbs(STEPS,SPEED), trypsin(STEPS,SPEED), air(STEPS,SPEED), 

    t_start = millis();

    tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
    tft.fillRect(23,65,150+130,130,BLACK);
    tft.setCursor(23+20,65+20);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    Serial.println("Running Program");
    delay(500);
      stepperA.setAcceleration(10000);
      stepperB.setAcceleration(10000);
      stepperC.setAcceleration(10000);
      stepperD.setAcceleration(10000);
      
      int fullCartrige = 17000;   //in steps
      int bubble = 2000;          //in steps
      int trypsin_time = 6000;    //CHANGE HERE// in milliseconds
      int medium_speed = 200;     // in steps/sec. 
      
      servo.write(CLOSED);
      medium(fullCartrige,medium_speed);if(emptyYN){return;}
      delay(6000);
      air(bubble,1000);if(emptyYN){return;}
      delay(5000);
      pbs(fullCartrige,3000);if(emptyYN){return;}
      air(bubble,1000);if(emptyYN){return;}
      trypsin(fullCartrige,3000);if(emptyYN){return;}
      delay(trypsin_time);                              //Trypsinierungs Zeit!!!
      air(bubble,1000);if(emptyYN){return;}
      medium(fullCartrige/3,1000);if(emptyYN){return;}
      air(bubble,1000);if(emptyYN){return;}
      medium(fullCartrige/3,1000);if(emptyYN){return;}
      air(bubble,1000);if(emptyYN){return;}
      medium(fullCartrige/3,1000);if(emptyYN){return;}
      impulse(2000);if(emptyYN){return;}
      impulse(2000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
        
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
      Serial.println("FINISHED");
      delay(10000);
      drawHomeScreen();
      tft.fillRect(20+1,5+1,280-2,30-2,BLACK);
      tft.setCursor(35,12);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
  
}

void program4(){   //  REFERENCE PROGRAM  with different Medium Speed
     // HOW TO USE:
    // wait(x): wait x seconds
    // medium(STEPS,SPEED), pbs(STEPS,SPEED), trypsin(STEPS,SPEED), air(STEPS,SPEED), 

    t_start = millis();

    tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
    tft.fillRect(23,65,150+130,130,BLACK);
    tft.setCursor(23+20,65+20);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    Serial.println("Running Program");
    delay(500);
      stepperA.setAcceleration(10000);
      stepperB.setAcceleration(10000);
      stepperC.setAcceleration(10000);
      stepperD.setAcceleration(10000);
      
      int fullCartrige = 17000;   //in steps
      int bubble = 2000;          //in steps
      int trypsin_time = 6000;    // in milliseconds
      int medium_speed = 200;     //CHANGE!!!  // in steps/sec. 
      
      servo.write(CLOSED);
      medium(fullCartrige,medium_speed);if(emptyYN){return;}
      delay(6000);
      air(bubble,1000);if(emptyYN){return;}
      delay(5000);
      pbs(fullCartrige,3000);if(emptyYN){return;}
      air(bubble,1000);if(emptyYN){return;}
      trypsin(fullCartrige,3000);if(emptyYN){return;}
      delay(trypsin_time);                              //Trypsinierungs Zeit!!!
      air(bubble,1000);if(emptyYN){return;}
      medium(fullCartrige/3,1000);if(emptyYN){return;}
      air(bubble,1000);if(emptyYN){return;}
      medium(fullCartrige/3,1000);if(emptyYN){return;}
      air(bubble,1000);if(emptyYN){return;}
      medium(fullCartrige/3,1000);if(emptyYN){return;}
      impulse(2000);if(emptyYN){return;}
      impulse(2000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
      air(10000,3000);if(emptyYN){return;}
        
      tft.drawRect(23-1,65-1,150+130+2,130+2,WHITE);
      tft.fillRect(23,65,150+130,130,BLACK);
      tft.setCursor(23+20,65+20);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
      Serial.println("FINISHED");
      delay(10000);
      drawHomeScreen();
      tft.fillRect(20+1,5+1,280-2,30-2,BLACK);
      tft.setCursor(35,12);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("FINISHED");
  
}

void clean(int times){ //CLEANING PROGRAM..  PLACE ALCOHOL SYRINGE IN B, AIR+SERVO IN A
    int x = 0;
    int fullCartrige = 17000;
    stepperA.setMaxSpeed(3000);
    stepperB.setMaxSpeed(3000);
    while(x<times){
      stepperB.move(fullCartrige);
      servo.write(CLOSED);
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      servo.write(OPEN);
      stepperA.move(116000);
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)&&!empty()){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      servo.write(REFILL);
      stepperA.move(-116000);
      while ((stepperA.distanceToGo() != 0 ||stepperB.distanceToGo() != 0 || stepperC.distanceToGo() != 0 ||stepperD.distanceToGo() != 0)){stepperA.run();stepperB.run();stepperC.run();stepperD.run();}; 
      servo.write(CLOSED);
      
      x++;
    }
    Serial.println("Finished Cleaning");
}
