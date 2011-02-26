#!/usr/bin/perl
###############################################################################
#Author: Paul Cox
#Date  : Feb 2011
#Notes
#everything in cm
#the terrain is defined in the x direction
#x=0 corresponds to spot directly under the hokyo sensor
#x is positive to the right
#x is negative to the left
#y is positive up
#kokuyo scans from left to right
#  Reference Diagram:
#                         +Y
#                          ^
#                          |
#                          |
#                          H 
#                          .
#                          .
#                          .
# -X <---|-----|-----|-----|-----|-----|-----|---> X
#       -15   -10   -5     0     5     10    15
#
###############################################################################

#pragmas and modules
use strict;
use warnings;
use diagnostics;
use Math::Trig; #for sin/cos/etc
use GD; #to draw
use Switch;

#constants
use constant MAXTERRAIN => 1000; #1000cm = 10 m
use constant RESDEG => 360/1440;
use constant RESRAD => (2*pi)/1440;
use constant HALFWAY => 1080/2;
use constant LASTSCAN => 1080;
use constant XINC => 1;

#image related constants
use constant IMGHEIGHT => 600;
use constant IMGWIDTH => 600;
use constant IMGBDR => 50;
#nominal terrain height
use constant NOMTH => IMGHEIGHT/2;
use constant SCALE => (IMGHEIGHT-2*IMGBDR)/(2*MAXTERRAIN);
use constant DEBUG => 0;
use constant COLORMAP => 0;
use constant HARDCODEDTERRAIN => 0;

###############################################################################
#height of hokyo sensor relative to terrain (in cm)
my $H = 700;
print "Hokuyo height ; $H cm\n";
print "Ouput image scale: ".1/SCALE." cm per pixel\n";

# create a new image and define globals (colors)
#  third param 1 is important as it defines 24bit color image
my $im = new GD::Image(IMGWIDTH,IMGHEIGHT,1);
my $im2 = new GD::Image(2*MAXTERRAIN*SCALE,2*MAXTERRAIN*SCALE,1); 
# allocate some colors
my $white = $im->colorAllocate(255,255,255);
my $black = $im->colorAllocate(0,0,0);       
my $red = $im->colorAllocate(255,0,0);      
my $blue = $im->colorAllocate(0,0,255);
my $green = $im->colorAllocate(0,255,0);

initimg();

showterrain();

my $raycnt = 0;
my $scnpt;
my @distance;
my $thetarad;
my $thetadeg;

open(LOG,">scan.log");

my $psideg = $ARGV[0];
#my $psideg = 0.25;
#my $psideg = 45;
my $psirad = deg2rad($psideg);
printf "psirad: %1.4f (%3.2f)\n",$psirad,$psideg;

print "scnpt   thetarad     (thetadeg)\n";
print "-----   --------     ----------\n";

#get distance between hokyo and terrain for each ray
#for ($scnpt = HALFWAY-135/RESDEG ; $scnpt <= HALFWAY+135/RESDEG ; $scnpt+=1) {
for ($scnpt = HALFWAY-45/RESDEG ; $scnpt <= HALFWAY+45/RESDEG ; $scnpt+=2/RESDEG) {
	$thetarad = ($scnpt - HALFWAY)*RESRAD;
	$thetadeg = ($scnpt - HALFWAY)*RESDEG;
	
	$distance[$scnpt] = getdist();
	

	#if (($scnpt % 25/RESDEG) == 0) 
	{
		my $psidist = $H*SCALE/cos($psirad);
		
		$im->line(	IMGWIDTH/2,
					IMGBDR+NOMTH + $psidist,
					IMGWIDTH/2   + $distance[$scnpt]*sin($thetarad)*SCALE,
					IMGBDR+NOMTH + $psidist - $distance[$scnpt]*cos($thetarad)*SCALE,
					$red);
		#draw little hokuyo
		$im->arc(IMGWIDTH/2,IMGBDR+NOMTH+$H*SCALE,10,10,135,45,$red);
	}
	$raycnt++;
	if ($distance[$scnpt]==0) {$distance[$scnpt]=1;}
	printf " dist : %4.2f\n",$distance[$scnpt];
	printf LOG "%04d       %d\n",$raycnt,$distance[$scnpt]*10; #multiply to 10 to go from cm to mm
}

writeimg();

close LOG;

exit;

###############################################################################
# SUBROUTINES

#create an image with border, scale bars, and nominal ground height
sub initimg {
	#we're going to assume the origin (0,0) is at bottom left as we are flipping
	#vertically before writing out the file.

    # make the background transparent and interlaced
    #$im->transparent($white);
    $im->interlaced('true');

    # Put a black frame around the picture
    $im->rectangle(0,0,IMGWIDTH-1,IMGHEIGHT,$black);

	#draw scale bars
	$im->line(IMGBDR/2,IMGBDR/2,IMGWIDTH-(IMGBDR/2),IMGBDR/2,$green);
	$im->line(IMGBDR/2,IMGBDR/2,IMGBDR/2,IMGHEIGHT-(IMGBDR/2),$green);
	
}


###############################################################################
#write image to file
sub writeimg {
	my $psiname = sprintf "%06.2f",$psideg;
	
	$im->flipVertical();
	#add scale bar ticks and text
		for (my $i=0 ; $i<IMGHEIGHT-(IMGBDR) ; $i+=10 ) {
		#vertical scale
		$im->line(IMGBDR/2,IMGBDR/2+$i,IMGBDR/2+5,IMGBDR/2+$i,$green);
		#horiz scale
		$im->line(IMGBDR/2+$i,IMGHEIGHT-(IMGBDR)/2,IMGBDR/2+$i,IMGHEIGHT-(IMGBDR)/2+5,$green);
	}
	#Print parameters
	$im->string(gdSmallFont,400,IMGBDR/2,"H:$H cm  Psi:$psiname ",$white);

	open(IMG, ">imgs/terrain$psiname.png") or die $1;
	binmode IMG;
	print "Writing image file : imgs/terrain$psiname.png\n";
	print IMG $im->png;
	close(IMG);
}

###############################################################################
#show terrain
sub showterrain {

	my $cmapfile = "imgs/colorhmap.png";
	print "checking if file exists : ";
	
    if (-r $cmapfile){
	#open colorhmap file
		print "Yes.\nOpening Terrain Color Map\n";
		$im2 = GD::Image->new($cmapfile);
	} else {
	#if the colorhmap doesn't exist we create it and save it to a file
		print "No.\nCreating Terrain Color Map\n";
		foreach my $col (0..2*MAXTERRAIN*SCALE) {
			foreach my $row (0..2*MAXTERRAIN*SCALE) {	
				my $t = terrain(($col-250)/SCALE,($row-250)/SCALE);
				my $ncol = $im2->colorAllocate(int($t+128),int($t+128),int($t+128));
				#my $ncol = $im->colorAllocate(int($row-50)/2,int($col-50)/2,0);
				if ($ncol == -1) {print "colorallocate failed\n";exit};
				$im2->setPixel($col,$row,$ncol);
				$im2->colorDeallocate($ncol);
			}
		}
		open(IMG, ">imgs/colorhmap.png") or die $1;
		binmode IMG;
		print "Writing colormap file : $cmapfile\n";
		print IMG $im2->png;
		close(IMG);
	}
	#copy colorhmap into our image
	$im->copy($im2,IMGBDR,IMGBDR,0,0,2*MAXTERRAIN*SCALE,2*MAXTERRAIN*SCALE);

	#show terrain baseline
	$im->line(IMGBDR,IMGBDR+NOMTH,IMGWIDTH-(IMGBDR),IMGBDR+NOMTH,$black);
	#draw little hokuyo
    $im->arc(IMGWIDTH/2,IMGBDR+NOMTH+$H*SCALE,10,10,135,45,$red);

	print "Terrain Cross-section Elevation\n";
	for (my $i=-(MAXTERRAIN) ; $i<MAXTERRAIN ; $i+=1/SCALE) {
		my $t = terrain($i,0);
		$im->setPixel(IMGWIDTH/2+$i*SCALE,IMGBDR+NOMTH+$t*SCALE,$blue);
		#print " $i $t\n";
	}
}

###############################################################################
#pass the terrain subroutine a x value and it return you the terrain height
sub terrain {
	my $theight;
	my ($xval,$zval) = @_;
	
	return 0 if (DEBUG);
	
	if (HARDCODEDTERRAIN) {
		if ($xval <= MAXTERRAIN && $xval >= -(MAXTERRAIN)){
			switch ($xval) {
				case { ($xval <= -500) || ($xval >= 500) } {$theight = 100*sin($xval/(25*pi));}
				#case { ($xval <= -500) || ($xval >= 500) } {$theight = abs($xval*.5)-500;}
				case {  ($xval < -200) || ($xval > 200)  } {$theight = 30;}
				case { ($xval >= -200) || ($xval <= 200) } {$theight = abs($xval*.5);}
				#else { print "terrain error!";$theight = 0;}
			}
		} else {
			print "Warning: out of bounds: $xval\n";
			$theight = 0;
		}	
	} else {
		my $index = $im2->getPixel($xval*SCALE+250,$zval*SCALE+250);
        my ($r,$g,$b) = $im2->rgb($index);
		$theight = $r-128;
	}
	
	return $theight;
}

###############################################################################
#dist approximation
sub distapx {
	my $x = $_[0];
	my $z = $_[1];

	my $Ht = (terrain($x,$z)+terrain($x-1,$z)+terrain($x,$z-1)+terrain($x-1,$z-1))/4;
	my $xest = ($H-$Ht)*tan($thetarad);
	my $zest = ($H-$Ht)*tan($psirad);
	my $distapx = sqrt( ($H-$Ht)**2+($xest)**2+($zest)**2);
	#print "   distapx: $distapx | Ht $Ht | x $xest | z $zest\n";
	return $distapx;
}

###############################################################################
#the following code calculates the exact distance but is more more slightly more computationally expensive maybe
sub distgeo {
	my $x = $_[0];
	my $y = $_[1];
	my $z = $_[2];
	my $yp = $_[3];
	my $zp = $_[4];
	my ($d2muchx,$d2much);

	my $ydist = $x/sin($thetarad);
	my $dist = sqrt($ydist**2 + $z**2);
	if ($thetarad >= RESRAD) {
		$d2muchx = ((terrain($x,$z)-$y)*(XINC)/($yp+terrain($x,$z)-$y-terrain($x-1,$z-1)))/sin($thetarad);
		$d2much = sqrt($d2muchx**2 + ((terrain($x,$z)-$y)*($z-$zp)/($yp+terrain($x,$z)-$y-terrain($x-1,$z-1)))**2);
	} else {
		$d2muchx = ((terrain($x,$z)-$y)*(XINC)/($yp+terrain($x,$z)-$y-terrain($x+1,$z+1)))/sin(-$thetarad);
		$d2much = sqrt($d2muchx**2 + ((terrain($x,$z)-$y)*($z-$zp)/($yp+terrain($x,$z)-$y-terrain($x+1,$z-1)))**2);
	}
	$dist -= $d2much;
	#print "   dist: $dist ($d2muchx) ($d2much)\n";
	return $dist;
}

###############################################################################
#subroutine to calculate distance
sub getdist {
	my $yp = $H;
	my $zp = 0;
	my $dist = 1;
	
	printf "pt $scnpt: theta %1.4f (%1.2f) ",$thetarad,$thetadeg;
		
	#if the angle is less than the resolution we take H directly
	if ($thetarad < RESRAD && $thetarad > -(RESRAD) ) { 
		$dist = $H/cos($psirad);
		print "dist: $dist\n";
		return $dist;
	} else {
		for (my $_x=1 ; $_x<=(MAXTERRAIN) ; $_x+=XINC) {	
			my $x;
			if ($thetarad <= RESRAD) {
			#if the angle is negative move away from the scanner in the -x direction
				$x = -$_x;
			} else {
			#if the angle is positive move away from the scanner in the x direction
				$x = $_x;
			}
			my $y = $H - $x/tan($thetarad);
			my $z = ($y-$H)*tan($psirad);
			#print "y: $y x: $x\n";
			#if we are below the terain, the ray has intersected and we're done
			if ($y < terrain($x,$z)) {					
				return distapx($x,$z);
				#return distgeo($x,$y,$z,$yp,$zp);
			}
 			$yp = $y;
 			$zp = $z;			
		}
	}
}

