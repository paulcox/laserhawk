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
#hokuyo scans from left to right
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
#use diagnostics;
use Math::Trig; #for sin/cos/etc
use GD; #to draw
use Switch;
use Math::Quaternion;
use Time::HiRes qw(gettimeofday);

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
use constant NOMTH => (IMGHEIGHT-(2*IMGBDR))/2;
use constant NOMTHZ => 150;
#With terrain from -1000..1000 and image 500 px wide, SCALE=1/4
use constant SCALE => (IMGHEIGHT-2*IMGBDR)/(2*MAXTERRAIN);
#DEBUG sets the terrain to all flat
use constant DEBUG => 0;
#set to 0 to use terrain bitmap for terrain height
use constant HARDCODEDTERRAIN => 1;
#draw cross section of terrain in image
use constant SHOWCS => 0;

###############################################################################

my ($psideg, $psirad,$phideg,$phirad);
my $subdir;
my $logenable = 0;

#height of hokyo sensor relative to terrain (in cm)
my $Hy;
#hokuyo displacement in theta direction
my $Hx;
#hokuyo displacement in psi direction
my $Hz;

readargs();

print "Hokuyo height : $Hy cm , position : $Hx,$Hz\n";
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
my $magenta = $im->colorAllocate(255,0,255);
my $cyan = $im->colorAllocate(0,255,255);

### Initialize globals
my $raycnt = 0;
my $scnpt;
my @distance;
my $thetarad;
my $thetadeg;


initimg();

showterrain();

printf "psirad: %1.4f (%3.2f)\n", $psirad, $psideg;
printf "phirad: %1.4f (%3.2f)\n", $phirad, $phideg;
print "scnpt   thetarad     (thetadeg)    dist\n";
print "-----   --------     ----------    ----------\n";

#draw little hokuyo
$im->arc(IMGWIDTH/2,IMGBDR+NOMTH+$Hy*SCALE,10,10,135,45,$red);

#get distance between hokyo and terrain for each ray
for ($scnpt = HALFWAY-135/RESDEG ; $scnpt < HALFWAY+135/RESDEG ; $scnpt+=1) {
#next one gives all 1081 points
#for ($scnpt = HALFWAY-135/RESDEG ; $scnpt <= HALFWAY+135/RESDEG ; $scnpt+=1) {
#for ($scnpt = HALFWAY-45/RESDEG ; $scnpt <= HALFWAY+45/RESDEG ; $scnpt+=2/RESDEG) {
	$thetarad = ($scnpt - HALFWAY)*RESRAD;
	$thetadeg = ($scnpt - HALFWAY)*RESDEG;
	
	$distance[$scnpt] = getdist();

	#replace distance of zero with 1 since hokuyo reports 1 if no distance reading
	if ($distance[$scnpt] == 0) {
		$distance[$scnpt]=1/10;
	} else {
		printf "pt $scnpt: theta %1.4f (%1.2f)   %4.2f\n",$thetarad,$thetadeg,$distance[$scnpt];
			#draw scan line	every 2 degrees
		if (($scnpt % 8) == 0) {
			#my $psidist = $Hy*SCALE/cos($psirad);
			my $vdist = $Hy*SCALE;		
			my $B = $distance[$scnpt]*SCALE/sqrt(1+tan($thetarad)**2+tan($psirad)**2);
			#show on elevation view
			$im->line(	IMGWIDTH/2,
						IMGBDR+NOMTH + $vdist,
						IMGWIDTH/2   + tan($thetarad)*$B,
						IMGBDR+NOMTH + $vdist - $B,
						$red);
			#show on bird's eye view		
			$im->line(	IMGWIDTH/2,
						IMGBDR+NOMTHZ +$Hz*SCALE,
						IMGWIDTH/2   + tan($thetarad)*$B,
						IMGBDR+NOMTHZ - tan($psirad)*$B +$Hz*SCALE,
						$red);
		}	
	}
	$raycnt++;
	printf LOG "%04d       %d\n",$raycnt,$distance[$scnpt]*10 if ($logenable); #multiply to 10 to go from cm to mm
}

writeimg();

if ($logenable) {
	my ($s,$us) = gettimeofday();
	my $timeofday = $s+$us/1000000;
	printf LOG "%f",$timeofday;
	close LOG;

	#1297965927.462918758 QUAT  0.072093  0.746111  0.661814 -0.011095 POS 377098.986 4824479.869    200.101  31T VEL   -0.0100    0.0200    0.0100
	my $q = Math::Quaternion::rotation($psirad,1,0,0);
	my $qtheta = $q->rotation_angle;
	my @v = $q->rotation_axis;
	my $mtiline = sprintf "%f QUAT  %f  %f  %f %f POS 0 0    %f  31T VEL   0    0    0\n",$timeofday,$qtheta,$v[0],$v[1],$v[2],$Hy/100;
	#print $mtiline;
	printf MTI $mtiline;
	close MTI;
}

exit;

###############################################################################
# SUBROUTINES

###############################################################################
#subroutine to calculate distance
sub getdist {
	#don't bother continuing over 60 degrees
	return 0 if ($thetarad < -(pi)/3 || $thetarad >pi/3);

	my $yp = $Hy;
	my $zp = $Hz;
	my $dist = 1;
		
	
	if ($thetarad < RESRAD && $thetarad > -(RESRAD) ) {
		#if the angles are less than the resolution we take height over terrain directly
		if ($psirad < RESRAD && $psirad > -(RESRAD) ) {	 
			$dist = $Hy/cos($psirad)-terrain($Hx,$Hz);
		#otherwise we propagate scan in y direction until we intersect terrain
		} else {
			for (my $z=1; $z<=MAXTERRAIN;$z++){
				my $y = $Hy - $z/tan($psirad);
				if ($y < terrain(0,$z)) {					
					$dist = distapx(0,$z);
				}	
				$yp = $y;		
			}
		}
		print "     dist: $dist\n";
		return $dist;
	} else {
		for (my $_x=1 ; $_x<=MAXTERRAIN ; $_x+=XINC) {	
			my $x;
			if ($thetarad < 0) {
			#if the angle is negative move away from the scanner in the -x direction
				$x = -$_x;
			} else {
			#if the angle is positive move away from the scanner in the x direction
				$x = $_x;
			}
			my $y = $Hy - $x/tan($thetarad);
			#below should potentially be $Hy-$y
			my $z = ($y-$Hy)*tan($psirad);
			#print "y: $y x: $x\n";
			#if we are below the terain, the ray has intersected and we're done
			if ($y < terrain($x,$z)) {					
				#return distapx($x,$z);
				return distgeo($x,$y,$z,$yp,$zp);
			}
 			$yp = $y;
 			$zp = $z;			
		}
	}
}

###Read command line arguments
sub readargs {
	#need a minimum of 6 arguments
	if ($#ARGV < 5){
	print "Usage: perl mkvirtlog.pl testname psideg phideg Hy Hx Hz <cnt>
		 testname will be used to create a subdirectory
		 psideg is the inclination of hokuyo away from vertically down 
		 phideg is rotation around vertical (aka y) axis (like yaw)
		 Hy Scanner height above terrain (cm)
		 Hx Scanner displacement in theta/scan direction (cm)
		 Hz Scanner displacement in psi direction (cm)
		 If cnt is specified, logging to file will be enabled 
		   (i.e. creation of MTI.out and scanxxx.txt)
		 example : perl mkvirtlog.pl test1 0.5 0.5 700 0 40 1\n";
	}

	#test for a valid number for psi (reg exp allows ints and floats)
	if ($ARGV[1] !~ /^[+-]?\ *(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$/){print "invalid psi\n";exit;}
	#test for a valid number for phi (reg exp allows ints and floats)
	if ($ARGV[2] !~ /^[+-]?\ *(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$/){print "invalid phi\n";exit;}
		#test for a valid number for Hy (reg exp allows ints and floats)
	if ($ARGV[3] !~ /^[+-]?\ *(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$/){print "invalid Hy\n";exit;}
		#test for a valid number for Hx (reg exp allows ints and floats)
	if ($ARGV[4] !~ /^[+-]?\ *(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$/){print "invalid Hx\n";exit;}
		#test for a valid number for Hz (reg exp allows ints and floats)
	if ($ARGV[5] !~ /^[+-]?\ *(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$/){print "invalid Hz\n";exit;}

	$subdir = $ARGV[0];
	if (!$subdir) {printf "please specify a subdirectory name\n"; exit;}
	`mkdir $subdir` if (! -e $subdir);
	`mkdir $subdir/imgs` if (! -e "$subdir/imgs");
	print "Files will be placed in $subdir directory\n";

	$psideg = $ARGV[1];
	#protect from divide by zero if no psi given or if user specifically asks for it
	$psideg = 0.25 if (!$psideg);
	$psirad = deg2rad($psideg);
	
	$phideg = $ARGV[2];
	#protect from divide by zero if no psi given or if user specifically asks for it
	$phirad = deg2rad($phideg);
	$Hy = $ARGV[3];
	$Hx = $ARGV[4];
	$Hz = $ARGV[5];

	#if the user doesn't want a log, only the image is created
	if ($#ARGV == 6){
		#test for a valid int for cnt
		if ($ARGV[6] !~ /^[+-]?\ *(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$/){print "invalid cnt\n";exit;}
		$logenable = 1;
		my $lognum = $ARGV[6];
		my $scanlogname = sprintf "scan%06d.txt",$lognum;
		open(LOG,">$subdir/$scanlogname");
		#we always append since the entries are timestamped and it doesn't hurt to add
		open(MTI,">>$subdir/MTI.out");
		print "Logging to $scanlogname and MTI.out\n";
	}
}

#create an image with border, scale bars, and nominal ground height
sub initimg {
	#we're going to assume the origin (0,0) is at bottom left as we are flipping
	#vertically before writing out the file.

    # make the background transparent and interlaced
    #$im->transparent($white);
    $im->interlaced('true');

    # Put a black frame around the picture
    $im->rectangle(0,0,IMGWIDTH-1,IMGHEIGHT,$black);

	#draw scale bars, first horiz
	$im->line(IMGBDR/2+10,IMGBDR/2,IMGWIDTH-(IMGBDR),IMGBDR/2,$cyan);
	#now vertical
	$im->line(IMGBDR/2+10,IMGBDR/2,IMGBDR/2+10,IMGHEIGHT-(IMGBDR),$cyan);
	
}


###############################################################################
#write image to file
sub writeimg {
	$im->flipVertical();
	#add scale bar ticks and text
	for (my $i=0 ; $i<=IMGHEIGHT-2*IMGBDR ; $i+=10 ) {
		my $txt;
		
		#vertical scale bar
		if ($i%50 == 0) {
			if ($i<=300) {
				$txt = sprintf "%d",-($i/SCALE-1000);
				$im->string(gdSmallFont,5,IMGBDR+$i-5,$txt,$magenta);
			} else {
				$txt = sprintf "%d",($i-350)/SCALE;
				$im->string(gdSmallFont,5,IMGBDR+$i-5,$txt,$green);
			}
			
			$im->line(IMGBDR/2+10,IMGBDR+$i,IMGBDR/2+18,IMGBDR+$i,$cyan);
		} else {
			$im->line(IMGBDR/2+10,IMGBDR+$i,IMGBDR/2+15,IMGBDR+$i,$cyan);
		}
		
		#horiz scale bar
		if ($i%50 == 0) {
			$txt = sprintf "%d",$i/SCALE-1000;
			$im->string(gdSmallFont,IMGBDR+$i-10,IMGHEIGHT-(IMGBDR)/2+5,$txt,$white);
			$im->line(IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2+3,IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2-8,$cyan);
		} else {
			$im->line(IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2,IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2-5,$cyan);
		}
	}
	#Print parameters
	$im->string(gdSmallFont,400,IMGBDR/6,"Hy: $Hy Hz: $Hz Hx: $Hx cm ",$white);
	$im->string(gdSmallFont,400,IMGBDR/2,"Psi: $psideg deg Phi: $phideg",$white);
	$im->string(gdSmallFont,20,IMGBDR/6,"Virtual terrain laser scanner and logger",$blue);
	$im->string(gdSmallFont,20,IMGBDR/2,"Paul Cox 2011 LAAS/CNRS",$blue);
	
	#label axes
	$im->string(gdSmallFont,IMGWIDTH-(IMGBDR)+8,IMGWIDTH/2-8,"-> +x",$blue);
	$im->stringUp(gdSmallFont,IMGWIDTH-(IMGBDR),IMGWIDTH/2-8,"-> +y",$magenta);
	
	$im->string(gdSmallFont,IMGWIDTH-(IMGBDR)+8,IMGHEIGHT-(NOMTHZ)-(IMGBDR)-8,"-> +x",$blue);
	$im->stringUp(gdSmallFont,IMGWIDTH-(IMGBDR),IMGHEIGHT-(NOMTHZ)-(IMGBDR)-8,"-> -z",$green);

	my $psiname = sprintf "%06.2f",$psideg;
	my $imgname = $psiname."_$Hy"."_$Hx"."_$Hz";
	open(IMG, ">$subdir/imgs/terrain$imgname.png") or die $1;
	binmode IMG;
	print "Writing image file : $subdir/imgs/terrain$imgname.png\n";
	print IMG $im->png;
	close(IMG);
}

###############################################################################
#show terrain
sub showterrain {

	my $cmapfile = "$subdir/imgs/colorhmap.png";
	print "checking if colorhmap file exists : ";
	
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
				my $ncol;
				if ($t==0) {
					$ncol = $black;
				} else {
					$ncol = $im2->colorAllocate(int($t+128),int($t+128),int($t+128));
					#my $ncol = $im->colorAllocate(int($row-50)/2,int($col-50)/2,0);
					if ($ncol == -1) {print "colorallocate failed\n";exit};
				}
				#$im2->setPixel($col+$Hx*SCALE,$row+Hz*SCALE,$ncol);
				#may need to apply translation her to make correspond to scan
				$im2->setPixel($col,$row,$ncol);
				$im2->colorDeallocate($ncol);
			}
		}
		open(IMG, ">$cmapfile") or die $1;
		binmode IMG;
		print "Writing colormap file : $cmapfile\n";
		print IMG $im2->png;
		close(IMG);
	}
	#copy colorhmap into our image
	$im->copy($im2,IMGBDR,IMGBDR,0,0,2*MAXTERRAIN*SCALE,2*MAXTERRAIN*SCALE);

	#show world reference axes
	#elevation x axis
	$im->line(IMGBDR,IMGBDR+NOMTH,IMGWIDTH-(IMGBDR),IMGBDR+NOMTH,$blue);
	#bird's eye x axis
	$im->line(IMGBDR,IMGBDR+NOMTHZ,IMGWIDTH-(IMGBDR),IMGBDR+NOMTHZ,$blue);
	#y axis for elevation 
	$im->line(IMGWIDTH/2,IMGBDR+(NOMTH+NOMTHZ)/2,IMGWIDTH/2,IMGHEIGHT-(IMGBDR),$magenta);
	#z axis for bird's eye view
	$im->line(IMGWIDTH/2,IMGBDR,IMGWIDTH/2,IMGBDR+(NOMTH+NOMTHZ)/2,$green);
	#draw little hokuyo
    $im->arc(IMGWIDTH/2,IMGBDR+NOMTH+$Hy*SCALE,10,10,135,45,$red);

	print "Terrain Cross-section Elevation\n";
	if (SHOWCS == 1) {
		for (my $i=-(MAXTERRAIN) ; $i<MAXTERRAIN ; $i+=1/SCALE) {
			my $t = terrain($i,0);
			$im->setPixel(IMGWIDTH/2+$i*SCALE,IMGBDR+NOMTH+$t*SCALE,$blue);
			#print " $i $t\n";
		}
	}
}

sub drawpt {
	my $x = $_[0]; #distance in x(theta) direction
	my $y = $_[1]; #height of terrain
	my $z = $_[2]; #distance in z(psi) direction
	my $color = $_[3]; #distance in z(psi) direction
	
	$im->setPixel(IMGWIDTH/2+$x*SCALE,IMGBDR+NOMTH+$y*SCALE,$color);
	$im->setPixel(IMGWIDTH/2+$x*SCALE,IMGBDR+NOMTHZ+$z*SCALE,$color);
}

###############################################################################
#pass the terrain subroutine a x value and it return you the terrain height
sub terrain {
	my $theight;
	my ($xval,$zval) = @_;

	return 0 if (DEBUG);
	
	#rotate 2d
	my $mag = sqrt($xval**2+$zval**2);
	my $dir = atan2($zval,$xval)-$phirad;
	$xval = cos($dir)*$mag;
	$zval = sin($dir)*$mag;
	#translate: if scanner moves positive, terrain moves negative
	$xval -= $Hx;
	$zval -= $Hz;
	
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
			#print "Warning: out of bounds: $xval\n";
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

	my $Ht = (terrain($x,$z) + terrain($x-1,$z) + terrain($x,$z-1) + terrain($x-1,$z-1)) / 4;
	my $xest = ($Hy-$Ht) * tan($thetarad);
	my $zest = ($Hy-$Ht) * tan($psirad);
	my $distapx = sqrt( ($Hy-$Ht)**2 + ($xest)**2 + ($zest)**2);
	#print "   distapx: $distapx | Ht $Ht | x $xest | z $zest\n";
	return $distapx;
}

###############################################################################
#the following code calculates the exact distance using a linear interpolation
#between terrain points.
sub distgeo {
	my $x = $_[0]; #distance in x(theta) direction
	my $y = $_[1]; #height of terrain
	my $z = $_[2]; #distance in z(psi) direction
	my $yp = $_[3]; #height of terrain before going below
	my $zp = $_[4]; #distance in zprevious inclination
	#no $xp is needed as we assume x increments by one at each iteration
	my ($d2muchx,$d2muchy,$d2much);

	my $psidist = $x/sin($thetarad);
	my $dist = sqrt($psidist**2 + $z**2);
	if ($thetarad >= RESRAD) {
		$d2muchx = ((terrain($x,$z)-$y)*(XINC)/($yp+terrain($x,$z)-$y-terrain($x-1,$z-1))) / sin($thetarad);
		$d2muchy = ((terrain($x,$z)-$y)*($z-$zp)/($yp+terrain($x,$z)-$y-terrain($x-1,$z-1)));
		$d2much = sqrt($d2muchx**2 + $d2muchy**2);
		drawpt($x-$d2muchx,$y-$d2muchy,$z,$magenta) if (($scnpt % 2) == 0);
	} else {
		$d2muchx = ((terrain($x,$z)-$y)*(XINC)/($yp+terrain($x,$z)-$y-terrain($x+1,$z-1))) / sin(-$thetarad);
		$d2muchy = ((terrain($x,$z)-$y)*($z-$zp)/($yp+terrain($x,$z)-$y-terrain($x+1,$z-1)));
		$d2much = sqrt($d2muchx**2 + $d2muchy**2);
		drawpt($x+$d2muchx,$y-$d2muchy,$z,$magenta) if (($scnpt % 2) == 0);
	}
	#will potentially need to detect negative psi?
	
	#print "   dist: $dist ($d2muchx) ($d2much)\n";
	$dist -= $d2much;
	return $dist;
}
