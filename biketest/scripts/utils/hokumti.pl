#!/usr/bin/perl
#uses MTIHardTest and getrange executables to generate periodic summary and write logs
#execute with 'perl hokumti.pl' or to write logs to test1 log dir 'perl hokumti.pl test1'

use strict;
use IO::Handle;
use POSIX qw(:termios_h);
use threads;
use threads::shared;

my $usemti = 1;
my $usexbee = 1;
my $writelog :shared = 0;
$writelog = 1 if ($#ARGV+1 == 1);

my $hokuyodev = '/dev/ttyACM0';
my $mtidev = '/dev/ttyUSB0';
my $xbeedev = '/dev/ttyUSB1';

my $time;
my $ptime;
my $time0 = 0;
my $sps = 0;
my $sec = 0;
my $psec;
my $cnt;
my $maxdist;
my $startang = 360;
my $endang = 720;
#my $startang = 480;
#my $endang = 600;

my $term = POSIX::Termios->new;
my $fh;

if ($usexbee) {
	init_serial();
}

my $mtiline :shared;
my $mtithr;

my $subdir;
my $logcnt = 0;

if ($writelog) {
	$subdir = $ARGV[0];
	if (!$subdir) {printf "please specify a subdirectory name\n"; exit;}
	`mkdir $subdir` if (! -e $subdir);
	print "Files will be placed in $subdir directory\n";
}

if ($usemti) {
	$mtithr = threads->create(\&mtithread);
}

#open GETRANGE, "./getrange.elf $hokuyodev $startang $endang 1 |" or die "Couldn't execute program: $!";
open GETRANGE, "perl fakerange.pl |" or die "Couldn't execute program: $!";

while (<GETRANGE>){
	#print $_;
	if (/^START(.*?)$/) {
		$cnt = 0;
		if ($writelog) {
			if (-e LOG) {
				print LOG $ptime;
				close LOG;
				$logcnt++;
			}
			my $scanlogname = sprintf "scan%06d.txt",$logcnt;
			open(LOG, ">$subdir/$scanlogname") or die $1;
			print LOG $mtiline;
			$ptime = $1;
		}
		
		if (!$time0){
			$time0 = $1; $time = $time0; #print "first hokuyo time detected $time0\n";
		} else {
			$time = $1 - $time0; #print "scan $cnt\n";
		}
		$psec = $sec;
		$sec = sprintf "%d",$time;
		if ($sec == $psec){
			$sps++;
		} else {
			printoutput();			
			$sps = 0; $maxdist = 0;
		}
	} else {
		if ($time0) {
			$cnt++;
			chomp $_;
			my $dist = $_;
			printf LOG "%04d       %d\n", $cnt, $dist if ($writelog);
			#printf "$dist\n";
			$maxdist = $dist if ($dist > $maxdist);
		}
	}
}

sub mtithread {
	#my $mtitime0 = 0;
	#my $mticnt = 0;

	printf "MTI thread Started.\n";
	open MTI, "MTIHardTest -o 2 -d 6 $mtidev |" or die "Couldn't execute program: $!";
	while (<MTI>){
		#printf $_;
		#$mticnt++;
		$mtiline = $_;
	}
}

sub printoutput {
	my $str = sprintf "\r\n%06.2f pts: %03d sps: %d max: %d ", $time, $cnt/($sps+1), $sps, $maxdist;
	#my $str = sprintf "\r\n%06.2f pts: %03d sps: %d max: %d ", $time, $cnt, $sps, $maxdist;

	if ($usemti) {
		$mtiline =~ /^(.*)\sACC.*EUL(.*)POS\s(.*)\sVEL/;
		my $mtitime = $1;
		my $att = $2;
		my $gps = $3;
	
		printf $att."\n";
		$att =~ /\s+(.+)\s+(.+)\s+(.+)/;
		$str .= sprintf "att: %06.2f %06.2f %06.2f ", $1, $2, $3;

		printf $gps."\n";
		$gps =~ /(\S+)\s(\S+)\s+(.+)/;
		$str .= sprintf "gps: %06.2f %06.2f %06.2f\r\n", $1, $2, $3;
	}

	printf $str;
	if ($usexbee) {syswrite($fh,$str,length($str));}
}

sub init_serial {
	open $fh, '+<', $xbeedev or die $!;

	$fh->blocking(0);

	$term->getattr(fileno($fh)) or die $!;

	$term->setiflag( $term->getiflag &
		( POSIX::IGNBRK | POSIX::IGNPAR &
		~POSIX::INPCK & ~POSIX::IXON &
		~POSIX::IXOFF));

	$term->setlflag( $term->getlflag &
		~( POSIX::ICANON | POSIX::ECHO |
		POSIX::ECHONL | POSIX::ISIG |
		POSIX::IEXTEN ));

	$term->setcflag( $term->getcflag &
		( POSIX::CSIZE | POSIX::CS8 & ~POSIX::PARENB));

	#hard code value of baud rate of 115.2kbps since perl POSIX doesn't have ready made for speeds > 38.4kbps
	$term->setospeed(0010002);
	$term->setispeed(0010002);
	
	$term->setattr(fileno($fh), POSIX::TCSANOW) or die $!;
}
