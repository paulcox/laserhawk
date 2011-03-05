#!/usr/bin/perl
###############################################################################
#Author: Paul Cox
#Date  : Feb 2011
#
# getmaps.pl : script to get google maps and video frames ready for plotting
#
# The script first looks throught the scan files to determine the start/end 
# times (epoch), then estimates a lat/lon center for the google maps (so the 
# map window stays constant), then grabs a google map for each second (using 
# the position reported in the scan at the appropriate time offset)and a 
# video frame for every tenth of a second.
#
# TODO:
#	1. Implement we_moved() to remove need to generate map when we don't move
#	2. Remove use of $scansps in getposition()
#	3. Reimplement findcenter() to remove assumption that middle time = middle pos
#
# Notes :
###############################################################################

#pragmas and modules
use strict;
use warnings;
use Geo::Coordinates::UTM;
use Image::Grab;

use constant GOOGURL => "http://maps.google.com/maps/api/staticmap";

sub findtimes;
sub findcenter;
sub getposition;

#require two args
if ($#ARGV !=1) {print "Specify scan folder name and movie name\nExample: ./getmaps.pl 2011-03-01-10-32-19 MVI_0009.AVI\n";exit;}

#my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $path = `pwd`;
chomp $path;
$path .= "/..";
#my $dirname = "2011-03-01-10-32-19";
my $dirname = $ARGV[0];
#whine if no subdir
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}
#create maps dir if not already there
`mkdir $path/$dirname-maps` if (! -e "$path/$dirname-maps" );
`mkdir $path/$dirname-frames` if (! -e "$path/$dirname-frames" );

#my $movie = "MVI_0009.AVI";
my $movie = $ARGV[1];

my ($east,$north,$zone);
my $middlefile;
my $scansps;

#find out end time (in seconds relative from start) for the group of scans
my $lastsec = findtimes();
printf "last second $lastsec\n";

#figure out a center for the track => cntr (for now just use position at middle time)
#$center is a string of "lat,lon" values
my $center = findcenter();
printf $center."\n";

foreach my $sec (0..$lastsec) {
	#get map with marker for current position and centered on cntr
	#save it directory with number of second in name
	my $timeinsec = sprintf "%03d",$sec;
	#for each second of scan get a position
	#this sets $east,$north,$zone
	getposition($timeinsec);
	print "sec $sec: east $east north $north zone $zone\n";
	for my $tenth (0..9) {
		my $frame = sprintf "$path/$dirname-frames/frame%03d_%1d.jpg",$sec,$tenth;
		#printf "  Getting video frame $frame\n";
		#`ffmpeg  -itsoffset -$sec.$tenth -i $path/$movie -vcodec mjpeg -vframes 1 -an -f rawvideo -s 250x187 $frame`;
	}
	
	if (we_moved()) { 
		fetchmap($east,$north,$zone,$center,$timeinsec);
		#beware google imposes an unspecified rate limit along with a max of 1000 per user per day
		#last;
		sleep 3;
	} else {
		#link to previous;
	}
}

exit;

###############################################################################

#return end time (in seconds relative from start) for the group of scans
sub findtimes {

	my @scanlist = <$path/$dirname/scan*.txt>;
	my $firstfile = shift @scanlist;
	$middlefile = $scanlist[int($#scanlist/2)];
	my $lastfile = pop @scanlist;
	
	printf "first: %s\nlast: %s\n", $firstfile,$lastfile;
	
	#grab the time off the last line
	my $lasttime = `tail -n 1 $lastfile`;
	chomp $lasttime;
	print "last line blank\n" if (!$lasttime);
	#need to check if it's really a time otherwise bail
	if ($lasttime < 1287161422) {print "not a good time...\n";exit;}
	
	my $firsttime = `tail -n 1 $firstfile`;
	chomp $firsttime;
	print "last line blank\n" if (!$firsttime);
	#need to check if it's really a time otherwise bail
	if ($firsttime < 1287161422) {print "not a good time...\n";exit;}	
	
	print "first/last times: $firsttime $lasttime\n";
	
	$scansps = $#scanlist/($lasttime-$firsttime);
	return sprintf "%d", $lasttime-$firsttime;
}

#figure out a center for the track => cntr (for now just use position at middle time)
#$center is a string of "lat,lon" values
sub findcenter() {

	printf "middle file: $middlefile\n";
	my $mtidata = `head -1 $middlefile`;

	#grab gps data
	#$mtidata =~ /^.*POS\s+(.*)T/;
	$mtidata =~ /^.*POS\s+(.*)\sVEL/;
	#split into fields
	my @gpspos = split(/\s+/,$1);
	
	my ($lat,$lon) = utm_to_latlon('wgs84', $gpspos[3], $gpspos[0], $gpspos[1]);
	#$center should be a string of "lat,lon" values
	return sprintf "%f,%f",$lat,$lon;
}

sub we_moved {
#eventually have this look to see if we actually moved because if we didn't there is 
#no need to get a new map as the old one would do fine...

	our $peast = 0;
	our $pnorth = 0;
	
	#return 0;
	return 1;
}

sub getposition() {
	
	#shouldn't do it this way...TODO: change it
	my $file = sprintf "$path/$dirname/scan%06d.txt",$_[0]*$scansps;
	printf "$file\n";
	my $mtidata = `head -1 $file`;

	#grab gps data
	#$mtidata =~ /^.*POS\s+(.*)T/;
	#$mtidata =~ /^.*GPS\s+(.*)T/;
	$mtidata =~ /^.*POS\s+(.*)\sVEL/;
	#split into fields
	my @gpspos = split(/\s+/,$1);

	$east = $gpspos[0];
	$north = $gpspos[1];
	$zone = $gpspos[3];
}

sub fetchmap {
	# my $east = 377979.888;
	# my $north = 4825430.921;
	# my $zone = 31;
	my $e = $_[0];
	my $n = $_[1];
	my $z = $_[2];
	my $c = $_[3];
	my $t = $_[4];

	my ($lat,$lon) = utm_to_latlon('wgs84', $z, $e, $n);
	print "lat/lon/zo: $lat $lon $z \n";

	my $zoom = 16;
	#maptypes can be roadmap | satellite | hybrid | terrain
	my $stuff = 'size=256x256&maptype=satellite&sensor=false';
	my $mark1 = "markers=color:blue|label:HOME|$lat,$lon";
	my $url = GOOGURL."?center=$c&zoom=$zoom&$stuff&$mark1";
	my $pic = new Image::Grab;

	$pic->url($url);
	$pic->grab;

	# Now to save the image to disk
	my $mapfile = sprintf "$path/$dirname-maps/map%03d.jpg",$t;
	print "  ".$mapfile."\n";
	open(IMAGE, ">$mapfile") || die "$mapfile: $!";
	if (!$pic->image){print "ERROR: No map was grabbed\n"; exit;} 
	print IMAGE $pic->image;
	close IMAGE;
}