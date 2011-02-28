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

/*
Hokuyo times:
~/Documents/LAAS/laserhawk/hokuyomti/2011-02-17-19-16-49
scan000000 1297966609.500544
scan001710 1297966710.938730

MTI times:
./Documents/LAAS/laserhawk/hokuyomti/MTI.out
points around hokuyo start:
1297966609.494408131 QUAT  0.524142  0.143444  0.818253  0.187515 POS 377094.231 4824479.079    208.328  31T VEL   -1.7512    0.2324    0.8404
1297966609.504407883 QUAT  0.524124  0.143390  0.818256  0.187592 POS 377094.228 4824479.062    208.337  31T VEL   -1.7577    0.2331    0.8399

Start and end
1297965927.462918758
1297966725.302998781
*/
 
#include <locale.h>


#include <QtGui>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>

#define FLOATHOKUYO double
//#define LOG_NAME	"/home/paul/Documents/LAAS/qtcreator_projs/hokuyomti/log/2011-06-14-22-54-34"
//#define LOG_NAME	"/home/bvdp/laserhawk/hokuyomti/2011-02-17-19-16-49"
#define LOG_NAME	"/home/paul/Documents/LAAS/laserhawk/log2gdhe"
//#define NB_SCAN_START   100
#define NB_SCAN_START   0
#define NB_SCAN_INCR 1
//#define MTI_LOG_NAME	"/home/bvdp/laserhawk/hokuyomti/MTI.out"
//#define MTI_LOG_NAME	"/home/paul/Documents/LAAS/laserhawk/hokuyomti/MTI.out"
#define MTI_LOG_NAME	"/home/paul/Documents/LAAS/laserhawk/log2gdhe/MTI.out"

QString truc = "proc truc {} { \nobject truc { \npushMatrix \ncolor 200 200 100 \nbox 0 0 -0.5 1 1 1 \ncolor 200 0 0 \n\
cylinder 0 0 0 x 1 0 2 24 \ncolor 0 200 0 \ncylinder 0 0 0 y 1 0 2 24 \ncolor 0 0 200 \ncylinder 0 0 0 z 1 0 2 24 \n\
popMatrix \n} \n} ";


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

printf("locale: %s \n ", setlocale( LC_ALL ,NULL))	;
setlocale( LC_ALL , "en_US.utf8");
printf("locale: %s \n ", setlocale( LC_ALL ,NULL))	;
printf("a: %lf \n ",1.423);
char chaine[1000]="1.2";
double d;
sscanf(chaine,"%lf\n",&d);
printf("d: %lf \n ",d);
 




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


     button_animate = new QPushButton(tr("&Animate"));
     button_animate->setDefault(true);
     button_animate->setToolTip("Animate");
     button_animate->show();

     connect(button_animate, SIGNAL(clicked()),
             this, SLOT(animate()));


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
     layout_G->addWidget(button_animate, 5,0);


animFlag=0;
     Timer=new QTimer(this);
          Timer->setInterval(100);
          connect(Timer, SIGNAL(timeout()), this, SLOT(on_timer_Event()));
            Timer->start();


            centralWidget()->setLayout(layout_G);

     setWindowTitle("log2gdhe App");
}


void MainWindow::on_timer_Event()
{
if (animFlag!=0)
    this->scanGDHE();
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


FILE *Fscanmti = NULL;
double R11,R12,R13;
double R21,R22,R23;
double R31,R32,R33;
double lat,lon,alt;
double latini=0;
double lonini=0;
double altini=0;
double quat[4];

void MainWindow::getRange(){
    char nomfich[1000];
    FILE *Fscan = NULL;
    FLOATHOKUYO theta,thetarad;
    FLOATHOKUYO x,y;
    //uint32_t depth;
	int var[2] = {0};
	double mti[11] = {0};
	int mtic = 0;
	char mtiz;
	double mtitime,hokuyotime;
	
	sprintf(nomfich, "%s/scan%06d.txt", LOG_NAME, nb_scan);
	//sprintf(nomfich, "%s/scan000001.txt", LOG_NAME, nb_scan);
	printf("fichier:%s\n", nomfich);
    Fscan = fopen(nomfich, "rt");
if (Fscan==NULL)
    {
    this->animFlag=0;
}
    if(Fscan != NULL)
    {
        for(int i=0 ; i < 1080 ; i++) {
            int res = fscanf(Fscan,"%d       %d\n", &var[0], &var[1]);
			if (res != 2) {printf("fscanf failed\n");}
            //printf("var0=%d , var1=%d \n",var[0],var[1]);

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
		//read last line of scan file which gives us time of acquisition
		fscanf(Fscan,"%lf\n", &hokuyotime);
		printf("hokuyotime: %lf \n",hokuyotime);
		fclose(Fscan);
#if 1				
		//1297965927.462918758 QUAT  0.072093  0.746111  0.661814 -0.011095 POS 377098.986 4824479.869    200.101  31T VEL   -0.0100    0.0200    0.0100
		char* mtiline;
		size_t mtilinelength = 1000;
	
		mtiline = (char *) malloc (mtilinelength + 1);
		//scan through mti log and find corresponding entry by looking for next closest timestamp (not necessarily closest)
		do { 
//char trash[1000];
			getline(&mtiline,&mtilinelength,Fscanmti);
                        //printf("MTI scan :  %s\n",mtiline);
		 	int res = sscanf(mtiline,"%lf QUAT  %lf  %lf  %lf %lf POS %lf %lf    %lf  %2d%c VEL   %lf    %lf    %lf\n",&mti[0],&mti[1],&mti[2],&mti[3],&mti[4],&mti[5],&mti[6],&mti[7],&mtic,&mtiz,&mti[8],&mti[9],&mti[10]);
			
//	int res = sscanf(mtiline,"%Lf %s  %lf  %lf  %lf %lf %s %lf %lf    %lf  %2d%c %s   %lf    %lf    %lf\n", 	&mti[0],trash,&mti[1],&mti[2],&mti[3],&mti[4],trash,&mti[5],&mti[6],&mti[7],&mtic,&mtiz,trash,&mti[8],&mti[9],&mti[10]);

			printf("fscanf RETURNED %d\n",res);
			mtitime = mti[0];
			printf("mti line time %lf\n",mtitime);
		} while (mtitime < hokuyotime);
		printf("mti line time %lf\n",mtitime);

#else
		mti[1]=0;mti[2]=0;mti[3]=1;mti[4]=0;mti[5]=0;mti[6]=0;mti[7]=0;
#endif
		float sumsqr = 0;
		for (int i=0;i<4;i++){
			quat[i] = mti[i+1];
			printf("Quat[%d]: %lf\n",i,quat[i]);
			sumsqr += quat[i]*quat[i]; 
		}
		//printf("quat norm : %lf\n",sqrt(sumsqr)); //check norm, should be unity

		
		//convert quat to rot matrix
		//http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
		//0=w		1=x		2=y		3=z
		R11 = 1-2*quat[2]*quat[2]-2*quat[3]*quat[3]; R12 = 2*quat[1]*quat[2] - 2*quat[3]*quat[0]; R13 = 2*quat[1]*quat[3] + 2*quat[2]*quat[0];
		R21 = 2*quat[1]*quat[2] + 2*quat[3]*quat[0]; R22 = 1-2*quat[1]*quat[1]-2*quat[3]*quat[3]; R23 = 2*quat[2]*quat[3] - 2*quat[1]*quat[0];
		R31 = 2*quat[1]*quat[3] - 2*quat[2]*quat[0]; R32 = 2*quat[2]*quat[3] + 2*quat[1]*quat[0]; R33 = 1-2*quat[1]*quat[1]-2*quat[2]*quat[2];
		
		savedata.rx=0;savedata.ry=0;savedata.rz=0;
		
		lat = mti[5];
		lon = mti[6];
		alt = mti[7];
		if (latini == 0) {latini=lat;lonini=lon;altini=alt;lat=0;lon=0;alt=0;} else {lat-=latini;lon-=lonini;alt-=altini;}
		printf("lat: %lf lon: %lf alt: %lf\n",lat,lon,alt);

    }
	//nb_testscan++;
}


#define FAC 0.5f
#define MAXDIF (1+FAC)
#define MINDIF (1-FAC)


void MainWindow::animate()
{
    animFlag=(animFlag+1)%2;
}
void MainWindow::scanGDHE()
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
	
	QString tpline_str;
	static QString tpline_str_pts;
	rgb color; color.r = 0; color.g = 200; color.b = (4*num_scans);

    if( port_open == true )
    {
        if( gdhe_open == true )
        {
			// Get range data
			getRange();

			/*Rx_xsens = 0; Ry_xsens = 0; Rz_xsens = 0;
            Rx_point = &Rx_xsens; Ry_point = &Ry_xsens; Rz_point = &Rz_xsens;*/

            // Rx = Rx_xsens*M_PI/180; Ry = Ry_xsens*M_PI/180; Rz = Rz_xsens*M_PI/180;
            // Calcul de la matrice de rotation World to Sensor
            /*double R[9]      = { cos(Ry)*cos(Rz), -cos(Rx)*sin(Rz)+cos(Rz)*sin(Rx)*sin(Ry),  sin(Rx)*sin(Rz)+cos(Rx)*cos(Rz)*sin(Ry),
			     cos(Ry)*sin(Rz),  cos(Rx)*cos(Rz)+sin(Rx)*sin(Ry)*sin(Rz), -cos(Rz)*sin(Rx)+cos(Rx)*sin(Ry)*sin(Rz),
			        -sin(Ry)    ,              cos(Ry)*sin(Rx)            ,              cos(Rx)*cos(Ry)            };*/
			
            // Draw xsens reference frame in GDHE
            //eval_expression((char *)"set robots(IMU) { color 0 0 255; repere }");
			eval_expression((char *)"set robots(IMU) { truc }");
			//QString setIMU = "set pos(IMU) { 0 0 0 0 0 0 }";
            QString setIMU = "set pos(IMU) { ";
			//QTextStream(&setIMU) << "0 0 0 " << lat << " " << lon << " " << alt << " }";
			//ATTENTION: order of the three angles in following line was done arbitrarily
			QTextStream(&setIMU) << atan2(-R31,R11) << " " << asin(R21) << " " << atan2(-R23,R22) << " " << lat << " " << lon << " " << alt << " }";
			eval_expression(setIMU.toLatin1().data());
            printf("\nimu gdhe string: %s \n",setIMU.toLatin1().data());
			
			//delete old trajectory pline (unless this is the first scan)
			if (num_scans != 0){
				QString unsetScan = "unset robots(traj)";
				eval_expression(unsetScan.toLatin1().data());
			}
			//draw trajectory pline
			tpline_str = "set robots(traj";
			QTextStream(&tpline_str) << \
							") { color " << color.r << " " << color.g << " " << color.b << " ; polyline " << num_scans+1 << " " ;
			QTextStream(&tpline_str_pts) << lat << " " << lon << " " << alt << " ";
			tpline_str += tpline_str_pts;
			QTextStream(&tpline_str) << "}";
			eval_expression(tpline_str.toLatin1().data());
			//printf("tpline: %s\n", tpline_str.toLatin1().data());
			QString setScan = "set pos(traj) { 0 0 0 0 0 0 }";
			eval_expression(setScan.toLatin1().data());


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

                // calculate angle in degrees
                theta = -(135) + scanpt * (270.0/1080);
                //theta=-theta+180;
				
                // calcul of angle in radians
                thetarad = theta * M_PI/180.0;

                // calculate coordonees x, y et z dans le repere sensor
                x = cos(thetarad) * depth;
                y = sin(thetarad) * depth;
                z = 0;
				
				/*int R[9] = {0,1,0,
							0,0,1,
							1,0,0};*/
				//cheating
				z = x;			
				x = y;
				y = z;
							
				new_x = R11*x + R12*y + R13*z;
                new_y = R21*x + R22*y + R23*z;
                new_z = R31*x + R32*y + R33*z;
                //printf("new_x=%f , new_y=%f , new_z=%f \n",new_x,new_y,new_z);

				//translate
				new_x += lat;
                new_y += lon;
                new_z += alt;


				pline_from_pos(md, new_x, new_y, new_z);
				//pline_from_pos(md, new_x, new_y, num_scans);

                savedata.angle[scanpt] = thetarad;
				//savedata.data[scanpt] = data[scanpt];
                savedata.x_data[scanpt] = new_x;
                savedata.y_data[scanpt] = new_y;
            } //end forall scanpts
			
			//let pline_from_pos know we are done with scan line
			pline_from_pos(mod_end, 0, 0, 0);
			printf("pts too close: %d\n",tooclose);
					
            //search(savedata);
            nb_scan += NB_SCAN_INCR;
			num_scans++;

            } else {
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
    char nomfich[1000];
	char* junk;
	size_t junklength = 1000;
	
	//char * fileName = strdup(LOG_NAME);
    //sprintf(nomfich, "%s/scan%06d.txt", fileName.c_str(), nb_scan);
	sprintf(nomfich, "%s", MTI_LOG_NAME);
	printf("mti file:%s\n", nomfich);
	Fscanmti = fopen(nomfich, "rt");
	
	junk = (char *) malloc (junklength + 1);
	for (int i=0;i<4;i++) { 
		getline(&junk,&junklength,Fscanmti);
		printf("Junk %s",junk);
	}
	/*int res = fscanf(Fscanmti,"%s\n", junk);
	printf("Junk %s",junk);
	res = fscanf(Fscanmti,"%s\n", junk);
	printf("Junk %s",junk);
	res = fscanf(Fscanmti,"%s\n", junk);
	printf("Junk %s",junk);*/

    if( gdhe_open == false ) {
        button_open_gdhe->setText("close GDHE");
        gdhe_open=true;

        // Open GDHE
        //int res = system("gdhe &");
		//if (res == 0) {printf("gdhe started ok\n");}
        // Sleep to allow full opening of GDHE, then connect
        //sleep(3);

        int error = get_connection((char *)"localhost");
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
