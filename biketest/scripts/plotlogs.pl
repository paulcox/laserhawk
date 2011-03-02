#!/usr/bin/perl
###############################################################################
#Author: Paul Cox
#Date  : Feb 2011
#Notes
###############################################################################

#pragmas and modules
use strict;
use warnings;
#use diagnostics;
use Math::Trig; #for sin/cos/etc
use GD; #to draw

#image related constants
use constant IMGHEIGHT => 600;
use constant IMGWIDTH => 600;
use constant IMGBDR => 50;
#nominal terrain height
use constant NOMTH => (IMGHEIGHT)/2;
use constant MAXTERRAIN => 1500; #1500cm = 15 m
use constant SCALE => (IMGHEIGHT-2*IMGBDR)/(2*MAXTERRAIN);
#hokuyo constants
use constant RESDEG => 360/1440;
use constant RESRAD => (2*pi)/1440;
use constant HALFWAY => 1080/2;
use constant LASTSCAN => 1080;

my $im = new GD::Image(IMGWIDTH,IMGHEIGHT,1);
my $white = $im->colorAllocate(255,255,255);
my $black = $im->colorAllocate(0,0,0);       
my $red = $im->colorAllocate(255,0,0);      
my $blue = $im->colorAllocate(0,0,255);
my $green = $im->colorAllocate(0,255,0);
my $purple = $im->colorAllocate(255,255,0);
my $yellow = $im->colorAllocate(0,255,255);

#usage
#./plotlogs.pl 2011-03-01-10-32-19 1 MTI_test3.out
#TODO check arguments
#TODO integrate with animatelogs and determine time between scans to show refresh rate in image
#TODO detect road angle from horizon (bike lean)

my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $dirname = $ARGV[0];
my $scncnt = $ARGV[1];
my $mtilogname = $ARGV[2];

my $file2open = sprintf "$path/$dirname/scan%06d.txt",$scncnt;
printf "Opening scan log : $file2open\n";
open SCNLOG, "<$file2open" or die $!;

printf "SCALE: ".SCALE."\n";

initimg();

my $cnt=0;
my $dist;
my $time=0;
my $maxdist=0;
my ($ang1,$ang2,$ang3);

#grab each scanpoint in log and plot on image
while (<SCNLOG>){
	chomp;
	if ($time != 0 ) {($ang1,$ang2,$ang3) = split(/ /,$_); last;}
	if ($cnt == 1079) {$time=$_ ; next;}
	($cnt,$dist) = split(/       /,$_);
	$maxdist = $dist if ($dist>$maxdist);
	my $theta = $cnt*RESRAD - deg2rad(135); 
	my $x = $dist*sin($theta)/10;
	my $y = $dist*cos($theta)/10;
	drawpt($x,$y);
}

printf "total cnt: %d maxdist $maxdist time: %f\n",$cnt+1,$time;
printf "MTI Angles: $ang1 $ang2 $ang3\n";

writeimg();

close SCNLOG;

exit;

###############################################################################
#coordinates defined by view from bike rider:
#positive y is towards the bottom of the image (hokuyo facing down)
#assuming hokuyo scan is ccw viewed from top (cw from bot)
#define theta equal zero for scan directly in front of hokuyo
#when theta is negative, x needs to be positive
sub drawpt {
	my $x = -$_[0]; #distance in x(theta) direction
	my $y = -$_[1]; #height of terrain
	#my $color = $_[3]; #distance in z(psi) direction
	my $color = $red;
	
	$im->setPixel(IMGWIDTH/2+$x*SCALE,IMGBDR+NOMTH+$y*SCALE,$color);
}

sub initimg {
	#we're going to assume the origin (0,0) is at bottom left as we are flipping
	#vertically before writing out the file.

    # make the background transparent and interlaced
    #$im->transparent($white);
    $im->interlaced('true');

    # Put a black frame around the picture
    $im->rectangle(0,0,IMGWIDTH-1,IMGHEIGHT,$black);

	#draw scale bars, first horiz
	$im->line(IMGBDR/2+10,IMGBDR/2,IMGWIDTH-(IMGBDR),IMGBDR/2,$green);
	#now vertical
	$im->line(IMGBDR/2+10,IMGBDR/2,IMGBDR/2+10,IMGHEIGHT-(IMGBDR),$green);
	
}


###############################################################################
#write image to file
sub writeimg {
	$im->flipVertical();
	#add scale bar ticks and text (now: 0..500 every 10)
	for (my $i=0 ; $i<=IMGHEIGHT-2*IMGBDR ; $i+=10 ) {
		my $txt;
		
		#vertical scale bar
		if ($i%50 == 0) {
			$txt = sprintf "%d",-($i-(IMGHEIGHT-2*IMGBDR)/2)/SCALE;
			$im->string(gdSmallFont,5,IMGBDR+$i-5,$txt,$white);
			$im->line(IMGBDR/2+10,IMGBDR+$i,IMGBDR/2+18,IMGBDR+$i,$green);
		} else {
			$im->line(IMGBDR/2+10,IMGBDR+$i,IMGBDR/2+15,IMGBDR+$i,$green);
		}
		
		#horiz scale bar
		if ($i%50 == 0) {
			$txt = sprintf "%d",($i-(IMGHEIGHT-2*IMGBDR)/2)/SCALE;
			$im->string(gdSmallFont,IMGBDR+$i-10,IMGHEIGHT-(IMGBDR)/2+5,$txt,$white);
			$im->line(IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2+3,IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2-8,$green);
		} else {
			$im->line(IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2,IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2-5,$green);
		}
	}
	
	#write MTI Angles
	$im->string(gdSmallFont,300,IMGBDR/6,$ang1,$red);
	$im->string(gdSmallFont,370,IMGBDR/6,$ang2,$white);
	$im->string(gdSmallFont,440,IMGBDR/6,$ang3,$blue);
	
	#Print parameters
	$im->string(gdSmallFont,300,IMGBDR/2,"File: $dirname  Count: $scncnt ",$white);
	$im->string(gdSmallFont,20,IMGBDR/6,"laser scanner log plotter",$blue);
	$im->string(gdSmallFont,20,IMGBDR/2,"Paul Cox 2011 LAAS/CNRS",$blue);

	my $imgfile = sprintf "$path/$dirname-imgs/scan%06d.png",$scncnt;
	open(IMG, ">$imgfile") or die $1;
	binmode IMG;
	print "Writing image file : $imgfile\n";
	print IMG $im->png;
	close(IMG);
}
