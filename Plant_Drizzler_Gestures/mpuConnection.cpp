#include "mpuConnection.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

/*
  Arduino and MPU6050 Accelerometer and Gyroscope Sensor Tutorial
  by Dejan, https:// howtomechatronics.com
*/
const int MPU = 0x68;  // MPU6050 I2C address
float accX, accY, accZ; // acceleration
float correctedAccX, correctedAccY, correctedAccZ; // acceleration corrected towards zero (when motionless)
// float speedX, speedY, speedZ; // velocity
float gyroX, gyroY, gyroZ; // degrees/s
float accAngleX, accAngleY; // degrees, from accelerometer data
float roll, pitch, yaw; // degrees, mostly from gyroscope data
// for calculating error values (on startup) in a motionless state, which can then be used to set the correction values
// !important: values aren't updated automatically, but have to be tested/set by the developer for each device
float accErrorX, accErrorY, accErrorZ, accRangeErrorX, accRangeErrorY;
float gyroErrorX, gyroErrorY, gyroErrorZ;
// these are the recorded error margins, so they are SUBTRACTED from the new values to correct them
float accCorrectionX = 426.75, accCorrectionY = -90.31, accCorrectionZ = -8720.30, accAngleCorrectionX = -0.31, accAngleCorrectionY = -2.81;
float gyroCorrectionX = 5.77, gyroCorrectionY = -0.12, gyroCorrectionZ = -0.67;
unsigned long currTime, prevTime;
float elapsedMillis;
#define ELAPSED_SECONDS elapsedMillis/1000
int c = 0;


/* ========================
  ===     FUNCTIONS     ===
  ====================== */

void getMpuValues() {
  // === Read accelerometer data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);  // read 6 registers total, each axis value is stored in 2 registers
  // for a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  accX = (Wire.read() << 8 | Wire.read()); // X-axis value
  accY = (Wire.read() << 8 | Wire.read()); // Y-axis value
  accZ = (Wire.read() << 8 | Wire.read()); // Z-axis value

  // for ease of use in gesture detection we correct acceleration towards zero
  correctedAccX = accX - accCorrectionX;
  correctedAccY = accY - accCorrectionY;
  correctedAccZ = accZ - accCorrectionZ;
  // Note: This worked, but the inacuraccies in acceleration were too big for it to be useful/reliable, even when forcing it to trend towards 0
  // multiply acceleration (m/s^2) with seconds to get velocity (m/s)
  // speedX = speedX + correctedAccX * ELAPSED_SECONDS;
  // speedY = speedY + correctedAccY * ELAPSED_SECONDS;
  // speedZ = speedZ + correctedAccZ * ELAPSED_SECONDS;

  // we don't want them corrected toward zero for angle calculation, but they do have to be divided by 8192 because math
  accX /= 8192.0;
  accY /= 8192.0;
  accZ /= 8192.0;
  // calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(accY / sqrt(pow(accX, 2) + pow(accZ, 2))) * 180 / PI) - accAngleCorrectionX;
  accAngleY = (atan(-1 * accX / sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / PI) - accAngleCorrectionY;

  // === Read gyroscope data === //
  prevTime = currTime;
  currTime = millis();
  elapsedMillis = currTime - prevTime;
  Wire.beginTransmission(MPU);
  Wire.write(0x43);  // gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);                    // read 4 registers total, each axis value is stored in 2 registers
  gyroX = (Wire.read() << 8 | Wire.read()) / 131.0;  // for a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  gyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  gyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  // correct the outputs with the calculated error values
  gyroX = gyroX - gyroCorrectionX;
  gyroY = gyroY - gyroCorrectionY;
  gyroZ = gyroZ - gyroCorrectionZ;

  // currently the raw values are in degrees per seconds, deg/s, so we need to multiply by seconds (s) to get the angle in degrees
  roll = roll + gyroX * ELAPSED_SECONDS;
  pitch = pitch + gyroY * ELAPSED_SECONDS;
  yaw = yaw + gyroZ * ELAPSED_SECONDS;
  // complementary filter - combine accelerometer and gyro angle values to compensate gyroAngle drift
  roll = 0.96 * roll + 0.04 * accAngleX;
  pitch = 0.96 * pitch + 0.04 * accAngleY;
  // yaw drifts so we make it slightly drift towards 0 as well
  yaw *= 0.99;

  storeMpuValues(roll, pitch, yaw, correctedAccX, correctedAccY, correctedAccZ);

  /// Print the values on the serial monitor/plotter
  // Serial.print("accX:");
  // Serial.print(accX);
  // Serial.print(",");
  // Serial.print("accY:");
  // Serial.print(accY);
  // Serial.print(",");
  // Serial.print("accZ:");
  // Serial.println(accZ);
  
  Serial.print("correctedAccX:");
  Serial.print(correctedAccX);
  Serial.print(",");
  Serial.print("correctedAccY:");
  Serial.print(correctedAccY);
  Serial.print(",");
  Serial.print("correctedAccZ:");
  Serial.println(correctedAccZ);

  // Serial.print("accAngleX:");
  // Serial.print(accAngleX);
  // Serial.print(",");
  // Serial.print("accAngleY:");
  // Serial.println(accAngleY);

  // Serial.print("gyroX:");
  // Serial.print(gyroX);
  // Serial.print(",");
  // Serial.print("gyroY:");
  // Serial.print(gyroY);
  // Serial.print(",");
  // Serial.print("gyroZ:");
  // Serial.println(gyroZ);
  
  // Serial.print("roll:");
  // Serial.print(roll);
  // Serial.print(",");
  // Serial.print("pitch:");
  // Serial.print(pitch);
  // Serial.print(",");
  // Serial.print("yaw:");
  // Serial.println(yaw);
  
  // Serial.print("speedX:");
  // Serial.print(speedX);
  // Serial.print(",");
  // Serial.print("speedY:");
  // Serial.print(speedY);
  // Serial.print(",");
  // Serial.print("speedZ:");
  // Serial.println(speedZ);
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
    /// Get acceleration values error rates
    accX = (Wire.read() << 8 | Wire.read());
    accY = (Wire.read() << 8 | Wire.read());
    accZ = (Wire.read() << 8 | Wire.read());
    // sum all readings for acc error margins
    accErrorX += accX;
    accErrorY += accY;
    accErrorZ += accZ;
    // divide by 8192 to make values suitable for calculating roll/pitch from acc values (accCorrection should NOT be used here)
    accX /= 8192.0;
    accY /= 8192.0;
    accZ /= 8192.0;
    // sum all readings for accRange error rates as well
    accRangeErrorX = accRangeErrorX + ((atan((accY) / sqrt(pow((accX), 2) + pow((accZ), 2))) * 180 / PI));
    accRangeErrorY = accRangeErrorY + ((atan(-1 * (accX) / sqrt(pow((accY), 2) + pow((accZ), 2))) * 180 / PI));
    c++;
  }
  // divide the sum by 200 to get the error value
  accErrorX = accErrorX / 200;
  accErrorY = accErrorY / 200;
  accErrorZ = accErrorZ / 200;
  accRangeErrorX = accRangeErrorX / 200;
  accRangeErrorY = accRangeErrorY / 200;

  c = 0;
  // read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    /// Get gyroscope values error rates
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

  // print all error values on the Serial Monitor
  Serial.print(F("accErrorX: "));
  Serial.println(accErrorX);
  Serial.print(F("accErrorY: "));
  Serial.println(accErrorY);
  Serial.print(F("accErrorZ: "));
  Serial.println(accErrorZ);
  Serial.print(F("accRangeErrorX: "));
  Serial.println(accRangeErrorX);
  Serial.print(F("accRangeErrorY: "));
  Serial.println(accRangeErrorY);
  Serial.print(F("gyroErrorX: "));
  Serial.println(gyroErrorX);
  Serial.print(F("gyroErrorY: "));
  Serial.println(gyroErrorY);
  Serial.print(F("gyroErrorZ: "));
  Serial.println(gyroErrorZ);
}


/* =========================
  ===   SETUP FUNCTION   ===
  ======================= */

void mpuConnectionSetup() {
  Wire.begin();                 // initialize comunication
  Wire.beginTransmission(MPU);  // start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);             // talk to the register 6B
  Wire.write(0x00);             // make reset - place a 0 into the 6B register
  Wire.endTransmission(true);   // end the transmission

  /// configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);  // talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x08);  // set the register bits as 00001000 (+/- 4g full scale range)
  Wire.endTransmission(true);

  /*
    /// configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
    Wire.beginTransmission(MPU);
    Wire.write(0x1B);                   // talk to the GYRO_CONFIG register (1B hex)
    Wire.write(0x10);                   // set the register bits as 00010000 (1000deg/s full scale)
    Wire.endTransmission(true);
    delay(20);
  */

  // call this function if you need to get the IMU error values for your module
  calculateDataEror();
}
