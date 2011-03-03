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

#MTI
#377979.888 4825430.921    234.178  31T  

#find out start and end time of scans
#for each second of scan get a position
#figure out a center for the track => cntr
#get map with marker for current position and centered on cntr

my $east = 377979.888;
my $north = 4825430.921;
my $zone = 31;
my ($lat,$lon) = utm_to_latlon('wgs84', ($zone . "V"), $east, $north);
my $zoom = 16;
my $stuff = 'size=256x256&maptype=roadmap&sensor=false';
my $mark1 = "markers=color:blue|label:HOME|$lat,$lon";

my $url = GOOGURL."?center=$lat,$lon&zoom=$zoom&$stuff&$mark1";

print "lat/lon: $lat $lon \n";
my $pic = new Image::Grab;

$pic->url($url);
$pic->grab;

# Now to save the image to disk
open(IMAGE, ">map.jpg") || die"map.jpg: $!";
print IMAGE $pic->image;
close IMAGE;