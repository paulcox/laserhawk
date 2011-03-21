#ifndef CINTERFACEWITHGDHE_H
#define CINTERFACEWITHGDHE_H


#include "gdhe/GDHE.h"


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <QtGui>

#include "pnm_io2.h"
#include "chokuyoplus.h"
#include "cmtilog.h"



#define SCALE_FACTOR_DISPLAYGDHE 1000.  // data en mm, depth in meters

#define FAC 0.5f
#define MAXDIF (1+FAC)
#define MINDIF (1-FAC)




#define mod_cont 0
#define mod_rest 1
#define mod_end  2


class CInterfaceWithGDHE
{
public:
    CInterfaceWithGDHE(char * IPGdhe);
    ~CInterfaceWithGDHE();
    void DisplayTrajectory( double x,double y,double z);

    void DisplayCoordinateFrame(double alpha,double beta,double gamma,double x,double y,double z);
    void DisplayScan(data_sensor *datasensor,CMtiLog* MtiLog);
    void SetColorScan(unsigned char r,unsigned char g,unsigned char b);
    void Clear();
    void pline_from_pos(int mode, float x_pos, float y_pos, float z_pos);

private:
    rgb colorScan;
    int num_scans_display  ; //to not unset trajectory for first scan

    QString tpline_str_pts; //string for the incremental trajectory
    //for pline_from_pos
    bool line_started  ;
    int num_pts ;
    int num_pline  ;
    QString pline_str_pts;//string for the incremental polyline of scans
    QString pline_str;
};

#endif // CINTERFACEWITHGDHE_H
