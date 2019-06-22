/* This is not a code 9 */

/*    To do list 

1-> define kardane zarayeb -> Done
2-> sharp haye darvazeban
3-> test tadakhol robot ha
4-> addade sharp state dar messi

*/

#include <LiquidCrystal.h>
#include <Wire.h>

//#define ROBOT "BLACK"
#define ROBOT "WHITE"

//-------------------------------------v Algorithm v---------------------------------
#define ZARIB_COMPASS_ZIRE_20 16  
#define ZARIB_COMPASS_BALAYE_20 11

#define ZARIBE_CHAP_RAST 0.7 //zarin chaprast jahate sensor haye 11 o 7
//#define ZARIBE_CHAP_RAST 0.6 //zarin chaprast jahate sensor haye 11 o 7

#define ZARIB_SHARP_DIFF 20; //20 
int state = 0;
int robotMode = 0; // 1 for Debuge -- 2 for Forward -- 3 for Goalkeeper 

//-------------------------------------v Motors v----------------------------------------
#define MOTOR_ID1 1 //Front Left
#define MOTOR_ID2 2 //Back Left
#define MOTOR_ID3 3 //Back Right
#define MOTOR_ID4 4 //Front Right

#define PRESENT_VOLTAGE 42
#define PRESENT_TEMP 43
#define GOAL_SPEED 32

Dynamixel Dxl(1); //1 : Is For OpenCM And 2 Or 3 Value Is For Other...


word syncPage[12]=
{ 
MOTOR_ID1,0,0,  
MOTOR_ID2,0,0,  
MOTOR_ID3,0,0, 
MOTOR_ID4,0,0}; 

//-------------------------------------v LCD v--------------------------------------------
LiquidCrystal lcd(21,20,19,18,17,16);
int currentPage = 0;
int currentSubPage = 0;

//-------------------------------------v Compass v----------------------------------------
#define ADDRESS 0x60 //defines address of compass
int firstValueCmpr = 0;
int cmpr = 0;  //current value of compass (converted value)

//-------------------------------------v Sharps v-----------------------------------------
#define SHARP_RIGHT 2
#define SHARP_BACK 3
#define SHARP_LEFT 4
int sharpRight;
int sharpLeft;
int sharpBack;
int sharpDiff;//diffrence between sharpRight and sharpLeft;

//-------------------------------------v Buttons v----------------------------------------  
#define BUTTON_OK 22
#define BUTTON_UP 23
#define BUTTON_DOWN 24
#define BUTTON_CANCEL 25

//-------------------------------------v Sensors v----------------------------------------
#define D 12
#define C 13
#define A 14
#define B 15

#define SENSOR_OUTPUT_BALL 8  //OUTPUT Sensor Ball
#define SENSOR_OUTPUT_KAF 9  //OUTPUT Sensor Kaf
int sensorArrayBall[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int activeSensorBall = 0;

//-------------------------------------v LEDs v--------------------------------------
#define LED2 0
#define LED3 1
//-------------------------------------v Buzzer v------------------------------------   
#define BUZZER 7


//#####################################v Functions v######################################

void warn(){
  
  
  
  if (Dxl.readByte(1,PRESENT_VOLTAGE) < 139){
    lcd.clear();
    lcd.print("!!Battery low!!");
    for(int i = 0;i<11;i++){
      digitalWrite(BUZZER,HIGH);
      delay(100);
      digitalWrite(BUZZER,LOW);
      delay(100);
    }
   }
   
  if (Dxl.readByte(1,PRESENT_TEMP) > 68){
    lcd.clear();
    lcd.print("Motor temp= ");
    lcd.print(Dxl.readByte(1,PRESENT_TEMP));
    lcd.print("!");
    for(int i = 0;i<11;i++){
      digitalWrite(BUZZER,HIGH);
      delay(500);
      digitalWrite(BUZZER,LOW);
      delay(100);
    }
    
  }
  
}

int compass() {  //this function return the real value of compass
  if(ROBOT == "BLACK"){
      byte first,second,third;
      int total=0;
      
      Wire.beginTransmission(0x60);  //starts communication with cmps03
      Wire.write(1);  //Sends the register we wish to read
      Wire.endTransmission();
      Wire.requestFrom(0x60,1);        //requests high byte
      //while(Wire.available()<2);
      first =   Wire.read();
      //second =  Wire.read();           //reads the byte as an integer
      //third=    Wire.read();
      
      //total = (first<<8)+(second);
       total = first;
       
       total = map(total,128,255,0,360);
      
      return total;
    
  }
  else{ /// white
      int first,second,third;
      int total=0;
      
      Wire.beginTransmission(ADDRESS);  //starts communication with cmps03
      //delay(1);
      Wire.write(1);  //Sends the register we wish to read
      //delay(1);
      Wire.endTransmission();
      //delay(1);
      Wire.requestFrom(ADDRESS, 3);        //requests high byte
      
      first = Wire.read();
      second =  Wire.read();           //reads the byte as an integer
      third=Wire.read();
      
      total = map((first<<8)+(second) , 57400, 65535, 0, 360);
      
      return total;
  }
}


void readCompass(){  //this function will set (cmpr) variable to converted compass value
  
    cmpr = compass() - firstValueCmpr;
    
  
  if(cmpr < 0 ){
    cmpr += 360;
  }
  if(cmpr > 180 ){
    cmpr -= 360;
  }
  if(cmpr < 180){
    cmpr = cmpr;
  }
  
  if(abs(cmpr) < 20) cmpr *= ZARIB_COMPASS_ZIRE_20;
  else cmpr *= ZARIB_COMPASS_BALAYE_20;
      
}

void readSharp(){
  
  int sum = 0;
  for(int i=0;i<5;i++) {
     sum+=analogRead(SHARP_RIGHT);
  }
  sharpRight = map((sum/5), 0, 4095, 0, 255);
  
  sum = 0;
  for(int i=0;i<5;i++) {
     sum+=analogRead(SHARP_LEFT);
  }
  sharpLeft =  map((sum/5), 0, 4095, 0, 255);
  
  sum = 0;
  for(int i=0;i<5;i++) {
     sum+=analogRead(SHARP_BACK);
  }
  sharpBack = (255 - 50 - map((sum/5), 0, 4095, 0, 255));
  
  sharpBack = map (sharpBack , -34, 125 , 0 , 10);
  
  
  sharpDiff = sharpRight - sharpLeft;
  if(abs(sharpDiff) < 7) sharpDiff = 0;
  else sharpDiff *= ZARIB_SHARP_DIFF;
  
}

void motor(int speed1, int speed2, int speed3, int speed4){
          
      if(speed1 > 1023) speed1 = 1020;
      if(speed2 > 1023) speed2 = 1020;
      if(speed3 > 1023) speed3 = 1020;
      if(speed4 > 1023) speed4 = 1020;
      
      if(speed1 < -1023) speed1 = -1020;
      if(speed2 < -1023) speed2 = -1020;
      if(speed3 < -1023) speed3 = -1020;
      if(speed4 < -1023) speed4 = -1020;
      
      
      if (speed1 >= 0) {
          syncPage[2] = speed1;
      }else {
          speed1 = speed1 * -1;
          syncPage[2] = speed1 + 1024;
      }

      if (speed2 >= 0) {
          syncPage[5] = speed2;
      }else {
          speed2 = speed2 * -1;
          syncPage[5] = speed2 + 1024;
      }

      if (speed3 >= 0) {
          syncPage[8]=speed3+1024;
      }else {
          speed3 = speed3 * -1;
          syncPage[8]=speed3;
      }
      
      if (speed4 >= 0) {
          syncPage[11] = speed4 + 1024;
      }else {
          speed4 = speed4 * -1;
          syncPage[11] = speed4;
      }
      
      Dxl.syncWrite(30,2,syncPage,12);
      
}

void readSensor(){
    
    sensorArrayBall[0] = 5000;

    for(int i=1; i<17; i++){
      
      int _d = ((i-1)/8)%2;
      int _c = ((i-1)/4)%2;
      int _b = ((i-1)/2)%2;
      int _a = (i-1)%2;

      digitalWrite(D,_d);
      digitalWrite(C,_c);
      digitalWrite(B,_b);
      digitalWrite(A,_a);

      int sum = 0;
      for(int j=0; j<1; j++){
        sum += analogRead(SENSOR_OUTPUT_BALL);
      }
      sensorArrayBall[i] = sum/1;
      
    }  
    
    activeSensorBall = 0;
    int minTemp = sensorArrayBall[0];
    
    for(int i=1; i<17; i++){
        
      if( sensorArrayBall[i] < minTemp ){
          minTemp = sensorArrayBall[i];
          activeSensorBall = i;
      }
      
    }
    
    if(sensorArrayBall[activeSensorBall] > 1900){
      activeSensorBall = 0;
    }
    
    
    

}


void aros(){

  while(true){
    
    readSharp();
    readCompass();
    motor((1000-cmpr), (1000-cmpr), (1000+cmpr), (1000+cmpr)); //orignal forward
    //motor((-1000-cmpr), (-1000-cmpr), (-1000+cmpr), (-1000+cmpr)); // orignal backward
    
  }

}


void damad(){

  while(true){
    
    readSharp();
    readCompass();
    motor(1000-cmpr-sharpDiff , 1000-cmpr+sharpDiff, 1000+cmpr-sharpDiff, 1000+cmpr+sharpDiff);
    
  }

}


void messi(){
  
  lcd.clear();
  lcd.print("MESSI!");
  int timeOut=0;
  
  while(true){
    
    readCompass();
    readSensor();
    readSharp();
    
    if( abs(cmpr) < 600 && ( (sharpLeft > 150 && sharpRight<35) || (sharpRight > 150 && sharpLeft<24) )   ){ //   130 shavad
      state = 1;
      digitalWrite(LED3,LOW);
    }else{
      state = 0;
      digitalWrite(LED3,HIGH);
    }
    
    if(state == 1){
      
      timeOut = 0;
      
      /*if(sharpDiff < 0)
        motor((1000-cmpr), (-1000-cmpr), (1000+cmpr), (-1000+cmpr));
      else if( sharpDiff > 0)
         motor((-1000-cmpr), (1000-cmpr), (-1000+cmpr), (1000+cmpr));
      delay(100); */
      
    }else if(state == 0){
    
          if(activeSensorBall == 9 ){ timeOut = 0;  //****** forward
            motor((1000-cmpr), (1000-cmpr), (1000+cmpr), (1000+cmpr));
            //delay(80);
            //motor(1000-cmpr-sharpDiff , 1000-cmpr+sharpDiff, 1000+cmpr-sharpDiff, 1000+cmpr+sharpDiff); 
          }
          else if(activeSensorBall == 14 ){ timeOut = 0;  //****** backward
            motor((-1000-cmpr), (-1000-cmpr), (-1000+cmpr), (-1000+cmpr));
          }
          else if(activeSensorBall == 4){ timeOut = 0;  //****** backward
            motor((-1000-cmpr), (-1000-cmpr), (-1000+cmpr), (-1000+cmpr));  
          }
          
          else if(activeSensorBall == 13 ){ timeOut = 0;
            //motor(((1000*0.2)-cmpr), -(1000-cmpr), ((1000*0.2)+cmpr), -(1000+cmpr));
            motor((1000-800-cmpr), (-1000-cmpr), (1000-800+cmpr), (-1000+cmpr));
          }
          else if(activeSensorBall == 5){ timeOut = 0;
            //motor( -((1000)-cmpr), ((1000*0.2)-cmpr), -((1000)+cmpr), ((1000*0.2)+cmpr));
            motor((-1000-cmpr), (1000-800-cmpr), (-1000+cmpr), (1000-800+cmpr));
          }
          
          else if(activeSensorBall == 12 ){ timeOut = 0;
            motor(((1000-800)-cmpr), (-1000-cmpr), ((1000-800)+cmpr), (-1000+cmpr));
          }
          
          else if(activeSensorBall == 6 ){ timeOut = 0;
            motor( (-(1000)-cmpr), ((1000-800)-cmpr), (-(1000)+cmpr), ((1000-800)+cmpr));
          } 
          
          else if(activeSensorBall == 11 ){ timeOut = 0;
            motor(((1000)*ZARIBE_CHAP_RAST-cmpr), (-(1000)*ZARIBE_CHAP_RAST-cmpr), ((1000)*ZARIBE_CHAP_RAST+cmpr), (-(1000)*ZARIBE_CHAP_RAST+cmpr));
          }
          else if(activeSensorBall == 7 ){ timeOut = 0;
            motor((-(1000)*ZARIBE_CHAP_RAST-cmpr), ((1000)*ZARIBE_CHAP_RAST-cmpr), (-(1000)*ZARIBE_CHAP_RAST+cmpr), ((1000)*ZARIBE_CHAP_RAST+cmpr));
          }
          
          else if(activeSensorBall == 10 ){ timeOut = 0;
            motor((1000-cmpr), (1000-cmpr), (1000+cmpr), (1000+cmpr));
            //delay(135);
            //motor(1000-cmpr-sharpDiff , 1000-cmpr+sharpDiff, 1000+cmpr-sharpDiff, 1000+cmpr+sharpDiff);
          }
          else if(activeSensorBall == 8 ){ timeOut = 0;
            motor((1000-cmpr), (1000-cmpr), (1000+cmpr), (1000+cmpr));
            //delay(135);
            //motor(1000-cmpr-sharpDiff , 1000-cmpr+sharpDiff, 1000+cmpr-sharpDiff, 1000+cmpr+sharpDiff);
          } 
              
          else if ( activeSensorBall == 1){timeOut =0;
            if(sharpDiff < 0)
              motor((1000-cmpr), (-1000-cmpr), (1000+cmpr), (-1000+cmpr)); 
            else 
              motor((-1000-cmpr), (1000-cmpr), (-1000+cmpr), (1000+cmpr));
          }
          
          else if (activeSensorBall == 16){timeOut =0;
              motor((-1000-cmpr), (-1000*0.5-cmpr), (-1000+cmpr), (-1000*0.5+cmpr));
          }
          else if(activeSensorBall == 2){timeOut =0;
            motor((-1000*0.5-cmpr), (-1000-cmpr), (-1000*0.5+cmpr), (-1000+cmpr));
          }
          
          
          else if(activeSensorBall == 15 ){ timeOut = 0;
              motor((-1000-cmpr), (-1000*0.3-cmpr), (-1000+cmpr), (-1000*0.3+cmpr));
          }
          else if(activeSensorBall == 3 ){ timeOut = 0;
              motor((-1000*0.3-cmpr), (-1000-cmpr), (-1000*0.3+cmpr), (-1000+cmpr));
          }
          
          
          else{
            timeOut++;
            if(timeOut > 4  ){
              motor(0-cmpr*0.6, 0-cmpr*0.6, 0+cmpr*0.6, 0+cmpr*0.6);
              //goToGoal();
            }   
          }
        
    }
    
    delay(100);
    lcd.clear();
    
  }
  
}


void goToGoal(){ // rafta

    readCompass();
    readSensor();
    readSharp();
    if (sharpBack > 8){
      //motor((1000-cmpr),(1000-cmpr),(1000+cmpr),(1000+cmpr));
      //delay(10);
      motor((-(1000+cmpr+sharpDiff))*0.4 , (-(1000+cmpr-sharpDiff))*0.4, (-(1000-cmpr+sharpDiff))*0.4, (-(1000-cmpr-sharpDiff))*0.4);
    }
    else if(sharpBack < 5 )
    {
      motor((1000-cmpr-sharpDiff)*0.4 , (1000-cmpr+sharpDiff)*0.4, (1000+cmpr-sharpDiff)*0.4, (1000+cmpr+sharpDiff)*0.4);
    }
    else{
      motor(0-cmpr-sharpDiff , 0-cmpr+sharpDiff, 0+cmpr-sharpDiff, 0+cmpr+sharpDiff);
    }
    
}

void stayInGoal(){

  //while(true){
    
    if (sharpBack > 9){ //were8
      //readSharp();
      motor(-((000+cmpr+sharpDiff))*0.8 , -((000+cmpr-sharpDiff))*0.8, -((000-cmpr+sharpDiff))*0.8, -((000-cmpr-sharpDiff))*0.8);
    }
    else if(sharpBack < 6 ) //were 5
    {
      motor((1000-cmpr-sharpDiff)*0.4 , (1000-cmpr+sharpDiff)*0.4, (1000+cmpr-sharpDiff)*0.4, (1000+cmpr+sharpDiff)*0.4);
    }
    else{
      motor(0-cmpr-sharpDiff , 0-cmpr+sharpDiff, 0+cmpr-sharpDiff, 0+cmpr+sharpDiff);
    }
    
  //}
  
}

void abedZadeh(){ // original
  
  lcd.clear();
  lcd.print("GoalKeeper");
  
  int timeOut = 0;

  while(true){
  
    readSharp();
    readSensor();
    readCompass();
    
    //       robot same chap                      robot samte rast 
    if(  (sharpLeft < 80 && sharpRight > 37) || (sharpRight < 80 && sharpLeft < 35)  ){ //   130 shavad
    
    //if(abs(sharpDiff) < 1000){
    
        if(activeSensorBall == 9 || activeSensorBall == 8 || activeSensorBall == 10){  timeOut = 0;
            if(sensorArrayBall[activeSensorBall] < 600){
               motor(1000-cmpr, 1000-cmpr, 1000+cmpr, 1000+cmpr);
               delay(150);
               goToGoal();               
            }
            else{
              motor(0-cmpr*0.6, 0-cmpr*0.6, 0+cmpr*0.6, 0+cmpr*0.6);
            }
             
        }
        else if(activeSensorBall == 11 || activeSensorBall == 12 || activeSensorBall == 13){  timeOut = 0;
            motor((1000-cmpr), -(1000-cmpr), (1000+cmpr), -(1000+cmpr));
            delay(60);  //150
        }
        else if(activeSensorBall ==  7 || activeSensorBall ==  6 || activeSensorBall ==  5){  timeOut = 0;
            motor(-(1000-cmpr), (1000-cmpr), -(1000+cmpr), (1000+cmpr));
            delay(60);  //150
        }
        else{  timeOut++;
          stayInGoal();
          
          if(timeOut > 100)  goToGoal();
        }
    
    }else{
      timeOut++;
      if(timeOut > 100){
        
        //motor(1000-cmpr, 1000-cmpr, 1000+cmpr, 1000+cmpr);
        //delay(150);
        goToGoal();
      }
      else stayInGoal();
    }
  
  }
  
}





void deBug(){
  
  if(digitalRead(BUTTON_UP) == HIGH){
      currentPage++;
      while(digitalRead(BUTTON_UP) == HIGH);
  }
  
  if(digitalRead(BUTTON_DOWN) == HIGH){
      currentPage--;
      while(digitalRead(BUTTON_DOWN) == HIGH);
  }
    
    if(currentPage == 0){  //compass page
    
      readCompass();
      lcd.print("cmpr=");
      lcd.print(cmpr);
      lcd.print(" c=");
      lcd.print(compass());
      
    }
    else if(currentPage == 1){ //sharps page
      
      readSharp();
      
      lcd.print("L=");
      lcd.print(sharpLeft);

      lcd.print(" B=");
      lcd.print(sharpBack);
      
      lcd.print(" R=");
      lcd.print(sharpRight);
      
    }
    
    else if(currentPage == 2){ //sharps diff page
      
      readSharp();
      
      lcd.print("R-L= ");
      lcd.print(sharpDiff);
      
    }
    
    else if(currentPage == 3){ //sensors page
    
      readSensor();
      lcd.print("i= ");
      lcd.print((activeSensorBall));
      
      int value = map(sensorArrayBall[activeSensorBall],0,4095,0,255);
      lcd.print(" v=");
      lcd.print(value);
      
    }
    
    else if(currentPage == 4){  //battery page
      
      byte voltage = Dxl.readByte(1,PRESENT_VOLTAGE); 
      lcd.print("v= ");
      lcd.print(voltage);
      
    }
    
    else if(currentPage == 5){  //temperature page
      
      int t1 = Dxl.readByte(MOTOR_ID1, PRESENT_TEMP); 
      int t2 = Dxl.readByte(MOTOR_ID2, PRESENT_TEMP); 
      int t3 = Dxl.readByte(MOTOR_ID3, PRESENT_TEMP); 
      int t4 = Dxl.readByte(MOTOR_ID4, PRESENT_TEMP); 
      
      lcd.print("T");
      lcd.print(t1);
      lcd.print(" ");
      lcd.print(t2);
      lcd.print(" ");
      lcd.print(t3);
      lcd.print(" ");
      lcd.print(t4);
      
    }
   else if(currentPage == 6){ //Ejraye tabe aros
      
      lcd.print("ok ->  arose");
      
      if(digitalRead(BUTTON_OK) == HIGH){
        aros();
        
      }
  }
  
  else if(currentPage == 7){ //Ejraye tabe damad
      
      lcd.print("ok ->  damad");
      
      if(digitalRead(BUTTON_OK) == HIGH){
        damad();
        
      }
  }
      
      
    
    if(currentPage > 7 || currentPage < 0){
      currentPage = 0;
    }
    
    delay(100);
    lcd.clear();
  
}

void boot(){
  
  warn();
  
  int currentVol = Dxl.readByte(1,PRESENT_VOLTAGE);
  
  
  //lcd.print("Mode?  V=");
  //lcd.print(currentVol/10);
  //lcd.print(".");
  //lcd.print(currentVol%10);
  
  lcd.print("Mode?  ");
  lcd.print(currentVol);
  
  
  while(true){
    
    int upButtonValue = digitalRead(BUTTON_UP);
    int downButtonValue = digitalRead(BUTTON_DOWN);
    int okButtonValue = digitalRead(BUTTON_OK);
    int cancelButtonValue = digitalRead(BUTTON_CANCEL);
    
    while(digitalRead(BUTTON_UP) == HIGH){
      robotMode = 2;
      
    }
    
    while(digitalRead(BUTTON_DOWN) == HIGH){
      robotMode = 3;
    }
    
    while(digitalRead(BUTTON_OK) == HIGH || digitalRead(BUTTON_CANCEL) == HIGH){
      robotMode = 1;
    }
    
    if(robotMode != 0)
      break;
    
  }
  
}

//*************************************v Main Functions v*********************************
void setup() {
  
  //---------------------------------------------> lcd
  lcd.begin(16,2);
  //---------------------------------------------> dynamixel
  Dxl.begin(3); // 3 Is For 1 Mbps And 0 Or 1,2 For Other BoundRate...
  //---------------------------------------------> compass
  SerialUSB.begin();
  Wire.begin(11,10);
  if(ROBOT != "BLACK"){ 
  pinMode(10,INPUT_PULLUP);
  pinMode(11,INPUT_PULLUP);
  }
 //---------------------------------------------> sharps
  pinMode(SHARP_LEFT,INPUT_ANALOG);
  pinMode(SHARP_RIGHT,INPUT_ANALOG);
  pinMode(SHARP_BACK,INPUT_ANALOG); 
  //---------------------------------------------> buttons
  pinMode(BUTTON_DOWN,INPUT);
  pinMode(BUTTON_OK,INPUT);
  pinMode(BUTTON_UP,INPUT);
  pinMode(BUTTON_CANCEL,INPUT);
  //---------------------------------------------> sensors
  pinMode(A,OUTPUT);
  pinMode(B,OUTPUT);
  pinMode(C,OUTPUT);
  pinMode(D,OUTPUT);
  pinMode(SENSOR_OUTPUT_BALL,INPUT_ANALOG);
  pinMode(SENSOR_OUTPUT_KAF,INPUT_ANALOG);
  //---------------------------------------------> led
  pinMode(LED2,OUTPUT);
  pinMode(LED2,OUTPUT);
  
  //---------------------------------------------> buzzer
  pinMode(BUZZER,OUTPUT);
  //---------------------------------------------> First Confiurations  
  digitalWrite(LED2,HIGH);
  digitalWrite(LED3,HIGH);
  
  boot();  
  
  firstValueCmpr = compass();
}

void loop() {
  
  //-----------------------------> *** Debug Mode *** <-----------------------------
  if(robotMode == 1 ) deBug(); //Debug Mode
  //-----------------------------> *** Main Mode *** <-----------------------------
  else if(robotMode == 2) messi();  //forward
        
  else if(robotMode == 3) abedZadeh();  //goalKeeper

}

