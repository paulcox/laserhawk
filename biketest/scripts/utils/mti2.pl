#!/usr/bin/perl
# runs MTIHardTest and outputs fields to stdout

use IO::Handle;
use strict;

my $mtitime0 = 0;
my $mtitime;
my $mticnt = 0;
my $att;
my $gps;

open MTI, "MTIHardTest -o 2 -d 6 /dev/ttyUSB0 |" or die "Couldn't execute program: $!";

while (<MTI>){
	#printf $_;
	$mticnt++;
	$_ =~ /^(.*)\sACC.*EUL(.*)POS\s+(.*)\s+31T\sVEL/;
	$gps = $3;	
	if ($mtitime0){
		$mtitime = $1 - $mtitime0;
	} else {
		$mtitime0 = $1; $mtitime = $mtitime0;
	}
	$att = $2;
	$2 =~ /\s+(.+)\s+(.+)\s+(.+)/;
	next if ($mticnt%50);	
	printf "%06.2f att: %06.2f %06.2f %06.2f ",$mtitime, $1, $2, $3;
	#my $str = sprintf "%06.2f att: %06.2f %06.2f %06.2f ",$time, $1, $2, $3;
	$gps =~ /(\S+)\s(\S+)\s+(.+)/;
	printf $gps."\n";	
	#printf "gps: %06.2f %06.2f %06.2f\n", $1, $2, $3;
}
