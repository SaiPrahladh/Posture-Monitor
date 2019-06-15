#include <Wire.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
//#include <TimeLib.h>

#include <WiFiClientSecure.h>



#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""

const int BUZZER_PIN = D8;

const char* ssid = "";
const char* password = "";

// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;

void setup() {
  pinMode(BUZZER_PIN,OUTPUT);
  Serial.begin(115200);
  Wire.begin(sda, scl);
  MPU6050_Init();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.println("");
  Serial.println("WiFi Connected!");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
 Serial.begin(115200);
}
 float average=0;
 float sum=0;
 float starttime;
 float endtime;
 float st;
 float et;

void loop() {
  DynamicJsonBuffer jsonBuffer;

  float Ax, Ay, Az, T, Gx, Gy, Gz;
  st = millis();
  long fwd = 0;
  long back = 0;
  long straight = 0;
  et = st;
  while((et-st) <= 10000){
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    
    //divide each with their sensitivity scale factor
    Ax = (float)AccelX/AccelScaleFactor;
    Ay = (float)AccelY/AccelScaleFactor;
    Az = (float)AccelZ/AccelScaleFactor;
    T = (float)Temperature/340+36.53; //temperature formula
    Gx = (float)GyroX/GyroScaleFactor;
    Gy = (float)GyroY/GyroScaleFactor;
    Gz = (float)GyroZ/GyroScaleFactor;


    starttime = millis();
    endtime = starttime;
    while ((endtime - starttime) <=1000) // do this loop for up to 1000mS
    {  
      Firebase.setFloat("accelz",Az);
      endtime = millis();
    }
    
    if(Az< -0.3) {++fwd; Serial.println(Az);}
    else if(Az > 0.3){ ++back; Serial.println(Az);}
    else {++straight;Serial.println(Az);}
    et = millis(); 
    
    
  }
  Serial.print("Backward: ");Serial.println(back);
  Serial.print("Forward: ");Serial.println(fwd);
  Serial.print("Straight: ");Serial.println(straight);

 
  if (straight<fwd or straight<back)
  {

   BUZZER_PIN == HIGH;
    
  }

//  String minutes = String(minute());
//  String hours = String(hour());
//  String days = String(day());
//  String months = String(month());
//  String years = String(year());
  
  String path = "data";
 // String input ="{\"straight\": "+st+",\"backward\": "+backward+",\"forward\": "+forward+"}";
 // JsonObject& root = jsonBuffer.parseObject(input);
  

 String path_for_forward = path + "/forward";
 String path_for_backward = path + "/backward";
 String path_for_straight = path + "/straight";

 Firebase.setFloat(path_for_forward, fwd);
 Firebase.setFloat(path_for_backward, back);
 Firebase.setFloat(path_for_straight, straight);
 if(straight<fwd + back){
     float ts = millis();
     String path_for_notification = "data/notification";
     Firebase.setFloat(path_for_notification, ts);
 }
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}
   
//configure MPU6050
void MPU6050_Init(){
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
