#!/usr/bin/perl

use strict;
use warnings;
use POSIX qw(pause);
use Time::HiRes qw(gettimeofday setitimer ITIMER_REAL);

use constant SCANSPERSEC => 40;
use constant POINTSPERSCAN => 10;
use constant INTERVAL => 1/(SCANSPERSEC*POINTSPERSCAN);

my $cnt = 0;

$SIG{ALRM} = sub { };
setitimer(ITIMER_REAL,INTERVAL,INTERVAL);

while (1) {
	if ($cnt++ eq POINTSPERSCAN){
		my ($s,$us) = gettimeofday();
		my $timeofday = $s+$us/1000000;	
		printf "START%f\n",$timeofday;
		$cnt = 0;
	} else {
		printf "$cnt\n";
	}
	pause;
}
