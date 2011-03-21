#include "cmtilog.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMtiLog::CMtiLog(char * name)
{
    Fscanmti = NULL;
    latini=0;
    lonini=0;
    altini=0;

    char nomfich[1000];
    char* junk;
    size_t junklength = 1000;

    //char * fileName = strdup(LOG_NAME);
    //sprintf(nomfich, "%s/scan%06d.txt", fileName.c_str(), nb_scan);
    sprintf(nomfich, "%s", name);
    printf("mti file:%s\n", nomfich);
    Fscanmti = fopen(nomfich, "rt");

    junk = (char *) malloc (junklength + 1);
    for (int i=0;i<4;i++)
    {
        getline(&junk,&junklength,Fscanmti);
        printf("Junk %s",junk);
    }
    /*int res = fscanf(Fscanmti,"%s\n", junk);
        printf("Junk %s",junk);
        res = fscanf(Fscanmti,"%s\n", junk);
        printf("Junk %s",junk);
        res = fscanf(Fscanmti,"%s\n", junk);
        printf("Junk %s",junk);*/

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMtiLog::~CMtiLog()
{
    fclose(Fscanmti);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMtiLog::Get(double timedemande)
{

    double mti[11] = {0};
    int mtic = 0;
    char mtiz;
    double mtitime;
    //1297965927.462918758 QUAT  0.072093  0.746111  0.661814 -0.011095 POS 377098.986 4824479.869    200.101  31T VEL   -0.0100    0.0200    0.0100
    char* mtiline;
    size_t mtilinelength = 1000;

    mtiline = (char *) malloc (mtilinelength + 1);
    //scan through mti log and find corresponding entry by looking for next closest timestamp (not necessarily closest)
    do {
        //char trash[1000];
        getline(&mtiline,&mtilinelength,Fscanmti);
        //printf("MTI scan :  %s\n",mtiline);
        int res = sscanf(mtiline,"%lf QUAT  %lf  %lf  %lf %lf POS %lf %lf    %lf  %2d%c VEL   %lf    %lf    %lf\n", 					&mti[0],&mti[1],&mti[2],&mti[3],&mti[4],&mti[5],&mti[6],&mti[7],&mtic,&mtiz,&mti[8],&mti[9],&mti[10]);
        //	int res = sscanf(mtiline,"%Lf %s  %lf  %lf  %lf %lf %s %lf %lf    %lf  %2d%c %s   %lf    %lf    %lf\n", 	&mti[0],trash,&mti[1],&mti[2],&mti[3],&mti[4],trash,&mti[5],&mti[6],&mti[7],&mtic,&mtiz,trash,&mti[8],&mti[9],&mti[10]);
        //printf("fscanf RETURNED %d\n",res);
        mtitime = mti[0];
        //printf("mti line time %lf\n",mtitime);
    } while (mtitime < timedemande);

    printf("mti line time %lf\n",mtitime);

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

    // savedata.rx=0;savedata.ry=0;savedata.rz=0;

    lat = mti[5];
    lon = mti[6];
    alt = mti[7];
    if (latini == 0) {latini=lat;lonini=lon;altini=alt;lat=0;lon=0;alt=0;} else {lat-=latini;lon-=lonini;alt-=altini;}
    printf("lat: %lf lon: %lf alt: %lf\n",lat,lon,alt);


}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

