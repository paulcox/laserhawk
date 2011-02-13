#ifndef HOKUYOPROCESS_H
#define HOKUYOPROCESS_H



#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <string.h>

#include "chokuyoplus.h"

#include"pnm_io2.h"
#include <cmath>


////////////////////////////////////////////////////////////////////////
void PolartoCart(FLOATHOKUYO  a, FLOATHOKUYO r, FLOATHOKUYO *x, FLOATHOKUYO *y);
void CartToPolar(FLOATHOKUYO * a, FLOATHOKUYO *r, FLOATHOKUYO x, FLOATHOKUYO y);
void testIntersectionCercle();

bool IntersectionRayonCercle(FLOATHOKUYO a,FLOATHOKUYO b ,FLOATHOKUYO d ,FLOATHOKUYO xc,FLOATHOKUYO yc,FLOATHOKUYO rc,
                             FLOATHOKUYO *xp1,FLOATHOKUYO *yp1,FLOATHOKUYO *xp2,FLOATHOKUYO *yp2);
bool IntersectionRayonCercleOptimD0(FLOATHOKUYO a,FLOATHOKUYO b ,FLOATHOKUYO xc,FLOATHOKUYO yc,FLOATHOKUYO rc,
                                    FLOATHOKUYO *xp1,FLOATHOKUYO *yp1,FLOATHOKUYO *xp2,FLOATHOKUYO *yp2);
////////////////////////////////////////////////////////////////////////
//liste chainée

typedef struct ElementListPt2d {
    int number;
    struct ElementListPt2d *next;
}ElementListPt2d;

typedef struct ListPt2d {
    ElementListPt2d *begin;  //NULL
    ElementListPt2d *end; //for adding element at the end //NULL
    int size; //0
}ListPt2d;
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


class CHokuyoProcess
{
public:
    CHokuyoProcess(CHokuyoPlus * HokuyoSensorInit,int sizeimagexyInit,int scaleImageInit );
    ~CHokuyoProcess();
    void EraseBitmap();
    void SaveBitmap(char * fileName);
    void GenerateFrameBitmap();
    void GenerateDataBitmap();
    int  FindPole(int borneimin,int borneimax,int incrementi, int limite ,bool SaveFileResults,bool Display,bool Verbose);
   // int  FindCap(int borneimin,int borneimax,int incrementi,int limite ,bool SaveFileResults,bool Display,bool Verbose);


    FLOATHOKUYO  polex,poley; //to transfer detected pole position in sensor frame from findpole to findcap

    //init arrays
    FLOATHOKUYO tabainit[1080];
    FLOATHOKUYO tabrinit[1080];

    //current arrays
    FLOATHOKUYO taba[1080];
    FLOATHOKUYO tabr[1080];

    //association  table after full search

//old====
    //  int tabassociated[1080]; //for each dot in the init array , give the index of the associated index in the current arrays , -1 if no association found (d2> threshold)
                    //a given point of the current array can be associated with multiple points of the initial array


//changed definition
    int tabassociated[1080]; //for each dot in the current array , give the  associated index in the initial arrays , -1 if no association found (d2> threshold)
                      //a given point of the initial array can be associated with multiple points of the current array

    FLOATHOKUYO tabassociateddist[1080]; //corresponding square distances for the association

    int  FindCap(int bornetmin,int bornetmax,int incrementt,FLOATHOKUYO* scoret);

    CHokuyoPlus * HokuyoSensor;
    int DisplayLoca(float theta,int ipole,int depthpole,int borneimin, int borneimax );

    void GetCylinderAndSector( int *nbc, int *nbs,FLOATHOKUYO a, FLOATHOKUYO r);


    FLOATHOKUYO DistThreshold;
    ListPt2d * TabInit; //[nbsect*nbcyl]; //organized ....


    int nbsect;
    int nbcyl;


    void AddTrajPoint();
    void DrawTraj();
    im_color * im_ray;

    im_color * im_traj;

    int *x_traj;
    int *y_traj;
    int nb_traj_max;
    int nb_traj;

protected:
    int scaleImagetraj;
    int sizeimagetraj;
    bool firstframe;
    int x_previous,y_previous;
    int x_loca,y_loca;


    int sizeimagexy;

    rgb  col1; //red
    rgb  col2; //green
    rgb  col3; //blue
    rgb  col4; //black
    rgb col5; //yellow
    rgb col6; // white
    rgb col7; //grey

    int scaleImage;


};

#endif // HOKUYOPROCESS_H
