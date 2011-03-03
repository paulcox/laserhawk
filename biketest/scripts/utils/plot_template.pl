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
use constant NOMTH => (IMGHEIGHT-(2*IMGBDR))/2;
use constant MAXTERRAIN => 1000; #1000cm = 10 m
use constant SCALE => (IMGHEIGHT-2*IMGBDR)/(2*MAXTERRAIN);

my $im = new GD::Image(IMGWIDTH,IMGHEIGHT,1);
my $white = $im->colorAllocate(255,255,255);
my $black = $im->colorAllocate(0,0,0);       
my $red = $im->colorAllocate(255,0,0);      
my $blue = $im->colorAllocate(0,0,255);
my $green = $im->colorAllocate(0,255,0);
my $purple = $im->colorAllocate(255,255,0);
my $yellow = $im->colorAllocate(0,255,255);

my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $dirname = $ARGV[0];
my $scncnt = $ARGV[1];


my $file2open = sprintf "Opening $path/$dirname/scan%06d",$scncnt;
open SCNLOG, "<$file2open";

initimg();

writeimg();

close SCNLOG;

exit;

###############################################################################
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
	#add scale bar ticks and text
	for (my $i=0 ; $i<=IMGHEIGHT-2*IMGBDR ; $i+=10 ) {
		my $txt;
		
		#vertical scale bar
		if ($i%50 == 0) {
			$txt = sprintf "%d",-($i/SCALE-1000);
			$im->string(gdSmallFont,5,IMGBDR+$i-5,$txt,$white);
			$im->line(IMGBDR/2+10,IMGBDR+$i,IMGBDR/2+18,IMGBDR+$i,$green);
		} else {
			$im->line(IMGBDR/2+10,IMGBDR+$i,IMGBDR/2+15,IMGBDR+$i,$green);
		}
		
		#horiz scale bar
		if ($i%50 == 0) {
			$txt = sprintf "%d",$i/SCALE-1000;
			$im->string(gdSmallFont,IMGBDR+$i-10,IMGHEIGHT-(IMGBDR)/2+5,$txt,$white);
			$im->line(IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2+3,IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2-8,$green);
		} else {
			$im->line(IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2,IMGBDR+$i,IMGHEIGHT-(IMGBDR)/2-5,$green);
		}
	}
	#Print parameters
	$im->string(gdSmallFont,400,IMGBDR/2,"File:$dirname cm  Count:$scncnt ",$white);
	$im->string(gdSmallFont,20,IMGBDR/6,"laser scanner log plotter",$blue);
	$im->string(gdSmallFont,20,IMGBDR/2,"Paul Cox 2011 LAAS/CNRS",$blue);

	open(IMG, ">$path/$dirname-imgs/scan$scncnt.png") or die $1;
	binmode IMG;
	print "Writing image file : $path/$dirname-imgs/scan$scncnt.png\n";
	print IMG $im->png;
	close(IMG);
}
