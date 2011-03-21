#include "chokuyoprocess.h"

CHokuyoProcess::CHokuyoProcess(CHokuyoPlus * HokuyoSensorInit, int sizeimagexyInit, int scaleImageInit) {
    col1.r = 255; col1.g = 0  ; col1.b = 0;
    col2.r = 0  ; col2.g = 255; col2.b = 0;
    col3.r = 0  ; col3.g = 0  ; col3.b = 255;
    col4.r = 0  ; col4.g = 0  ; col4.b = 0;
    col5.r = 255; col5.g = 255; col5.b = 0  ;
    col6.r = 255; col6.g = 255; col6.b = 255;
    col7.r = 168; col7.g = 168; col7.b = 168;

    scaleImage = scaleImageInit;
    sizeimagexy = sizeimagexyInit;

    im_ray = alloc_im_color(sizeimagexy, sizeimagexy);
    HokuyoSensor = HokuyoSensorInit;

    scaleImagetraj = scaleImageInit;
    sizeimagetraj = 600;
    firstframe = true;
    im_traj = alloc_im_color(sizeimagetraj,sizeimagetraj);
    //memset((void*)im_traj->data[0].r,0,sizeimagetraj*sizeimagetraj);

    erase_ppm(im_traj,&col6);

    nb_traj_max = 10000;
    x_traj = (int*)malloc(nb_traj_max*sizeof(int));
    y_traj = (int*)malloc(nb_traj_max*sizeof(int));
    nb_traj = 0;

    DistThreshold = 100000;

    nbsect = 180; // 2° resolutions
    nbcyl = 100; //up to 30m, deal with saturation for the last cylinder to avoid get  out the array

    /*
    nbsect=45; // 8° resolutions
    nbcyl=25; //up to 30m, deal with saturation for the last cylinder to avoid get  out the array
    */

    //10000 à 100000 fonctionne bien comme seuil pour la saturation du m estimateur
    //correspond à distances cartesiennes de 100 à 300
    //        --> taille de cylindre= 300mm -> 100 cylindres de 300mm
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoProcess::AddTrajPoint()
{

    if (nb_traj < nb_traj_max)
    {
        x_traj[nb_traj] = x_loca;
        y_traj[nb_traj] = y_loca;
        nb_traj++;
    }
}
void CHokuyoProcess::DrawTraj()
{
    /*    if (firstframe)
    {
        firstframe=false;
        //  set_color_pixel_with_boundaries(im_traj,x_loca,y_loca,&col1) ;
        set_color_pixel_with_boundaries(im_traj,
                                        (sizeimagetraj/2)-y_loca/scaleImagetraj,
                                        (sizeimagetraj/2)-x_loca/scaleImagetraj,
                                        &col1) ;
    }
    else*/
    if (nb_traj > 1)
        for (int n=1 ; n<nb_traj ; n++) {
			int u1 = (sizeimagetraj/2) + y_traj[n-1]/scaleImagetraj;
			int v1 = (sizeimagetraj/2) + x_traj[n-1]/scaleImagetraj;
			int u2 = (sizeimagetraj/2) + y_traj[n]/scaleImagetraj;
			int v2 = (sizeimagetraj/2) + x_traj[n]/scaleImagetraj;
			//drawline2d(im_traj,u1,v1,u2,v2,&col2);
			drawline2d(im_ray, u1, v1, u2, v2, &col2);
			//printf("draw from (%d,%d) to (%d,%d)\n",u1,v1,u2,v2);
	}

    //drawline2d(im_traj,x_loca,y_loca,x_previous,y_previous,&col1);
    x_previous = x_loca;
    y_previous = y_loca;

}
///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////
CHokuyoProcess::~CHokuyoProcess()
{
    dealloc_im_color(im_ray);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoProcess::EraseBitmap()
{
    memset(im_ray->data, 255, 3*sizeimagexy*sizeimagexy);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoProcess::SaveBitmap(char * fileName)
{
    save_ppm(fileName,im_ray);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoProcess::GenerateFrameBitmap()
{
    //draw_circles(sizeimagexy,im_ray,col);
    //********************************//
	int cpt;
    int nb_ray = 10; // nombre de rayon

    for(cpt=1; cpt<=nb_ray ; cpt++)
    {
        //   TracerCercle(im_ray,(sizeimagexy/2), (sizeimagexy/2), cpt*100./scaleImage, &col1);
    }

    // Permet de dessiner les carrées verts représentants le repère du capteur
    int x_im = (sizeimagexy/2);
    int y_im = (sizeimagexy/2);
    //draw_square(im_ray,x_im,y_im,&col3,24/scaleImage);

    x_im = (sizeimagexy/2);
    y_im = (sizeimagexy/2) - (sizeimagexy/20)/scaleImage;
    //draw_square(im_ray,x_im,y_im,&col1,24/scaleImage);

    x_im = (sizeimagexy/2) - (sizeimagexy/20)/scaleImage;
    y_im = (sizeimagexy/2);
    //draw_square(im_ray,x_im,y_im,&col2,24/scaleImage);
}


///////////////////////////////////////////////////////////////////////////////////////////////
void CHokuyoProcess::GenerateDataBitmap()
{
    int x_im;
    int y_im;
    FLOATHOKUYO x,y;
    // Permet de dessiner chaques points de données
    for (int i=0 ; i<1080 ; i++)
    {
        //depth=this->HokuyoSensor  */->data.depth[i]; // en mm
        //thetarad=this->HokuyoSensor->data.angle[i];
        x = this->HokuyoSensor->data.x_data[i];
        y = this->HokuyoSensor->data.y_data[i];
        x_im = (sizeimagexy/2) - (y/scaleImage);
        y_im = (sizeimagexy/2) - (x/scaleImage);
        draw_square(im_ray, x_im, y_im, &col2, 1);
    }
}

////////////////////////////////////////////////////////////////////////
void PolartoCart(FLOATHOKUYO  a, FLOATHOKUYO r, FLOATHOKUYO *x, FLOATHOKUYO *y)
{
    *x = r*cos(a);
    *y = r*sin(a);
}
////////////////////////////////////////////////////////////////////////
void CartToPolar(FLOATHOKUYO * a, FLOATHOKUYO *r, FLOATHOKUYO x, FLOATHOKUYO y)
{
    *r = sqrt((x*x) +(y*y));
    *a = atan2(y,x);
}


////////////////////////////////////////////////////////////////////////
bool IntersectionRayonCercle(FLOATHOKUYO a, FLOATHOKUYO b, FLOATHOKUYO d, FLOATHOKUYO xc, FLOATHOKUYO yc, FLOATHOKUYO rc,
                             FLOATHOKUYO *xp1, FLOATHOKUYO *yp1, FLOATHOKUYO *xp2, FLOATHOKUYO *yp2)
{
    //renvoie true si le rayon défini par   ax+by=d intersecte le cercle de centre xc,yc et de rayon rc
    //dans ce cas, xp1,yp1 est l'intersection la plus proche de l'origine du repere, et xp2,yp2 la plus lointaine
    //renvoie false si il n'y a pas d'intersection

    FLOATHOKUYO A,B,C,D;
    FLOATHOKUYO n1,n2,temp;
    //FLOATHOKUYO x_im,y_im;


    if ( (b<1e-15) && (b>-1e-15) ) {
        //printf("cas particulier \n");
        //cas particulier a traiter à part...
        //  printf("a=%f , b=%f , d=%f , xc=%f , yc=%f , rc=%f \n",a,b,d,xc,yc,rc);
        A = 1;
        B = -2*yc;
        C = (-(rc*rc)) + (yc*yc) - (2*d*xc/a) + (xc*xc) + (d*d)/(a*a);
        D = B*B - (4*A*C);

        //  printf("A=%f , B=%f , C=%f , Delta=%f \n",A,B,C,D);

        if (D<1e-10) {
            //printf("cas particulier  -  sans intersection  \n");
            //pas d'intersection ou tangence
            return false;
        } else {
            //printf("cas particulier  -  avec intersection  \n");
            *yp1 = (-B-sqrt(D)) / (2*A);
            *xp1 = d/a;
            *yp2 = (-B+sqrt(D)) / (2*A);
            *xp2 = d/a;

            // FLOATHOKUYO ya=(-B-sqrt(D))/(2*A);
            // FLOATHOKUYO yb=(-B+sqrt(D))/(2*A);

            //printf("\n\r ya: %f\n\r",ya);
            //printf("\n\r yb: %f\n\r",yb);
            //printf("\n\r avant tri: xp1: %f, yp1: %f, xp2:%f yp2: %f\n\n ",*xp1,*yp1,*xp2,*yp2);
        }
    } else {
        //printf("cas normal \n");

        //printf("a=%f , b=%f , d=%f , xc=%f , yc=%f , rc=%f \n",a,b,d,xc,yc,rc);

        A = 1. + ((a*a)/(b*b));
        B = -(2*xc) - ((2*d*a)/(b*b)) + (2*a*yc/b);
        C = (xc*xc) + (yc*yc) - (2*d*yc/b) + ((d*d)/(b*b)) - (rc*rc);
        D = B*B - (4*A*C);
		
        //printf("A=%f , B=%f , C=%f , Delta=%f \n",A,B,C,D);

        if (D<1e-10) {
            //      printf("cas normal  -  sans intersection  \n");
            //pas d'intersection ou tangence
            return false;
        } else {
            //      printf("cas normal  -  avec intersection  \n");
            //calcul des intersections

            *xp1 = (-B-sqrt(D)) / (2*A);
            *yp1 = ((-a*(*xp1))+d)/b;

            *xp2 = (-B+sqrt(D))/(2*A);
            *yp2 = ((-a*(*xp2))+d)/b;
        }
    }
    n1 = ((*xp1) * (*xp1)) + ((*yp1) * (*yp1));
    n2 = ((*xp2) * (*xp2)) + ((*yp2) * (*yp2));

    //printf("\n\r avant tri: xp1: %f, yp1: %f, xp2:%f yp2: %f\n\n ",*xp1,*yp1,*xp2,*yp2);

	//il faut inverser l'ordre
    if (n2<n1) {
        temp = *xp1;
        *xp1 = *xp2;
        *xp2 = temp;
        temp = *yp1;
        *yp1 = *yp2;
        *yp2 = temp;
    }

    //printf("\n\r apres tri: xp1: %f, yp1: %f, xp2:%f yp2: %f\n\n ",*xp1,*yp1,*xp2,*yp2);
    //verification si xp1,yp1 appartient a droite et a cercle

    //FLOATHOKUYO erreurdroite=a* (*xp1) + b* (*yp1) -d;
    //FLOATHOKUYO erreurcercle=sqrt ( ( ( (*xp1)-xc) * ( (*xp1)-xc))  +  ( ( (*yp1)-yc) * ( (*yp1)-yc))) -rc;

    //printf("erreur p1: %f    ;   %f\n\r",   erreurdroite ,erreurcercle);

    //erreurdroite=a* (*xp2) + b* (*yp2) -d;
    //erreurcercle=sqrt ( ( ( (*xp2)-xc) * ( (*xp2)-xc))  +  ( ( (*yp2)-yc) * ( (*yp2)-yc))) -rc;
    //printf("erreur p2: %f    ;   %f\n\r",   erreurdroite ,erreurcercle);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool IntersectionRayonCercleOptimD0(FLOATHOKUYO a,FLOATHOKUYO b ,FLOATHOKUYO xc,FLOATHOKUYO yc,FLOATHOKUYO rc,
                                    FLOATHOKUYO *xp1,FLOATHOKUYO *yp1,FLOATHOKUYO *xp2,FLOATHOKUYO *yp2)
{
    //renvoie true si le rayon défini par   ax+by=d intersecte le cercle de centre xc,yc et de rayon rc
    //dans ce cas, xp1,yp1 est l'intersection la plus proche de l'origine du repere, et xp2,yp2 la plus lointaine
    //renvoie false si il n'y a pas d'intersection

    FLOATHOKUYO A,B,C,D;
    FLOATHOKUYO n1,n2,temp;
    FLOATHOKUYO rc2 = rc*rc;
    //FLOATHOKUYO x_im,y_im;

    if ( (b<1e-15) && (b>-1e-15) ) {
        //printf("cas particulier \n");
        //cas particulier a traiter à part...
        //  printf("a=%f , b=%f , d=%f , xc=%f , yc=%f , rc=%f \n",a,b,d,xc,yc,rc);
        // A=1;
        B = -2*yc;
        C = (-(rc2))+(yc*yc)+(xc*xc);
        D = B*B-(4*C);
        //  printf("A=%f , B=%f , C=%f , Delta=%f \n",A,B,C,D);
        if (D<1e-10) {
            //       printf("cas particulier  -  sans intersection  \n");
            //pas d'intersection ou tangence
            return false;
        } else {
            //     printf("cas particulier  -  avec intersection  \n");
            *yp1=(-B-sqrt(D))/(2);
            *xp1=0; //d/a;
            *yp2=(-B+sqrt(D))/(2);
            *xp2=0;  //d/a;
        }
    } else {
        //printf("cas normal \n");

        //   printf("a=%f , b=%f , d=%f , xc=%f , yc=%f , rc=%f \n",a,b,d,xc,yc,rc);
        FLOATHOKUYO a2 = a*a;
        FLOATHOKUYO b2 = b*b;
        A = 1. + ((a2)/(b2));
        B = 2*((-xc) + (a*yc/b));
        C = (xc*xc) + (yc*yc)-(rc2 );
        D = B*B - (4*A*C);

        //   printf("A=%f , B=%f , C=%f , Delta=%f \n",A,B,C,D);

        if (D<1e-10)
        {
            //      printf("cas normal  -  sans intersection  \n");
            //pas d'intersection ou tangence
            return false;
        }
        else
        {
            //      printf("cas normal  -  avec intersection  \n");
            //calcul des intersections
            float sqrtD=sqrt(D);
            *xp1 = (-B-sqrtD)/(2*A);
            *yp1 = ((-a* (*xp1) ))/b;

            *xp2 = (-B+sqrtD)/(2*A);
            *yp2 = ((-a* (*xp2) ))/b;
        }
    }
    n1 = ((*xp1) * (*xp1)) + ((*yp1) * (*yp1));
    n2 = ((*xp2) * (*xp2)) + ((*yp2) * (*yp2));

    //printf("\n\r avant tri: xp1: %f, yp1: %f, xp2:%f yp2: %f\n\n ",*xp1,*yp1,*xp2,*yp2);

    if (n2<n1)//il faut inverser l'ordre
    {
        temp = *xp1;
        *xp1 = *xp2;
        *xp2 = temp;
        temp = *yp1;
        *yp1 = *yp2;
        *yp2 = temp;
    }

    //printf("\n\r apres tri: xp1: %f, yp1: %f, xp2:%f yp2: %f\n\n ",*xp1,*yp1,*xp2,*yp2);
    //verification si xp1,yp1 appartient a droite et a cercle

    // FLOATHOKUYO erreurdroite=a* (*xp1) + b* (*yp1) -d;
    // FLOATHOKUYO erreurcercle=sqrt ( ( ( (*xp1)-xc) * ( (*xp1)-xc))  +  ( ( (*yp1)-yc) * ( (*yp1)-yc))) -rc;

    //   printf("erreur p1: %f    ;   %f\n\r",   erreurdroite ,erreurcercle);

    // erreurdroite=a* (*xp2) + b* (*yp2) -d;
    // erreurcercle=sqrt ( ( ( (*xp2)-xc) * ( (*xp2)-xc))  +  ( ( (*yp2)-yc) * ( (*yp2)-yc))) -rc;
    //printf("erreur p2: %f    ;   %f\n\r",   erreurdroite ,erreurcercle);

    return true;
}


void testIntersectionCercle()
{

    FLOATHOKUYO xp1,yp1,xp2,yp2;
    FLOATHOKUYO theta,thetarad;
    FLOATHOKUYO depth,rc,i,xc,yc,x,y;

    rc = 2;
    depth = 100;
    i = 536;


    theta = -135 + i*(270.0/1080.0);
    theta = 10;
    thetarad = theta * M_PI/180.0;

    x = cos(thetarad)*depth;
    y = sin(thetarad)*depth;

    xc = cos(thetarad)*(depth+rc);
    yc = sin(thetarad)*(depth+rc);

    printf("x=%f , y=%f , xc=%f , yc=%f , angle=%f _ %f , rc=%f \n",x,y,xc,yc,thetarad, theta,rc);

    IntersectionRayonCercle(sin(thetarad),-cos(thetarad),0,xc,yc,rc,&xp1,&yp1,&xp2,&yp2);

    printf("xp1=%f , yp1=%f , xp2=%f , yp2=%f \n",xp1,yp1,xp2,yp2);

    //IntersectionRayonCercle(0,1,2,5,2.5,1.5,&xp1,&yp1,&xp2,&yp2);
    //IntersectionRayonCercle(0,1,1,1,1,2,&xp1,&yp1,&xp2,&yp2);
    //IntersectionRayonCercle(0,1,2,1,1,1,&xp1,&yp1,&xp2,&yp2);
}

///////////////////////////////////////////////////////////////////////////////////////:
int  CHokuyoProcess::FindPole(int borneimin, int borneimax, int incrementi, int limite , 
								bool SaveFileResults, bool Display, bool Verbose)
{
    // declaration / initialisation
    FLOATHOKUYO rayon_cercle_1 = 150; // en millimetre
    FLOATHOKUYO rayon_cercle_2 = 1200; // en millimetre
    FLOATHOKUYO erreurn2;
	FLOATHOKUYO erreurC1[1080]; //somme des distances au carrés des points a c1
    int nbimpact[1080];
    int erreurC2[1080];  //nombre de points dans C2 et pas dans C1 étendu (r*1.33)
    int x_im = 0;
    int y_im = 0;
    int x_im2 = 0;
    int y_im2 = 0;
	int i = 0;
	FILE *F=NULL;

    /*
    int limite = 50;
	int borneimin=limite;
	int borneimax=1080-limite;
	int incrementi=1;
	*/
    //if ((borneimin<limite) ||(borneimax>1080-limite) ||   (incrementi<1))
    //  return -1;

    if (borneimin < limite) borneimin = limite;
    if (borneimax > 1080-limite) borneimax = 1080-limite;

    printf("\nsearching between i= %d and i= %d\n",borneimin,borneimax);

    for( i=borneimin ; i<borneimax ; i += incrementi )
    {
        erreurC1[i] = 1e15;  //big value for pixel which will not be computed
        erreurC2[i] = 0;
        nbimpact[i] = 0;
    }
    // creation du fichier texte pour enregistrer des infos
    if (SaveFileResults) F = fopen("../data/resultats_search.txt","wt");

    // pour chaque rayon en dehor de la limitte on fait une recherche de cercle

    for(i=borneimin ; i<borneimax ; i += incrementi)
        //  i=500;
    {
        erreurC1[i] = 0; //start with a 0 error
        if (Display)
        {
            // affichage du point dans l'image dans le repere hokuyo
            x_im=(sizeimagexy/2)-(this->HokuyoSensor->data.y_data[i])/scaleImage;
            y_im=(sizeimagexy/2)-(this->HokuyoSensor->data.x_data[i])/scaleImage;
            draw_square(im_ray, x_im, y_im,&col4,4);
            //drawline2d(im_ray, (sizeimagexy/2),(sizeimagexy/2), x_im, y_im,&col1);
        }
        // declarations
        FLOATHOKUYO a,b;//,d;
        FLOATHOKUYO xc,yc;
        FLOATHOKUYO xp1, yp1, xp2, yp2;
        nbimpact[i]=0;

        // calcul du centre du cercle dans le repère Hokuyo
        xc = (this->HokuyoSensor->data.depth[i]+rayon_cercle_1)*cos(this->HokuyoSensor->data.angle[i]);
        yc = (this->HokuyoSensor->data.depth[i]+rayon_cercle_1)*sin(this->HokuyoSensor->data.angle[i]);
        if (Display)
        {
            // affichage dans l'image du centre du cercle
            //   draw_square(im_ray, (sizeimagexy/2)-yc/scaleImage, (sizeimagexy/2)-xc/scaleImage,&col5,4);
            //  TracerCercle(im_ray,(sizeimagexy/2)-yc/scaleImage, (sizeimagexy/2)-xc/scaleImage, rayon_cercle_1/scaleImage, &col5);
            //circle 2
            //  TracerCercle(im_ray,(sizeimagexy/2)-yc/scaleImage, (sizeimagexy/2)-xc/scaleImage, rayon_cercle_2/scaleImage, &col4);
        }
        // recherche pour tous les rayons a +/- la limite
        for (int n2=i-limite ; n2 < i+limite ; n2++)
            // int n2=i+26;
            //    int n2=903;
        {
            // calcul de l'equation de droite correspondant au rayon
            // pour une equation : ax+by=d
            //d=0; inutile
            a=sin(this->HokuyoSensor->data.angle[n2]);
            b=-cos(this->HokuyoSensor->data.angle[n2]);

            //plus tard il faudra appliquer la rotation 3D
            FLOATHOKUYO x_data_rectified=this->HokuyoSensor->data.x_data[n2];
            FLOATHOKUYO y_data_rectified=this->HokuyoSensor->data.y_data[n2];

            if (Display)
            {

                //afichage dans l'image des point correspondant au rayon
                x_im=(sizeimagexy/2)-(y_data_rectified)/scaleImage;
                y_im=(sizeimagexy/2)-(x_data_rectified)/scaleImage;
                if(i==n2){
                    draw_square(im_ray, x_im, y_im,&col4,4);
                }
                else
                {
                    draw_square(im_ray, x_im, y_im,&col1,2);
                }

            }
            // recherche d'une intersection avec le cercle C1
            // printf("\n \n \n \n n2=%d a=%f b=%f d=%f xc=%f yc=%f rc=%f\n",n2,a,b,d,xc,yc,rayon_cercle_1);
            //FLOATHOKUYO d;
            //bool intersection1= IntersectionRayonCercle(a,b,d,xc,yc,rayon_cercle_1,&xp1,&yp1,&xp2,&yp2);
            bool  intersection1= IntersectionRayonCercleOptimD0(a,b,xc,yc,rayon_cercle_1,&xp1,&yp1,&xp2,&yp2);
            //si intersection1==false, il ne faut pas utiliser xp1,yp1, xp2,yp2 qui n'ont pas été initialisés
            //est ce que le point est dans C2 et hors C1
            FLOATHOKUYO distaucentre=sqrt( (x_data_rectified-xc)* (x_data_rectified-xc) + (y_data_rectified-yc)* (y_data_rectified-yc) );
            if ( (distaucentre>rayon_cercle_1*1.33) && (distaucentre<=rayon_cercle_2) )

            {
                if (Display)
                {
                    x_im2=(sizeimagexy/2)-(y_data_rectified)/scaleImage;
                    y_im2=(sizeimagexy/2)-(x_data_rectified)/scaleImage;
                    draw_square(im_ray, x_im2, y_im2,&col3,1);
                }
                erreurC2[i]++;  //nombre de points dans C2 et pas dans C1 étendu (r*1.33)

                //HACK//HACK//HACK//HACK//HACK//HACK
                erreurC1[i]+=10; //hack pour ne gerer que C1
                //HACK//HACK//HACK//HACK//HACK//HACK
            }
            if ( intersection1)
            {
                if (Display)
                {

                    // affichage du point dans l'image dans le repere hokuyo
                    // printf("a=%f,b=%f,d=%f,xc=%f,yc=%f , xp1=%f , yp1=%f , xp2=%f , yp2=%f    \n",a,b,d,xc,yc, xp1, xp2, yp1, yp2);
                    //point d'impact le plus proche
                    x_im2=(sizeimagexy/2)-(yp1)/scaleImage;
                    y_im2=(sizeimagexy/2)-(xp1)/scaleImage;
                    //draw_square(im_ray, x_im2, y_im2,&col5,4);
                    /* x_im=(sizeimagexy/2)-(yp2);
            y_im=(sizeimagexy/2)-(xp2);
           draw_square(im_ray, x_im, y_im,&col5,4);
           */
                }
                //calcul de l'angle entre le rayon et la tangente au cercle en le point d'impact prévu

                FLOATHOKUYO a2=xp1-xc;
                FLOATHOKUYO b2=yp1-yc;
                //FLOATHOKUYO d2=(xp1-xc)*xp1 + (yp1-yc)*yp1; //non utilisé
                FLOATHOKUYO alpha= acos( ((a*a2) + (b*b2) ) / ( sqrt( (a*a) + (b*b) ) * sqrt( (a2*a2) + (b2*b2) ) ));
                //   drawline2d(im_ray, x_im, y_im,x_im2, y_im2,&col4);
                //on veut une valeur entre -PI/2 et PI/2
                if (alpha>M_PI/2)
                {
                    alpha=alpha-M_PI;
                }
                //en degrés
                alpha=(alpha/M_PI)*180;
                //valeur absolue
                if (alpha<0)
                    alpha=-alpha;
                //affiche tous en rouge
                //   drawline2d(im_ray, x_im, y_im,x_im2, y_im2,&col1);
                FLOATHOKUYO diff=atan2(yp1,xp1);
                diff=diff- this->HokuyoSensor->data.angle[n2];
                if (diff>M_PI)
                    diff=diff-2*M_PI;
                bool gooddirection=( std::fabs(diff)< M_PI/2);

                //est ce que xp1 est dans la direction du rayon et qu'il y a intersection
                // if ( intersection1 &&  gooddirection   && ( alpha >10 ))
                //   if ( gooddirection && ( alpha >10 ))
                if ( gooddirection && ( alpha >20 ))
                {
                    if (Display)
                    {
                        //reaffiche en noir ceux qui sont utilisé pour le calcul d'erreur
                        // drawline2d(im_ray, x_im, y_im,x_im2, y_im2,&col4);
                    }
                    erreurn2= sqrt(  ((xp1-this->HokuyoSensor->data.x_data[n2])*(xp1-this->HokuyoSensor->data.x_data[n2]) ) + ((yp1-this->HokuyoSensor->data.y_data[n2])*(yp1-this->HokuyoSensor->data.y_data[n2]) ));
                    erreurC1[i]+=erreurn2;
                    nbimpact[i]++;
                }
                else
                {
                    erreurn2=0;
                }
                //fprintf(F,"pour n2 = %05d      intersection=%s \n",n2 ,(intersection1)?"true":"false");
                //printf("i: %05d -> %f   \n",i,erreur[i]);
            }
        }
        // calcul de la moyenne
        erreurC1[i]=erreurC1[i]/nbimpact[i];

        // affichage des erreur pour chaque rayon
        //   printf("i: %05d -> %f     angle = %f   nbimpact=%f \n",i,erreur[i],this->HokuyoSensor->data.angle[i], nbimpact[i]);
    }

    //recherche du rayon pour lequel l'erreur est mini:
    FLOATHOKUYO erreurmin=1e15;
    int ierreurmin=0;
    FLOATHOKUYO erreurmax=-1e15;
    int ierreurmax=0;
    if (Verbose)
        for(i=borneimin;i<borneimax;i+=incrementi)
        {
        printf("i: %04d  -> %6d :  %7.2f   %5d\n",i, nbimpact[i],  erreurC1[i],erreurC2[i]);
    }
    if (SaveFileResults)
    {
        // pour remplir le fichier resultats_search.txt
        for(i=borneimin;i<borneimax;i+=incrementi)
        {
            //fprintf(F," i = %05d      erreur=%f    nbimpact=%d \n",i,erreur[i],nbimpact[i]);
            fprintf(F,"i: %04d  -> %6d :  %7.2f   %5d\n",i, nbimpact[i],  erreurC1[i],erreurC2[i]);
        }
    }
    //recherche du rayon pour lequel l'erreur C1 est mini:
    erreurmin=1e15;
    ierreurmin=0;
    erreurmax=-1e15;
    ierreurmax=0;
    for(i=borneimin;i<borneimax;i+=incrementi)
    {
        if (erreurC1[i]<erreurmin)
        {
            ierreurmin=i;
            erreurmin=erreurC1[i];
        }
        /*   if (erreurC1[i]>erreurmax)
        {
            ierreurmax=i;
            erreurmax=erreurC1[i];
        }
        */
    }


    //save pole position for cap estimation
    polex = (this->HokuyoSensor->data.depth[ierreurmin]+rayon_cercle_1)*cos(this->HokuyoSensor->data.angle[ierreurmin]);
    poley = (this->HokuyoSensor->data.depth[ierreurmin]+rayon_cercle_1)*sin(this->HokuyoSensor->data.angle[ierreurmin]);

    printf("\n\nPole located at: %f , %f \n \n",polex,poley);

    if (Verbose)
        printf("erreur min en i: %04d  -> %d impacts, C1:%7.2lf  C2:  %5d\n\n",ierreurmin,  nbimpact[ierreurmin], erreurC1[ierreurmin],erreurC2[ierreurmin]);
    if (Display)
    {
        i=ierreurmin;
        x_im=(sizeimagexy/2)-(this->HokuyoSensor->data.y_data[i])/scaleImage;
        y_im=(sizeimagexy/2)-(this->HokuyoSensor->data.x_data[i])/scaleImage;
        //         draw_square(im_ray, x_im, y_im,&col1,20);
        drawline2d(im_ray, (sizeimagexy/2),(sizeimagexy/2), x_im, y_im,&col5);
        for(i=borneimin;i<borneimax;i+=incrementi)
        {
            unsigned char val=255.0*(erreurC1[i]-erreurmin)/(FLOATHOKUYO)(erreurmax-erreurmin);
            rgb colt;
            colt.r=val;
            colt.g=255-val;
            colt.b=0;
            x_im=(sizeimagexy/2)-(this->HokuyoSensor->data.y_data[i])/scaleImage;
            y_im=(sizeimagexy/2)-(this->HokuyoSensor->data.x_data[i])/scaleImage;
            //   draw_square(im_ray, x_im, y_im,&colt,4);
            // drawline2d(im_ray, (sizeimagexy/2),(sizeimagexy/2), x_im, y_im,&colt);
        }

        //affichage des 2 cercles:
        FLOATHOKUYO a,b;//,d;
        FLOATHOKUYO xc,yc;
        FLOATHOKUYO xp1, yp1, xp2, yp2;
        nbimpact[i]=0;
        i=ierreurmin;

        // calcul du centre du cercle dans le repère Hokuyo
        xc = (this->HokuyoSensor->data.depth[i]+rayon_cercle_1)*cos(this->HokuyoSensor->data.angle[i]);
        yc = (this->HokuyoSensor->data.depth[i]+rayon_cercle_1)*sin(this->HokuyoSensor->data.angle[i]);
        if (Display)
        {
            // affichage dans l'image du centre du cercle
            //   draw_square(im_ray, (sizeimagexy/2)-yc/scaleImage, (sizeimagexy/2)-xc/scaleImage,&col5,4);
            TracerCercle(im_ray,(sizeimagexy/2)-yc/scaleImage, (sizeimagexy/2)-xc/scaleImage, rayon_cercle_1/scaleImage, &col5);
            //circle 2
            TracerCercle(im_ray,(sizeimagexy/2)-yc/scaleImage, (sizeimagexy/2)-xc/scaleImage, rayon_cercle_2/scaleImage, &col4);
        }

    }
    if (SaveFileResults)
    {
        // fermeture du fichier resultats_search.txt
        fclose(F);
    }
    //parametre de sortie
    return ierreurmin;
}







///////////////////////////////////////////////////////////////////////////////////////:
int  CHokuyoProcess::DisplayLoca(float theta,int ipole,int depthpole,int borneimin, int borneimax )
{

    //pour l'instant simple transformation 2D, on tiendra compte des autres rotations plus tard...
    // calcul des angles en radian
    float Rx=0;
    float Ry=0;
    float Rz=theta;

    // Calcul de la matrice de rotation World to Sensor
    double R[9]      = { cos(Ry)*cos(Rz), -cos(Rx)*sin(Rz)+cos(Rz)*sin(Rx)*sin(Ry),  sin(Rx)*sin(Rz)+cos(Rx)*cos(Rz)*sin(Ry),
                         cos(Ry)*sin(Rz),  cos(Rx)*cos(Rz)+sin(Rx)*sin(Ry)*sin(Rz), -cos(Rz)*sin(Rx)+cos(Rx)*sin(Ry)*sin(Rz),
                         -sin(Ry)    ,              cos(Ry)*sin(Rx)            ,              cos(Rx)*cos(Ry)            };

    // declaration / initialisation

    int i=0;
    FLOATHOKUYO rayon_cercle_1 = 150; // en millimetre
    FLOATHOKUYO rayon_cercle_2 = 1200; // en millimetre
    FLOATHOKUYO erreurn2;

    int x_im=0;
    int y_im=0;
    int x_im2=0;
    int y_im2=0;

    float xrot,yrot,xorg,yorg;

    i=ipole;

    // declarations
    FLOATHOKUYO a,b;//,d;
    FLOATHOKUYO xc,yc;
    FLOATHOKUYO xp1, yp1, xp2, yp2;

    // calcul du centre du cercle dans le repère Hokuyo
    xc = (this->HokuyoSensor->data.depth[i]+rayon_cercle_1)*cos(this->HokuyoSensor->data.angle[i]);
    yc = (this->HokuyoSensor->data.depth[i]+rayon_cercle_1)*sin(this->HokuyoSensor->data.angle[i]);
    
    
    xorg=xc;
    yorg=yc;

    xrot=R[0]*xorg+R[1]*yorg;
    yrot=R[3]*xorg+R[4]*yorg;

    float xpoleNED= xrot;
    float ypoleNED= yrot;


    x_loca=xpoleNED;
    y_loca=ypoleNED;



    xrot=xrot-xpoleNED;
    yrot=yrot-ypoleNED;

    
    // affichage dans l'image du centre du cercle
    //   draw_square(im_ray, (sizeimagexy/2)-yc/scaleImage, (sizeimagexy/2)-xc/scaleImage,&col5,4);
    TracerCercle(im_ray,(sizeimagexy/2)-yrot/scaleImage, (sizeimagexy/2)-xrot/scaleImage, rayon_cercle_1/scaleImage, &col5);
    //circle 2
    TracerCercle(im_ray,(sizeimagexy/2)-yrot/scaleImage, (sizeimagexy/2)-xrot/scaleImage, rayon_cercle_2/scaleImage, &col4);




    x_im=(sizeimagexy/2)-(yrot)/scaleImage;
    y_im=(sizeimagexy/2)-(xrot)/scaleImage;
    //         draw_square(im_ray, x_im, y_im,&col1,20);
    drawline2d(im_ray, (sizeimagexy/2),(sizeimagexy/2), x_im, y_im,&col5);

    int di;
    printf("display boundaries for tracking\n");
    //affichage des bornes pour le tracking
    i=borneimin;
    di=10000;
    xc = (di)*cos(this->HokuyoSensor->data.angle[i]);
    yc = (di)*sin(this->HokuyoSensor->data.angle[i]);
    xorg=xc;
    yorg=yc;
    xrot=R[0]*xorg+R[1]*yorg;
    yrot=R[3]*xorg+R[4]*yorg;    
    xrot=xrot-xpoleNED;
    yrot=yrot-ypoleNED;
    x_im=(sizeimagexy/2)-(yrot)/scaleImage;
    y_im=(sizeimagexy/2)-(xrot)/scaleImage;
    drawline2d(im_ray, (sizeimagexy/2)+ypoleNED/scaleImage,(sizeimagexy/2)+xpoleNED/scaleImage, x_im, y_im,&col2);

    i=borneimax;
    di=10000;
    xc = (di)*cos(this->HokuyoSensor->data.angle[i]);
    yc = (di)*sin(this->HokuyoSensor->data.angle[i]);
    xorg=xc;
    yorg=yc;
    xrot=R[0]*xorg+R[1]*yorg;
    yrot=R[3]*xorg+R[4]*yorg;
    xrot=xrot-xpoleNED;
    yrot=yrot-ypoleNED;
    x_im=(sizeimagexy/2)-(yrot)/scaleImage;
    y_im=(sizeimagexy/2)-(xrot)/scaleImage;
    //         draw_square(im_ray, x_im, y_im,&col1,20);
    drawline2d(im_ray, (sizeimagexy/2)+ypoleNED/scaleImage,(sizeimagexy/2)+xpoleNED/scaleImage, x_im, y_im,&col2);



    printf("display boundaries for fov\n");
    //affichage des bornes pour le tracking
    i=0;
    di=10000;
    xc = (di)*cos(this->HokuyoSensor->data.angle[i]);
    yc = (di)*sin(this->HokuyoSensor->data.angle[i]);
    xorg=xc;
    yorg=yc;
    xrot=R[0]*xorg+R[1]*yorg;
    yrot=R[3]*xorg+R[4]*yorg;
    xrot=xrot-xpoleNED;
    yrot=yrot-ypoleNED;
    x_im=(sizeimagexy/2)-(yrot)/scaleImage;
    y_im=(sizeimagexy/2)-(xrot)/scaleImage;
    drawline2d(im_ray, (sizeimagexy/2)+ypoleNED/scaleImage,(sizeimagexy/2)+xpoleNED/scaleImage, x_im, y_im,&col3);

    i=1079;
    di=10000;
    xc = (di)*cos(this->HokuyoSensor->data.angle[i]);
    yc = (di)*sin(this->HokuyoSensor->data.angle[i]);
    xorg=xc;
    yorg=yc;
    xrot=R[0]*xorg+R[1]*yorg;
    yrot=R[3]*xorg+R[4]*yorg;
    xrot=xrot-xpoleNED;
    yrot=yrot-ypoleNED;
    x_im=(sizeimagexy/2)-(yrot)/scaleImage;
    y_im=(sizeimagexy/2)-(xrot)/scaleImage;
    //         draw_square(im_ray, x_im, y_im,&col1,20);
    drawline2d(im_ray, (sizeimagexy/2)+ypoleNED/scaleImage,(sizeimagexy/2)+xpoleNED/scaleImage, x_im, y_im,&col3);


    printf("display direction of sight\n");
    //affichage des bornes pour le tracking
    i=1080/2;
    di=1000;
    xc = (di)*cos(this->HokuyoSensor->data.angle[i]);
    yc = (di)*sin(this->HokuyoSensor->data.angle[i]);
    xorg=xc;
    yorg=yc;
    xrot=R[0]*xorg+R[1]*yorg;
    yrot=R[3]*xorg+R[4]*yorg;
    xrot=xrot-xpoleNED;
    yrot=yrot-ypoleNED;
    x_im=(sizeimagexy/2)-(yrot)/scaleImage;
    y_im=(sizeimagexy/2)-(xrot)/scaleImage;
    drawline2d(im_ray, (sizeimagexy/2)+ypoleNED/scaleImage,(sizeimagexy/2)+xpoleNED/scaleImage, x_im, y_im,&col3);

    for(i=0;i<1080;i++)
    {
        // affichage du point dans l'image dans le repere hokuyo

        /*        x_im=(sizeimagexy/2)-(this->HokuyoSensor->data.y_data[i])/scaleImage;
        y_im=(sizeimagexy/2)-(this->HokuyoSensor->data.x_data[i])/scaleImage;
*/

        xorg=this->HokuyoSensor->data.x_data[i];
        yorg=this->HokuyoSensor->data.y_data[i];

        xrot=R[0]*xorg+R[1]*yorg;
        yrot=R[3]*xorg+R[4]*yorg;


        xrot=xrot-xpoleNED;
        yrot=yrot-ypoleNED;


        x_im=(sizeimagexy/2)-(yrot)/scaleImage;
        y_im=(sizeimagexy/2)-(xrot)/scaleImage;

        draw_square(im_ray, x_im, y_im,&col4,4);
        //  drawline2d(im_ray, (sizeimagexy/2),(sizeimagexy/2), x_im, y_im,&col1);
    }



    for(i=0;i<1080;i++)
    {
        int jmin=i;
        PolartoCart(  tabainit[jmin], tabrinit[jmin],&xorg,&yorg);
        x_im2=(sizeimagexy/2)-(yorg)/scaleImage;
        y_im2=(sizeimagexy/2)-(xorg)/scaleImage;
        draw_square(im_ray, x_im2, y_im2,&col2,4);
    }

    for(i=0;i<1080;i++)
    {
        // affichage des points initiaux dans l'image dans le repere hokuyo
        PolartoCart(  taba[i]+theta, tabr[i],&xorg,&yorg);
        x_im=(sizeimagexy/2)-(yorg)/scaleImage;
        y_im=(sizeimagexy/2)-(xorg)/scaleImage;
        draw_square(im_ray, x_im, y_im,&col7,4);
        //  drawline2d(im_ray, (sizeimagexy/2),(sizeimagexy/2), x_im, y_im,&col1);
        int jmin=tabassociated[i];
        //  printf("i:%d associated to j:%d\n",i,jmin);
        if (jmin!=-1)
        {
            PolartoCart(  tabainit[jmin], tabrinit[jmin],&xorg,&yorg);
            x_im2=(sizeimagexy/2)-(yorg)/scaleImage;
            y_im2=(sizeimagexy/2)-(xorg)/scaleImage;
            drawline2d(im_ray,  x_im, y_im, x_im2, y_im2,&col1);
        }


    }
    /*
    static int t=0;
    if (t==1)
    exit(0);
    else t++;
*/
}


////////////////////////////////////////////////////////////////////////

void CHokuyoProcess::GetCylinderAndSector( int *nbc, int *nbs,FLOATHOKUYO a, FLOATHOKUYO r)
{
    *nbs = (a*180./(M_PI))*nbsect/360.;
    *nbc = r/(30000./nbcyl);
}

////////////////////////////////////////////////////////////////////////
int  CHokuyoProcess::FindCap(int bornetmin,int bornetmax,int incrementt,FLOATHOKUYO *scoret)//, FLOATHOKUYO  polex, FLOATHOKUYO poley)
        //int limite ,bool SaveFileResults,bool Display,bool Verbose)
{
    /*
	 bornetmin=7;
     bornetmax=8;
     incrementt=1;
	*/
    // declaration / initialisation
    static int firstAcq=1;

    int tmin; //returned result of the function

    printf("\nerase the association list\n");

    for (int i=0;i<1080;i++)  //i index for init array
    {
        tabassociated[i]=-1; //by default there is no associated data...
    }


    ////////////////////////////
    printf("\nConvert Data to polar coordinates centered on the pole\n");
    /* FLOATHOKUYO polex=10;
    FLOATHOKUYO poley=30;
    */
    for (int i=0;i<1080;i++)
        if (this->HokuyoSensor->data.depth[i]!=1) //available data for that point
        {
			FLOATHOKUYO xc = (this->HokuyoSensor->data.depth[i]*cos(this->HokuyoSensor->data.angle[i]))-polex;
			FLOATHOKUYO yc = (this->HokuyoSensor->data.depth[i]*sin(this->HokuyoSensor->data.angle[i]))-poley;
			CartToPolar(& taba[i],&tabr[i], xc,yc);
			if (taba[i] < 0)
				taba[i] = taba[i] + 2*M_PI;
		} else {  //no available data for that point
			tabr[i]=-1;
			taba[i]=0;
			//printf("no acquisition for:%d\n",i);
    }
    ////////////////////////////
    if (firstAcq == 1)
    {
        printf("Saving scene geometry for cap and estimated pole position\n");
        firstAcq = 0;
        for (int i=0;i<1080;i++)
        {
            tabainit[i]=taba[i];
            tabrinit[i]=tabr[i];
        }
        tmin = 0;

        printf("generate the structure for fast search\n");

        TabInit=(ListPt2d *)malloc(nbsect*nbcyl*sizeof(ListPt2d));
        int nbs;
        int nbc;
        for ( nbs=0;nbs<nbsect;nbs++)
            for ( nbc=0;nbc<nbcyl;nbc++)
            {
            TabInit[nbs+nbc*nbsect].begin=NULL;
            TabInit[nbs+nbc*nbsect].end=NULL;
            TabInit[nbs+nbc*nbsect].size=NULL;
        }
        for (int i=0;i<1080;i++)
        {
            if(tabrinit[i] != -1)
            {
                GetCylinderAndSector(&nbc,&nbs,tabainit[i],tabrinit[i]);

                printf("point %8.3f:%8.3f in  sector %5d and cylinder %5d\n",tabainit[i],tabrinit[i],nbs,nbc);

                //create one element
                ElementListPt2d *current;
                current =(ElementListPt2d *)malloc(sizeof(ElementListPt2d ));
                current->next=NULL;
                current->number=i;

                //add it to the list
                if (TabInit[nbs+nbc*nbsect].size==0) //it is an empty list
                {
                    TabInit[nbs+nbc*nbsect].begin=current;
                    TabInit[nbs+nbc*nbsect].end=current;
                }
                else
                {
                    TabInit[nbs+nbc*nbsect].end->next=current;
                    TabInit[nbs+nbc*nbsect].end=current;
                }
                TabInit[nbs+nbc*nbsect].size++;

            }

        }

        //verify the list
        for ( nbs=0;nbs<nbsect;nbs++)
            for (nbc=0;nbc<nbcyl;nbc++)
            {
            if (TabInit[nbs+nbc*nbsect].size>0)
                printf("exploring c: %3i, s: %3i:\n",nbc,nbs);
            ElementListPt2d *current=TabInit[nbs+nbc*nbsect].begin;
            while (current!=NULL)
            {
                printf("     i:%5i\n",current->number);
                current=current->next;
            }
        }

        tmin= 0;
        //free(TabInit);  // A FAIRE A LA FIN!!!!!!!!!! -> mettre dans le destructeur, et déplacer l'init dans le constructeur

    }
    else // (firstAcq!=1)
    {
        //code poubelle pour ajouter un biais
        /*    for (int i=0;i<1080;i++)
        {
            taba[i]=taba[i]+M_PI*7./180.;

        }
      */

        FLOATHOKUYO s[360];
        FLOATHOKUYO dmin;
        FLOATHOKUYO stmin=1e37; //  http://en.wikipedia.org/wiki/Single_precision_floating-point_format
        int tmod; //t %360; //t can be > 359 because of tracking, to use the index, it is restricted to 0-359

        /*
        bornetmin=359;
        bornetmax=360;
        incrementt=1;
*/

        /*  bornetmin=17;
        bornetmax=18;
        incrementt=1;
*/

        int tabassociatedtemp[1080]; //temporary association list
        FLOATHOKUYO tabassociateddisttemp[1080];

        bool ttested[360];
        int t;

        for (t=0;t<360;t++)
            ttested[t]=false;

        for (int t=bornetmin;t<bornetmax;t+=incrementt) //theta
        {
            s[t]=0;
            tmod=t%360;
            if (tmod<0)
                tmod=tmod+360;

            ttested[tmod]=true;

            /*
            for (int j=0;j<1080;j++)  //j index for current array
            {
                printf("%4d : %f  : %i\n",j,tabr[j],this->HokuyoSensor->data.depth[j]);
            }
//            exit(0);
*/

            //il faudra gerer la saturation ici


            for (int j=0;j<1080;j++)  //j index for current array
            {
                dmin=DistThreshold;
                tabassociatedtemp[j]=-1; //by default there is no associated data... maybe one will be found close enough in the next steps...

                //  if(tabr[j]==1) // there is no available distance
                //      tabassociatedtemp[i]=-1;

                if(tabr[j]!=1) // there is an available distance
                {
                    //smart search
                    int nbc,nbs;
                    FLOATHOKUYO trad=M_PI*t/180.;
                    FLOATHOKUYO tabajshift=taba[j]-trad;
                    if (tabajshift<0)
                        tabajshift=tabajshift+2*M_PI;
                    if (tabajshift>2*M_PI)
                        tabajshift=tabajshift-2*M_PI;

                    GetCylinderAndSector(&nbc,&nbs,tabajshift,tabr[j]);


                    //etendre la taille du tableau pour localiser les données de 1 à nbsect compris, rajouter une cellule de chaque coté...
                    //il faudra changer getCylinderAndSector pour faire +1


                    int nbcmin=nbc-1;
                    int nbcmax=nbc+1;
                    int nbsmin=nbs-1;
                    int nbsmax=nbs+1;

                    if (nbcmin<0)
                        nbcmin=0;
                    if (nbsmin<0)
                        nbsmin=0;
                    if (nbcmax>nbcyl-1)
                        nbcmax=nbcyl-1;
                    if (nbsmax>nbsect-1)
                        nbsmax=nbsect-1;


                     for (int nbcc=nbcmin;nbcc<=nbcmax;nbcc++)
                        for (int nbss=nbsmin;nbss<=nbsmax;nbss++)
                        {
                        //pour l'instant je recherche juste dans la même cellule
                        ElementListPt2d *current;
                        // current=TabInit[nbs+nbc*nbsect].begin;
                        current=TabInit[nbss+nbcc*nbsect].begin;
                        while (current!=NULL)
                        {
                            int i=current->number;
                            if(tabrinit[i]!=1)
                            {
                                FLOATHOKUYO deltaa=abs(tabainit[i]-(taba[j]-M_PI*t/180.)); //the angles without the modulo operation is used here
                                if (deltaa>M_PI) //get a centered around 0 value
                                    deltaa=deltaa-2*M_PI;
                                FLOATHOKUYO deltar=abs(tabrinit[i]-tabr[j]);
                                FLOATHOKUYO d= (deltar*deltar) + (tabrinit[i]*tabr[j])*deltaa*deltaa;
                                //il faudra gerer la saturation ici
                                if (d>=DistThreshold)
                                    d=DistThreshold;
                                if (d<dmin)
                                {
                                    dmin=d;
                                    if (dmin>=DistThreshold)
                                        tabassociatedtemp[j]=-1;
                                    else
                                        tabassociatedtemp[j]=i;
                                    tabassociateddisttemp[j]=d;
                                }
                            }
                            current=current->next;
                        }
                    }

                    //IL FAUT CHERCHER DANS LES CELLULES VOISINES!!!!!


                    /*   //brute search
                    for (int i=0;i<1080;i++)  //i index for init array
                        if(tabrinit[i]!=1)
                        {


                            FLOATHOKUYO deltaa=abs(tabainit[i]-(taba[j]-M_PI*t/180.)); //the angles without the modulo operation is used here
                            if (deltaa>M_PI) //get a centered around 0 value
                                deltaa=deltaa-2*M_PI;
                            FLOATHOKUYO deltar=abs(tabrinit[i]-tabr[j]);
                            FLOATHOKUYO d= (deltar*deltar) + (tabrinit[i]*tabr[j])*deltaa*deltaa;
                            if (d>=DistThreshold)
                                d=DistThreshold;
                            if (d<dmin)
                            {
                                dmin=d;
                                if (dmin>=DistThreshold)
                                    tabassociatedtemp[j]=-1;
                                else
                                    tabassociatedtemp[j]=i;
                                tabassociateddisttemp[j]=d;
                            }
                        }
                  */


                    s[tmod]=s[tmod]+dmin;
                }
            }

            if (s[tmod]<=stmin)
            {
                stmin=s[tmod];
                tmin=tmod;
                //copy the association table for the best t up to now
                //TODO: avoid the copy by using two list pointers: best and current that
                //are exchanged each time a best association is found....
                for (int i=0;i<1080;i++)
                {
                    tabassociated[i]= tabassociatedtemp[i];
                    tabassociateddist[i]= tabassociateddisttemp[i];

                }
            }
        }

#ifdef DISPLAYASSOCIATION
        for (int i=0;i<1080;i++)
        {
            printf("association %4d -> %4d  , square dist: %f\n",i,tabassociated[i],tabassociateddist[i]);
        }
#endif

#define DISPLAYTHETASCORE

#ifdef DISPLAYTHETASCORE
        for (int t=0;t<360;t++)
        {
            if ( ttested[t])
                printf("t:  %4d => %16.2f",t,s[t]);
            if (t==tmin)
                printf("  <---- MIN");
            printf("\n");
        }
#endif


        printf("\n\ntmin: %4d => %16.2f\n",tmin,stmin);
        fflush(stdout);
        FILE *F=fopen("LOGCAP.txt","at");
        fprintf(F,"tmin: %6d => %16.2f\n",tmin,stmin);
        fclose(F);

        *scoret=stmin;
    }
    return tmin;
}
///////////////////////////////////////////////////////
