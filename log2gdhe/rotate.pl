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

foreach my $psicnt (1..45) {
	my $psideg = $psicnt;
	if ($psideg%5 == 0 ){
		`perl mkvirtlog.pl $psideg $cnt`;
		#`perl mkvirtlog.pl $psideg`;
		$cnt++;
		my $psiname = sprintf "%06.2f",$psideg;
		print "$psiname\n";
		push(@files,"imgs/terrain$psiname.png");
	}
}

system COMBINE,@COMBINE_OPTIONS,@files,"gif:imgs/terrain.gif";