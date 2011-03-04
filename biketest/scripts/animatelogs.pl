#!/usr/bin/perl
###############################################################################
#
###############################################################################

#pragmas and modules
use strict;
use warnings;

use constant COMBINE    => '/usr/bin/convert';

#my $dirname = "2011-03-01-10-32-19";
#my $skipnum = 1;
my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";

#require four args
if ($#ARGV !=3) {print "Specify start and stop indexes and skip param.\nExample: ./animatelogs.pl 2011-03-01-10-32-19 0 9 1\n";exit;}

my $dirname = $ARGV[0];
#whine if no subdir
if (! -e "$path/$dirname") {printf "please specify a valid log folder\n"; exit;}

#gif animation parameters : flush previous image, wait 100 ms
my @COMBINE_OPTIONS = (-dispose => 'previous', -delay => 10, -loop  => 10000);
			 
my @files = ();
#my $cnt = 0;

printf "including: ";
foreach my $scncnt ($ARGV[1]..$ARGV[2]) {
	if ($scncnt % $ARGV[3] == 0 ){
		$cnt++;
		#my $cntname = sprintf "%06.2f",$scncnt;
		print "($scncnt)";
		my $imgfile = sprintf "$path/$dirname-imgs/scan%06d.png",$scncnt;
		push(@files,"$imgfile");
	}
}
printf "\nCreating animation from %d files...",$#files+1;
system COMBINE,@COMBINE_OPTIONS,@files,"gif:$path/$dirname-imgs/loganim.gif";
printf "done : $path/$dirname-imgs/loganim.gif\n";
#for my $i (1..2) { `mencoder mf://$path/$dirname-imgs/*.png -mf fps=5 -ovc x264 -x264encopts qp=40:subq=7:pass=$i -o loganim264.avi`; }
#`mencoder "mf://$path/$dirname-imgs/*.png" -mf fps 5 -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=1400 -ffourcc DIVX -o loganimavc.avi`;
