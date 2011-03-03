#!/usr/bin/perl
#filters MTIHardTest output to show only Euler Angles
#Use as follows with MTIG:
# MTIHardTest /dev/ttyUSB0 -o 2 -d 6 -v | perl mti.pl


while (<>){
	$_ =~ /^(.*)\sACC.*EUL(.*)POS/;
	$time = $1;
	$2 =~ /\s+(.+)\s+(.+)\s+(.+)/;
	print printf "%f %f %f %f\n",$time, $1, $2, $3;
}
