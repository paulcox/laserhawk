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
if ($lasttime < 1287161422) {print "not a good last time...\n";exit;}

my $firsttime = `tail -n 1 $firstfile`;
chomp $firsttime;
print "first line blank\n" if (!$firsttime);
#need to check if it's really a time otherwise bail
if ($firsttime < 1287161422) {print "not a good first time...\n";exit;}	

print "first/last times: $firsttime $lasttime\n";

printf "delta: %d seconds\n", $lasttime-$firsttime;

my $ptcnt = 0;
my @Ipoints	= ( ) ;

goto MOOSE;
	
foreach my $file (@scanlist) {
	$ptcnt++;
	#next if ($ptcnt < 10000);
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

	#last if ($ptcnt eq 20000);
}

# Call as: (<Encoded Levels String>, <Encoded Points String>) = &Google_Encode(<Reference to array of points>, <tolerance in meters>);
# Points Array Format:
# ([lat1,lng1],[lat2,lng2],...[latn,lngn])

my $tolerance = 0.3;
my ($lstr,$pstr) = Google_Encode(\@Ipoints,$tolerance);

print $lstr."\n";
print $pstr."\n";

MOOSE:
my $pstr2 = 'cn{hGeo`HQLm@p@AG`@[g@ZDJCHKF?EoArASHH?IBKEFDA@@DFCK@PMDAPBTHK_@JBHUNH?c@B?DMTUz@y@J?AI?L@ICEEP@EAD?CGDy@p@w@x@UPY\\URAFMDWXIDG?@DB{@Y\\CDB?BFKSAEHFP?BB]Me@KPEVCRBK?a@Ey@OgAYwAEmAFyAPlO\\f@MYFa@B}@R';
my $pstr1 = 'oo|pGzj`D@PBOSPNMKOeBdAa@B]O}BuDc@qB`@yAdBcAt@Ip@^t@pDcAdDeAX}@Y{@kA[{ATmB~A_BbAKv@h@n@jAN~@I`Bq@xAoBr@q@rAo@R]OUc@P{BnA}CfEmErA_AhAEd@l@|@|D@nAw@zA_DfCaC`@yBr@{@Aw@]o@y@a@cAGgA\\sBj@mAdFeEb@E\\`@R|Bl@zBCxAa@bAsAl@oAOeAeASyAPm@vCuCnAs@nABnCxAn@Ar@a@f@{AAyA}@gBkAe@q@Ng@h@u@~Cs@t@aDX}[xIy@LkAQwCaCkBbBWv@DpCx@tArAJjBcArEiElFgBrNoDfDqAf@Bb@\\v@rCAjC[p@i@\\}@DcAYs@k@a@eAEoAXsAdAmA|@_@jAXbArATfBS|AeArAqAPcA]y@qAWmANyB~@}@rAMjAXn@z@TtB[rAeBjBc@Fe@MqAcB]sBb@aBlBiA|@Cx@j@VxBOnCk@|@w@X}@ScAkA[oAFeBr@mApAi@bAJj@p@ZfBE|Ac@jBq@n@aAJu@e@o@uAQiCTwAhAaA|AEbAj@d@|AKhBe@bBeA|@o@?w@a@}@{AUyAZuB~@}@zAMfAl@`@lADrAYfBk@bA_A`@aAa@{@kAa@oB`@{BjA_ArAIz@j@\\vA?|Bi@bB_Ax@}@?u@i@m@}AGaB^yB~@cAp@G|@n@dAfDMbAm@|@gAb@w@Gu@k@[gA@wBx@{Al@Wj@Ah@L\\Z^dALdBSjAo@r@yAR}@[i@y@QeBd@oB|@u@dAKfAj@\\|@J`BKbAu@bAkA`@w@Ku@_A[eBTsBbA}@|AIbAp@ZxAG~Am@`Bu@d@{@?eAg@i@_AKwAZ{A`AcAv@[fAEf@]l@cEfA_Al@FXb@hArEBjAc@`AsAr@oEfE';
#my $pstr2 = '}oagFemfmCi@E[Nl@sCi@eDt@kBQqGZaAXEh@Lh@x@rBtMJjBMxBi@`BcAxAiA`AmBx@sARaB?cA[s@s@LcBfDoHr@c@r@Mv@?fAXrAdAbBfDl@zCBvE_@jCc@nAiCnC}@XaAE{@g@Us@BmEw@sIBsA|@mCzCoDl@Kx@Pl@j@\\~@NvEw@~FoBnDwCpBqAVkA@y@Om@k@bA_GjA}Kd@{AbAiAn@UxDk@fBDo@[cAZaAtAwBnBqApBs@`@e@CGWVSnBQ`@aCUi@k@Mi@Je@f@QlBPr@`@XhEoB';

#exit;
	#my $zoom = "zoom=$lstr";
	#maptypes can be roadmap | satellite | hybrid | terrain
	my $stuff = 'size=256x256&maptype=satellite&sensor=false';
	#my $mark1 = "markers=color:blue|label:HOME|$lat,$lon";
	my $gpath1= "path=weight:3|color:orange|enc:$pstr1";
	my $gpath2= "path=weight:3|color:blue|enc:$pstr2";
	#my $url = GOOGURL."?$zoom&$stuff&$gpath";
	my $url = GOOGURL."?$stuff&$gpath1&$gpath2";
	my $pic = new Image::Grab;
	print $url."\n";

	$pic->url($url);
	$pic->grab;

	# Now to save the image to disk
	my $mapfile = sprintf "$path/$dirname/map2.jpg";
	print "  ".$mapfile."\n";
	open(IMAGE, ">$mapfile") || die "$mapfile: $!";
	if (!$pic->image){print "ERROR: No map was grabbed\n"; exit;} 
	print IMAGE $pic->image;
	close IMAGE;
