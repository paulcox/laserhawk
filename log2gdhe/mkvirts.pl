#!/usr/bin/perl
###############################################################################
#
###############################################################################

#pragmas and modules
use strict;
use warnings;

use constant COMBINE    => '/usr/bin/convert';

#gif animation parameters : flush previous image, wait 250 ms
my @COMBINE_OPTIONS = (-dispose => 'previous', -delay => 25, -loop  => 10000);
			 
my @files = ();
my $cnt = 0;
my $testname = $ARGV[0];
if (!$testname) {print "please enter a test name\n";exit;}
my $Hy = 700;
my $Hx = 0;
my $Hz = 0;
my $psideg = 0.25;
#my $phideg = 0;

foreach my $loopcnt (0..45) {
	my $phideg = $loopcnt;
	#my $psideg = $loopcnt;
	#my $Hz = 2*$loopcnt;
	if ($loopcnt%5 == 0 ){
		printf "Calling mkvirtlog...";
		`perl mkvirtlog.pl $testname $psideg $phideg $Hy $Hx $Hz $cnt`;
		#perl mkvirtlog.pl test1 0.5 0.5 700 0 40 
		$cnt++;
		my $psiname = sprintf "%06.2f",$psideg;
		my $phiname = sprintf "%06.2f",$phideg;
		my $imgname = $psiname."_".$phiname."_$Hy"."_$Hx"."_$Hz";
		print "ok. scan and image output for $cnt : terrain$imgname.png .\n";
		push(@files,"$testname/imgs/terrain$imgname.png");
	}
}

system COMBINE,@COMBINE_OPTIONS,@files,"gif:$testname/imgs/terrain.gif";
