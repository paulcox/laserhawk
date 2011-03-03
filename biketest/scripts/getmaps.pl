#!/usr/bin/perl
###############################################################################
#Author: Paul Cox
#Date  : Feb 2011
#Notes
# use HTTP::Request;
# use LWP::UserAgent;
# my $request = HTTP::Request->new(GET => $url);
# my $ua = LWP::UserAgent->new;
# my $response = $ua->request($request);
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

my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $dirname = "2011-03-01-10-32-19";
#my $dirname = $ARGV[0];
#whine if no subdir
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}
#create maps dir if not already there
#`mkdir $path/$dirname-maps` if (! -e "$path/$dirname-maps" );

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
	if (we_moved()) { 
		fetchmap($east,$north,$zone,$center,$timeinsec);
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
	$mtidata =~ /^.*POS\s+(.*)T/;
	#split into fields
	my @gpspos = split(/\s+/,$1);
	
	my ($lat,$lon) = utm_to_latlon('wgs84', $gpspos[3], $gpspos[0], $gpspos[1]);
	#$center should be a string of "lat,lon" values
	return sprintf "%f,%f",$lat,$lon;
}


sub we_moved {


	our $peast =0;
	our $pnorth =0;
	
	#return 0;
	
	
	return 1;
}

sub getposition() {
	
	#shouldn't do it this way...TODO: change it
	my $file = sprintf "$path/$dirname/scan%06d.txt",$_[0]*$scansps;
	printf "$file\n";
	my $mtidata = `head -1 $file`;

	#grab gps data
	$mtidata =~ /^.*POS\s+(.*)T/;
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
	print "lat/lon: $lat $lon \n";

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
	open(IMAGE, ">$mapfile") || die "$mapfile: $!";
	print IMAGE $pic->image;
	close IMAGE;
}