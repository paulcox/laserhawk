#!/usr/bin/perl
#filters MTIHardTest output to show only Euler Angles
#Use as follows with MTIG:
# MTIHardTest /dev/ttyUSB0 -o 2 -d 6 -v | perl mti.pl

use IO::Handle;
use POSIX qw(:termios_h);

my $time;
my $time0 = 0;
my $cnt = 0;
my $term = POSIX::Termios->new;
my $fh;
my $sec = 0;
my $psec;

init_serial();

while (<>){

	$_ =~ m/^(.*)\spts\:\s(.*)/;
	next if (!$1);
	#printf "1:%s 2:%s\n",$1,$2;
	if (!$time0){
		$time0 = $1;
	}
	$time = $1 - $time0;
	$psec = $sec;
	$sec = sprintf "%d",$time;
	if ($sec == $psec){
		$cnt++;
	} else {
		printf "%06.2f pts: %03d %d\n", $time, $2,$cnt;
		my $str = sprintf "%06.2f pts: %03d %d\r\n", $time, $2,$cnt;
		$cnt = 0;
		syswrite($fh,$str,length($str));
	}
}

sub init_serial {
	open $fh, '+<', '/dev/ttyUSB0' or die $!;

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