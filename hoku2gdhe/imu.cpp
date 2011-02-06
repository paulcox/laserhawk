#include "imu.h"

IMU::IMU(QObject *parent)
    : QThread(parent)
{
    // create instance
    stopped = false;
}

void IMU::run()
{
    //char *prog = "/dev/ttyUSB0";    // IMU Device
    INERTIAL_DATA data;

    //create MTI object
    MTI mti("/dev/ttyUSB0",MTI_OPMODE_ORIENTATION , MTI_OPFORMAT_EULER  );

    while(!stopped)
    {
        bool success = mti.read(&data, false);

        if (success)
        {
            // get Euler Angle
            euler_x = data.EULER[0];
            euler_y = data.EULER[1];
            euler_z = data.EULER[2];
        }
    }
    euler_x=0;
    euler_y=0;
    euler_z=0;
}

void IMU::stop() {
    stopped = true;
}

//void IMU::getValues(double& Rx, double& Ry, double& Rz)
void IMU::getValues(double * Rx, double * Ry, double * Rz)//, double& Ry, double& Rz)
{
    *Rx = euler_x;
    *Ry = euler_y;
    *Rz = euler_z;

    //printf("resultat IMUthread => Rx=%f Ry=%f Rz=%f \n", *Rx, *Ry, *Rz);
}
