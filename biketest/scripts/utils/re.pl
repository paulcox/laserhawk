#!/usr/bin/perl

$text = "1234.5678";

printf $text;

$text =~ /\.(\d)/;
print "found :".$1."\n";