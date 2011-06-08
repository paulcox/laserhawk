#!/usr/bin/perl -w

use warnings;
use strict;

my $dirname = $ARGV[0];
my $firstlog = $ARGV[1];
my $lastlog =  $ARGV[2];

my $path = `pwd`;
chomp $path;

my @gmaxdist;
my @gmindist;
my @gptcnt;

foreach my $scncnt ($firstlog..$lastlog) {
	my $mindist = 30000;
	my $maxdist = 0;
	my $ptcnt = 0;
	
	my $file2open = sprintf "$path/$dirname/scan%06d.txt",$scncnt;
	#printf "Opening scan log : $file2open\n";
	open SCNLOG, "<$file2open" or die $!;
	
	while (<SCNLOG>){
		chomp;
		my ($lcnt,$dist) = split(/       /,$_);
		if (!$dist) {
			next;
		} else {
			$ptcnt++;
		}
		$mindist = $dist if ($dist < $mindist);
		$maxdist = $dist if ($dist > $maxdist);
	}
	$gmindist[$scncnt] = $mindist;
	$gmaxdist[$scncnt] = $maxdist; 
	$gptcnt[$scncnt] = $ptcnt;
	print "$mindist, $ptcnt\n";
}
