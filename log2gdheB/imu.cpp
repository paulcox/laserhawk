#include "imu.h"

IMU::IMU(QObject *parent)
    : QThread(parent)
{
    // create instance
    stopped = false;
}

void IMU::run()
{

//TODO: get values from file

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
