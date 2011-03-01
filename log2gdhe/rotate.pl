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

foreach my $psicnt (1..45) {
	my $psideg = $psicnt;
	if ($psideg%5 == 0 ){
		printf "Calling mkvirtlog...";
		`perl mkvirtlog.pl $testname $psideg $cnt`;
		#`perl mkvirtlog.pl $psideg`;
		$cnt++;
		my $psiname = sprintf "%06.2f",$psideg;
		print "ok. scan and image output for psi $psiname degrees.\n";
		push(@files,"$testname/imgs/terrain$psiname.png");
	}
}

system COMBINE,@COMBINE_OPTIONS,@files,"gif:$testname/imgs/terrain.gif";