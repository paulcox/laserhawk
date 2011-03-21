#include "cinterfacewithgdhe.h"





//TODO use a non QT type
QString truc = "proc truc {} { \nobject truc { \npushMatrix \ncolor 200 200 100 \nbox 0 0 -0.5 1 1 1 \ncolor 200 0 0 \n\
                                               cylinder 0 0 0 x 1 0 2 24 \ncolor 0 200 0 \ncylinder 0 0 0 y 1 0 2 24 \ncolor 0 0 200 \ncylinder 0 0 0 z 1 0 2 24 \n\
                                               popMatrix \n} \n} ";




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CInterfaceWithGDHE::CInterfaceWithGDHE(char * IPGdhe)
{

    //for pline_from_pos
    line_started = false;
    num_pts = 0;
    num_pline = 0;
    pline_str_pts="";
    pline_str="";


    num_scans_display = 0; //to not unset trajectory for first scan
    tpline_str_pts="";


    // Open GDHE
    //int res = system("gdhe &");
    //if (res == 0) {printf("gdhe started ok\n");}
    // Sleep to allow full opening of GDHE, then connect
    //sleep(3);

    int error = get_connection(IPGdhe);
    //int error = get_connection((char *)"140.93.7.255");
    //int error = get_connection((char *)"140.93.4.43");

    printf("get_con ret: %d\n",error);

    // Coordinate Frame at origin
    //eval_expression((char *)"set robots(coordFrame) { color 0 255 0; repere }");
    //eval_expression((char *)"set pos(coordFrame) { 0 0 0 0 0 0  }");

    // Draw IMU Coordinate System
    //eval_expression((char *)"set robots(IMU) { color 0 0 255; repere }");
    // eval_expression("set pos(IMU) { -90 0 -90 0 0 8 }");

    // Draw Camera
    //eval_expression("set robots(Drone) { camera 7.5 47.5 39.6 }");
    //eval_expression("set pos(Drone) { -90 0 -90 0 0 8 }");
    //eval_expression("set robots(esfera) { color 0 0 255 ; sphere 1 1 3 .05 }");
    //eval_expression("set pos(esfera) { 0 0 0 0 0 0 }");

    // Grid
    eval_expression((char *)"set robots(grid) { color 75 75 0; grille -50 -50 50 50 1 }");
    eval_expression((char *)"set pos(grid) { 0 0 0 0 0 0 }");

    eval_expression(truc.toLatin1().data());
    printf("%s\n",truc.toLatin1().data());

    //eval_expression((char *)"set robots(truc) { truc }");
    //eval_expression((char *)"set pos(truc) { 0 0 0 }");

    //eval_expression((char *)"set robots(r1) { xr4000 }");
    //eval_expression((char *)"set pos(r1) { 0 0 0 0 0 0 }");

    //GDHE_client_prot::disconnect();??
    /*
    while(1)
    {
        bool success = mti.read(&data, true);

        // If GDHE is open, show in real time the IMU orientation
            // Form instruction to change IMU orientation on-screen
            QString setIMU = "set pos(IMU) { ";
            QTextStream(&setIMU) << data.EULER[2] << " " << data.EULER[1] << " " << data.EULER[0] << " 0 0 8 }";
            // Evaluate expression
            QByteArray text = setIMU.toLatin1();
            char *text2 = text.data();
            eval_expression(text2);
        }*/
    // (gdhe == true)

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CInterfaceWithGDHE::~CInterfaceWithGDHE()
{
    /*
    QString disconnect = "disconnect()";
    QByteArray textdisconnect = disconnect.toLatin1();
    char *text2disconnect = textdisconnect.data();
    eval_expression(text2disconnect);*/
    QString exit = "exit";
    eval_expression(exit.toLatin1().data());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInterfaceWithGDHE::Clear( )
{
    for(int x=0; x < num_scans_display; x++)
    {
        QString unsetScan = "unset robots(scan";
        QTextStream(&unsetScan) << x << ")";
        eval_expression(unsetScan.toLatin1().data());
        //printf("unset %s: \n", text2unsetScan);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*Function to draw pline in gdhe from number of points along a scan line.
  Each scan line is decomposed into multiple plines, using depth differential threshold.

  mode 0 : continue_pline - add point to current pline
  mode 1 : restart_pline  - add point as first point of new pline
  mode 2 : end_pline      - end/flush current pline

  function's state machine state variable:
  line_started 0 : no pline currently initialized
  line_started 1 : pline initialized

  pseudocode realization:

  if line_started
    switch mode
      0: add_pt
      1: cap_it, start_line, add_pt
      2: cap_it
  else //line not started
    switch mode
      0: start_line, add_pt
      1: start_line, add_pt
      2: do_nothing
  endif

  switch mode
    0: if !line_started start_line; endif; add pt;
    1: if line_started cap_it; endif; start_line, add_pt;
    2: if line_started cap_it; endif;

if (line_started && (mode 1||2))
  cap_it
if ((!line_started && mode0) || mode1)
  start_line
  line_started=1;
if (mode 0 || 1)
  add_pt

*/

void CInterfaceWithGDHE::pline_from_pos(int mode, float x_pos, float y_pos, float z_pos)
{



    //  rgb color; color.r = 255.; color.g = 0; color.b = (1*num_pline);

    //We need to end current pline and flush
    if (line_started && ((mode == mod_end) || (mode == mod_rest)) ){
        pline_str = "set robots(scan";   //crée un objet par portion continue de scan, soit potentiellement 500 objets par scan :( !!!!!!!!!!!!!!!!!!!!!
        QTextStream(&pline_str) << num_pline << \
                ") { color " << colorScan.r << " " << colorScan.g << " " << colorScan.b << " ; polyline " << num_pts;
        pline_str += pline_str_pts;
        QTextStream(&pline_str) << "}";

        //convert and send
        eval_expression(pline_str.toLatin1().data());
        //printf("pline: %s\n", text4);
        printf("pline%d pts: %d\n", num_pline,num_pts);

        QString setScan = "set pos(scan";
        QTextStream(&setScan) << num_pline << ") { 0 0 0 0 0 0 }";
        eval_expression(setScan.toLatin1().data());
        line_started = false;
    }
    //We need to start a pline
    //if ((!line_started && mode0) || mode1)
    if ( (!line_started && mode == mod_cont) || (mode == mod_rest) ){
        num_pts = 0;
        line_started = true;
        num_pline++;
        pline_str_pts = " ";
    }
    //add point
    if ( (mode == mod_rest) || (mode == mod_cont) ) {
        QTextStream(&pline_str_pts) << x_pos << " " << y_pos << " " << z_pos << " ";
        num_pts++;
    }
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInterfaceWithGDHE::SetColorScan(unsigned char r,unsigned char g,unsigned char b)
{
    colorScan.r = r;
    colorScan.g = g;
    colorScan.b = b;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInterfaceWithGDHE::DisplayCoordinateFrame(double alpha,double beta,double gamma,double x,double y,double z)
{

    // Draw xsens reference frame in GDHE
    //eval_expression((char *)"set robots(IMU) { color 0 0 255; repere }");
    eval_expression((char *)"set robots(IMU) { truc }");
    //QString setIMU = "set pos(IMU) { 0 0 0 0 0 0 }";
    QString setIMU = "set pos(IMU) { ";
    //QTextStream(&setIMU) << "0 0 0 " << lat << " " << lon << " " << alt << " }";
    //ATTENTION: order of the three angles in following line was done arbitrarily
    // QTextStream(&setIMU) << atan2(-MtiLog->R31,MtiLog->R11) << " " << asin(MtiLog->R21) << " " << atan2(-MtiLog->R23,MtiLog->R22) << " " << MtiLog->lat << " " << MtiLog->lon << " " << MtiLog->alt << " }";
    QTextStream(&setIMU) << alpha << " " << beta << " " << gamma << " " << x << " " << y << " " << z << " }";
    eval_expression(setIMU.toLatin1().data());
    printf("\nimu gdhe string: %s \n",setIMU.toLatin1().data());

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void CInterfaceWithGDHE::DisplayTrajectory( double x,double y,double z)
{
    //ce n'est pas bien d'utiliser une chaine qu'on augmente, il faudra utiliser plutot un tableau de coordonnées stocké...
    //danger: doit etre appelée avant DisplayScan et 1 fois par DisplayScan


    //delete old trajectory pline (unless this is the first scan)
    if ( num_scans_display > 2)
    {
        QString unsetScan = "unset robots(traj)";
        eval_expression(unsetScan.toLatin1().data());
    }

    if (num_scans_display> 1)
    {
        QTextStream(&tpline_str_pts) << x << " " << y << " " << z << " "; //add all points, even the first one before drawing
        QString tpline_str;
        //draw trajectory pline
        tpline_str = "set robots(traj";
        QTextStream(&tpline_str) << \
                ") { color " << 255 << " " << 255 << " " << 255 << " ; polyline " << num_scans_display-1  << " " ;
        //QTextStream(&tpline_str_pts) << MtiLog->lat << " " << MtiLog->lon << " " << MtiLog->alt << " ";

        tpline_str += tpline_str_pts; //add  the complete trajectory string
        QTextStream(&tpline_str) << "}";
        eval_expression(tpline_str.toLatin1().data());
        //printf("tpline: %s\n", tpline_str.toLatin1().data());

        // QString setScan = "set pos(traj) { 0 0 0 0 0 0 }";
        // eval_expression(setScan.toLatin1().data());
        eval_expression("set pos(traj) { 0 0 0 0 0 0 }");

    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInterfaceWithGDHE::DisplayScan(data_sensor *datasensor,CMtiLog* MtiLog)
{
    int scanpt;
    static int md;
    //    double Rx, Ry, Rz;
    double theta,thetarad;
    float x,y,z, new_x, new_y, new_z;
    double depth = 0.;
    double pdepth = 0.;
    int tooclose = 0;
    //int sizeimagexy=500;



    /*Rx_xsens = 0; Ry_xsens = 0; Rz_xsens = 0;
        Rx_point = &Rx_xsens; Ry_point = &Ry_xsens; Rz_point = &Rz_xsens;*/

    // Rx = Rx_xsens*M_PI/180; Ry = Ry_xsens*M_PI/180; Rz = Rz_xsens*M_PI/180;
    // Calcul de la matrice de rotation World to Sensor
    /*double R[9]      = { cos(Ry)*cos(Rz), -cos(Rx)*sin(Rz)+cos(Rz)*sin(Rx)*sin(Ry),  sin(Rx)*sin(Rz)+cos(Rx)*cos(Rz)*sin(Ry),
                         cos(Ry)*sin(Rz),  cos(Rx)*cos(Rz)+sin(Rx)*sin(Ry)*sin(Rz), -cos(Rz)*sin(Rx)+cos(Rx)*sin(Ry)*sin(Rz),
                            -sin(Ry)    ,              cos(Ry)*sin(Rx)            ,              cos(Rx)*cos(Ry)            };*/





    // for each scan point
    for (scanpt=0; scanpt<1080; scanpt++) //TODO en fait il ya 1081 points
    {
        // get data from hokuyo
        depth = datasensor->depth[scanpt]/SCALE_FACTOR_DISPLAYGDHE; // data en mm, depth in meters
        x = datasensor->x_data [scanpt]/SCALE_FACTOR_DISPLAYGDHE;
        y = datasensor->y_data [scanpt]/SCALE_FACTOR_DISPLAYGDHE;
        z = 0;

        //IL FAUDRAIT SURTOUT TRAITER LES POINTS POUR LESQUELS IL Y A ERREUR (==1)

        //If too close disregard
        if (depth < 0.1)
        {
            tooclose++;
            continue;
        }

        //Restart polyline if jump in depth occurs
        if ((depth > (pdepth*MAXDIF)) || (depth < (pdepth*MINDIF)))
        {
            md = mod_rest;
        }
        else
        {
            md = mod_cont;
        }
        pdepth = depth;

        //int R[9] = {0,1,0,  0,0,1,   1,0,0};
        //cheating------------------------------<<<<<<<<<<<<<<<<< PAUL YOU DID A BIG MISTAKE THERE,
        /*
         z = x;
        x = y;
        y = z;  //y=x in that case.......
        */

       z = x;
        x = y;
        y = 0;

    /*    x = y;
        z = x;
        y = 0;
*/

        new_x = MtiLog->R11*x + MtiLog->R12*y + MtiLog->R13*z;
        new_y = MtiLog->R21*x + MtiLog->R22*y + MtiLog->R23*z;
        new_z = MtiLog->R31*x + MtiLog->R32*y + MtiLog->R33*z;
        //printf("new_x=%f , new_y=%f , new_z=%f \n",new_x,new_y,new_z);

        //translate
        new_x += MtiLog->lat;
        new_y += MtiLog->lon;
        new_z += MtiLog->alt;


        pline_from_pos(md, new_x, new_y, new_z);

    } //end forall scanpts


    //let pline_from_pos know we are done with scan line
    pline_from_pos(mod_end, 0, 0, 0);
    printf("pts too close: %d\n",tooclose);


    num_scans_display++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
for(x=0 ; x<50 ; x++) {
    printf("test button , sphere num = %d , x=%f , y=%f\n", nb_sphere, x_test, y_test);

    //eval_expression("set robots(esfera) { color 255 0 0 ; sphere 1 1 3 1 }");
    //eval_expression("set pos(esfera) { 0 0 0 0 0 0 }");

    QString createSphere = "set robots(esfera";
    QTextStream(&createSphere) << nb_sphere << ") { color 255 0 0 ; sphere " << x_test << " " << y_test << " " << 1 << " .005}";

    QByteArray text3 = createSphere.toLatin1();
    char *text4 = text3.data();
    eval_expression(text4);

    // Now we show it
    QString setSphere = "set pos(esfera";
    QTextStream(&setSphere) << nb_sphere << ") { 0 0 0 0 0 0 }";

    // Evaluate expression
    QByteArray text = setSphere.toLatin1();
    char *text2 = text.data();
    eval_expression(text2);

    y_test+=0.1;

    if( y_test>10  {
        printf("y_test = %f , x_test = %f \n", y_test, x_test);
        y_test=-10;
        x_test++;
    }
    nb_sphere++;
}*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

