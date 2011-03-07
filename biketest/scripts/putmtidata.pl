#!/usr/bin/perl
###############################################################################
#Author: Paul Cox
#Date  : Feb 2011
#
# putmtidata.pl : This script inserts the MTI data corresponding to a laser
#	scan into that scanline logfile.
#	This allows the plot script to extract whatever info it needs such as the 
#	angles and position.
#
# TODO: more argument checking
###############################################################################

#pragmas and modules
use strict;
use warnings;

#use constant ANGLETYPE => 'EUL';
use constant ANGLETYPE => 'QUAT';

#require two args
if ($#ARGV !=1) {print "Specify scan folder name and MTI log file\nExample: ./putmtidata.pl 2011-03-01-10-32-19 MTI_test3.out\n";exit;}

#my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $path = `pwd`;
chomp $path;
$path .= "/..";

my $cnt = 0;
my @mti = ();
my $dirname = $ARGV[0];
if (!$dirname) {print "duh, you forgot to specify the name of the directory\n";exit;}
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}

#my $mtilogname = "MTI_test3.out";
my $mtilogname = $ARGV[1];

init_mti_AoH();

my @scanlist = <$path/$dirname/scan*.txt>;

#loop for each scan logfile in directory
foreach (@scanlist) {
	#for debug stop after certain number
	#last if ($cnt++ == 226);
	print "file: $_\n";
	#grab the time off the last line
	my $htime = `tail -n 1 $_`;
	chomp $htime;
	if (!$htime){print "last line blank (probably last scan file)\n I suggest you do a: rm $_\n"; exit;}
	
	#need to check if it's really a time otherwise bail
	if ($htime < 1287161422) {print "skipping...\n"; next;}
	print "htime: $htime\n";
	
	open(LOG, "<$_") || die("Cannot Open File");
	my @raw_data=<LOG>; 
	close(LOG);
	
	#open file for writing
	open LOG,">$_";
	
	#print data corresponding to time in file
	printf LOG getdata($htime)."\n";
	
	#print everything
	#my $init = 0; #skip first to remove old first line
	for my $line (@raw_data) {
		#if ($init == 0) {$init=1;}else{
		print LOG $line;
		#}
	}

	close LOG;
	#for debug, this will stop the script after the first log file is processed:
	#last;
}

###############################################################################

sub init_mti_AoH {
	open MTILOG, "<$path/$mtilogname" or die $!;
	#load MTI loginfo in array of hashes
	while (<MTILOG>) {
		my $rec = {};
		my $time;

		#$_ =~ /^(.*)\sACC\s+(.*)$/;
		
		if (ANGLETYPE eq 'EUL'){
			#For MTIHardTest output in Euler angles
			$_ =~ /^(.*)\sEUL\s+(.*)$/;
			if ($1){
				$time = $1;
				#hash value is the mti data
				$rec->{$time} = $2;
			}
		} else { if (ANGLETYPE eq 'QUAT') {
			#For MTIHardTest output in Quaternions
			$_ =~ /^(.*)\sQUAT\s+(.*)$/;
			if ($1){
				$time = $1;
				#hash value is the mti data
				$rec->{$time} = $2;
			}
		} else {
			printf "Unknown angle type". ANGLETYPE ."\n";
			exit;
		}}
		
		#Handle no-match
		if (!$time) {
			#ignore any comments in file by skipping to the next line if # is detected at start of line
			if ($_ =~ m/^#/){
				print "ignoring comment\n";
				next;
			} else {
				print "ERROR: Didn't find ". ANGLETYPE ." in log line\n.";
				exit;
			}

		}

		push @mti, $rec;
	}
	close MTILOG;
}

sub getdata {
	my $hokuyotime = $_[0];
	my $prevtime;
	my $data;
	my $prevdata;

search: for my $href (@mti) {
		for my $mtitime (keys %$href) {
			#if we've gone past hokuyo time stop
			if ($mtitime >= $hokuyotime){
				#print "   $prevtime $hokuyotime $mtitime\n";
				#printf "   %6f %6f\n",$hokuyotime-$prevtime,$mtitime-$hokuyotime;
				#print "   $href->{$mtitime}\n";
				
				#check which mti data line is closest to our scan time
				if ($hokuyotime-$prevtime > $mtitime-$hokuyotime){
					$data = $href->{$mtitime};
				} else {
					$data = $prevdata;
				}
				last search;
			} else {
				$prevtime = $mtitime;
				$prevdata = $href->{$mtitime};
			}
		}
	}
	#print "  getdata returning : ".$data."\n";
	return $data;
}

