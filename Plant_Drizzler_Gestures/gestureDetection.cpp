#include "gestureDetection.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

/* Possible states:
  0 --> 'idle': Controller isn't being used, default mode
  1 --> 'detect': Detecting gesture
*/
int deviceState = 0;
unsigned long deviceTimestamp = 0;

/*
  Arduino and MPU6050 Accelerometer and Gyroscope Sensor Tutorial
  by Dejan, https:// howtomechatronics.com
*/
const int MPU = 0x68;         // MPU6050 I2C address
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float accErrorX, accErrorY, gyroErrorX, gyroErrorY, gyroErrorZ;
unsigned long currTime, prevTime, elapsedSeconds;
int c = 0;


/* ========================
  ===     FUNCTIONS     ===
  ====================== */

// TODO use actual error values

void getValuesloop() {
  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // read 6 registers total, each axis value is stored in 2 registers
  // for a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  accX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  accY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  accZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
  // calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(accY / sqrt(pow(accX, 2) + pow(accZ, 2))) * 180 / PI) - 0.58; // accErrorX ~(0.58) See the calculateDataEror() custom function for more details
  accAngleY = (atan(-1 * accX / sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / PI) + 1.58; // accErrorY ~(-1.58)

  // === Read gyroscope data === //
  prevTime = currTime;
  currTime = millis();
  elapsedSeconds = (currTime - prevTime) / 1000; // divide by 1000 to get seconds
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // read 4 registers total, each axis value is stored in 2 registers
  gyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // for a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  gyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  gyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  // correct the outputs with the calculated error values
  gyroX = gyroX + 0.56; // gyroErrorX ~(-0.56)
  gyroY = gyroY - 2; // gyroErrorY ~(2)
  gyroZ = gyroZ + 0.79; // gyroErrorZ ~ (-0.8)
  // currently the raw values are in degrees per seconds, deg/s, so we need to multiply by seconds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + gyroX * elapsedSeconds; // deg/s * s = deg
  gyroAngleY = gyroAngleY + gyroY * elapsedSeconds;
  yaw =  yaw + gyroZ * elapsedSeconds;

  // complementary filter - combine accelerometer and gyro angle values
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  // print the values on the serial monitor
  Serial.print(roll);
  Serial.print(F("/"));
  Serial.print(pitch);
  Serial.print(F("/"));
  Serial.println(yaw);
}

void calculateDataEror() {
  // we can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // read accelerometer values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    accX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    accY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    accZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // sum all readings
    accErrorX = accErrorX + ((atan((accY) / sqrt(pow((accX), 2) + pow((accZ), 2))) * 180 / PI));
    accErrorY = accErrorY + ((atan(-1 * (accX) / sqrt(pow((accY), 2) + pow((accZ), 2))) * 180 / PI));
    c++;
  }
  // divide the sum by 200 to get the error value
  accErrorX = accErrorX / 200;
  accErrorY = accErrorY / 200;
  c = 0;
  // read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    gyroX = Wire.read() << 8 | Wire.read();
    gyroY = Wire.read() << 8 | Wire.read();
    gyroZ = Wire.read() << 8 | Wire.read();
    // sum all readings
    gyroErrorX = gyroErrorX + (gyroX / 131.0);
    gyroErrorY = gyroErrorY + (gyroY / 131.0);
    gyroErrorZ = gyroErrorZ + (gyroZ / 131.0);
    c++;
  }
  // divide the sum by 200 to get the error value
  gyroErrorX = gyroErrorX / 200;
  gyroErrorY = gyroErrorY / 200;
  gyroErrorZ = gyroErrorZ / 200;
  // print the error values on the Serial Monitor
  Serial.print(F("accErrorX: "));
  Serial.println(accErrorX);
  Serial.print(F("accErrorY: "));
  Serial.println(accErrorY);
  Serial.print(F("gyroErrorX: "));
  Serial.println(gyroErrorX);
  Serial.print(F("gyroErrorY: "));
  Serial.println(gyroErrorY);
  Serial.print(F("gyroErrorZ: "));
  Serial.println(gyroErrorZ);
}

// setup for when we enter a specific state
void changeDeviceState(int newState) {
  if (deviceState != newState) {
    deviceTimestamp = millis();
    deviceState = newState;

    Serial.print(F("Switched to deviceState "));
    Serial.println(newState);

    // state initialisation logic
    switch (newState) {
      case 0:
        break;
      case 1:
        break;
    }
  }
}


/* =========================
  ===   SETUP FUNCTION   ===
  ======================= */

void gestureDetectionSetup() {
  Wire.begin();                      // initialize comunication
  Wire.beginTransmission(MPU);       // start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // talk to the register 6B
  Wire.write(0x00);                  // make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        // end the transmission

  /*
    /// configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
    Wire.beginTransmission(MPU);
    Wire.write(0x1C);                  // talk to the ACCEL_CONFIG register (1C hex)
    Wire.write(0x10);                  // set the register bits as 00010000 (+/- 8g full scale range)
    Wire.endTransmission(true);

    /// configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
    Wire.beginTransmission(MPU);
    Wire.write(0x1B);                   // talk to the GYRO_CONFIG register (1B hex)
    Wire.write(0x10);                   // set the register bits as 00010000 (1000deg/s full scale)
    Wire.endTransmission(true);
    delay(20);
  */

  // call this function if you need to get the IMU error values for your module
  calculateDataEror();

  // small delay before we finish to be extra sure hardware is fully ready
  // TODO check if we need this
  delay(500);
}


/* ========================
  ===      GETTERS      ===
  ====================== */

// TODO implement public methods


/* =========================
  ===        LOOP        ===
  ======================= */

void deviceLoop() {
  // update accelerometer and gyroscope output data
  getValuesloop();

  // TODO implement state logic

  // switch (deviceState) {
  //   case 0:
  //     break;
  //   case 1:
  //     break;
  // }
}
