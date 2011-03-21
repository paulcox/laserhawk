#include "chokuyoplus.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHokuyoPlus::CHokuyoPlus(char * deviceNameInit)
{
    sprintf(deviceName,"%s",deviceNameInit);
    //deviceName = strdup(deviceNameInit);

    data.angle = (FLOATHOKUYO*)malloc(1080*sizeof(FLOATHOKUYO));  //TODO: IL y a 1081 points à traiter!!!!
    data.depth = (uint32_t*)malloc(1080*sizeof(uint32_t));
    data.x_data = (FLOATHOKUYO*)malloc(1080*sizeof(FLOATHOKUYO));
    data.y_data = (FLOATHOKUYO*)malloc(1080*sizeof(FLOATHOKUYO));

    nb_scan = 0;
    stopSaveReplay();
    port_open = false;
    setVirtualSensor(); //par défaut, capteur virtuel
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHokuyoPlus::~CHokuyoPlus()

{
    free(data.angle);
    free(data.depth);
    free(data.x_data);
    free(data.y_data);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoPlus::setRealSensor()
{
    VirtualHokuyo = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoPlus::setVirtualSensor()
{
    VirtualHokuyo = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoPlus::open_port()
{
    char chaineinit[1000];

    if ( (VirtualHokuyo == 0) && (port_open == false) ) {
        sprintf( chaineinit,"type=serial, device=%s, timeout=1", deviceName);
        string portOptions =  (string)chaineinit;

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
            // Set the laser to verbose mode (    if (VirtualHokuyo==0)so we see more information in the console)
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
                printf("Exception What() : Failed to open device '%s' levée\n",deviceName);
                //    label1->setText("Hokuyo port is closed\nUSB port of the Hokuyo is not connected");
                return;
            }
            try
            {    if (VirtualHokuyo == 0)
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
            cout << info.AsString ();
            // FILE *F;
            // F=fopen("out.xyz","wt");

            // Mise a jour du texte dans le Label
            /*    QString hokuyo_info;
            QTextStream(&hokuyo_info) << "Laser sensor information:\n" << info.AsString().c_str();

            label1->setText(hokuyo_info);
            */
        }
        catch (hokuyo_aist::HokuyoError e)
        {
            cerr << "Caught exception: (" << e.Code () << ") " << e.what () << endl;
        }
        port_open = true;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoPlus::close_port()
{
    if ( (VirtualHokuyo==0) && (port_open == true) )
    {
        laser.Close ();

        //maj des flags
        port_open = false;
        //view_image=false;

        //maj de l'affichage dans la fenetre

        /*label_image_init =  new QLabel("fenetre");
        QImage image("enac.png");
        label_image_init->setPixmap(QPixmap::fromImage(image));

        depthImage_init = new QScrollArea;
        depthImage_init->setBackgroundRole(QPalette::Dark);
        depthImage_init->setWidget(label_image_init);

        layout_G->addWidget(depthImage_init, 0, 4, 3, 3);
*/
    }
    //  label1->setText("Hokuyo port is closed \n");
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoPlus::setReplayFileName(string fileNameInit)
{
    fileName = fileNameInit;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoPlus::startSaveReplay()
{
    if (VirtualHokuyo == 0)
        saveReplay = 1;
    else
        saveReplay = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoPlus::stopSaveReplay()
{
    saveReplay=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CHokuyoPlus::seekReplay(int new_nb_scan)
{
    nb_scan=new_nb_scan;
    if (nb_scan<0)
        nb_scan=0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CHokuyoPlus::getRange()
{
    char nomfich[1000];
    FILE *Fscan = NULL;
    //FLOATHOKUYO Rx, Ry, Rz;
    FLOATHOKUYO theta,thetarad;
    FLOATHOKUYO x,y,z;
    //FLOATHOKUYO  new_x, new_y, new_z;
    uint32_t depth;

    if (VirtualHokuyo == 1) // read from the file virtual sensor
    {
        sprintf(nomfich, "%s/scan%06d.txt", fileName.c_str(), nb_scan);
        printf("fichier:%s\n", nomfich);
        Fscan = fopen(nomfich, "rt");

        int var[2] = {0};
        FLOATHOKUYO theta,thetarad;
        FLOATHOKUYO x,y;

        if(Fscan != NULL)
        {
            for(int i=0 ; i<1080 ; i++)  //////////////// TODO, en fait il y a 1081 points de 0 à 1080 compris, mais ca va poser probleme pour les fichiers de logs déjà créés...
            {
                fscanf(Fscan,"%d       %d\n", &var[0], &var[1]);
                //printf("var0=%d , var1=%d \n",var[0],var[1]);

                // calcul : angle , distance, profondeur

                // calcul de l'angle
                theta = -(135) + i*(270.0/1081);   //DONE : correction
                thetarad = theta * M_PI/180.0;

                // calcul des coordonnées x, y dans le repère sensor
                x = cos(thetarad) * var[1];
                y = sin(thetarad) * var[1];

                data.depth[i] = var[1];
                data.angle[i] = thetarad;
                data.x_data[i] = x;
                data.y_data[i] = y;
                //printf("i=%d data=%d \n", i, var[1]);
            }

            //read last line of scan file which gives us time of acquisition
            fscanf(Fscan,"%lf\n", &hokuyotime);
            // printf("hokuyotime: %lf \n",hokuyotime);

            fclose(Fscan);


            //sauve en binaire
            char Fname[100];
            sprintf(Fname,"./zip/scanbin%06d",nb_scan);
            uint16_t tabdist[1081];
            for (int j=0;j<1080;j++)
                tabdist[j]=data.depth[j];
            FILE *Fbin=fopen(Fname,"wb");
            fwrite(tabdist,sizeof(tabdist[0]),1080,Fbin);
            fclose(Fbin);
            char cmd[100];

            sprintf(cmd,"gzip ./zip/scanbin%06d",nb_scan);
            system(cmd);

           // sprintf(cmd,"scp ./zip/scanbin%06d bvandepo@borderouge.laas.fr:~/zip/",nb_scan);

            //il faut avoir les droits sur /dev/ttyS0
            sprintf(cmd,"cat ./zip/scanbin%06d.gz >/dev/ttyS0",nb_scan);
            system(cmd);

            //exit(0);


        }
      //  nb_testscan++;   //removed
    } 
    else
    {
	//VirtualHokuyo==0; read from the real sensor
        if(port_open == false)
        {
            //le port n'est pas ouvert
            //label1->setText("Hokuyo port is closed\n");
        } else {
            //port_open is true
            if (saveReplay)
            {
                sprintf(nomfich, "%s/scan%06d.txt", fileName.c_str(), nb_scan);
                Fscan = fopen(nomfich,"wt");
                printf("\n\n\n%s\n\n",nomfich);
                //  exit(0);
            }


            timespec  __tp;
            int toto=clock_gettime (CLOCK_REALTIME,&__tp);
            /*    struct timespec
                  {
                    __time_t tv_sec;		/* Seconds.  */
                 //   long int tv_nsec;		/* Nanoseconds.  */




            // Get range data
            hokuyo_aist::HokuyoData hokuyo_aistdata;
            if ((firstStep == -1 && lastStep == -1) &&
                (startAngle == 0.0 && endAngle == 0.0))
            {
                // Get all ranges
                if (getNew)
                    laser.GetNewRanges (&hokuyo_aistdata, -1, -1, clusterCount);
                else if (getIntensities)
                    laser.GetNewRangesAndIntensities (&hokuyo_aistdata, -1, -1, clusterCount);
                else
                    laser.GetRanges (&hokuyo_aistdata, -1, -1, clusterCount);
            }
            else if (firstStep != -1 || lastStep != -1)
            {
                // Get by step
                if (getNew)
                    laser.GetNewRanges (&hokuyo_aistdata, firstStep, lastStep, clusterCount);
                else if (getIntensities)
                    laser.GetNewRangesAndIntensities (&hokuyo_aistdata, firstStep, lastStep, clusterCount);
                else
                    laser.GetRanges (&hokuyo_aistdata, firstStep, lastStep, clusterCount);
            }
            else
            {
                // Get by angle
                if (getNew)
                    laser.GetNewRangesByAngle (&hokuyo_aistdata, startAngle, endAngle, clusterCount);
                else if (getIntensities)
                    laser.GetNewRangesAndIntensitiesByAngle (&hokuyo_aistdata, startAngle, endAngle, clusterCount);
                else
                    laser.GetRangesByAngle (&hokuyo_aistdata, startAngle, endAngle, clusterCount);
            }
            /*
        }
		#ifdef save_scan
		#endif

        QString scan_image = "base_scan/im_ray";
        if(nb_scan<10)
            QTextStream(&scan_image) << "0";
        QTextStream(&scan_image) << nb_scan << ".ppm";
        QByteArray text_scan_image = scan_image.toLatin1();
        char *textimage = text_scan_image.data();

        save_ppm(textimage,im_ray);

       // stat= system("convert im_ray.ppm im_ray.png");
        //printf("stat:%d\n",stat);
       // stat= system("open im_ray.png");
        //printf("stat:%d\n",stat);

        nb_scan++;
        // maj de l'affichage dans la fenetre

        QImage image("im_ray.png");

        image.scaled(10,10,Qt::KeepAspectRatioByExpanding);
        label_image->setPixmap(QPixmap::fromImage(image));

        if(view_image==false)
        {
            depthImage = new QScrollArea;
            depthImage->setBackgroundRole(QPalette::Dark);
            depthImage->setWidget(label_image);
            //layout_G->removeWidget(depthImage_init);
            layout_G->addWidget(depthImage, 0, 4, 3, 3);
            view_image=true;
        }
		*/

            // pour chaque point du scan
            for (int i=0 ; i<1080 ; i++)
            {
                // get data of hokuyo
                depth = hokuyo_aistdata[i]; // en mm
                //depth=1000;

                // clacul of angle in degree
                theta = -(135) + i*(270.0/1081);
                //theta=-theta+180;
                // calcul of angle in radian
                thetarad = theta * M_PI/180.0;

                // calcul des coordonnées x, y et z dans le repère sensor
                x = cos(thetarad)*depth;
                y = sin(thetarad)*depth;
                z = 0;

                // calcul des coordonnées x, y et z dans le repère worl
                /*new_x = R[0]*x+R[1]*y+R[2]*z;
                    new_y = R[3]*x+R[4]*y+R[5]*z;
                    new_z = R[6]*x+R[7]*y+R[8]*z;
*/

                //enregistre les valeurs pour la polyligne
                //QTextStream(&createScan) << new_x/100 << " " << new_y/100 << " " << new_z/100 << " ";
                //affichage des valeurs dans le repère World
                //printf("new_x=%f , new_y=%f , new_z=%f \n",new_x,new_y,new_z);

                data.angle[i] = thetarad;
                data.depth[i] = depth;
                data.x_data[i] = x;
                data.y_data[i] = y;
                if (saveReplay)
                {
                    fprintf(Fscan,"%04d       %u \n",i,depth);
                }
            }
            hokuyotime=__tp.tv_sec+__tp.tv_nsec*1e-9;
            if (saveReplay)
            {
                fprintf( Fscan, "%lf\n",hokuyotime);
                fclose(Fscan);
            }
        }
        }
        nb_scan++;
    }
