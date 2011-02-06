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
#include <flexiport/flexiport.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // init
    port_open = false;
    view_image = false;
    gdhe_open = false;
    //limite = 50; // limite du champs de rayon pour la recherche
    nb_scan = 0;
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

    // create instance of IMU and start thread
    threadIMU = new IMU(this);
    connect(this, SIGNAL(xsensValues(double *, double *, double *)),
            threadIMU, SLOT(getValues(double *, double *, double *)));//, Qt::BlockingQueuedConnection);
    threadIMU->start();

    openGDHE();

    //init hokuyo com
    open_port();
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

     button_scan = new QPushButton(tr("&Hokuyo Scanning"));
     button_scan->setDefault(true);
     button_scan->setToolTip("Scan de l'hokuyo");
     button_scan->show();

     button_open_gdhe = new QPushButton(tr("&Open GDHE"));
     button_open_gdhe->setDefault(true);
     button_open_gdhe->setToolTip("Ouverture de GDHE");
     button_open_gdhe->show();

     button_scan_gdhe = new QPushButton(tr("&Scanning in GDHE"));
     button_scan_gdhe->setDefault(true);
     button_scan_gdhe->setToolTip("Scanning of Hokuyo in GDHE");
     button_scan_gdhe->show();

     button_clear = new QPushButton(tr("&Clear GDHE"));
     button_clear->setDefault(true);
     button_clear->setToolTip("Clear");
     button_clear->show();

    /* connect(button_kill_gdhe, SIGNAL(clicked()),
             this, SLOT(kill_gdhe_function()));*/

     connect(button_clear, SIGNAL(clicked()),
             this, SLOT(clear_function()));

     connect(button_open, SIGNAL(clicked()),
             this, SLOT(open_port()));

     connect(button_close, SIGNAL(clicked()),
             this, SLOT(close_port()));

     connect(button_scan, SIGNAL(clicked()),
             this, SLOT(scanning()));

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
     layout_G->addWidget(button_scan, 2, 0);
     //layout_G->addWidget(label1, 2, 0, 1, 4);
     //layout_G->addWidget(depthImage_init, 0, 4, 3, 3);
     layout_G->addWidget(button_open_gdhe, 3,0);
     layout_G->addWidget(button_scan_gdhe, 4,0);
     layout_G->addWidget(button_clear, 5,0);
     //layout_G->addWidget(button_kill_gdhe, 6,3);

     centralWidget()->setLayout(layout_G);

     setWindowTitle("hoku2gdhe App");
}

MainWindow::~MainWindow()
{
    threadIMU->stop();
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

void MainWindow::open_port()
{
    if(port_open == false)
    {
        string portOptions = "type=serial,device=/dev/ttyACM0,timeout=1";
        //string portOptions = "type=serial,device=/dev/tty.usbmodem411,timeout=1";
        startAngle = 0.0;
        endAngle = 0.0;
        firstStep = -1;
        lastStep = -1;
        baud = 19200;
        speed = 0;
        clusterCount = 1;
        getIntensities = false;
        getNew = false;
        verbose = false;

        try
        {
            // Set the laser to verbose mode (so we see more information in the console)
            if (verbose)
                laser.SetVerbose (true);

            // Open the laser
            try
            {
                laser.Open (portOptions);
                // Turn the laser on
                laser.SetPower (true);
            }
            catch(flexiport::PortException e)
            {
                printf("Exception What() : Failed to open device '/dev/ttyACM0' levée\n");
                label1->setText("Hokuyo port is closed\nUSB port of the Hokuyo is not connected");
                printf("Hokuyo port is closed\nUSB port of the Hokuyo is not connected");
                return;
            }
            try
            {
                laser.SetBaud (baud);
            }
            catch (hokuyo_aist::HokuyoError e)
            {
                cerr << "Failed to change baud rate: (" << e.Code () << ") " << e.what () << endl;
            }
            // Set the motor speed
            try
            {
                laser.SetMotorSpeed (speed);
            }
            catch (hokuyo_aist::HokuyoError e)
            {
                cerr << "Failed to set motor speed: (" << e.Code () << ") " << e.what () << endl;
            }

            // Get some laser info
            //    cout << "Laser sensor information:" << endl;
            hokuyo_aist::HokuyoSensorInfo info;
            laser.GetSensorInfo (&info);
            //    cout << "bla"<<endl;
            //    cout << info.AsString ();
            // FILE *F;
            // F=fopen("out.xyz","wt");

            // Mise a jour du texte dans le Label
            QString hokuyo_info;
            QTextStream(&hokuyo_info) << "Laser sensor information:\n" << info.AsString().c_str();

            label1->setText(hokuyo_info);
            printf("%s",hokuyo_info.toAscii().constData());
        }
        catch (hokuyo_aist::HokuyoError e)
        {
            cerr << "Caught exception: (" << e.Code () << ") " << e.what () << endl;
        }
        port_open = true;
    }
}

void MainWindow::close_port()
{
    if(port_open == true)
    {
        laser.Close ();
        port_open = false;
        view_image = false;
        label_image_init =  new QLabel("fenetre");
        QImage image("enac.png");
        label_image_init->setPixmap(QPixmap::fromImage(image));

        depthImage_init = new QScrollArea;
        depthImage_init->setBackgroundRole(QPalette::Dark);
        depthImage_init->setWidget(label_image_init);

        layout_G->addWidget(depthImage_init, 0, 4, 3, 3);

    } else {
        label1->setText("Hokuyo port is closed \n");
        printf("Hokuyo port is closed \n");
    }
}

void MainWindow::scanning()
{
    if(port_open == true)
    {
        int sizeimagexy=2000;
        int cer,cpt;
        int nb_ray=10; // nombre de rayon
        int rayon=100; // en cm
        int stat;
        int nray;
        int nraymax=1;
        rgb  col;  col.r  = 255;  col.g  =   0; col.b  =   0;
        rgb  col2; col2.r =   0;  col2.g =   0; col2.b = 255;
        rgb  col3; col3.r =   0;  col3.g = 255; col3.b =   0;
	int pix_x,pix_y;

	//wtf? multiplication without assignment? how does this compile?
        im_color * im_ray;
        im_ray = alloc_im_color(sizeimagexy,sizeimagexy);
        memset(im_ray->data, 255, 3*sizeimagexy*sizeimagexy);

        //draw_circles(sizeimagexy,im_ray,col);

        for(cpt=0; cpt<nb_ray ; cpt++)
        {
            for(cer=0;cer<2160;cer++)
            {
                pix_x = (sizeimagexy/2) + rayon*cos(cer);
                pix_y = (sizeimagexy/2) + rayon*sin(cer);
                draw_square(im_ray,pix_x,pix_y,&col,1);
            }
            rayon+=100;
        }

        // Get range data
        hokuyo_aist::HokuyoData data;
	
        if ((firstStep == -1 && lastStep == -1) && (startAngle == 0.0 && endAngle == 0.0)) {
            // Get all ranges
            if (getNew)
                laser.GetNewRanges (&data, -1, -1, clusterCount);
            else if (getIntensities)
                laser.GetNewRangesAndIntensities (&data, -1, -1, clusterCount);
            else
                laser.GetRanges (&data, -1, -1, clusterCount);

        } else if (firstStep != -1 || lastStep != -1) {
            // Get by step
            if (getNew)
                laser.GetNewRanges (&data, firstStep, lastStep, clusterCount);
            else if (getIntensities)
                laser.GetNewRangesAndIntensities (&data, firstStep, lastStep, clusterCount);
            else
                laser.GetRanges (&data, firstStep, lastStep, clusterCount);
        } else {
            // Get by angle
            if (getNew)
                laser.GetNewRangesByAngle (&data, startAngle, endAngle, clusterCount);
            else if (getIntensities)
                laser.GetNewRangesAndIntensitiesByAngle (&data, startAngle, endAngle, clusterCount);
            else
                laser.GetRangesByAngle (&data, startAngle, endAngle, clusterCount);
        }

        for (nray=0; nray<nraymax; nray++)
        {
            printf("nray:%d\n",nray);

            double theta,thetarad;
            double x,y,z;
            double depth;

            // Permet de dessiner les carrées verts représentants le repère du capteur
            int x_im = (sizeimagexy/2);
            int y_im = (sizeimagexy/2);
            draw_square(im_ray,x_im,y_im,&col3,24*(sizeimagexy/2000.));

            x_im = (sizeimagexy/2)+(sizeimagexy/20);
            y_im = (sizeimagexy/2);
            draw_square(im_ray,x_im,y_im,&col3,24*(sizeimagexy/2000.));

            x_im = (sizeimagexy/2);
            y_im = (sizeimagexy/2)+(sizeimagexy/20);
            draw_square(im_ray,x_im,y_im,&col3,24*(sizeimagexy/2000.));

            // Permet de dessiner chaques points de données
            for (int i=0; i<1080; i++)
            {
                depth=data[i]; // en mm

                theta = (-45) + i*(270.0/1080);
                theta = -theta + 180;
                thetarad = theta*3.14159/180.0;

                x = cos(thetarad)*depth;
                y = sin(thetarad)*depth;
                z = nray;

                col2.r = ((double)nray/nraymax)*255.;
                col2.g = 0;
                col2.b = 255 - ((double)nray/nraymax)*255;

                x_im = (sizeimagexy/2) + (x/(10*(2000./sizeimagexy)));
                y_im = (sizeimagexy/2) + (y/(10*(2000./sizeimagexy)));

                draw_square(im_ray,x_im,y_im,&col2,1);
            }
            //printf("sizeimagexy=%d \n", sizeimagexy);
            //char name[1000];
        }

        save_ppm((char *)"im_ray.ppm",im_ray);
        stat = system("convert im_ray.ppm im_ray.png");
        //printf("stat:%d\n",stat);
        stat = system("open im_ray.png");
        //printf("stat:%d\n",stat);

        // maj de l'affichage dans la fenetre
        QImage image("im_ray.png");

        image.scaled(10,10,Qt::KeepAspectRatioByExpanding);
        label_image->setPixmap(QPixmap::fromImage(image));

        if(view_image == false) {
            depthImage = new QScrollArea;
            depthImage->setBackgroundRole(QPalette::Dark);
            depthImage->setWidget(label_image);
            //layout_G->removeWidget(depthImage_init);
            layout_G->addWidget(depthImage, 0, 4, 3, 3);
            view_image = true;
        }
    } else {
        // le port n'est pas ouvert
        label1->setText("Hokuyo port is closed\n");
        printf("Hokuyo port is closed \n");
    }
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

//#define ACQ_CONTINUE
//#define ACQ_STATIC
#define ACQ_INCR

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
    double depth, pdepth;
    int tooclose = 0;
    //int sizeimagexy=500;

    if( port_open == true )
    {
        if( gdhe_open == true )
        {
#ifdef ACQ_CONTINUE
            while(1){
#endif
                // Get range data
                hokuyo_aist::HokuyoData data;
                if ((firstStep == -1 && lastStep == -1) && (startAngle == 0.0 && endAngle == 0.0))
                {
                    // Get all ranges
                    if (getNew)
                        laser.GetNewRanges(&data, -1, -1, clusterCount);
                    else if (getIntensities)
                        laser.GetNewRangesAndIntensities(&data, -1, -1, clusterCount);
                    else
                        laser.GetRanges(&data, -1, -1, clusterCount);
                }
                else if (firstStep != -1 || lastStep != -1)
                {
                    // Get by step
                    if (getNew)
                        laser.GetNewRanges(&data, firstStep, lastStep, clusterCount);
                    else if (getIntensities)
                        laser.GetNewRangesAndIntensities(&data, firstStep, lastStep, clusterCount);
                    else
                        laser.GetRanges(&data, firstStep, lastStep, clusterCount);
                }
                else
                {
                    // Get by angle
                    if (getNew)
                        laser.GetNewRangesByAngle(&data, startAngle, endAngle, clusterCount);
                    else if (getIntensities)
                        laser.GetNewRangesAndIntensitiesByAngle(&data, startAngle, endAngle, clusterCount);
                    else
                        laser.GetRangesByAngle(&data, startAngle, endAngle, clusterCount);
                }

                //Tell IMU thread I'm about to get a new frame from the camera
                //emit xsensValues(Rx_xsens , Ry_xsens, Rz_xsens);
                Rx_xsens = 0; Ry_xsens = 0; Rz_xsens = 0;
                Rx_point = &Rx_xsens; Ry_point = &Ry_xsens; Rz_point = &Rz_xsens;

                // envoi d'un signal au thread IMU pour recuperer les donnees de l'imu
                emit xsensValues(Rx_point, Ry_point, Rz_point);
                printf("Xsens => Rx=%f , Ry=%f , Rz=%f \n", Rx_xsens , Ry_xsens, Rz_xsens);

#ifdef ACQ_STATIC
                Rx=0; Ry=0; Rz=0;
#else
                // calcul des angles en radian
                Rx = Rx_xsens*M_PI/180; Ry = Ry_xsens*M_PI/180; Rz = Rz_xsens*M_PI/180;
#endif
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
                    depth = data[scanpt]/1000.; // data en mm, depth in meters
		    
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

		    pline_from_pos(md, new_x, new_y, new_z);

                    savedata.angle[scanpt] = thetarad;
                    savedata.data[scanpt] = data[scanpt];
                    savedata.x_data[scanpt] = new_x;
                    savedata.y_data[scanpt] = new_y;
                }
		//let pline_from_pos know we are done with scan line
		pline_from_pos(mod_end, 0, 0, 0);
		printf("pts too close: %d\n",tooclose);

                //search(savedata);
#ifdef ACQ_INCR
                nb_scan++;
#endif
            }
#ifdef ACQ_CONTINUE
        }
#endif
        else {
            label1->setText("GDHE program is closed\n");
            printf("GDHE program is closed\n");
        }

    } else {
        label1->setText("Hokuyo port is closed\n");
        printf("Hokuyo port is closed\n");
    }

}

//returns true if the ray defined by ax+by=d intersects the circle whose center is located at xc,yc and has a radius = rc
//dans ce cas, xp1,yp1 est l'intersection la plus proche de l'origine du repere, et xp2,yp2 la plus lointaine
//renvoie false si il n'y a pas d'intersection
bool IntersectionRayonCercle(double a,double b,double d,double xc,double yc,double rc,
                             double *xp1,double *yp1,double *xp2,double *yp2)
{
    double A, B, C, D;
    double n1, n2, temp;

    if (abs(b) < 1e-15) {
        //cas particulier a traiter a  part...
        A = 1;
        B = -2 * yc;
        C = (-(rc*rc)) + (yc*yc) - (2*d*xc/a) + (xc*xc) + (d*d) / (a*a);
        D = B*B-(4*A*C);

        if ( D < 1e-10 ) {
            //pas d'intersection ou tangence
            return false;
        } else {
            *yp1 = (-B-sqrt(D)) / (2*A);
            *xp1 = d/a;
            *yp2 = (-B+sqrt(D)) / (2*A);
            *xp2 = d/a;
        }
    } else {
        A = 1 + ((a*a)/(b*b));
        B = -(2*xc) - ((2*d*a)/(b*b)) + (2*a*yc/b);
        C = (xc*xc) + (yc*yc) - (2*d*yc/b) - (d*d)/(b*b) - (rc*rc);
        D = B*B - (4*A*C);

        if (D < 1e-10) {
            //pas d'intersection ou tangence
            return false;
        } else {
            //calcul des intersections
            *xp1 = (-B-sqrt(D)) / (2*A);
            *yp1 = ( (-a* (*xp1) ) + d ) / b;
            *xp2 = (-B+sqrt(D)) / (2*A);
            *yp2 = ( (-a* (*xp2) ) + d ) / b;
        }
    }
    n1 = ((*xp1) * (*xp1)) + ((*yp1) * (*yp1));
    n2 = ((*xp2) * (*xp2)) + ((*yp2) * (*yp2));

    //il faut inverser l'ordre
    if (n2 < n1) {
        temp = *xp1; *xp1 = *xp2; *xp2 = temp;
        temp = *yp1; *yp1 = *yp2; *yp2 = temp;
    }
    return true;
}


void MainWindow::search(MainWindow::data_sensor sensor)
{
    limite = 50;
    int i;
    double rayon_cercle_1 = 300; // en millimetre
    double erreurn2;
    double erreur[1080];
    double a,b,d;
    double xc,yc;
    double xp1, yp1, xp2, yp2;
    bool intersection1;
    double erreurmin = 1e15;
    int ierreurmin = 0;

	/*for each point on laser scan we're going to assume it's along the line between sensor and center of circle.
	  we test for this condition by looking at neighboring samples, and */
    for(i=limite ; i<(1080-limite) ; i++) {
        // calcul de l'equation de droite du rayon
        d = 0;
        a = cos(sensor.angle[i]);
        b = sin(sensor.angle[i]);
        // calcul du centre du cercle dans le repere Hokuyo
	// x and y location of potential circle center
        xc = (sensor.data[i]+rayon_cercle_1)*a;
        yc = (sensor.data[i]+rayon_cercle_1)*b;
        erreur[i]=0;
	
	/*For each neighbor we calculate look for intersection and calculate error*/
        for (int n2=i-limite ; n2<i+limite ; n2++) {
            intersection1 = IntersectionRayonCercle(a,b,d,xc,yc,rayon_cercle_1,&xp1,&yp1,&xp2,&yp2);

            //Check if xp1 is in the direction of the ray and there is intersection
            if ( intersection1 && ( abs(atan2(yp1,xp1)-sensor.angle[i]) < 1e-10 ) ) {
	        //calculate distance (sqrt of sum of squares)
                erreurn2 = sqrt(  ((xp1-sensor.x_data[i]) * (xp1-sensor.x_data[i]) ) 
				+ ((yp1-sensor.y_data[i]) * (yp1-sensor.y_data[i]) ) );
            } else {
                erreurn2 = 0;
            }
	    //sum error
            erreur[i] += erreurn2;
        }
    }

    //recherche du i pour lequel l'erreur est mini:
    for( i=limite ; i < (1080-limite) ; i++) {
        //printf("i: %05d -> %lf   \n",i,erreur[i]);

        if (erreur[i] < erreurmin) {
            ierreurmin = i;
            erreurmin = erreur[i];
        }
    }
    printf("erreurmin: %lf en i:%d \n",erreurmin,ierreurmin);
}


void MainWindow::openGDHE()
{
    if( gdhe_open == false ) {
        button_open_gdhe->setText("close GDHE");
        gdhe_open=true;

        // Open GDHE
        system("gdhe &");

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
        for(int x=0; x<nb_scan; x++) {
            QString unsetScan = "unset robots(scan";
            QTextStream(&unsetScan) << x << ")";
            eval_expression(unsetScan.toLatin1().data());
            //printf("unset %s: \n", text2unsetScan);
        }
        nb_scan=0;
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
