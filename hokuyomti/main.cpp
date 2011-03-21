/*todo:

  mettre sur git ou cvs

  voir:  http://openquadrotor.org/ et papier GRZONKA...

Approximate Nearest Neighboor
http://www.cs.umd.edu/~mount/ANN/
./ann_sample  -d 2 -max 1000 -df ../sample/data.pts -qf ../sample/query.pts

voir http://openslam.org/
->coreslam

attention il y a pas mal de points à une distance 1 du capteur dans les logs (par ex: log/cap2/scan000000.txt )

faire un fichier de log de quelques acquisitions,
a traiter sans tracking
detection de cap d'apres la 1° sequence
affichage video avec cap estimé par les données

OK: afficher le modele initial et des traits vers les données associées

tenir compte des rotation tangage et roulis pour corriger les données avant estimation cap


implementer le tracking sur cap (en limitant la fenetre de recherche)

implementer recherche de voisinage avec tableau de liste chainée

probleme sur les sequences il y a 2 poteaux donc souvent il recale le cap a partir du mauvais poteau

  dans le projet, il y a 2 configurations hokuyo, attention, 1 seule permet de débugger.

 virer :/opt/local/lib de DYLDPATH dans le projet QT@

verifier qu'il y a bien un fclose par fopen
*/

/*
gdb
r
bt :pour voir l'etat quand ca a planté
*/

/*
faire le ulimit -n 10000
sinon il n'ouvre pas les fichiers log au dela de 254
*/

#include <stdio.h>
#include <stdlib.h>
#include "chokuyoplus.h"
#include "chokuyoprocess.h"

#define INCOPENCV
//#define RECORDPPM

#ifdef INCOPENCV
//#include <cv.h>
//#include <highgui.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
using namespace std;
#endif

#define WIDTH	(600)
#define HEIGHT	(600)
#define OUT_FILE_NAME	"./out.avi"
#define OUT_TEST_AVI	"./out-test.avi"
#define OUT_TEST_JPG	"./out-test%06d.jpg"
//#define LOG_NAME	"/Users/bvandepo/Desktop/hokuyoplus/hokuyomti/log/cap2"
//#define LOG_NAME	"/home/paul/Documents/LAAS/from_bertrand/qtcreator_projs/hokuyomti/log/cap2"
//#define LOG_NAME	"/home/paul/Documents/LAAS/from_bertrand/qtcreator_projs/hokuyomti/log/2010-06-14-22-54-34"
//#define LOG_NAME	"/home/paul/Documents/LAAS/from_bertrand/qtcreator_projs/hokuyomti/log/2010-06-14-22-58-56"
//#define LOG_NAME	"/home/paul/Documents/LAAS/from_bertrand/qtcreator_projs/hokuyomti/log/2010-06-14-23-02-25"

#define LOG_NAME	"/home/paul/Documents/LAAS/from_bertrand/qtcreator_projs/hokuyomti/log/2011-06-14-22-54-34"


#define HOKUYO_DEV	(char*)"/dev/ttyACM0"
#define SENS_VIRT	TRUE

CHokuyoPlus *HokuyoSensor;
CHokuyoProcess *HokuyoProcess ;


 char * directorylog()
{
    char commandline[1000];
    char dateline[1000];
    static  char BasenameLog[1000];
       char BasenameInit[1000]       ="/home/paul/Documents/LAAS/laserhawk/hokuyomti/log/";

    time_t t;
    time(&t);
    tm * tmm;
    tmm=localtime(&t);

    //char =ctime(&t);
    printf("il est %04d:%02d:%02d  %02d:%02d:%02d",tmm->tm_year-110+2010,tmm->tm_mon+1,tmm->tm_mday,tmm->tm_hour,tmm->tm_min,tmm->tm_sec);
    sprintf(dateline,"%04d-%02d-%02d-%02d-%02d-%02d",tmm->tm_year-110+2010,tmm->tm_mon+1,tmm->tm_mday,tmm->tm_hour,tmm->tm_min,tmm->tm_sec);
    //    int ret=gettimeofday(&timenow,NULL);
    sprintf(BasenameLog,"%s/%s",BasenameInit,dateline);
    sprintf(commandline,"mkdir %s",BasenameInit);
    system(commandline);
    sprintf(commandline,"mkdir %s",BasenameLog);
    system(commandline);
   // nblog=0;
   // nbinsidelog=0;
    //LogFile=NULL;
    return BasenameLog;
}

void TestAviRecord(void)
{
    //test enregistrement video
    {
        printf("test enregistrement video\n");
        cv::Mat *Img;
        cv::VideoWriter *Video;
		unsigned char * im;
        im = (unsigned char *)malloc(sizeof(unsigned char )*WIDTH*HEIGHT*3);
        Img = new cv::Mat(HEIGHT, WIDTH, CV_8UC3, (char *)im);

        //system("rm out-test.avi");
        //Video=new cv::VideoWriter("./out-test.avi",CV_FOURCC('D','I','V','X') ,15,cv::Size(width,height),true);
        Video = new cv::VideoWriter(OUT_TEST_AVI, CV_FOURCC('U','2','6','3') , 15, cv::Size(WIDTH, HEIGHT), true);

        for (int j=0 ; j<WIDTH*HEIGHT ; j++)
            im[j] = 0;
	    
        for (int k=0 ; k<10 ; k++) {
            char text[100];
            sprintf(text, "salut %03d", k);
            for (int j=0 ; j<600*k*5*3 ; j++)
                im[j] = 255; 
            //cv::putText(*Img, text,cv::Point(10,550),CV_FONT_HERSHEY_COMPLEX,1,0x00FF0000);
            //http://opencv.willowgarage.com/documentation/cpp/basic_structures.html?highlight=type#Mat::type
            char namejpg[1000];
            sprintf(namejpg, OUT_TEST_JPG, k);
            printf("saving %s\n", namejpg);
            cv::imwrite(namejpg, *Img);
            Video->operator << (*Img);
        }
        delete(Img);
        delete Video;
        free(im);
        exit(0);
    }
}
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    bool DisplayBitmap = false;

    //CvMat* cvCreateMat(int rows, int cols, int type);
    //CvMat* M = cvCreateMat(4,4,CV_32FC1);

    printf("\nHokuyo Good v 0.47\n");
    //TestAviRecord();

#ifdef INCOPENCV
    printf("OPENCV\n");
    cv::VideoWriter *Video;
    Video = new cv::VideoWriter(OUT_FILE_NAME, CV_FOURCC('D','I','V','X'), 15, cv::Size(WIDTH,HEIGHT), true);
    printf("fin ouverture fichier video\n");
#endif

	system("rm LOGCAP.txt ");
	
    if (argc == 1) {

        HokuyoSensor = new CHokuyoPlus(HOKUYO_DEV);

        char * chaineNomLog = strdup(LOG_NAME);   //for replay of a recorded sequence
        int sizesequence = 2;  // to eventually limit  the size of the sequence to process

        HokuyoSensor->setReplayFileName(chaineNomLog);

#ifdef SENS_VIRT
        HokuyoSensor->setVirtualSensor();
#else
        HokuyoSensor->setRealSensor();
#endif
        HokuyoSensor->startSaveReplay();
        HokuyoSensor->stopSaveReplay();
        HokuyoSensor->open_port();
		//what is the 600 and 20?
        HokuyoProcess = new CHokuyoProcess(HokuyoSensor, 600, 20);

        int limite = 200;
        int borneimin = limite;
        int borneimax = 1080-limite;
        int incrementi = 1;
		
        for (int ii=0 ; ii<sizesequence ; ii++) {
            //HokuyoProcess-> EraseBitmap();
            bool WorldCoordinate = true;

            HokuyoProcess-> EraseBitmap();
            if (!WorldCoordinate) {
                HokuyoProcess->GenerateFrameBitmap();
                HokuyoProcess->GenerateDataBitmap();
            }
            HokuyoSensor->getRange();
			
            // int lim=50;
            //  int ipole=HokuyoProcess->FindPole(lim,1080-lim,1,lim,false,true,true);

            borneimin = limite;
            int borneimax = 1080-limite;
            int ipole;
            incrementi = 1;

            if (!WorldCoordinate) {
                ipole = HokuyoProcess->FindPole(borneimin, borneimax, incrementi, limite ,false,true,false);
            } else {
                ipole = HokuyoProcess->FindPole(borneimin, borneimax, incrementi, limite ,false,false,false);

                float phi;
                float psi;
                float theta;
                float psiCap;
                int bornetmin = 0;
                int bornetmax = 360;
                int incrementt = 1 ;
                FLOATHOKUYO scoret;
                FLOATHOKUYO scoretbest = 1e37;
                int capbest = 0;
                int cap = 0;

                if (ii == 0)
                    cap = HokuyoProcess->FindCap(bornetmin, bornetmax, incrementt, &scoret);
                else
                    for (bornetmin=0 ; bornetmin<360 ; bornetmin++)
                    //for (bornetmin=150;bornetmin<180;bornetmin++)
		    //for (bornetmin=167;bornetmin<168;bornetmin++)
                    {

                    printf("trying value t:%d\n",bornetmin);
                    bornetmax = bornetmin + 1;

                    cap = HokuyoProcess->FindCap(bornetmin, bornetmax, incrementt ,&scoret);
                    psiCap = cap *M_PI/180.;  // in radians
                    phi = 0;
                    psi = 0;
                    theta = 0;

                    printf("phi: %f psi: %f theta: %f\n psiCap: %f", phi, psi, theta, psiCap);
                    psi = psiCap;
		    
                    //for a given value of tested t
                    HokuyoProcess-> EraseBitmap();
                    HokuyoProcess->DisplayLoca(-psi, ipole, HokuyoProcess->HokuyoSensor->data.depth[ipole], borneimin, borneimax);
		    
                    cv::Mat *Img;
                    Img = new cv::Mat(HEIGHT, WIDTH, CV_8UC3, (char *)HokuyoProcess->im_ray->data);
                    //  http://opencv.willowgarage.com/documentation/cpp/drawing_functions.html?highlight=text#getTextSize
                    char chainescore[100];
		    
                    sprintf(chainescore,"%03d: %16.2f",cap, scoret);
                    string text = chainescore;
                    cv::putText(*Img, text,cv::Point(10,580),CV_FONT_HERSHEY_COMPLEX,1,0x00);
                    
		    if (scoret<scoretbest) {
                        capbest = cap;
                        scoretbest = scoret;
                    }
                    sprintf(chainescore,"%03d: %16.2f",capbest, scoretbest);
                    text = chainescore;
                    cv::putText(*Img, text, cv::Point(10,550), CV_FONT_HERSHEY_COMPLEX, 1, 0x00FF0000);
                    if (0) {
                        char namejpg[1000];
                        sprintf(namejpg,"./out%06d-%03d.jpg",ii,bornetmin);
                        printf("saving %s\n",namejpg);
                        cv::imwrite(namejpg,*Img);
                    }
                    Video->operator <<(*Img);
                    delete(Img);
                }

                //encodage avi
                /*
                char chaine[1000];
                sprintf(chaine,"mencoder \"mf://./out*.jpg\" -mf w=600:h=600:fps=15:type=jpg -ovc lavc  -o outputcap.avi");
                system(chaine);
                */
                //   mencoder "mf://./out*.jpg" -mf w=600:h=600:fps=15:type=jpg -ovc lavc  -o outputcap.avi

                //for a given set of laser data
                HokuyoProcess->DisplayLoca(-psi,ipole,HokuyoProcess->HokuyoSensor->data.depth[ipole],borneimin, borneimax);
                HokuyoProcess->AddTrajPoint();
                HokuyoProcess->DrawTraj();
            }

            /*
            depth=1.000m =limite =250
            depth=10.0000m= limite =30
            */

            /*
            float depth=HokuyoProcess->HokuyoSensor->data.depth[ipole];
            limite=250-(depth-1000)*220/3000.;
            if (depth<1000)
                limite=250;
            if (depth>3000)
                limite=30;
*/
            //track from ipole
            borneimin = ipole - limite/2;
            borneimax = ipole + limite/2;

            printf("n: %6d , pole at i:%4d , depth: %6d mm \n", HokuyoProcess->HokuyoSensor->nb_scan, ipole, HokuyoProcess->HokuyoSensor->data.depth[ipole]);
            //printf("depth= %f limite: %d\n",depth,limite);

#ifdef RECORDPPM
            printf("RECORD PPM\n");
            char nombitmap[1000];
            sprintf(nombitmap, "../data/bitmap%06d.ppm", ii);
            HokuyoProcess-> SaveBitmap(nombitmap);
            sprintf(chaine, "convert %s %s.jpg", nombitmap, nombitmap);
            system(chaine);
            sprintf(chaine, "rm %s ", nombitmap);
            system(chaine);
#endif

#ifdef INCOPENCV
            cv::Mat *Img;
            //Img=new cv::Mat(height,width,CV_8UC3);
            Img = new cv::Mat(HEIGHT, WIDTH, CV_8UC3, (char *)HokuyoProcess->im_ray->data);
            //pour generer une image aleatoire
            //     randu(*Img, cv::Scalar(0), cv::Scalar(256));
            //pour sauver en jpg
            //cv::imwrite("./out.jpg",*Img);

            // http://opencv.willowgarage.com/documentation/cpp/basic_structures.html?highlight=type#Mat::type
            Video->operator <<(*Img);
            char namejpg[1000];
            sprintf(namejpg,"./out%06d.jpg",ii);
            cv::imwrite(namejpg,*Img);
            delete(Img);
            // ImgTRAJ=new cv::Mat(height,width,CV_8UC3,( char *)HokuyoProcess->im_traj->data);
            //Videotraj->operator <<(*ImgTRAJ);
            //printf("img depth: %d img width: %d  img height: %d\n", img->depth, img->width, img->height);
            /*   for(i=0;i<nFrames;i++){
              cvGrabFrame(capture);          // capture a frame
              img=cvRetrieveFrame(capture);  // retrieve the captured frame
              cvWriteFrame(writer,img);      // add the frame to the file
            }*/
#endif
        }
#ifdef INCOPENCV
        printf("OPENCV\n");
        delete Video;
        // delete Videotraj;
#endif        
        //    save_ppm("traj.ppm", HokuyoProcess->im_traj);
        cv::Mat *ImgT;
        ImgT = new cv::Mat(HokuyoProcess->im_traj->yd, HokuyoProcess->im_traj->xd, CV_8UC3, (char *)HokuyoProcess->im_traj->data);
        cv::imwrite("./traj.jpg", *ImgT);
        char chaine[1000];
        sprintf(chaine,"mencoder \"mf://../data/bitmap*.jpg\" -mf w=600:h=600:fps=15:type=jpg -ovc lavc  -o output.avi");
        //      system(chaine);
        //  mencoder "mf://../data/bitmap*.bmp" -mf w=600:h=600:fps=15:type=bmp -ovc lavc  -o output.avi
        delete(HokuyoProcess);
        //   system("source ~/.bash_profile ;  display ../data/bitmap.ppm &");
        //system("display ../data/bitmap.ppm");
	//////////////////////////////////////////////////////////////////////////////////////////////////
    } else {
        //execution sur gumstix ou en continu sur pc : " ./hokuyomti /dev/ttyACM0 "
        printf("Usage: hokuyomti : test for the pole detector only on pc\n");
        printf("hokuyomti hokuyodevice serialdevice\n \
               ex for gumstix: hokuyomti /dev/ttyACM0 /dev/ttyS0\n \
               ex for pc: hokuyomti cu.usbmodem621 /dev/cu.usbserial-00002006");
				
        HokuyoSensor = new CHokuyoPlus(argv[1]);
        HokuyoSensor->setReplayFileName(directorylog());
        HokuyoSensor->setVirtualSensor();
        HokuyoSensor->setRealSensor();
        HokuyoSensor->startSaveReplay();
        //HokuyoSensor->stopSaveReplay();
        HokuyoSensor->open_port();
        HokuyoProcess = new CHokuyoProcess(HokuyoSensor,1000,5);
        int limite = 50;
        int borneimin = limite;
        int borneimax = 1080-limite;
        int incrementi = 1;
        int ipole;
        float thetapole, distpole;
		
        while(1) {
            if (DisplayBitmap)
            {
                HokuyoProcess-> EraseBitmap();
                HokuyoProcess->GenerateFrameBitmap();
                HokuyoProcess->GenerateDataBitmap();
            }
            //attendre reception de la derniere trame tiny pour connaître l'orientation du hokuyo
            float theta, phi, psi;
            //get attitude from MTI
            //PaparazziComm->GetAttitudeFromTiny(&theta,&phi,&psi); //bloquante
            //printf("\nadding log tiny\n");

            //PaparazziComm->AddLogTiny(theta,  phi, psi, HokuyoProcess->HokuyoSensor->nb_scan);

            HokuyoSensor->getRange();
			
         //   ipole = HokuyoProcess->FindPole(borneimin, borneimax, incrementi, limite , false, DisplayBitmap, false);
           ipole=500;


            printf("n: %6d , pole at i:%4d , depth: %6d mm \n",HokuyoProcess->HokuyoSensor->nb_scan,ipole,HokuyoProcess->HokuyoSensor->data.depth[ipole]);
            
            thetapole = -135 + (ipole*270./1080);
            distpole = HokuyoProcess->HokuyoSensor->data.depth[ipole]/1000.;

            //envoie à la tiny de la position detectée pour le poteau...
            //PaparazziComm->SendPolePositionToTiny(thetapole, distpole); //bloquante

            //status=0 pas de position de poteau à interpreter
            //status=1 position du poteau 1 à l'initialisation
            //status=2 position du poteau 2 à l'initialisation
            //status=3 position du poteau 1 à l'initialisation

            //track from ipole
            borneimin = ipole - limite/2;
            borneimax = ipole + limite/2;

            if (DisplayBitmap) {
                HokuyoProcess-> SaveBitmap((char*)"../data/bitmap.ppm");
            }
        }
        delete (HokuyoProcess);
    }
    /*
	for (int i=0;i<1080;i++)
	{
		printf("%d: %d\n",i,HokuyoSensor->data.depth[i]);
	}
	*/
    HokuyoSensor->close_port();
    delete(HokuyoSensor);
    printf("fini\n");
}
