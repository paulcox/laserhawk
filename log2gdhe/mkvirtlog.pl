#!/usr/bin/perl
#Author: Paul Cox
#Date  : Feb 2011
#Notes
#everything in cm
#the terrain is defined in the x direction
#x=0 corresponds to spot directly under the hokyo sensor
#x is positive to the right
#x is negative to the left
#kokuyo scans from left to right
#
#                          H 
#                          .
#                          .
#                          .
# -X <---|-----|-----|-----|-----|-----|-----|---> X
#       -15   -10   -5     0     5     10    15
#


use strict;
use warnings;
use Math::Trig;

use constant MAXTERRAIN => 1000; #1000cm = 10 m
use constant RESDEG => 360/1440;
use constant RESRAD => (2*pi)/1440;
use constant HALFWAY => 1080/2;
use constant LASTSCAN => 1080;
use constant XINC => 1;

#height of hokyo sensor relative to terrain (in cm)
my $H = 500;

showterrain();

my $i=0;
my $scnpt;
my @distance;

print "scnpt thetarad (thetadeg)\n";

#get distance between hokyo and terrain for each ray
for ($scnpt = HALFWAY-45/RESDEG ; $scnpt <= HALFWAY+45/RESDEG ; $scnpt+=1) {
	$distance[$i] = getdist();
	print " $distance[$i]\n";
	$i++;
}

exit;

#show terrain
sub showterrain {
	for (my $i=-(MAXTERRAIN) ; $i<MAXTERRAIN ; $i+=50) {
		my $t = terrain($i);
		print "$i $t\n";
	}
}

#pass the terrain subroutine a x value and it return you the terrain height
sub terrain {
	my $theight;
	my $xval = $_[0]; 
	
	if ($xval <= MAXTERRAIN && $xval >= -(MAXTERRAIN)){
		if (($xval <= -500) || ($xval >= 500)) { 
			#$theight = 100;
			$theight = 0;
		} else {
			if (($xval < -200) || ($xval > 200)) {
				$theight = 0;
			} else {
				#$theight = abs($xval*.5);
				$theight = 0;
			}
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
	my $dist = 1;
	
	my $thetarad = ($scnpt - HALFWAY)*RESRAD;
	my $thetadeg = ($scnpt - HALFWAY)*RESDEG;
	print "$scnpt $thetarad ($thetadeg)\n";
	
	#if the angle is less than the resolution we take H directly
	if ($thetarad < RESRAD && $thetarad > -(RESRAD)) { 
		$dist = $H;
		#print "dist: $dist\n";
		return $dist;
	} else {
		#if the angle is positive
		#move away from the scanner in the x direction
		if ($thetarad >= RESRAD) {
			for (my $x=1 ; $x<=MAXTERRAIN ; $x+=XINC){
				my $y = $H - $x/tan($thetarad);
				#print "y: $y\n";
				#if we are below the terain, the ray has intersected and we're done
				if ($y < terrain($x)) {
					$dist = $x/sin($thetarad);
					my $d2much = ((terrain($x)-$y)*(XINC)/($yp+terrain($x)-$y-terrain($x-1)))/sin($thetarad);
					$dist -= $d2much;
					#print "   dist: $dist\n";
					return $dist;
					#last;
				}
				$yp = $y;
			}
		#if the angle is negative, we use the same code but with some sign changes
		#move away from the scanner in the -x direction
		} else {
			for (my $x=-1 ; $x>=-(MAXTERRAIN) ; $x-=XINC){
				my $y = $H + $x/tan(-$thetarad);
				#print "y: $y x: $x\n";
				#if we are below the terain, the ray has intersected and we're done
				if ($y < terrain($x)) {
					$dist = -$x/sin(-$thetarad);
					my $d2much = ((terrain($x)-$y)*(XINC)/($yp+terrain($x)-$y-terrain($x+1)))/sin(-$thetarad);
					$dist -= $d2much;
					#print "   dist: $dist\n";
					return $dist;
					#last;
				}
				$yp = $y;			
			}
		}
	}
}

