#!/usr/bin/perl
###############################################################################
#this script inserts the MTI data corresponding to a laser scan into that scan line logfile
#this allows the plot script to show the angles and position in the output image
###############################################################################

#pragmas and modules
use strict;
use warnings;

#usage: ./putangles.pl 2011-03-01-10-32-19
#TODO check arguments

my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $mtilogname = "MTI_test3.out";
my $cnt = 0;
my @mti = ();
my $dirname = $ARGV[0];
if (!$dirname) {print "duh, you forgot to specify the name of the directory\n";exit;}

init_mti_AoH();

my @scanlist = <$path/$dirname/scan*.txt>;

#loop for each scan logfile in directory
foreach (@scanlist) {
	#for debug stop after certain number
	#last if ($cnt++ == 11);
	print "file: $_\n";
	#grab the time off the last line
	my $htime = `tail -n 1 $_`;
	chomp $htime;
	print "last line blank\n" if (!$htime);
	#need to check if it's really a time otherwise bail
	if ($htime < 1287161422) {print "skipping...\n";next;}
	print "htime: $htime\n";
	#open file for appending
	open LOG,">>$_";
	#print angles corresponding to time in file
	printf LOG getangles($htime)."\n";
	#close log
	close LOG;
}

###############################################################################

sub init_mti_AoH {
	open MTILOG, "<$path/$mtilogname" or die $!;
	#load MTI loginfo in array of hashes
	while (<MTILOG>) {
		my $rec = {};
		#split into fields on any number of spaces and stick first four fields into variables, sweet!
		my ($time,$junk,$ang1,$ang2,$ang3) = split(/\s+/,$_);
		#ignore any comments in file
		next if (substr($time,0,1) eq '#');
		#hash value is a space delimited string of the angles
		$rec->{$time} =  $ang1." ".$ang2." ".$ang3;
		push @mti, $rec;
	}
	close MTILOG;
}

sub getangles {
	my $hokuyotime = $_[0];
	my $prevtime;
	my $angles;
	my $prevangles;

search: for my $href (@mti) {
		for my $mtitime (keys %$href) {
			#if we've gone past hokuyo time stop
			if ($mtitime >= $hokuyotime){
				#print "   $prevtime $hokuyotime $mtitime\n";
				#printf "   %6f %6f\n",$hokuyotime-$prevtime,$mtitime-$hokuyotime;
				#print "   $href->{$mtitime}\n";
				
				#check which mti data line is closest to our scan time
				if ($hokuyotime-$prevtime > $mtitime-$hokuyotime){
					$angles = $href->{$mtitime};
				} else {
					$angles = $prevangles;
				}
				last search;
			} else {
				$prevtime = $mtitime;
				$prevangles = $href->{$mtitime};
			}
		}
	}
	#print "  getangles returning : ".$angles."\n";
	return $angles;
}

