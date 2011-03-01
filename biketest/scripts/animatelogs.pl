#!/usr/bin/perl
###############################################################################
#
###############################################################################

#pragmas and modules
use strict;
use warnings;

use constant COMBINE    => '/usr/bin/convert';

my $dirname = "2011-03-01-10-32-19";
my $path = "/home/paul/Documents/LAAS/laserhawk/biketest";
#print "image files : $path/$dirname-imgs/scan$scncnt.png\n";
my $skipnum = 1;

#gif animation parameters : flush previous image, wait 100 ms
my @COMBINE_OPTIONS = (-dispose => 'previous', -delay => 10, -loop  => 10000);
			 
my @files = ();
my $cnt = 0;


#TODO: get number of scans from directory listing
foreach my $scncnt (1..500) {
	if ($scncnt % $skipnum == 0 ){
		$cnt++;
		#my $cntname = sprintf "%06.2f",$scncnt;
		print "plotting scan ($scncnt) $cnt\n";
		`perl plotlogs.pl $dirname $scncnt`;
		my $imgfile = sprintf "$path/$dirname-imgs/scan%06d.png",$scncnt;
		push(@files,"$imgfile");
	}
}
printf "creating animation from files\n",$#files+1;
system COMBINE,@COMBINE_OPTIONS,@files,"gif:$path/$dirname-imgs/loganim.gif";
printf "creating avi";
for my $i (1..2) { `mencoder mf://$path/$dirname-imgs/*.png -mf fps=5 -ovc x264 -x264encopts qp=40:subq=7:pass=$i -o loganim264.avi`; }
`mencoder "mf://$path/$dirname-imgs/*.png" -mf fps 5 -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=1400 -ffourcc DIVX -o loganimavc.avi`;
