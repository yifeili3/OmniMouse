// Testing 8 Dot Sensor Configuration
// Capacitive Pins: 0, 1, 7, 8, 14, 15, 16, 17,
// 18, 19, 22, 23

int sensorPins[] = {0,1,15,16,17,18,19,22};  //8 Sensors (No pads)
int defVal[] = {0,0,0,0,0,0,0,0};

typedef struct dir{
  float x;
  float y;  
}Direction;

void setup() {
  pinMode(13, OUTPUT);    //Set LED to output
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
  int sensors[16];
  int buttons[4];
  for(int i = 0; i < 16; i++){
    sensors[i] = touchRead(sensorPins[i%8]);
  }
  mouseAlgorithm(sensors,buttons);
}

void printValues(int* sensors, int num) {
  for(int i = 0; i < num; i++){
    Serial.print(sensors[i]);
    printCS();
  }
  printLN();
}

void mouseAlgorithm(int * sensors, int * buttons){
    //mouse move
    Direction mouseDir = MajorityVote(sensors);
    if(mouseDir.x!=0 || mouseDir.y!=0) Mouse.move(mouseDir.x,mouseDir.y);
    //TODO
}


Direction MajorityVote(int * sensors){
      int sensorCalculated[8];
      printValues(sensors,8);  
      for(int i=0;i<8;i++){
      sensorCalculated[i]=0;  
      }
      Direction dir;
      for(int i=0;i<16;i++){
        sensorCalculated[i%8]+=sensors[i];
      }
      for(int i=0;i<8;i++){
        sensorCalculated[i]/=2;
        if(sensorCalculated[i]-defVal[i] > 25.0) sensorCalculated[i]=1.0;
        else sensorCalculated[i]=0.0;
      }
      //printValues(sensorCalculated,8);  
      dir.y = (float)sensorCalculated[4] + (float)sensorCalculated[3]*0.5 + (float)sensorCalculated[5]*0.5 - (float)sensorCalculated[1]*0.5- (float)sensorCalculated[7]*0.5- sensorCalculated[0];
      dir.x = (float)sensorCalculated[6] + (float)sensorCalculated[5]*0.5 + (float)sensorCalculated[7]*0.5 - (float)sensorCalculated[3]*0.5- (float)sensorCalculated[1]*0.5- sensorCalculated[2];
      return dir;
}
 
void printCS(){   //Print comma + space
  Serial.print(", ");
}
void printLN(){   //Print newline
  Serial.print("\n");
}
