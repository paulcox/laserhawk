/*
Application that performs the following:
1. reads Xsens and hokuyo sensors
2. calculates absolute 3D point cloud 
3. displays this in gdhe

Authors : Paul Cox, Bertrand Vandeportaele

Currently compiles only on x86 (ubuntu 10.04)  Feb 6 2011

TODO:
- make dist too close adjustable in GUI
- make FAC adjustable in GUI
- get png output working and cleaned up
- look into segmentation of points into surfaces
- work in log sensor vals to file and read back as virtual sensor
- implement virtual surface that is virtually sensed during fly over
*/

#include <QtGui>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#define FLOATHOKUYO double
//#define LOG_NAME	"/home/paul/Documents/LAAS/qtcreator_projs/hokuyomti/log/2011-06-14-22-54-34"
#define LOG_NAME	"/home/paul/Documents/LAAS/laserhawk/hokuyomti/2011-02-17-19-16-49"
#define NB_SCAN_START   100
#define NB_SCAN_INCR 20

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // init
    port_open = true;
    view_image = false;
    gdhe_open = false;
    //limite = 50; // limite du champs de rayon pour la recherche
    nb_scan = NB_SCAN_START;
	num_scans = 0;
    nb_sphere = 0;
    y_test = -10;
    x_test = 0;
    nb_point = 10;//24;

    populate_GUI();

    savedata.angle = (float*)malloc(1080*sizeof(float));
    savedata.data = (uint32_t*)malloc(1080*sizeof(uint32_t));
    savedata.x_data = (float*)malloc(1080*sizeof(float));
    savedata.y_data = (float*)malloc(1080*sizeof(float));

    this->resize(240,320);

    openGDHE();
    
    //open_port();
    //scanGDHE();
    clear_function();
}

void MainWindow::populate_GUI() {
    /* button_kill_gdhe = new QPushButton(tr("&Kill GDHE"));
     button_kill_gdhe->setDefault(true);
     button_kill_gdhe->setToolTip("kill gdhe");
     button_kill_gdhe->show();*/

     button_open = new QPushButton(tr("&Open device"));
     button_open->setDefault(true);
     button_open->setToolTip("Ouverture du port Hokuyo");
     button_open->show();

     button_close = new QPushButton(tr("&Close device"));
     button_close->setDefault(true);
     button_close->setToolTip("Fermeture du port Hokuyo");
     button_close->show();

     button_open_gdhe = new QPushButton(tr("&Open GDHE"));
     button_open_gdhe->setDefault(true);
     button_open_gdhe->setToolTip("Ouverture de GDHE");
     button_open_gdhe->show();

     button_scan_gdhe = new QPushButton(tr("&Scan in GDHE"));
     button_scan_gdhe->setDefault(true);
     button_scan_gdhe->setToolTip("Scan Hokuyo in GDHE");
     button_scan_gdhe->show();

     button_clear = new QPushButton(tr("&Clear GDHE"));
     button_clear->setDefault(true);
     button_clear->setToolTip("Clear");
     button_clear->show();

    /* connect(button_kill_gdhe, SIGNAL(clicked()),
             this, SLOT(kill_gdhe_function()));*/

     connect(button_clear, SIGNAL(clicked()),
             this, SLOT(clear_function()));

/*     connect(button_open, SIGNAL(clicked()),
             this, SLOT(open_port()));

     connect(button_close, SIGNAL(clicked()),
             this, SLOT(close_port()));
*/
     connect(button_scan_gdhe, SIGNAL(clicked()),
             this, SLOT(scanGDHE()));

     connect(button_open_gdhe, SIGNAL(clicked()),
             this, SLOT(openGDHE()));

     label_image =  new QLabel("Image Window");

     label1 = new QLabel("Hokuyo port is closed");

     label_image_init =  new QLabel("Image Window");
     QImage image("enac.png");
     label_image_init->setPixmap(QPixmap::fromImage(image));

     depthImage_init = new QScrollArea;
     depthImage_init->setBackgroundRole(QPalette::Dark);
     depthImage_init->setWidget(label_image_init);
     //depthImage->addScrollBarWidget(label_image,Qt::Vertical);
     //depthImage->setFixedSize(2000,2000);

     layout_G = new QGridLayout;// slots:
     //void open_port()
     layout_G->addWidget(button_open, 0, 0);
     layout_G->addWidget(button_close, 1, 0);
     //layout_G->addWidget(button_scan, 2, 0);
     //layout_G->addWidget(label1, 2, 0, 1, 4);
     //layout_G->addWidget(depthImage_init, 0, 4, 3, 3);
     layout_G->addWidget(button_open_gdhe, 2,0);
     layout_G->addWidget(button_scan_gdhe, 3,0);
     layout_G->addWidget(button_clear, 4,0);
     //layout_G->addWidget(button_kill_gdhe, 6,3);

     centralWidget()->setLayout(layout_G);

     setWindowTitle("log2gdhe App");
}

MainWindow::~MainWindow()
{
    free(savedata.angle);
    free(savedata.data);
    free(savedata.x_data);
    free(savedata.y_data);
    //system("pkill gdhe");
    delete ui;
}
void MainWindow::kill_gdhe_function()
{
    //system("pkill gdhe");
}


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
#define mod_cont 0
#define mod_rest 1
#define mod_end  2

void pline_from_pos(int mode, float x_pos, float y_pos, float z_pos){
	static bool line_started = false;
	static int num_pts = 0;
	static int num_pline = 0;
	QString pline_str; 
	static QString pline_str_pts;
	rgb color; color.r = 255.; color.g = 0; color.b = (1*num_pline);
	
	//We need to end current pline and flush
	if (line_started && ((mode == mod_end) || (mode == mod_rest)) ){
		pline_str = "set robots(scan";
                QTextStream(&pline_str) << num_pline << \
                        ") { color " << color.r << " " << color.g << " " << color.b << " ; polyline " << num_pts;
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


void MainWindow::getRange(){
    char nomfich[1000];
    FILE *Fscan = NULL;
    FLOATHOKUYO theta,thetarad;
    FLOATHOKUYO x,y;
    //uint32_t depth;
	int var[2] = {0};
	
	
	//char * fileName = strdup(LOG_NAME);
    //sprintf(nomfich, "%s/scan%06d.txt", fileName.c_str(), nb_scan);
	sprintf(nomfich, "%s/scan%06d.txt", LOG_NAME, nb_scan);
	printf("fichier:%s\n", nomfich);
    Fscan = fopen(nomfich, "rt");

    if(Fscan != NULL)
    {
        for(int i=0 ; i <= 1080 ; i++) {
            int res = fscanf(Fscan,"%d       %d\n", &var[0], &var[1]);
			if (res != 2) {printf("fscanf failed\n");}

            //printf("var0=%d , var1=%d \n",var[0],var[1]);

            // calcul : angle , distance, profondeur

            // calcul de l'angle
            theta = -(135) + i*(270.0/1080);
            thetarad = theta * M_PI/180.0;

            // calcul des coordonnées x, y dans le repère sensor
            x = cos(thetarad) * var[1];
            y = sin(thetarad) * var[1];

            savedata.data[i] = var[1];
            savedata.angle[i] = thetarad;
            savedata.x_data[i] = x;
            savedata.y_data[i] = y;
            //printf("i=%d data=%d \n", i, var[1]);
        }
		fclose(Fscan);
    }
	//nb_testscan++;
}


#define FAC 0.5f
#define MAXDIF (1+FAC)
#define MINDIF (1-FAC)

void MainWindow::scanGDHE()
{
    int scanpt;
    static int md;
    double Rx, Ry, Rz;
    double theta,thetarad;
    float x,y,z, new_x, new_y, new_z;
    double depth = 0.;
	double pdepth = 0.;
    int tooclose = 0;
    //int sizeimagexy=500;

    if( port_open == true )
    {
        if( gdhe_open == true )
        {
                // Get range data
                //put it in this variable : data;
				getRange();

                //Tell IMU thread I'm about to get a new frame from the camera
                //emit xsensValues(Rx_xsens , Ry_xsens, Rz_xsens);
                Rx_xsens = 0; Ry_xsens = 0; Rz_xsens = 0;
                Rx_point = &Rx_xsens; Ry_point = &Ry_xsens; Rz_point = &Rz_xsens;

                // envoi d'un signal au thread IMU pour recuperer les donnees de l'imu
                //pupt xsens angles in these variables :(Rx_point, Ry_point, Rz_point);
				//getIMU(Rx_point, Ry_point, Rz_point);
                printf("Xsens => Rx=%f , Ry=%f , Rz=%f \n", Rx_xsens , Ry_xsens, Rz_xsens);

                // calcul des angles en radian
                Rx = Rx_xsens*M_PI/180; Ry = Ry_xsens*M_PI/180; Rz = Rz_xsens*M_PI/180;

                // Calcul de la matrice de rotation World to Sensor
                double R[9]      = { cos(Ry)*cos(Rz), -cos(Rx)*sin(Rz)+cos(Rz)*sin(Rx)*sin(Ry),  sin(Rx)*sin(Rz)+cos(Rx)*cos(Rz)*sin(Ry),
				     cos(Ry)*sin(Rz),  cos(Rx)*cos(Rz)+sin(Rx)*sin(Ry)*sin(Rz), -cos(Rz)*sin(Rx)+cos(Rx)*sin(Ry)*sin(Rz),
				        -sin(Ry)    ,              cos(Ry)*sin(Rx)            ,              cos(Rx)*cos(Ry)            };

                // Draw xsens reference frame in GDHE
                eval_expression((char *)"set robots(IMU) { color 0 0 255; repere }");
                QString setIMU = "set pos(IMU) { ";
                QTextStream(&setIMU) << Rz_xsens << " " << Ry_xsens << " " <<  Rx_xsens << " 0 0 0 }";
                eval_expression(setIMU.toLatin1().data());
                //printf("\n\n\n\nchaine gdhe pour imu : %s \n\n\n\n",text6);

                // for each scan point
                for (scanpt=0; scanpt<1080; scanpt++) {
                    /* get data from hokuyo*/
                    depth = savedata.data[scanpt]/1000.; // data en mm, depth in meters
		    
		    /*If too close disregard*/
		    if (depth < 0.1) {
		        tooclose++;
				continue;
		    }

		    //Restart polyline if jump in depth occurs
		    if ((depth > (pdepth*MAXDIF)) || (depth < (pdepth*MINDIF))) {
			md = mod_rest;
		    } else {
			md = mod_cont;
		    }
		    pdepth = depth;
		    
                    //depth=100*sin(6.28*i/270);

                    // calculate angle in degrees
                    theta = -(135) + scanpt * (270.0/1080);
                    //theta=-theta+180;
                    // calcul of angle in radians
                    thetarad = theta * M_PI/180.0;

                    // calculate coordonees x, y et z dans le repere sensor
                    x = cos(thetarad) * depth;
                    y = sin(thetarad) * depth;
                    z = 0;

                    // calcul des coordonnees x, y et z dans le repere world
                    new_x = R[0]*x + R[1]*y + R[2]*z;
                    new_y = R[3]*x + R[4]*y + R[5]*z;
                    new_z = R[6]*x + R[7]*y + R[8]*z;
                    //printf("new_x=%f , new_y=%f , new_z=%f \n",new_x,new_y,new_z);

					//pline_from_pos(md, new_x, new_y, new_z);
					pline_from_pos(md, new_x, new_y, num_scans);

                    savedata.angle[scanpt] = thetarad;
//                    savedata.data[scanpt] = data[scanpt];
                    savedata.x_data[scanpt] = new_x;
                    savedata.y_data[scanpt] = new_y;
                }
		//let pline_from_pos know we are done with scan line
		pline_from_pos(mod_end, 0, 0, 0);
		printf("pts too close: %d\n",tooclose);

                //search(savedata);
                nb_scan += NB_SCAN_INCR;
				num_scans++;

            }
        else {
            label1->setText("GDHE program is closed\n");
            printf("GDHE program is closed\n");
        }

    } else {
        label1->setText("Hokuyo port is closed\n");
        printf("Hokuyo port is closed\n");
    }

}


void MainWindow::openGDHE()
{
    if( gdhe_open == false ) {
        button_open_gdhe->setText("close GDHE");
        gdhe_open=true;

        // Open GDHE
        int res = system("gdhe &");
		
		if (res == 0) {printf("gdhe started ok\n");}

        // Sleep to allow full opening of GDHE, then connect
        sleep(3);

        int error = get_connection((char *)"localhost");
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
	
	eval_expression((char *)"set robots(r1) { xr4000 }");
        eval_expression((char *)"set pos(r1) { 0 0 0 0 0 0 }");

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
    } else {
        gdhe_open = false;
        button_open_gdhe->setText("open GDHE");
        /*
            QString disconnect = "disconnect()";
            QByteArray textdisconnect = disconnect.toLatin1();
            char *text2disconnect = textdisconnect.data();
            eval_expression(text2disconnect);*/
        QString exit = "exit";
        eval_expression(exit.toLatin1().data());
    }
}

void MainWindow::clear_function()
{
    if(gdhe_open == true) {
        for(int x=0; x < num_scans; x++) {
            QString unsetScan = "unset robots(scan";
            QTextStream(&unsetScan) << x << ")";
            eval_expression(unsetScan.toLatin1().data());
            //printf("unset %s: \n", text2unsetScan);
        }
        nb_scan=NB_SCAN_START;
    }


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
}
