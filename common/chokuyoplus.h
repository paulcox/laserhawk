#ifndef HOKUYOPLUS_H
#define HOKUYOPLUS_H


#include <flexiport/flexiport.h>
#include <hokuyo_aist/hokuyo_aist.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <string>


#define FLOATHOKUYO float
//attention si on passe en double, il faudra modifier les printf %f en %lf

//voir http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
//pour faire une macro



//in order to be able to run the application and debug from QT:
//Projects->Run settings, add variable corresponding to
//export LD_LIBRARY_PATH=/home/guillaume/openrobots/lib/gearbox:/home/guillaume/openrobots/lib

//for mac os
//export DYLD_LIBRARY_PATH=/Users/bvandepo/openrobots/lib/gearbox:/Users/bvandepo/openrobots/lib


using namespace std;

typedef struct {
    uint32_t* depth;
    FLOATHOKUYO * x_data;
    FLOATHOKUYO * y_data;
    FLOATHOKUYO * angle;
} data_sensor;



class CHokuyoPlus
{
public:
    CHokuyoPlus(char * deviceNameInit);
    ~CHokuyoPlus();
    void open_port();
    void close_port();
    void getRange();
    void setReplayFileName(string fileNameInit);
    void setRealSensor();
    void setVirtualSensor();
    void startSaveReplay();
    void stopSaveReplay();
    void seekReplay(int new_nb_scan);

    // Hokuyo variables
    hokuyo_aist::HokuyoLaser laser; // Laser scanner object

    double hokuyotime;  //time read & written from/to the log file
    char  deviceName[1000];
    data_sensor data;
    int nb_point;
    int limite;
    FLOATHOKUYO rayon_cercle;
    FLOATHOKUYO startAngle, endAngle;
    FLOATHOKUYO  *Rx_point , *Ry_point , *Rz_point;
    FLOATHOKUYO  Rx_xsens , Ry_xsens , Rz_xsens;
    FLOATHOKUYO x_test, y_test;

    int firstStep, lastStep;
    int nb_scan; //counter of scans
    int nb_sphere;
    //, nb_testscan;  //removed
    unsigned int baud, speed, clusterCount;
    bool getIntensities, getNew, verbose;

    // Hokuyo flag
    bool port_open;

protected:
    string fileName;
    bool saveReplay;

    int VirtualHokuyo;//0 for real sensor, 1 for VirtualSensor grabbing data from files

};

#endif // HOKUYOPLUS_H
