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
use constant NOMTH => 50;
use constant SCALE => (IMGHEIGHT-2*IMGBDR)/(2*MAXTERRAIN);

#height of hokyo sensor relative to terrain (in cm)
my $H = 700;

# create a new image and define globals (colors)
my $im = new GD::Image(IMGWIDTH,IMGHEIGHT);
# allocate some colors
my $white = $im->colorAllocate(255,255,255);
my $black = $im->colorAllocate(0,0,0);       
my $red = $im->colorAllocate(255,0,0);      
my $blue = $im->colorAllocate(0,0,255);
my $green = $im->colorAllocate(0,255,0);

initimg();

showterrain();

my $i=0;
my $scnpt;
my @distance;
my $thetarad;
my $thetadeg;

my $psideg = 0.25;
#my $psideg = 45;
my $psirad = deg2rad($psideg);
printf "psirad: %1.4f (%3.2f)\n",$psirad,$psideg;

print "scnpt   thetarad     (thetadeg)\n";
print "-----   --------     ----------\n";

#get distance between hokyo and terrain for each ray
#for ($scnpt = HALFWAY-45/RESDEG ; $scnpt <= HALFWAY+45/RESDEG ; $scnpt+=1) {
for ($scnpt = HALFWAY-65/RESDEG ; $scnpt <= HALFWAY+65/RESDEG ; $scnpt+=4) {
	$thetarad = ($scnpt - HALFWAY)*RESRAD;
	$thetadeg = ($scnpt - HALFWAY)*RESDEG;
	
	$distance[$i] = getdist();
	printf " dist : %4.2f\n",$distance[$i];
	#if (($scnpt % 25/RESDEG) == 0) 
	{
		$im->line(	IMGWIDTH/2, IMGBDR+NOMTH+$H*SCALE/cos($psirad),
					IMGWIDTH/2+$distance[$i]*sin($thetarad)*SCALE, IMGBDR+NOMTH+$H*SCALE/cos($psirad)-$distance[$i]*cos($thetarad)*SCALE,$red);
	}
	$i++;
}

writeimg();

exit;

###############################################################################
# SUBROUTINES

sub initimg {
	#we're going to assume the origin (0,0) is at bottom left as we are flipping
	#vertically before writing out the file.

    # make the background transparent and interlaced
    $im->transparent($white);
    $im->interlaced('true');

    # Put a black frame around the picture
    $im->rectangle(0,0,IMGWIDTH-1,IMGHEIGHT,$black);

	$im->line(IMGBDR,IMGBDR+NOMTH,IMGWIDTH-(IMGBDR),IMGBDR+NOMTH,$black);
    $im->arc(IMGWIDTH/2,IMGBDR+NOMTH+$H*SCALE,10,10,135,45,$red);

	#draw scale bars
	$im->line(IMGBDR/2,IMGBDR/2,IMGWIDTH-(IMGBDR/2),IMGBDR/2,$green);
	$im->line(IMGBDR/2,IMGBDR/2,IMGBDR/2,IMGHEIGHT-(IMGBDR/2),$green);
	
	for (my $i=0 ; $i<IMGHEIGHT-(IMGBDR) ; $i+=10 ) {
		#vertical scale
		$im->line(IMGBDR/2,IMGBDR/2+$i,IMGBDR/2+5,IMGBDR/2+$i,$green);
		#horiz scale
		$im->line(IMGBDR/2+$i,IMGBDR/2,IMGBDR/2+$i,IMGBDR/2+5,$green);
	}
}

sub writeimg {
	#write image to file
	$im->flipVertical();
	open(IMG, ">terrain.png") or die $1;
	binmode IMG;
	print IMG $im->png;
	close(IMG);
}

#show terrain
sub showterrain {

	print "Terrain Elevation\n";
	for (my $i=-(MAXTERRAIN) ; $i<MAXTERRAIN ; $i+=2) {
		my $t = terrain($i,0);
		$im->setPixel(IMGWIDTH/2+$i*SCALE,IMGBDR+NOMTH+$t*SCALE,$blue);
		#print " $i $t\n";
	}

}

#pass the terrain subroutine a x value and it return you the terrain height
sub terrain {
	my $theight;
	my $xval = $_[0];
	my $zval = $_[1]; 
	
	if ($xval <= MAXTERRAIN && $xval >= -(MAXTERRAIN)){
		switch ($xval) {
			#case { ($xval <= -500) || ($xval >= 500) } {$theight = 100*sin($xval/pi);}
			case { ($xval <= -500) || ($xval >= 500) } {$theight = abs($xval*.5)-500;}
			case {  ($xval < -200) || ($xval > 200)  } {$theight = 30;}
			case { ($xval >= -200) || ($xval <= 200) } {$theight = abs($xval*.5);}
#			else { print "terrain error!";$theight = 0;}
		}
	} else {
		print "Warning: out of bounds: $xval\n";
		$theight = 0;
	}	

	return $theight;
}


#subroutine to calculate distance
sub getdist {
	my $yp = $H;
	my $zp = 0;
	my $dist = 1;
	
	printf "pt $scnpt: theta %1.4f (%1.2f) ",$thetarad,$thetadeg;
		
	
	#if the angle is less than the resolution we take H directly
	if ($thetarad < RESRAD && $thetarad > -(RESRAD)) { 
		$dist = $H/sin($psirad);
		#print "dist: $dist\n";
		return $dist;
	} else {
		#if the angle is positive
		#move away from the scanner in the x direction
		if ($thetarad >= RESRAD) {
			for (my $x=1 ; $x<=MAXTERRAIN ; $x+=XINC){
				my $y = $H - $x/tan($thetarad);
				my $z = ($x/tan($thetarad))*tan($psirad);
				#print "y: $y z: $z\n";
				#if we are below the terrain, the ray has intersected and we're done
				if ($y < terrain($x,$z)) {
# 					my $ydist = $x/sin($thetarad);
# 					#my $zdist = $z/sin($psirad);
# 					$dist = sqrt($ydist**2 + $z**2);
# 					my $d2muchx = ((terrain($x,$z)-$y)*(XINC)/($yp+terrain($x,$z)-$y-terrain($x-1,$z-1)))/sin($thetarad);
# 					#$dist -= $d2muchx;
# 					my $d2much = sqrt($d2muchx**2 + ((terrain($x,$z)-$y)*($z-$zp)/($yp+terrain($x,$z)-$y-terrain($x-1,$z-1)))**2);
# 					$dist -= $d2much;
# 					print "   dist: $dist ($d2muchx) ($d2much)\n";

					#test new approximation (not finished.)
					my $Ht = (terrain($x,$z)+terrain($x-1,$z)+terrain($x,$z-1)+terrain($x-1,$z-1))/4;
					my $xest = ($H-$Ht)*tan($thetarad);
					my $zest = ($H-$Ht)*tan($psirad);
					my $distapx = sqrt( ($H-$Ht)**2+($xest)**2+($zest)**2);
					#print "   distapx: $distapx | Ht $Ht | x $xest | z $zest\n";
					return $distapx;
#					return $dist;
					#last;
				}
#				$yp = $y;
#				$zp = $z;
			}
		#if the angle is negative, we use the same code but with some sign changes
		#move away from the scanner in the -x direction
		} else {
			for (my $x=-1 ; $x>=-(MAXTERRAIN) ; $x-=XINC){
				my $y = $H + $x/tan(-$thetarad);
				my $z = ($y-$H)*tan($psirad);
				#print "y: $y x: $x\n";
				#if we are below the terain, the ray has intersected and we're done
				if ($y < terrain($x,$z)) {
#  					my $ydist = -$x/sin(-$thetarad);
#  					my $zdist = $z/sin($psirad);
#  					$dist = sqrt($ydist**2 + $z**2);
#  					my $d2muchx = ((terrain($x,$z)-$y)*(XINC)/($yp+terrain($x,$z)-$y-terrain($x+1,$z+1)))/sin(-$thetarad);
#  					#following line has issue somewhere
#  					my $d2much = sqrt($d2muchx**2 + ((terrain($x,$z)-$y)*($z-$zp)/($yp+terrain($x,$z)-$y-terrain($x+1,$z+1)))**2);
#  					$dist -= $d2much;
#  					print "   dist: $dist ($d2muchx) ($d2much)\n";
					
					#test new approximation
					my $Ht = (terrain($x,$z)+terrain($x-1,$z)+terrain($x,$z-1)+terrain($x-1,$z-1))/4;
					my $xest = ($H-$Ht)*tan($thetarad);
					my $zest = ($H-$Ht)*tan($psirad);
					my $distapx = sqrt( ($H-$Ht)**2+($xest)**2+($zest)**2);
					#print "   distapx: $distapx | Ht $Ht | x $xest | z $zest\n";
					return $distapx;
#					return $dist;
					#last;
				}
# 				$yp = $y;
# 				$zp = $z;			
			}
		}
	}
}

