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

#ifndef PNM_IO_H_INCLUDED
#define PNM_IO_H_INCLUDED





/*
 * Allocation and deallocation functions for grayscale images 
 * Read and written in PGM format
 *
 */

typedef unsigned char byte;


/* Representation of a grayscale image */
typedef struct {
  int xd,yd;         /* width and height of image, in pixels */
  byte *data;        /* image pixels, represented in latin reading order */
} im_gray;



/* allocates an image of size xd*yd pixels */
im_gray *alloc_im_gray(int xd,int yd);

/* loads an image from a file in 8bpp PGM raw format */
im_gray *load_pgm(char *filename);

/* saves an image to a file in 8bpp PGM raw format  */
void save_pgm(char *filename,im_gray *im);

/* deallocates the storage for an image created with alloc_im_gray or load_pgm */
void dealloc_im_gray(im_gray *im);






/*
 * Allocation and deallocation functions for color images 
 * Read and written in PPM format
 *
 */

typedef struct {
  byte r,g,b;      /* red, green and blue components */
} rgb;

/* Representation of a color image */
typedef struct {
  int xd,yd;         /* width and height of image, in pixels */
  rgb *data;         /* image pixels, represented in latin reading order */
} im_color;


/* allocates an image of size xd*yd pixels */
im_color *alloc_im_color(int xd,int yd);

/* loads an image from a file in 24 bpp PPM raw format */
im_color *load_ppm(char *filename);

/* saves an image to a file in 24 bpp PPM raw format  */
void save_ppm(char *filename,im_color *im);

/* deallocates the storage for an image created with alloc_im_color or load_ppm */
void dealloc_im_color(im_color *im);


int set_color_pixel_with_boundaries(im_color *im,int x, int y,rgb * col) ;
	
void draw_square(im_color *im,int xd,int yd,rgb *col,int size);
	

void TracerCercle(im_color * im, int x_centre, int y_centre, int r, rgb * col);


 void drawline2d(im_color *im,int x0, int y0, int x1, int y1, rgb * col);

#define PIX(im,x,y) (im->data[(y)*im->xd+(x)])

void erase_ppm(im_color *im,rgb * col);
#endif

