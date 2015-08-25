#include "../debug.h"
#include "GY87.h"

void GY87::initialize()
{
    // MPU6050
    mpu.initialize();
    if (!mpu.testConnection()) {
        // Test failed
        err("MPU6050 test connection failed!");
    }
    uint8_t mpuStatus = mpu.dmpInitialize();
    if (mpuStatus != 0) {
        err("MPU6050 DMP setup failed!");
    }

    setOffset();

    // Setting HMC5883L
    mpu.setI2CMasterModeEnabled(0);
    mpu.setI2CBypassEnabled(1);
    // Now we can see HMC5883L
    HMC5883L hmc(HMC5883L_DEFAULT_ADDRESS);
    hmc.initialize();
    if (!hmc.testConnection()) {
        // Test failed
        err("HMC5883L test connection failed!");
    }
    hmc.setDataRate(HMC5883L_RATE_75);
    hmc.setMode(HMC5883L_MODE_CONTINUOUS);
    mpu.setI2CBypassEnabled(0);

    mpu.setSlaveAddress(0, HMC5883L_DEFAULT_ADDRESS | 0x80); // 0x80 turns 7th bit ON, according to datasheet, 7th bit controls Read/Write direction
    mpu.setSlaveRegister(0, HMC5883L_RA_DATAX_H);
    mpu.setSlaveEnabled(0, true);
    mpu.setSlaveWordByteSwap(0, false);
    mpu.setSlaveWriteMode(0, false);
    mpu.setSlaveWordGroupOffset(0, false);
    mpu.setSlaveDataLength(0, 2);

    // Y axis word
    mpu.setSlaveAddress(1, HMC5883L_DEFAULT_ADDRESS | 0x80);
    mpu.setSlaveRegister(1, HMC5883L_RA_DATAY_H);
    mpu.setSlaveEnabled(1, true);
    mpu.setSlaveWordByteSwap(1, false);
    mpu.setSlaveWriteMode(1, false);
    mpu.setSlaveWordGroupOffset(1, false);
    mpu.setSlaveDataLength(1, 2);

    // Z axis word
    mpu.setSlaveAddress(2, HMC5883L_DEFAULT_ADDRESS | 0x80);
    mpu.setSlaveRegister(2, HMC5883L_RA_DATAZ_H);
    mpu.setSlaveEnabled(2, true);
    mpu.setSlaveWordByteSwap(2, false);
    mpu.setSlaveWriteMode(2, false);
    mpu.setSlaveWordGroupOffset(2, false);
    mpu.setSlaveDataLength(2, 2);

    mpu.setI2CMasterModeEnabled(1);
    //Setting HMC5883L Done
    
    mpu.setDMPEnabled(true);
    packetSize = mpu.dmpGetFIFOPacketSize();


    //BMP085 TODO

}

void GY87::setOffset()
{
    // MPU6050
    mpu.setXAccelOffset(-3591);
    mpu.setYAccelOffset(-842);
    mpu.setZAccelOffset(578);

    mpu.setXGyroOffset(-51);
    mpu.setYGyroOffset(-57);
    mpu.setZGyroOffset(0);
}

bool GY87::testConnection()
{
    return mpu.testConnection() && bmp.testConnection();
}  

void GY87::update()
{
    fifoCount = mpu.getFIFOCount();
    if (fifoCount == 1024) {
        // Overflow, reset so we can continue cleanly
        mpu.resetFIFO();
        warn("FIFO Overflow!");
    } else {
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
        mpu.getFIFOBytes(fifoBuffer, packetSize);

        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGyro(gyro, fifoBuffer);
        //    mpu.dmpGetAccel(&aa, fifoBuffer);  //Use this if you want accelerometer measures
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        //    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);  //Use this to get linear acceleration apart from gravity.
        //    mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);  //NOT RECOMMENDED. Gives you linear acceleration rotated to initial position.

        //Read magnetometer measures
        mx=mpu.getExternalSensorWord(0);
        my=mpu.getExternalSensorWord(2);
        mz=mpu.getExternalSensorWord(4);

        float xh = mx*cos(ypr[1])+my*sin(ypr[1])*sin(ypr[2])+mz*sin(ypr[1])*cos(ypr[2]);
        float yh = my*cos(ypr[2])+mz*sin(ypr[2]);
        heading = atan2(-yh, xh);
        if(heading < 0) heading += 2 * M_PI;
    }
}





