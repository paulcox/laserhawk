#!/usr/bin/perl
#filters MTIHardTest output to show only Euler Angles
#Use as follows with MTIG:
# MTIHardTest /dev/ttyUSB0 -o 2 -d 6 -v | perl mti.pl

use IO::Handle;
use POSIX qw(:termios_h);

my $time0 = 0;
my $cnt = 0;
my $term = POSIX::Termios->new;
my $fh;

init_serial();

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
	my $str = sprintf "%06.2f att: %06.2f %06.2f %06.2f ",$time, $1, $2, $3;
	$gps =~ /(\S+)\s(\S+)\s+(.+)/;
	#printf $gps."\n";	
	printf "gps: %06.2f %06.2f %06.2f\n", $1, $2, $3;
	$str .= sprintf "gps: %06.2f %06.2f %06.2f\n", $1, $2, $3;
	my $strcnt= sprintf "%06d\n",$cnt;	
	syswrite($fh,$str,length($str));
}

sub init_serial {
	open $fh, '+<', '/dev/ttyUSB1' or die $!;

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

	$term->setospeed(POSIX::B115200);
	$term->setispeed(POSIX::B115200);
	$term->setattr(fileno($fh), POSIX::TCSANOW) or die $!;
}