#!/usr/bin/perl
###############################################################################
#this script inserts the MTI data corresponding to a laser scan into that scan line logfile
#this allows the plot script to show the angles and position in the output image
###############################################################################

#pragmas and modules
use strict;
use warnings;

#usage: ./killangles.pl 2011-03-01-10-32-19
#TODO check arguments

my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $cnt = 0;
my $dirname = $ARGV[0];
if (!$dirname) {print "duh, you forgot to specify the name of the directory\n";exit;}

my @scanlist = <$path/$dirname/scan*.txt>;

#loop for each scan logfile in directory
foreach (@scanlist) {
	#for debug stop after certain number
	#last if ($cnt++ == 11);
	print "file: $_\n";
	open(LOG, "<$_") || die("Cannot Open File");
	my @raw_data=<LOG>; 
	close(LOG);
	
	#open file for writing
	open LOG,">$_";
	#print everything except the last line (the angles)
	for my $line (@raw_data) {
		print LOG $line if ($line !~ /^\S+\s+\S+\s+\S+/);
	}
	#close log
	close LOG;
	#last;
}