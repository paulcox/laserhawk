/**
 * \file pnm_io2.cpp
 * \brief creation of images
 * \author Bertrand Vandeportaele
 * \version ?
 * \date 03.12.2010
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "pnm_io2.h"





/*
 *           Memory managment
 *
 */



#define MAX_DIM 15000



im_gray *alloc_im_gray(int xd,int yd) {
  im_gray *ret;


  if(xd<0 || xd>MAX_DIM || yd<0 || yd>MAX_DIM) {
    fprintf(stderr,"Weird image size: %d*%d\n",xd,yd);
    exit(1);    
  }

  ret=(im_gray*)malloc(sizeof(im_gray));

  ret->xd=xd;
  ret->yd=yd;

  ret->data=(unsigned char*)malloc(xd*yd);

  if(!ret->data) {
    fprintf(stderr,"malloc %d bytes failed\n",xd*yd);
    exit(1);
  }
  
  return ret;
}


void dealloc_im_gray(im_gray *im) {
  free(im->data);
  free(im);
}

/*
 *  WARNING: this works only if  sizeof(struct {char a,b,c})==3
 *  this is normaly the case, because there is no risk of unaligned access on chars
 */

im_color *alloc_im_color(int xd,int yd) {
  im_color *ret;


  if(xd<0 || xd>MAX_DIM || yd<0 || yd>MAX_DIM) {
    fprintf(stderr,"Weird image size: %d*%d\n",xd,yd);
    exit(1);    
  }

  ret=(im_color*)malloc(sizeof(im_color));

  ret->xd=xd;
  ret->yd=yd;

  ret->data=(rgb*)malloc(xd*yd*3);

  if(!ret->data) {
    fprintf(stderr,"malloc %d bytes failed\n",xd*yd);
    exit(1);
  }
  
  return ret;
}


void dealloc_im_color(im_color *im) {
  free(im->data);
  free(im);
}



/*
 *           Common functions for all pnm files
 *
 */


static void get_line(FILE *f,char *buffer) {
  while(1) {
    char *c;
    c=fgets(buffer,79,f);
    if(!c) {
      fprintf(stderr,"EOF while reading header\n");
      exit(1);      
    }
    for(c=buffer;isspace(*c);c++);
    if(*c!='#') return ;
  }
}

/* consumes the header of the pnm file, returns a handle to the rest */
static FILE* open_pnm(char *filename,
		      int *pn,int *xd,int *yd,int *maxval) {
  FILE *f;
  char buffer[80];

  if(!(f=fopen(filename,"rb"))) {
    fprintf(stderr,"Could not open `%s'",filename);
    perror("");
    exit(1);
  }
  
  get_line(f,buffer);  
  if(sscanf(buffer,"P%d",pn)!=1) {
    fprintf(stderr,"`%s' does not have a Px header\n",filename);
    exit(1);
  }
  
  get_line(f,buffer);  
  if(sscanf(buffer,"%d %d ",xd,yd)!=2) {
    fprintf(stderr,"`%s' has invalid dimensions\n",filename);
    exit(1);
  }
  
  get_line(f,buffer);  
  if(sscanf(buffer,"%d ",maxval)!=1) {
    fprintf(stderr,"`%s' mentions invalid maxval\n",filename);
    exit(1);
  }

  return f;
}


/*
 *           PGM functions
 *
 */


im_gray *load_pgm(char *filename) {
  FILE *f;
  int pn,xd,yd,maxval;
  im_gray *ret;

  f=open_pnm(filename,&pn,&xd,&yd,&maxval);

  if(pn!=5 || maxval!=255) {
    fprintf(stderr,"load_pgm(`%s'): not a 8 bpp PGM raw file\n",filename);
    exit(1);
  }

  ret=alloc_im_gray(xd,yd);
  if((int)fread(ret->data,1,xd*yd,f)!=xd*yd)
    fprintf(stderr,"load_pgm(`%s'): warning, file truncated\n",filename);

  fclose(f);

  return ret;
}


void save_pgm(char *filename,im_gray *im) {
  FILE *f;
  if(!(f=fopen(filename,"wb"))) {
    fprintf(stderr,"Could not open `%s'",filename);
    perror("");
    exit(1);
  }

  fprintf(f,"P5\n%d %d\n255\n",im->xd,im->yd);
  fwrite(im->data,1,im->xd*im->yd,f);
  fclose(f);
}


/*
 *           PPM functions
 *
 */

im_color *load_ppm(char *filename) {
  FILE *f;
  int pn,xd,yd,maxval;
  im_color *ret;

  f=open_pnm(filename,&pn,&xd,&yd,&maxval);

  if(pn!=6 || maxval!=255) {
    fprintf(stderr,"load_ppm(`%s'): not a 8 bpp PPM raw file\n",filename);
    exit(1);
  }

  ret=alloc_im_color(xd,yd);
  if((int)fread(ret->data,3,xd*yd,f)!=xd*yd)
    fprintf(stderr,"load_ppm(`%s'): warning, file truncated\n",filename);

  fclose(f);

  return ret;
}


void save_ppm(char *filename,im_color *im) {
  FILE *f;
  if(!(f=fopen(filename,"wb"))) {
    fprintf(stderr,"Could not open `%s'",filename);
    perror("");
    exit(1);
  }

  fprintf(f,"P6\n%d %d\n255\n",im->xd,im->yd);
  fwrite(im->data,1,im->xd*im->yd*3,f);
  fclose(f);
}


int set_color_pixel_with_boundaries(im_color *im,int x, int y,rgb * col) {
	unsigned char *ptr;
	if ( (x>=0) && (x<im->xd) && (y>=0) && (y<im->yd) )
		{
	
		ptr=(unsigned char *)(&im->data[0].r)+(x+y*im->xd)*3;
	//	printf("x:%d, y:%d, PTR: %ld\n",x,y,ptr);
	 	ptr[0]=col->r;
	 	ptr[1]=col->g;
		ptr[2]=col->b;	
		return 0;
		}
	else
		return -1;
	
}


void draw_square(im_color *im,int xd,int yd,rgb *col,int size){
	if (size==1)
	 	set_color_pixel_with_boundaries(im,xd, yd,col); 
	else
		for (int x=xd-size/2;x<xd+size/2;x++)
			for (int y=yd-size/2;y<yd+size/2;y++)
			 	set_color_pixel_with_boundaries(im,x, y,col); 
}



void TracerCercle(im_color * im, int x_centre, int y_centre, int r, rgb * col)
{
    int x = 0;
    int y=r;
    int d=r-1;

    while (y>=x)
    {
   set_color_pixel_with_boundaries(im,x+x_centre , y+y_centre,col);
   set_color_pixel_with_boundaries(im,y+x_centre , x+y_centre,col);
   set_color_pixel_with_boundaries(im,-x+x_centre , y+y_centre,col);
   set_color_pixel_with_boundaries(im,-y+x_centre , x+y_centre,col);

   set_color_pixel_with_boundaries(im,x+x_centre , -y+y_centre,col);
   set_color_pixel_with_boundaries(im,y+x_centre , -x+y_centre,col);
   set_color_pixel_with_boundaries(im,-x+x_centre , -y+y_centre,col);
   set_color_pixel_with_boundaries(im,-y+x_centre , -x+y_centre,col);


    if  (d >= 2*x )
    {
        d=d-2*x-1;
        x=x+1;
    }
    else if( d <= 2*(r-y))
    {
        d=d+2*y-1;
        y= y-1;
    }
    else
    {

        d =d+2*(y-x-1);
        y=y-1;
        x=x+1;
    }
}
}




 void drawline2d(im_color *im,int x0, int y0, int x1, int y1, rgb * col)
 { int i;
     int steep = 1;
     int sx, sy; /* step positive or negative (1 or -1) */
     int dx, dy; /* delta (difference in X and Y between points) */
     int e;

/* * inline swap. On some architectures, the XOR trick may be faster */
     int tmpswap;
#define SWAP(a,b) tmpswap = a; a = b; b = tmpswap;

/* * optimize for vertical and horizontal lines here */
     dx = abs(x1 - x0);
     sx = ((x1 - x0) > 0) ? 1 : -1;
     dy = abs(y1 - y0);
     sy = ((y1 - y0) > 0) ? 1 : -1;
     if (dy > dx)
     {
         steep = 0;
         SWAP(x0, y0);
         SWAP(dx, dy);
         SWAP(sx, sy);
     }
     e = (dy << 1) - dx;
     for (i = 0; i < dx; i++)
     {
         if (steep)
         {
               set_color_pixel_with_boundaries(im,x0 , y0,col);
 //             plot(x0,y0,color);
         }
         else
         {
                 set_color_pixel_with_boundaries(im,y0 , x0,col);
           //  plot(y0,x0,color);
         }
         while (e >= 0)
         {
             y0 += sy;
             e -= (dx << 1);
         }
         x0 += sx;
         e += (dy << 1);
     }
 }

 //////////////////////////////
 void erase_ppm(im_color *im,rgb * col)
 {
     for (int i=0;i<im->xd*im->yd;i++)
     {
         im->data[i].r=col->r;
         im->data[i].g=col->g;
         im->data[i].b=col->b;

     }

 }

