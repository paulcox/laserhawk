#!/usr/bin/perl
###############################################################################
#Author: Paul Cox
#Date  : Feb 2011
#
# plotlogs.pl : Script that uses scan logs, video frames, and google maps to 
#	generate png images showing all relevant info.
#
#
#
#Notes
#TODO: check arguments in more detail
#TODO: detect road angle from horizon (bike lean) and corelate to MTI angle
#TODO: look for 'all' on ARGV1 and do them all if so
#TODO: plot summary at end with use GD::Graph::lines; MTI raw sensors, angles,
#		maxdist, etc.
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
#use constant MAXTERRAIN => 1500; #1500cm = 15 m
use constant MAXTERRAIN => 3000;
use constant SCALE => (IMGHEIGHT-2*IMGBDR)/(2*MAXTERRAIN);
#hokuyo constants
use constant RESDEG => 360/1440;
use constant RESRAD => (2*pi)/1440;
use constant HALFWAY => 1080/2;
use constant LASTSCAN => 1080;
use constant DEBUG => 0;
use constant VIDOFFSET => 9.1;
use constant SHOWSTATS => 1;

#usage
#./plotlogs.pl 2011-03-01-10-32-19 100 200 1
if ($#ARGV !=3) {print "Specify start and stop indexes and skip param.\nExample: ./plotlogs.pl 1 10 1\n";exit;}

#my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $path = `pwd`;
chomp $path;
#$path .= "/..";
#my $dirname = "2011-03-01-10-32-19";
my $dirname = $ARGV[0];
#whine if no subdir
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}
if (! -e "$path/$dirname-maps") {printf "No maps folder found\n"; exit;}
if (! -e "$path/$dirname-frames") {printf "No video frames folder found\n"; exit;}
#create imgs dir if not already there
`mkdir $path/$dirname-imgs` if (! -e "$path/$dirname-imgs" );

#my $firstlog = 0;
#my $lastlog = 100;
#my $skipnum = 1;
my $firstlog = $ARGV[1];
#we always process first scan so if user asks to start at 0 we need to change it to 1
$firstlog = 1 if (!$firstlog);
my $lastlog =  $ARGV[2];
my $skipnum = $ARGV[3];

printf "SCALE: ".SCALE."\n";

my $gcnt = 0;
my @deltats;
my @maxdists;

my ($ang1,$ang2,$ang3);
my ($im,$white,$black,$red,$blue,$green,$magenta,$cyan);
my $pscntime=0;

#issue: starting from non zero firstlog will not use correct map/frame
#fixing by always running first scan
my $time0 = 0;
$pscntime = plotlog(0);
$time0 = $pscntime;
print "time of first scan: $time0\n";
			
foreach my $scncnt ($firstlog..$lastlog) {
	if ($scncnt % $skipnum == 0 ){	
		printf "scan (%04d) %04d:",$scncnt,$gcnt;
		my $scntime = plotlog($scncnt);
		#store deltas for statistics
		$deltats[$gcnt++] = ($scntime - $pscntime)*1000;
		$pscntime = $scntime;
	}
}

#print deltats stats (there won't be any if this script is run for a single which is why we test if @deltats exists
if (@deltats and SHOWSTATS){
	my $tstat = Statistics::Descriptive::Full->new();
	$tstat->add_data(@deltats);
	printf "time deltas stats (ms) - cnt: %d min: %d max: %d mean: %d stddev: %d\n",
			$tstat->count(),$tstat->min(),$tstat->max(),$tstat->mean(), $tstat->standard_deviation();
}
if (@maxdists and SHOWSTATS){
	my $tstat = Statistics::Descriptive::Full->new();
	$tstat->add_data(@maxdists);
	printf "max distance stats (mm) - cnt: %d min: %d max: %d mean: %d stddev: %d\n",
			$tstat->count(),$tstat->min(),$tstat->max(),$tstat->mean(), $tstat->standard_deviation();
}
		
exit;

###############################################################################

sub plotlog {
	my $scncnt = $_[0];
	my $file2open = sprintf "$path/$dirname/scan%06d.txt",$scncnt;

	printf "Opening scan log : $file2open\n" if (DEBUG);
	open SCNLOG, "<$file2open" or die $!;

	initimg();

	my $lcnt = 0; my $lcnt2 = 0;
	my $dist = 0;
	my $time = 0;
	my $maxdist = 0;
	my $mindist = 10; #cm
	$ang1 = 0; #needs to be reset so below we know to grab it off first line
	
	#grab each scanpoint in log and plot on image
	while (<SCNLOG>){
		chomp;
		#mti data now at start of log thanks to putmtidata script
		if ($ang1 == 0 ) {
			#extract angles and position from mti data line
			#$_ =~ /^(.*).*EUL(.*)POS/;
			#no match print error
			#if (!$1){
			#	print "ERROR: didn't find angles in mti data on first line\n" ;
			#	exit;
			#}
			#my $junk = $1;
			#my $other = $2;
			
			#$_ =~ /^(.*)\sPOS/;
			#my $other = $1;
			#print "MTI fields :".$other."\n";
			#$other =~ /\s*(\S+)\s+(\S+)\s+(\S+)/;
			
			/^(.*)\sACC.*EUL(.*)POS\s(.*)\sVEL/;
			my $mtitime = $1;
			my $att = $2;
			my $gps = $3;
			
			$att =~ /\s+(.+)\s+(.+)\s+(.+)/;
			printf "att: %06.2f %06.2f %06.2f\n", $1, $2, $3;
			$ang1=$1;$ang2=$2;$ang3=$3;	
			
					
			#($ang1,$ang2,$ang3) = split(/\s+/,$_);
			next;
		}
		if ($lcnt == 1079) {
			$time = $_;
			next;
		}
		#($lcnt,$dist) = split(/       /,$_);
		($lcnt2,$dist) = split(/       /,$_);
		if (!$dist) {
			$time = $_;
			next;
		}
		next if ($dist < $mindist);
		$maxdist = $dist if ($dist>$maxdist);

		#my $theta = $lcnt*RESRAD;# - deg2rad(135);
		my $theta = $lcnt*RESRAD - deg2rad(135) + deg2rad($ang1);
		my $x = $dist*sin($theta)/10;
		my $y = $dist*cos($theta)/10;
		drawpt($x,$y);
		$lcnt++;
		#print $lcnt;
	}
	$maxdists[$gcnt] = $maxdist;
	printf " %6.2f maxdist: %05d ",$time-$time0,$maxdist;
	printf "MTI Angles: $ang1, $ang2, $ang3\n";

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
	my ($time,$timeinsec,$tenth,$vidtime);

	if ( $time0 == 0 ) {
		$time = $_[1];
		$timeinsec = "000";
		$vidtime = 0;
		$tenth = 0;
	} else {
		$time = sprintf "%d",$_[1]*1000; #ms since first
		$timeinsec = sprintf "%03d",$_[1]; #seconds since first
		$vidtime = $_[1]-(VIDOFFSET);
		if ($vidtime < 0){
			$vidtime = 0;
			$tenth = 0;
		} else {
			$vidtime =~ /\.(\d)/;
			$tenth = $1; #tenth of a second digit
			$vidtime = sprintf "%03d",$vidtime;
		}
	}
	#print "tenth:($tenth) ->$time0\n";

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

goto SKIP;
	#insert google map
	my $mapfile = sprintf "$path/$dirname-maps/map$timeinsec.jpg";

	if (-r $mapfile){
		my $im2 = new GD::Image(250,250,1);
		#open map file
		print "Opening Map $mapfile\n" if (DEBUG);
		$im2 = GD::Image->new($mapfile);
		if (!$im2) {print "ERROR: unable to create image from file.\n";exit;}
		#copy map into our image
		$im->copy($im2,IMGBDR,IMGHEIGHT/2,0,0,250,250);
	} else {
		#go get it!
		print "ERROR: Unable to open $mapfile\n";
	}

	my $thetime = sprintf "", $timeinsec, $tenth;


	#insert video frame
	my $framefile = sprintf "$path/$dirname-frames/frame%03d_%1d.jpg", $vidtime, $tenth;

	if (-r $framefile){
		my $im3 = new GD::Image(250,187,1);
		#open frame file
		print "Opening Frame $framefile\n"  if (DEBUG);
		$im3 = GD::Image->newFromJpeg($framefile,1);
		if (!$im3) {print "ERROR: unable to create image from file.\n";exit;}
		#copy map into our image
		$im->copy($im3,IMGWIDTH/2,IMGHEIGHT/2,0,0,250,187);
	} else {
		#go get it!
		#print "ERROR: Unable to open $framefile\n";
	}
SKIP:
	my $imgfile = sprintf "$path/$dirname-imgs/scan%06d.png",$scncnt;
	open(IMG, ">$imgfile") or die $1;
	binmode IMG;
	print "Writing image file : $imgfile\n" if (DEBUG);
	print IMG $im->png;
	close(IMG);
}
