#!/usr/bin/perl
#filters MTIHardTest output to show only Euler Angles
#Use as follows with MTIG:
# MTIHardTest /dev/ttyUSB0 -o 2 -d 6 -v | perl mti.pl

my $time0 = 0;
my $cnt = 0;

while (<>){

	$cnt++;
	$_ =~ /^(.*)\sACC.*EUL(.*)POS\s+(.*)\s+31T\sVEL/;
	my $gps = $3;	
	if ($time0){
		$time = $1 - $time0;
	} else {
		$time0 = $1;
	}
	$2 =~ /\s+(.+)\s+(.+)\s+(.+)/;
	next if ($cnt%50);	
	printf "%06.2f att: %06.2f %06.2f %06.2f ",$time, $1, $2, $3;
	$gps =~ /(\S+)\s(\S+)\s+(.+)/;
	#printf $gps."\n";	
	printf "gps: %06.2f %06.2f %06.2f\n", $1, $2, $3;
}
