#include <Bounce.h>
#define THRESHOLD 20
#define UNIT 7.5
#define RADIAN 57.2
// Testing 8 Dot Sensor Configuration
// Capacitive Pins: 0, 1, 18, 15, 16, 17, 19, 23
// 18, 19, 22, 23

int sensorPins[] = {0,1,18,15,16,17,19,23};  //8 Sensors: The order is scattered because of the board configuration
int defVal[] = {0,0,0,0,0,0,0,0};

int buttonPin=2;
int debounceTime=10;
Bounce pushbutton=Bounce(buttonPin,debounceTime);

typedef struct dir{
  float x;
  float y;  
}Direction;

typedef struct sensor{
  int sensorNum;
  bool binaryReading;
  int CapacitiveReading;
}Sensor;


typedef struct touchQueue{
  Sensor s1;
  Sensor s2;
  Sensor s3;
}TouchQueue;

void setup() {
  pinMode(13, OUTPUT);    //Set LED to output
  pinMode(buttonPin,INPUT_PULLUP);
  Serial.begin(9600);
  calibrate();
  Mouse.begin();
}

void calibrate() {  // Calibrate the ambient capacitance values for the sensor.
  digitalWrite(13, LOW);  //Set LED low when the device is calibrating. 
  for(int i = 0; i < 100; i++) {    // Get values of ambient readings for the sensors
    for(int j = 0; j < 8; j++) {
      defVal[j] += touchRead(sensorPins[j]);
    }
  }
  for(int i = 0; i < 8; i++) {  // Print out the values
    defVal[i] /= 100;
    Serial.println(defVal[i]);
  }
  digitalWrite(13, HIGH); //Set LED on when the device is on and ready.
}

void loop() {
  // Get sensor readings
  Sensor sensors[8];
  bool buttons[4];
  for(int i = 0; i < 8; i++){
    sensors[i].sensorNum = i;
    int val=touchRead(sensorPins[i]);
    if(abs(val-defVal[i]) >= THRESHOLD)
      sensors[i].binaryReading =true;
     else
      sensors[i].binaryReading =false;
    sensors[i].CapacitiveReading = (val-defVal[i]); 
    // space for button reading
  }
  bool buttonPressed=false;
  if(pushbutton.update()){
   if(pushbutton.risingEdge()){
      buttonPressed=true;
      Serial.print("------------");
    } 
   }
  
  //printValues(sensors,8);
  mouseAlgorithm(sensors,buttons);
  delay(20);
}
/*
void printValues(Sensor * sensors, int num) {
  for(int i = 0; i < num; i++){
    Serial.print(sensors[i].CapacitiveReading);
    printCS();
  }
  printLN();
}
*/
void mouseAlgorithm(Sensor * sensors,bool * buttons){
    //mouse move
    bool activated=false;
    float angle = AngleCalculation(sensors,activated);
    Serial.print(angle);printCS();
    Direction mouseDir=getDirection(angle,activated);
    if(mouseDir.x!=0 || mouseDir.y!=0) Mouse.move(mouseDir.x,mouseDir.y);
    //TODO button control
   
    
}

Direction getDirection(float angle,bool activated){
  Direction dir;
  dir.x=0.0;
  dir.y=0.0;
  if(activated){
  if(angle<=180.0){
    angle/=RADIAN;
    dir.y=-UNIT*cos(angle);
    dir.x=-UNIT*sin(angle);
  }
  else{
    angle-=180.0;
    angle/=RADIAN;
    dir.y=UNIT*cos(angle);
    dir.x=UNIT*sin(angle);
    }
  }
  //Serial.print(dir.x);printCS();
  //Serial.print(dir.y);printCS();
  return dir;
}

void setVal(Sensor & s, int Num,int reading){
  
  s.sensorNum = Num;
  s.binaryReading= reading >= THRESHOLD;
  s.CapacitiveReading=reading;
}


TouchQueue findLargestThree(Sensor * sensors){
    TouchQueue tq;
    Sensor s1,s2,s3;
    s1.sensorNum=0;
    s2.sensorNum=0;
    s3.sensorNum=0;
    s1.binaryReading=false;
    s2.binaryReading=false;
    s3.binaryReading=false;
    s1.CapacitiveReading=-50;
    s2.CapacitiveReading=-50;
    s3.CapacitiveReading=-50;
    
    for(int i=0;i<8;i++){
      int reading = sensors[i].CapacitiveReading;
      if(reading>=s1.CapacitiveReading){
          setVal(s3,s2.sensorNum,s2.CapacitiveReading);
          setVal(s2,s1.sensorNum,s1.CapacitiveReading);
          setVal(s1,i,reading);
      }
      else if(reading> s2.CapacitiveReading && reading< s1.CapacitiveReading){
          setVal(s3,s2.sensorNum,s2.CapacitiveReading);
          setVal(s2,i,reading);
      }
      else if (reading>= s3.CapacitiveReading && reading< s2.CapacitiveReading){
          setVal(s3,i,reading);
      }
    }
    tq.s1=s1;
    tq.s2=s2;
    tq.s3=s3;
    return tq;
}


float Vote(sensor s1, sensor s2, float * degreeArray){
      float deg = abs(degreeArray[s2.sensorNum] - degreeArray[s1.sensorNum]);
      float angle=0.0;
      if(deg <= 90.0){
        angle = (degreeArray[s1.sensorNum] + degreeArray[s2.sensorNum])/2;
      }
      else if (90.0 < deg && deg < 270.0){
        angle = degreeArray[s1.CapacitiveReading > s2.CapacitiveReading ? s1.sensorNum:s2.sensorNum];
      }
      else{ 
          float base= s1.sensorNum < s2.sensorNum ? degreeArray[s1.sensorNum] : degreeArray[s2.sensorNum];
          angle= base - (360.0-degreeArray[s1.sensorNum]-degreeArray[s2.sensorNum])/2.0;
        }
        if(angle<0.0) angle+=360.0;
      return angle;
}

float AngleCalculation(Sensor * sensors, bool & activated){
      printValues(sensors,8);
      TouchQueue res=findLargestThree(sensors);
      Serial.print(res.s1.sensorNum);printCS();
      Serial.print(res.s2.sensorNum);printCS();
      Serial.print(res.s3.sensorNum);printCS();
      float angle=0.0;
      
      float degreeArray[8]={0.0,45.0,90.0,135.0,180.0,225.0,270.0,315.0};
      if(res.s1.binaryReading && res.s2.binaryReading && res.s3.binaryReading){
        //float angle1 = Vote(res.s1, res.s2, degreeArray);
        //float angle2 = Vote(res.s2, res.s3, degreeArray);
        //float angle3 = Vote(res.s1, res.s3, degreeArray);
        //angle=(angle1+angle2+angle3)/3;
      }
      if(res.s1.binaryReading && res.s2.binaryReading){
        angle = Vote(res.s1,res.s2,degreeArray);
        activated=true;
      }
      else if(res.s1.binaryReading){
        angle = degreeArray[res.s1.sensorNum];
        activated=true;
      }
      return angle;
}

void printValues(Sensor* sensors, int num) {
  for(int i = 0; i < num; i++){
    Serial.print(sensors[i].binaryReading);
    printCS();
  }
  printLN();
}

void printCS(){   //Print comma + space
  Serial.print(", ");
}
void printLN(){   //Print newline
  Serial.print("\n");
}




