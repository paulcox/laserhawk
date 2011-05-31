#!/usr/bin/perl
###############################################################################
#Author: Paul Cox
#Date  : Feb 2011
#
# animatelogs.pl	Script that uses the output images (pngs) of plotlogs.pl to
#	generate an animated gif (viewable in firefox for example).
#		
#
# TODO : more arg checking
#
# Notes :	- Beware, doing more than 100 can take a long time and lots of memory
#			- Adjust DECISECDELAY for animation to play more/less quickly
###############################################################################

#pragmas and modules
use strict;
use warnings;

use constant COMBINE    => '/usr/bin/convert';
use constant DECISECDELAY => 10; #result runs about realtime since scans acquired about 10-15Hz for now...

#my $dirname = "2011-03-01-10-32-19";
#my $skipnum = 1;
#my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
my $path = `pwd`;
chomp $path;
#$path .= "/..";

#require four args
if ($#ARGV !=3) {print "Specify start and stop indexes and skip param.\nExample: ./animatelogs.pl 2011-03-01-10-32-19 0 9 1\n";exit;}

my $dirname = $ARGV[0];
#whine if no subdir
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}

#gif animation parameters : flush previous image, wait 100 ms
my @COMBINE_OPTIONS = (-dispose => 'previous', -delay => DECISECDELAY, -loop  => 10000);
			 
my @files = ();
#my $cnt = 0;

printf "including: ";
foreach my $scncnt ($ARGV[1]..$ARGV[2]) {
	if ($scncnt % $ARGV[3] == 0 ){
		#$cnt++;
		#my $cntname = sprintf "%06.2f",$scncnt;
		print "($scncnt)";
		my $imgfile = sprintf "$path/$dirname-imgs/scan%06d.png",$scncnt;
		push(@files,"$imgfile");
	}
}
printf "\nCreating animation from %d files...",$#files+1;
system COMBINE,@COMBINE_OPTIONS,@files,"gif:$path/$dirname-imgs/loganim$ARGV[1]_$ARGV[2]_$ARGV[3].gif";
printf "done : $path/$dirname-imgs/loganim$ARGV[1]_$ARGV[2]_$ARGV[3].gif\n";
#for my $i (1..2) { `mencoder mf://$path/$dirname-imgs/*.png -mf fps=5 -ovc x264 -x264encopts qp=40:subq=7:pass=$i -o loganim264.avi`; }
#`mencoder "mf://$path/$dirname-imgs/*.png" -mf fps 5 -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=1400 -ffourcc DIVX -o loganimavc.avi`;
