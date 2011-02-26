#!/usr/bin/perl
###############################################################################
#
###############################################################################

#pragmas and modules
use strict;
use warnings;

use constant COMBINE    => '/usr/bin/convert';

my @COMBINE_OPTIONS = (-dispose => 'previous', -delay => 50, -loop  => 10000);
			 
my @files = ();

foreach my $psicnt (1..45) {
	my $psideg = $psicnt;
	if ($psideg%5 == 0 ){
		`perl mkvirtlog.pl $psideg`;
		my $psiname = sprintf "%06.2f",$psideg;
		print "$psiname\n";
		push(@files,"imgs/terrain$psiname.png");
	}
}

system COMBINE,@COMBINE_OPTIONS,@files,"gif:imgs/terrain.gif";