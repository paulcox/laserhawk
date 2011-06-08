#!/usr/bin/perl
###############################################################################
#Author: Paul Cox

#pragmas and modules
use strict;
use warnings;
use Geo::Coordinates::UTM;
use Image::Grab;
use USNaviguide_Google_Encode;

use constant GOOGURL => "http://maps.google.com/maps/api/staticmap";

my $path = `pwd`;
chomp $path;
#$path .= "/..";
my $dirname = $ARGV[0];
#whine if no subdir
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}

my ($east,$north,$zone);
my $middlefile;

SCAN:
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
if ($lasttime < 1287161422) {
	print "not a good last time...\n";
	#exit;
	`rm $lastfile`;
	goto SCAN;
}

my $firsttime = `tail -n 1 $firstfile`;
chomp $firsttime;
print "first line blank\n" if (!$firsttime);
#need to check if it's really a time otherwise bail
if ($firsttime < 1287161422) {print "not a good first time...\n";exit;}	

print "first/last times: $firsttime $lasttime\n";

printf "delta: %d seconds\n", $lasttime-$firsttime;

my $ptcnt = 0;
my @Ipoints	= ( ) ;

my $startcnt = 21000;
my $lastcnt = 25000;
	
foreach my $file (@scanlist) {
	$ptcnt++;
	
	next if ($ptcnt < $startcnt);
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
	#print "file $file: east $east north $north zone $zone\n";
	
	my ($lat,$lon) = utm_to_latlon('wgs84', $zone, $east, $north);
	print "$ptcnt: lat/lon/zo: $lat $lon $zone \n";

	push( @Ipoints, [$lat,$lon] );

	last if ($ptcnt eq $lastcnt);
}

# Call as: (<Encoded Levels String>, <Encoded Points String>) = &Google_Encode(<Reference to array of points>, <tolerance in meters>);
# Points Array Format:
# ([lat1,lng1],[lat2,lng2],...[latn,lngn])

print "Encoding points into google polyline\n";

my $tolerance = 0.01;
my ($lstr,$pstr) = Google_Encode(\@Ipoints,$tolerance);

print "lstr: ".$lstr."\n";
print "pstr: ".$pstr."\n";

#there's a problem regarding double backslashes, getting rid of the second backslash seems to do the trick...
$pstr =~ s/\\\\/\\/g;

	#my $zoom = "zoom=$lstr";
	#maptypes can be roadmap | satellite | hybrid | terrain
	my $stuff = 'size=256x256&maptype=satellite&sensor=false';
	#my $mark1 = "markers=color:blue|label:HOME|$lat,$lon";
	my $gpath= "path=weight:3|color:orange|enc:$pstr";
	#my $gpath2= "path=weight:3|color:blue|enc:$pstr2";
	#my $url = GOOGURL."?$zoom&$stuff&$gpath";
	my $url = GOOGURL."?$stuff&$gpath";
	#my $url = GOOGURL."?$stuff&$gpath1&$gpath2";

	print "Requesting map from google\n";
	print $url."\n";
	
	my $pic = new Image::Grab;
	$pic->url($url);
	$pic->grab;

	# save the image to disk
	my $mapfile = sprintf "$path/map_$dirname\_$startcnt\_$lastcnt.jpg";
	print "  ".$mapfile."\n";
	
	if (!$pic->image){print "ERROR: No map was grabbed\n"; exit;} 
	open(IMAGE, ">$mapfile") || die "$mapfile: $!";
	print IMAGE $pic->image;
	close IMAGE;
	`display $mapfile`