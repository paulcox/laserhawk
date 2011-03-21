#ifndef CMTILOG_H
#define CMTILOG_H



#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>


class CMtiLog
{

private:


    FILE *Fscanmti;

public:
    CMtiLog(char * name);
    ~CMtiLog();
    void Get(double timedemande);



    double R11,R12,R13;
    double R21,R22,R23;
    double R31,R32,R33;
    double lat,lon,alt;
    double latini;
    double lonini;
    double altini;
    double quat[4];



};

#endif // CMTILOG_H
