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
use Statistics::Descriptive; #for time stats

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

#usage
#./plotlogs.pl 2011-03-01-10-32-19 100 200 1
#TODO check arguments
#TODO integrate with animatelogs and determine time between scans to show refresh rate in image
#TODO detect road angle from horizon (bike lean)
#google map of the track 

my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $dirname = "2011-03-01-10-32-19";
#my $dirname = $ARGV[0];
#whine if no subdir
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}
#create imgs dir if not already there
#`mkdir $path$dirname-imgs` if (! -e "$path/$dirname-imgs" );
#`mkdir $path/$dirname-maps` if (! -e "$path/$dirname-maps" );

my $firstlog = 1; #$ARGV[1]
my $lastlog = 2; #$ARGV[2]
my $skipnum = 1; #$ARGV[3]

printf "SCALE: ".SCALE."\n";

my $time0 = 0;
my $cnt = 0;
my @deltats;

my ($ang1,$ang2,$ang3);
my ($im,$white,$black,$red,$blue,$green,$magenta,$cyan);

foreach my $scncnt ($firstlog..$lastlog) {



	if ($scncnt % $skipnum == 0 ){	
		print "plotting scan ($scncnt) $cnt\n";
		my $scntime = plotlog($scncnt);
		if ($time0 == 0){
			print "time of first scan: $scntime\n";
			$time0 = $scntime;
		} else {
			#store deltas for running statistics (mean, min, max, stand dev)
			$deltats[$cnt++] = ($scntime - $time0)*1000;
		}
	}
}

#print deltats stats
my $tstat = Statistics::Descriptive::Full->new();
$tstat->add_data(@deltats); 
printf "time deltas stats (ms) - cnt: %d min: %d max: %d mean: %d stddev: %d\n",
		$tstat->count(),$tstat->min(),$tstat->max(),$tstat->mean(), $tstat->standard_deviation();
		
exit;

###############################################################################

sub plotlog {
	my $scncnt = $_[0];
	my $file2open = sprintf "$path/$dirname/scan%06d.txt",$scncnt;

	printf "Opening scan log : $file2open\n";
	open SCNLOG, "<$file2open" or die $!;

	initimg();

	my $cnt = 0;
	my $dist = 0;
	my $time = 0;
	my $maxdist = 0;
	my $mindist = 10; #cm
	$ang1=0; #needs to be reset so below we know to grab it off first line
	
	#grab each scanpoint in log and plot on image
	while (<SCNLOG>){
		chomp;
		#angles now at start of log thanks to putangles script
		if ($ang1 == 0 ) {($ang1,$ang2,$ang3) = split(/\s+/,$_);next;}
		if ($cnt == 1079) {$time=$_ ; next;}
		($cnt,$dist) = split(/       /,$_);
		next if ($dist < $mindist);
		$maxdist = $dist if ($dist>$maxdist);
		my $theta = $cnt*RESRAD - deg2rad(135);
		my $x = $dist*sin($theta-deg2rad($ang2))/10;
		my $y = $dist*cos($theta-deg2rad($ang2))/10;
		drawpt($x,$y);
	}

	printf "total cnt: %d maxdist $maxdist time: %f\n",$cnt+1,$time-$time0;
	printf "MTI Angles: $ang1 $ang2 $ang3\n";

	writeimg($scncnt, $time-$time0);
	close SCNLOG;

	return $time;
}

#coordinates defined by view from bike rider:
#positive y is towards the bottom of the image (hokuyo facing down)
#assuming hokuyo scan is ccw viewed from top (cw from bot)
#define theta equal zero for scan directly in front of hokuyo
#when theta is negative, x needs to be positive
sub drawpt {
	my $x = -$_[0]; #distance in x(theta) direction
	my $y = -$_[1]; #height of terrain
	my $color = $red;
	
	$im->setPixel(IMGWIDTH/2+$x*SCALE,IMGBDR+NOMTH+$y*SCALE,$color);
}

sub initimg {
	$im = new GD::Image(IMGWIDTH,IMGHEIGHT,1);
	$white = $im->colorAllocate(255,255,255);
	$black = $im->colorAllocate(0,0,0);       
	$red = $im->colorAllocate(255,0,0);      
	$blue = $im->colorAllocate(0,0,255);
	$green = $im->colorAllocate(0,255,0);
	$magenta = $im->colorAllocate(255,255,0);
	$cyan = $im->colorAllocate(0,255,255);
	
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
	my $scncnt = $_[0];
	my $time;
	if ( $time0 == 0 ) {
		$time = $_[1];
	} else {
		$time = sprintf "%d",$_[1]*1000;
	}

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
	$im->string(gdSmallFont,IMGWIDTH/2-70,IMGBDR/6,"Angles:",$white);
	$im->string(gdSmallFont,IMGWIDTH/2-20,IMGBDR/6,$ang1,$red);
	$im->string(gdSmallFont,IMGWIDTH/2+50,IMGBDR/6,$ang2,$white);
	$im->string(gdSmallFont,IMGWIDTH/2+120,IMGBDR/6,$ang3,$blue);
	
	#Print parameters
	$im->string(gdSmallFont,IMGWIDTH/2-70,IMGBDR/2,"File: $dirname  Count: $scncnt Time: $time ms",$white);
	$im->string(gdSmallFont,IMGBDR/2,IMGBDR/6,"laser scanner log plotter",$blue);
	$im->string(gdSmallFont,IMGBDR/2,IMGBDR/2,"Paul Cox 2011 LAAS/CNRS",$blue);

	my $im2 = new GD::Image(250,250,1);
	my $timeinsec;
	
	if ($time0 != 0) {
		$timeinsec = sprintf "%03d",$time/1000;
	} else {
		$timeinsec = "000";
	}
	my $mapfile = sprintf "$path/$dirname-maps/map$timeinsec.jpg";
	#insert google map
	if (-r $mapfile){
		#open map file
		print "Opening Map $mapfile\n";
		$im2 = GD::Image->new($mapfile);
	} else {
		#go get it!
		print "ERROR: Unable to open $mapfile\n";
	}
	#copy map into our image
	$im->copy($im2,IMGBDR,IMGHEIGHT/2,0,0,250,250);

	my $imgfile = sprintf "$path/$dirname-imgs/scan%06d.png",$scncnt;
	open(IMG, ">$imgfile") or die $1;
	binmode IMG;
	print "Writing image file : $imgfile\n";
	print IMG $im->png;
	close(IMG);
}
