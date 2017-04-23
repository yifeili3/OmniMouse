#include <Bounce.h>
#include <math.h>

#define THRESHOLD 20
#define UNIT 7.5
#define RADIAN 57.2
// 8 Dot Sensor Configuration
// Capacitive Pins: 0, 1, 18, 15, 16, 17, 19, 23

int sensorPins[] = {0,1,18,15,16,17,19,23};  //8 Sensors: The order is scattered because of the board configuration
int defVal[] = {0,0,0,0,0,0,0,0};

int buttonPin=2;
int debounceTime=5;
Bounce pushbutton=Bounce(buttonPin,debounceTime);

bool btConn = false;
char butIn = '0';
/*unsigned long startTime = 0;
int packets = 0;*/

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
  Serial.begin(38400);
  Serial2.begin(9600);
  calibrate();
  Mouse.begin();
  //startTime = millis();
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
  if(Serial2.available()){
    Serial.print("----------BLUETOOTH ON-------");
    Serial2.read();
    btConn = true;
  }
//  if(btConn)
  //  Serial2.print(". ");
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
  }
  int buttonType = 0;
  if(pushbutton.update()){
    if(pushbutton.risingEdge()){
      if(sensors[2].binaryReading == true){
        if(sensors[6].binaryReading == true){
          Mouse.click(MOUSE_MIDDLE);
          Serial.print("M");printCS(0);
          if(btConn)
            butIn = 'M';
        }
        else {
        Mouse.click(MOUSE_LEFT);
        Serial.print("L");printCS(0);
        if(btConn)
          butIn = 'L';
        }
      }
      else if(sensors[6].binaryReading == true){
        Mouse.click(MOUSE_RIGHT);
        Serial.print("R");printCS(0);
        if(btConn)
          butIn = 'R';
      }
      else if(sensors[0].binaryReading == true){
        Mouse.move(0, 0, 3);
        buttonType = 1;
        Serial.print("U");printCS(0);
        if(btConn)
          butIn = 'U';
      }
      else if(sensors[4].binaryReading == true){
        Mouse.move(0, 0, -3);
        buttonType = 2;
        Serial.print("D");printCS(0);
        if(btConn)
          butIn = 'D';
      }
      else {
        Mouse.click(MOUSE_LEFT);
        Serial.print("L");printCS(0);
        if(btConn)
          butIn = 'L';
      }
    }
    else {
      buttonType = 0;
    }
  }
  else{
    if(buttonType == 1){
      Mouse.move(0, 0, 3);
      Serial.print("U");printCS(0);
      if(btConn)
        butIn = 'U';
    }
    else if(buttonType == 2){
      Mouse.move(0, 0, -3); 
      Serial.print("D");printCS(0);
      if(btConn)
        butIn = 'D';
    }
    else {
      Serial.print("0");printCS(0);
      if(btConn)
        butIn = '0';
    }
  }
  mouseAlgorithm(sensors,buttons);

  //Checking packets/second
  /*
  if((millis() - startTime) < 10000){
    packets++;
  }
  if((startTime + millis()) > 10000){
    Serial.println("");
    Serial.print("Total Packets: ");
    Serial.println(packets);
    Serial.print("Packets/Second: ");
    Serial.println((float)packets/10);
  }
  */
  delay(15);
}

void mouseAlgorithm(Sensor * sensors,bool * buttons){
    bool activated=false;
    float angle = AngleCalculation(sensors,activated);
    Serial.print(angle);printCS(0);
    Direction mouseDir=getDirection(angle,activated);
    if(mouseDir.x!=0 || mouseDir.y!=0) Mouse.move(mouseDir.x,mouseDir.y);
}

Direction getDirection(float angle,bool activated){
  float c = angle;
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
  if(btConn && ((dir.y != 0)||(dir.x != 0)||(butIn != '0'))){
    int denom = c/22.5;
    Serial.print("--");Serial.print(denom);Serial.print("---");
    Serial2.print(butIn);printS(2);
    Serial2.print(denom);
    //Serial2.print(dir.x);printS(2);
    //Serial2.print(dir.y);
    Serial2.print("\n");
    butIn = '0';
  }
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
      Serial.print(res.s1.sensorNum);printCS(0);
      Serial.print(res.s2.sensorNum);printCS(0);
      Serial.print(res.s3.sensorNum);printCS(0);

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
//Print formatting functions

void printValues(Sensor* sensors, int num) {
  for(int i = 0; i < num; i++){
    Serial.print(sensors[i].binaryReading);
    printCS(0);
  }
  printLN(0);
}

void printCS(int arg){   //Print comma + space
  if(arg==0)
  Serial.print(", ");
  else if(arg==2)
  Serial2.print(", ");
}
void printLN(int arg){   //Print newline
 if(arg==0)
   Serial.print("\n");
 else if(arg==2)
   Serial2.print("\n");
}
void printS(int arg){
  if(arg==0)
    Serial.print(" ");
  else if(arg == 2)
    Serial2.print(" ");
}

